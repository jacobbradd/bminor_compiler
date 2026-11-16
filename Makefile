HEADERS = $(shell find . -maxdepth 2 -name "*.h" ! -path "*/.*")
#(wildcard lexer/*.h data_types/*.h ./*.h)
SOURCES = $(shell find . -maxdepth 2 -name "*.c" ! -path "*/.*")
#(wildcard lexer/*.c data_types/*.c ./*.c)
OBJECTS = $(SOURCES:.c=.o)
DIRS = $(shell find . -type d ! -path "*/.*")

TARGET = b_minor

CC = gcc
CFLAGS = -g -fdiagnostics-color=always -Wall -std=c2x $(addprefix -I,$(DIRS))

.PHONY = all clean

all: b_minor

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
