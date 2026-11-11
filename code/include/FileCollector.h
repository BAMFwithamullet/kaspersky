#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <atomic>

class FileCollector {
public:
    void CollectFile(uint32_t fileId, size_t fileSize);                         //создаёт структуру данных для файла
    void OnNewChunk(uint32_t fileId, size_t pos, std::vector<uint8_t>&& chunk); //кладёт байты чанка в buffer на нужное место (с позиции pos)
    std::shared_ptr<const std::vector<uint8_t>> GetFile(uint32_t fileId) const; //метод получения собранного файла

private:
    struct File {
        size_t total;                                    //сколько байт должно быть в файле
        std::vector<uint8_t> buffer;                     //массив на total байт (изначально нули)
        std::atomic<size_t> bytes;                      //индекс первого ещё не заполненного байта в массиве
        std::map<size_t, std::vector<uint8_t>> chunks;  //пары (позиция, байты)
        mutable std::mutex fileMutex;                   //замок на этот File (пока поток один держит его, другие ждут)

        File(size_t size) : total(size), buffer(size), bytes(0) {} //конструктор (всё сразу нужного размера)
    };

    mutable std::shared_mutex mapMutex;                         //замок на files (пока держим, никто не читает и не пишет)
    std::unordered_map<uint32_t, std::shared_ptr<File>> files;  //по fileId находим File
}; 