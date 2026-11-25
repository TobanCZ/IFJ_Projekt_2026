# Kompilátor a přepínače (vyžadováno gcc 14/15/16 nebo clang 19/23 na Merlinovi)
CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -g -O2

# Název výsledné binárky
TARGET = ifj26

# Složka pro objektové soubory (.o)
OBJ_DIR = obj

# Vyhledání všech .c souborů ve složce src/
SRCS = $(wildcard src/*.c)
# Vytvoření seznamu .o souborů ze seznamu .c souborů
OBJS = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all build clean check format test run unit-test clean-tests

# Výchozí cíl – spustí se při prostém zavolání 'make'
all: build

# Sestavení překladače
build: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Pravidlo pro kompilaci .c na .o do složky $(OBJ_DIR)
$(OBJ_DIR)/%.o: src/%.c $(wildcard src/*.h) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Vytvoření složky pro objektové soubory
$(OBJ_DIR):
	mkdir -p $@

#Rovnou spusti
run: build
	./$(TARGET)

# Úklid po překladu
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

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
CXXFLAGS = -std=c++17 -Wall -g -Isrc
GTEST_LIBS = -lgtest_main -lgtest -pthread

# Najde všechny C++ testy
TEST_SRCS = $(wildcard test/unit/*.cpp)
# Vyloučíme src/main.c, abychom neměli dva main() (jeden z projektu, druhý z gtest_main)
SRC_NO_MAIN = $(filter-out src/main.c, $(SRCS))
OBJ_NO_MAIN = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SRC_NO_MAIN))


# Cíl pro překlad a spuštění unit testů
unit-test: $(OBJ_NO_MAIN)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) $(OBJ_NO_MAIN) -o run_unit_tests $(GTEST_LIBS)
	./run_unit_tests

clean-tests:
	rm -f run_unit_tests