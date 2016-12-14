.build/test.o: test/test.c src/jread.h src/jread.c
	mkdir -p .build
	gcc test/test.c src/jread.c -o .build/test.o

all: .build/test.o

clean:
	rm -Rf .build

test: .build/test.o
	@if .build/test.o ; then echo "PASSED"; else echo "FAILED"; exit 1; fi;
