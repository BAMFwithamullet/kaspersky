#include <gtest/gtest.h>
#include "FileCollector.h"
#include <thread>
#include <vector>
#include <algorithm>

TEST(FileCollectorTest, PoPoryadku) {   //тест 1: чанки приходят по порядку
    FileCollector c;
    c.CollectFile(1, 5);
    c.OnNewChunk(1, 0, {'A', 'B'});
    EXPECT_EQ(c.GetFile(1), nullptr);
    c.OnNewChunk(1, 2, {'C', 'D', 'E'});
    auto f = c.GetFile(1);
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(*f, (std::vector<uint8_t>{'A', 'B', 'C', 'D', 'E'}));
}

TEST(FileCollectorTest, Overlap) {    //тест 2: чанки пересекаются
    FileCollector c;
    c.CollectFile(2, 10);
    c.OnNewChunk(2, 0, {'A', 'B', 'C', 'D'});
    c.OnNewChunk(2, 2, {'C', 'D', 'E', 'F'});   
    c.OnNewChunk(2, 6, {'G', 'H', 'I', 'J'});   
    auto f = c.GetFile(2);
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(*f, (std::vector<uint8_t>{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'}));
}

TEST(FileCollectorTest, Reverse) {   //тест 3: половины в обратном порядке
    FileCollector c;
    c.CollectFile(3, 6);
    c.OnNewChunk(3, 3, {'D', 'E', 'F'});
    EXPECT_EQ(c.GetFile(3), nullptr);
    c.OnNewChunk(3, 0, {'A', 'B', 'C'});
    auto f = c.GetFile(3);
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(*f, (std::vector<uint8_t>{'A', 'B', 'C', 'D', 'E', 'F'}));
}


TEST(FileCollectorTest, BeyondEnd) {  //тест 4: чанк больше файла
    FileCollector c;
    c.CollectFile(4, 4);
    c.OnNewChunk(4, 0, {'A', 'B', 'C', 'D', 'X', 'Y'});
    auto f = c.GetFile(4);
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(*f, (std::vector<uint8_t>{'A', 'B', 'C', 'D'}));
}