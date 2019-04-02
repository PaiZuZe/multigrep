CFLAGS=-Wall -Wextra -Werror -pedantic -lpthread

file: pgrep.cpp
	g++ pgrep.cpp $(CFLAGS) -g -O2 -o pgrep

clean:
	rm pgrep

test:
	./pgrep 32 i ~/Documents