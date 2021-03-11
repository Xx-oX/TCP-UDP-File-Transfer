all: lab1_file_transfer
#variables
objects := lab1_file_transfer.o functions.o

lab1_file_transfer: $(objects)
	gcc -o lab1_file_transfer $(objects)

%.o: %.c functions.h
	gcc -c $<

.PHONY: clean
clean:
	rm *.o lab1_file_transfer

.PHONY: clear
clear:
	rm *_receive