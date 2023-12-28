# Compiler flags
CFLAGS += -Ilib391 -Wall -nostdlib -ffreestanding -g
CC = gcc

# Specify source files using a wildcard
LIB391_SOURCE_FILES := $(wildcard lib391/*.c lib391/*.S)
SOURCE_FILES := $(wildcard *.c *.S)

# Generate object file names by replacing .c with .o
LIB391_OBJECT_FILES := $(patsubst lib391/%.S, lib391/%.o, $(patsubst lib391/%.c, lib391/%.o, $(LIB391_SOURCE_FILES)))
OBJECT_FILES := $(patsubst %.c, %.o, $(SOURCE_FILES))

all: write_test

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

write_test: $(LIB391_OBJECT_FILES) $(OBJECT_FILES)
	$(CC) $(CFLAGS) -o write_test $^
	cp write_test write_test.exe
	../elfconvert $@
	mv $@.converted $@

# Print the values of variables
.PHONY: print
print:
    $(info OBJ = $(LIB391_OBJECT_FILES) $(OBJECT_FILES))
    $(info CC = $(CC))

.PHONY: clean
clean:
	rm -f *.exe *.o lib391/*.o write_test
