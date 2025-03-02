#include "process.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
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
    // Семафор
    int semid;
    char pathname[] = "hasher.cpp";
    key_t key;
    struct sembuf mybuf;

    if ((key = ftok(pathname, 0)) < 0)
    {
        std::cout << "Hasher: Can't generate key" << std::endl;
        exit(-1);
    }

    if ((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0)
    {
        std::cout << "Hasher: Can't create semaphore set" << std::endl;
        exit(-1);
    }

    mybuf.sem_num = 0;
    mybuf.sem_op = 1;
    mybuf.sem_flg = 0;

    if (semop(semid, &mybuf, 1) < 0)
    {
        std::cout << "Hasher: Can't add 1 to semaphore" << std::endl;
        exit(-1);
    }

    // Создание разделяемой памяти
    std::uint32_t *hash = (std::uint32_t *)mmap(NULL, sizeof(std::uint32_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (hash == MAP_FAILED)
    {
        std::cout << "Hasher: mmap failed" << std::endl;
        exit(-1);
    }

    if (argc == 1)
    {
        std::cout << "Hasher: No file entered" << std::endl;
        exit(-1);
    }
    // Для хранения айди дочерних процессов
    pid_t pid;
    std::vector<pid_t> pids(argc - 1);
    // Для дочернего процесса - исполняем функцию и завершаем
    // для родительского - сохраняем айди
    for (int i = 1; i < argc; i++)
    {
        if ((pid = fork()) < 0)
        {
            std::cout << "Hasher: Can't fork child " << std::endl;
            exit(-1);
        }
        else if (pid == 0)
        {
            std::string filename(argv[i]);
            std::uint32_t tmp_hash = hash_file(filename.c_str());
            // Закрываем доступ
            mybuf.sem_num = 0;
            mybuf.sem_op = -1;
            mybuf.sem_flg = 0;

            if (semop(semid, &mybuf, 1) < 0)
            {
                std::cout << "Hasher: Can't dec 1 from semaphore" << std::endl;
                exit(-1);
            }

            (*hash) ^= tmp_hash;
            // Открываем доступ
            mybuf.sem_num = 0;
            mybuf.sem_op = 1;
            mybuf.sem_flg = 0;

            if (semop(semid, &mybuf, 1) < 0)
            {
                std::cout << "Hasher: Can't add 1 to semaphore" << std::endl;
                exit(-1);
            }
            exit(0);
        }
        else
        {
            pids[i - 1] = pid;
        }
    }

    for (int i = 0; i < argc - 1; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
    printf("0x%08x\n", *hash);
    if (munmap(hash, sizeof(std::uint32_t)) == -1)
    {
        std::cout << "Hasher: munmap failed" << std::endl;
        exit(-1);
    }
    return 0;
}
