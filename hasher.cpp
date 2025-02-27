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
    std::vector<std::uint32_t> block(BLOCK_SIZE, 0);
    while (to_continue)
    {
        
        //Считываем из файла байты для отдельного элемента 
        // (чтобы обработать случай, когда не до конца считали элдемент)
        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            //Это необходимо для корректного случая, когда не полностью считали элемент
            block[i] = 0;
            ssize_t read_bytes = read(fd, &block[i], sizeof(std::uint32_t));
            if (read_bytes < 0)
            {
                std::cout << "Hasher: Can't read from file " << filename << std::endl;
                exit(-1);
            }
            if ((size_t)read_bytes < sizeof(std::uint32_t))
            {
                to_continue = false;
                //Убераем лишние элементы (стоящие после последнего считанного)
                block.resize(i + 1);
                break;
            }
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

