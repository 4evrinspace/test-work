CC = g++
include vars.txt
FLAGS = -std=c++17 -Wall -Wextra -Werror -Wpedantic -g -fsanitize=address
DEFINES = -D MAX_BLOCKS=$(MAX_BLOCKS) -D MIN_BLOCKS=$(MIN_BLOCKS) -D BLOCK_SIZE=$(BLOCK_SIZE) -D ZERO_PROPORTION=$(ZERO_PROPORTION)

.PHONY: all
all: hasher.o


.PHONY: clean
clean: clean_bin clean_test
	rm -f test.txt

.PHONY: clean_bin
clean_bin:
	rm -f *.o *.out

.PHONY: clean_test
clean_test:
	rm -f *.in 

.PHONY: test
test: testgenerator.o hasher.out
	./testgenerator.o $(TESTS_QUANTITY)
	touch test.txt
	./hasher.out $(shell echo *.in) >> test.txt
	$(MAKE) clean_bin


testgenerator.o:
	$(CC) $(DEFINES) $(FLAGS) -o testgenerator.o testgenerator.cpp

hasher.out: process.o
	$(CC) $(DEFINES) $(FLAGS) -o hasher.out hasher.cpp process.o


process.o:
	$(CC) $(FLAGS) -c process.cpp
