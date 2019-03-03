#include <iostream>
#include <fstream>
#include <string>
#include <regex>

int main (int argc, char **argv) {
    std::ifstream file;
    std::string line;
    std::regex pattern;

    if (argc != 3) {
        std::cout << "Wrong amount of arguments\n";
        return 1;
    }

    pattern = argv[2];

    file.open(argv[1]);
    while (getline(file, line)) {
        if (std::regex_search(line, pattern)) {
            std::cout << line << '\n';
        }
    }
    file.close();
    return 0;
}