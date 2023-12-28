CFLAGS += -Wall -nostdlib -ffreestanding -g
LDFLAGS += -nostdlib -ffreestanding -g
CC = gcc

all: cat grep hello ls pingpong counter shell sigtest testprint syserr build_fs index_sym

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -Wall -o $@ $<

%.exe: ece391%.o ece391syscall.o ece391support.o
	$(CC) $(LDFLAGS) -o ./exe/$@ $^

%: %.exe
	../elfconvert ./exe/$<
	mv ./exe/$<.converted fs/$@

build_fs:
	../createfs -i ./fs -o ../student-distrib/test_filesys_img

index_sym:
	readelf -S "./exe/cat.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/cat.txt
	readelf -S "./exe/counter.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/counter.txt
	readelf -S "./exe/grep.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/grep.txt
	readelf -S "./exe/hello.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/hello.txt
	readelf -S "./exe/ls.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/ls.txt
	readelf -S "./exe/pingpong.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/pingpong.txt
	readelf -S "./exe/shell.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/shell.txt
	readelf -S "./exe/sigtest.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/sigtest.txt
	readelf -S "./exe/syserr.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/syserr.txt
	readelf -S "./exe/testprint.exe" | grep -A 0 '\.text' | cut -d ' ' -f 26 > ./exe/testprint.txt

clean::
	rm -f *~ *.o

clear: clean
	rm -f *.txt
	rm -f *.converted
	rm -f exe/*
	rm -f fs/*
