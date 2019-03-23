CFLAGS=-Wall -Wextra -Werror -pedantic -lpthread

file: file.cpp
	g++ file.cpp $(CFLAGS) -g -o pgrep

clean:
	rm pgrep

test:
	./pgrep 32 i ~/Documents