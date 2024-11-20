CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -ljpeg -lm
SOURCES = mandel.c mandelMovie.c jpegrw.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLES = mandel mandelMovie

# Default target
all: $(EXECUTABLES)

# Target to build the mandel executable
mandel: mandel.o jpegrw.o
	$(CC) mandel.o jpegrw.o $(LDFLAGS) -o mandel

# Target to build the mandelMovie executable
mandelMovie: mandelMovie.o jpegrw.o
	$(CC) mandelMovie.o jpegrw.o $(LDFLAGS) -o mandelMovie

# Generic rule for compiling .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -rf *.o $(EXECUTABLES)
