#include "process.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 0x100000
#endif

int hash_file(const char *filename)
{
    // Открываем файл
    int fd;
    if ((fd = open(filename, O_RDONLY)) < 0)
    {
        std::cout << "Hasher: Can't open file " << filename << std::endl;
        exit(-1);
    }

    data_processor_t processor;
    std::uint32_t result = 0;
    bool to_continue = true;
    ssize_t to_read = BLOCK_SIZE * sizeof(std::uint32_t); // Сколько байт считывать за итерацию
    std::vector<std::uint32_t> block(BLOCK_SIZE, 0);
    while (to_continue)
    {
        ssize_t read_bytes = read(fd, block.data(), to_read);
        if (read_bytes < 0)
        {
            std::cout << "Hasher: Can't read from file " << filename << std::endl;
            exit(-1);
        }
        else if (read_bytes < to_read)
        {
            // Когда считали меньше чем нужно, смотрим сколько элементов считали
            to_continue = false;
            std::uint32_t number_of_read_elements = (read_bytes + sizeof(std::uint32_t) - 1) / sizeof(std::uint32_t);
            if (read_bytes % sizeof(std::uint32_t) != 0)
            {
                // В случае, когда последний элемент считали не полностью, обнуляем лишние байты
                std::uint32_t bytes_in_last = read_bytes % sizeof(std::uint32_t);
                block[number_of_read_elements - 1] &= (1 << (bytes_in_last * 8)) - 1;
            }
            block.resize(number_of_read_elements);
        }
        result = processor.process_block(block);
    }

    if (close(fd) < 0)
    {
        std::cout << "Hasher: Can't close file " << filename << std::endl;
        exit(-1);
    }

    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Hasher: Unexpected number of arguments" << std::endl;
        exit(-1);
    }

    std::string filename(argv[1]);
    std::uint32_t hash = hash_file(filename.c_str());
    printf("0x%08x\n", hash);
    return 0;
}

