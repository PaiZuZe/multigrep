CFLAGS=-Wall -Wextra -Werror -pedantic -lpthread
TEST=-pedantic

file: file.cpp
	g++ file.cpp $(TEST) -o bob

clean: 
	rm bob