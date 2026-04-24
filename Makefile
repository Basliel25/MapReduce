CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -pthread -std=gnu11 -g

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = MapReduce

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean: $(TARGET)
	rm -f $(TARGET) $(OBJ)

docs:
	doxygen docs/Doxyfile
	@echo "Docs generated at docs/html/index.html"

tidy:
	clang-tidy $(SRC) -- $(CFLAGS) -Iinclude


