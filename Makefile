CFLAGS=-Wall -Wextra -Werror -pedantic -lpthread

file: file.cpp
	g++ file.cpp $(CFLAGS) -o bob

clean: 
	rm bob