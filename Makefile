# Kompilátor a přepínače (vyžadováno gcc 14/15/16 nebo clang 19/23 na Merlinovi)
CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -g -O2

# Název výsledné binárky
TARGET = ifj26

# Vyhledání všech .c souborů ve složce src/
SRCS = $(wildcard src/*.c)
# Vytvoření seznamu .o souborů ze seznamu .c souborů
OBJS = $(SRCS:.c=.o)

.PHONY: all build clean check format test

# Výchozí cíl – spustí se při prostém zavolání 'make'
all: build

# Sestavení překladače
build: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Pravidlo pro kompilaci .c na .o
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Úklid po překladu
clean:
	rm -f $(OBJS) $(TARGET)

# Kontrola formátování a statická analýza (doporučeno v zadání, netestuje se na Merlinovi)
check:
	clang-format --dry-run --Werror src/*.c src/*.h
	cppcheck --enable=all --suppress=missingIncludeSystem src/*.c

# Cíl, který vám kód fyzicky přeformátuje podle .clang-format
format:
	clang-format -i src/*.c src/*.h

# Cíl pro rychlé spuštění vašich paměťových testů
test: build
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) < tests/e2e/test1.scala

CXX = g++
CXXFLAGS = -std=c++14 -Wall -g
GTEST_LIBS = -lgtest -lgtest_main -pthread

# Najde všechny C++ testy
TEST_SRCS = $(wildcard tests/unit/*.cpp)
# Vyloučíme src/main.c, abychom neměli dva main() (jeden z projektu, druhý z gtest_main)
SRC_NO_MAIN = $(filter-out src/main.c, $(SRCS))
OBJ_NO_MAIN = $(SRC_NO_MAIN:.c=.o)

.PHONY: unit-test clean-tests

# Cíl pro překlad a spuštění unit testů
unit-test: $(OBJ_NO_MAIN)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) $(OBJ_NO_MAIN) -o run_unit_tests $(GTEST_LIBS)
	./run_unit_tests

clean-tests:
	rm -f run_unit_tests