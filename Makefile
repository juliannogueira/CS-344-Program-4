CC = gcc --std=gnu99 -pthread
CFLAGS = -g -Wall
TARGET = line_processor

output: main.o util.o line_processor.o
	$(CC) $(CFLAGS) main.o util.o line_processor.o -o $(TARGET)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

line_processor.o: line_processor.c line_processor.h
	$(CC) $(CFLAGS) -c line_processor.c

clean:
	rm -f *.o $(TARGET)

run:
	./$(TARGET)

check:
	valgrind --leak-check=yes -s ./$(TARGET)