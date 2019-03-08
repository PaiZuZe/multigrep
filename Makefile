CFLAGS=-Wall -Wextra -Werror -pedantic -lpthread
TEST=-lpthread

file: file.cpp
	g++ file.cpp $(CFLAGS) -g -o bob

clean: 
	rm bob