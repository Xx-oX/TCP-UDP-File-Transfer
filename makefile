SRC = $(wildcard ./src/*.c)
DIR = $(notdir $(SRC))
OBJ = $(patsubst %.c,./obj/%.o,$(DIR))

TARGET = lab1_file_transfer

CC = gcc
CFLAGS = -Wall -I./inc

all: mkdir $(TARGET)
	@echo "Completed!!!"

./obj/%.o: ./src/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mkdir:
	mkdir -p ./obj

clean:
	find ${OBJ} -name *.o -exec rm -f lab1_file_transfer {} \;

clear:
	rm *_receive

.PHONY: clean clear