.build/test.o: test/test.c src/jread.h src/jread.c
	mkdir -p .build
	$(CC) $(CFLAGS) -O3 test/test.c src/jread.c -o .build/test.o

all: .build/test.o

clean:
	rm -Rf .build

run: .build/test.o
	@if .build/test.o test/test.json ; then echo "PASSED"; else echo "FAILED"; exit 1; fi;

