#include "FileCollector.h"
#include <cstring>
#include <algorithm>

void FileCollector::CollectFile(uint32_t fileId, size_t fileSize) {
    std::unique_lock<std::shared_mutex> lock(mapMutex);      //берёт замок mapMutex (пока lock задан, другие потоки не читают и не пишут files)
    if (files.find(fileId) == files.end()) {                 //если файла с таким id ещё нет, регистрируем
        files[fileId] = std::make_shared<File>(fileSize);    //одной аллокацией создаёт структуру File и указатель на неё
    }
}

void FileCollector::OnNewChunk(uint32_t fileId, size_t pos, std::vector<uint8_t>&& chunk) {
    std::shared_ptr<File> file;                             //указатель на файл, который продлит его жизнь вне замка
    {
        std::shared_lock<std::shared_mutex> lock(mapMutex); //берёт замок mapMutex (пока lock задан, другие потоки могут одновременно читать, но не писать)
        auto it = files.find(fileId);                       //ищет в files ключ fileId
        if (it == files.end()) return;                      //если ключа нет, то выходит
        file = it->second;                                  //теперь file указывает на тот же File, что и files[fileId]
    } 
    //lock уничтожен

    std::lock_guard<std::mutex> fileLock(file->fileMutex); //берёт замок file->fileMutex (пока fileLock задан, ни один другой поток не меняет указатели именно у этого File)

    if (file->bytes == file->total) return; //если bytes = total, то все байты buffer уже заполнены, выходит
    if (chunk.empty()) return;              //чанк пустой - выходит
    if (pos >= file->total) return;         //начало чанка за концом файла, выходит

    size_t chunkSize = std::min(chunk.size(), file->total - pos); //выбираем меньшее из байтов чанка и в buffer после pos

    size_t offset = 0;
    if (pos < file->bytes) {                 //если начало чанка pos < bytes, то первые байты чанка уже записаны в buffer с позиции pos
        size_t overlap = file->bytes - pos;  //сколько первых байт чанка уже есть в buffer
        if (overlap >= chunkSize) return;    //если накладка - это весь buffer, то выходит
        offset = overlap;                    //сколько первых байт chunk пропустить (они уже в buffer)
    }

    size_t real_pos = pos + offset;        //где начинаются новые байты чанка
    size_t real_size = chunkSize - offset; //сколько новых байт из чанка надо записать в buffer

    if (real_pos == file->bytes) { 
        for (size_t i = 0; i < real_size; ++i) file->buffer[real_pos + i] = chunk[offset + i]; //копирует real_size байт из chunk в buffer
        file->bytes += real_size;  //сдвигаем bytes на real_size

        bool progress = true;
        while (progress) {
            progress = false;
            auto it = file->chunks.find(file->bytes);   //ищет в file->chunks ключ равный текущему file->bytes
            if (it != file->chunks.end()) {             //если есть, то копирует байты этого чанка в buffer начиная с file->bytes
                for (size_t i = 0; i < it->second.size(); ++i) file->buffer[file->bytes + i] = it->second[i]; 
                file->bytes += it->second.size();       //сдвигает bytes на размер скопированного чанка
                file->chunks.erase(it);                 //удаляет использованный чанк 
                progress = true;
            }
        }
    }

    else {   
        file->chunks.emplace(real_pos, std::vector<uint8_t>(chunk.begin() + offset, chunk.begin() + chunkSize)); //откладываем новые байты чанка в массив, пока не заполнится разрыв
    }
}

std::shared_ptr<const std::vector<uint8_t>> FileCollector::GetFile(uint32_t fileId) const {
    std::shared_ptr<File> file;
    {
        std::shared_lock<std::shared_mutex> lock(mapMutex); 
        auto it = files.find(fileId);           
        if (it == files.end()) return nullptr;
        file = it->second;
    }

    std::lock_guard<std::mutex> fileLock(file->fileMutex);
    if (file->bytes == file->total) { 
        return std::make_shared<const std::vector<uint8_t>>(file->buffer); //если файл собран, то возвращаем копию буфера в shared_ptr<const vector>
    }
    return nullptr; //ещё н собран
}