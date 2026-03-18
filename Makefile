CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = fault_tolerant_calc
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
