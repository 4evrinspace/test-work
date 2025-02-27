#include <random>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include <iostream>
#include <string>

#ifndef MIN_BLOCKS
#define MIN_BLOCKS 1
#endif

#ifndef MAX_BLOCKS
#define MAX_BLOCKS 100
#endif

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 0x100000
#endif
#ifndef ZERO_PROPORTION
#define ZERO_PROPORTION 0.3
#endif

class TestGenerator
{
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<std::uint32_t> block_quantity_dis;
    std::uniform_int_distribution<std::uint32_t> block_dis;

public:
    TestGenerator()
        : gen(rd()), block_quantity_dis(MIN_BLOCKS, MAX_BLOCKS), block_dis(1, UINT32_MAX)
    {
    }
    void Generate(std::string filename)
    {
        // Открываем файл
        FILE *file = fopen(filename.c_str(), "wb");
        if (!file)
        {
            std::cout << "Generator: Can't open file " << filename << std::endl;
            exit(-1);
        }

        std::uint32_t blocks_quantity = block_quantity_dis(gen);                              // Кол-во блоков
        std::vector<std::uint32_t> data(BLOCK_SIZE * blocks_quantity);                        // Вектор для хранения блоков (решил делать не массив т.к не при компиляции считаем)
        std::uint64_t all_zeros = blocks_quantity * (uint64_t)(BLOCK_SIZE * ZERO_PROPORTION); // Кол-во нулей

        // Генерируем наши числа (сначала нужное число нулей, потом остальное)
        auto lambda_gen = [&]()
        {
            return block_dis(gen);
        };
        std::fill(data.begin(), data.begin() + all_zeros, 0);
        std::generate(data.begin() + all_zeros, data.end(), lambda_gen);
        std::shuffle(data.begin(), data.end(), gen);

        // Записываем
        if (fwrite(data.data(), sizeof(std::uint32_t), BLOCK_SIZE * (std::uint64_t)blocks_quantity, file) !=
            BLOCK_SIZE * (std::uint64_t)blocks_quantity)
        {
            std::cout << "Generator: Can't write to file " << filename << std::endl;
            exit(-1);
        }

        if (fclose(file))
        {
            std::cout << "Generator: Can't close file " << filename << std::endl;
            exit(-1);
        }
    }
};

int main(int argc, char *argv[])
{
    int test_quantity = 0;
    if (argc != 2)
    {
        std::cout << "Generator: Unexpected number of arguments" << std::endl;
        exit(-1);
    }
    else
    {
        test_quantity = std::stoi(argv[1]);
    }
    TestGenerator tg;
    for (int i = 1; i <= test_quantity; i++)
    {
        tg.Generate(std::to_string(i) + ".in");
    }
    return 0;
}