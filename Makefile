CFLAGS=-Wall -Wextra -Werror -pedantic

file: file.cpp
	g++ file.cpp $(CFLAGS) -o bob

clean: 
	rm bob