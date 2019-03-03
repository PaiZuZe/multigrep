#include <iostream>     //cout
#include <fstream>      //file
#include <string>
#include <regex>
#include <sys/types.h>  //dir
#include <unistd.h>     //chdir
#include <dirent.h>     //dir

/*
    This function will look recursively for all files .txt inside dirr_name and put their names on name_list.
*/
void getfiles (std::string dirr_name, std::vector <std::string> &name_list) {
    DIR *current = opendir(dirr_name.c_str());
    dirent *curr_file;
    curr_file = readdir(current);  
    std::regex pattern ("\\.txt"); //will only look for .txt for now.    

    //Change this for a for.
    while (curr_file != NULL) {
        if (curr_file->d_type == DT_DIR) {            
            if (std::strcmp(curr_file->d_name, ".") && std::strcmp(curr_file->d_name, "..")) {
                getfiles(dirr_name + "/" + curr_file->d_name, name_list);
            }
        }
        else if (std::regex_search(curr_file->d_name, pattern)) {
            name_list.push_back(dirr_name + "/" + curr_file->d_name);
        }
        curr_file = readdir(current);
    }
    
    closedir(current);
    return;
}

/*
    This function will look for all lines with the regex pattern pat in the file name. All lines that have a match will be printed to de stdout.
*/
int find (std::string name, char *pat) {
    std::ifstream file;
    std::string line;
    std::regex pattern;
    int line_number = 0;
    pattern = pat;
    file.open(name.c_str());
    
    while (getline(file, line)) {
        if (std::regex_search(line, pattern)) {
            std::cout << name << ' ' << line_number << std::endl;
            std::cout << line << std::endl;
        }
        line_number++;
    }
    
    file.close();
    return 0;
}

int main (int argc, char **argv) {
    std::vector <std::string> names_list;
    std::string blob;
    blob = "blob";
    if (argc < 2) {
        return 1;
    }
    getfiles(argv[1], names_list);
    for (auto i = names_list.begin(); i != names_list.end(); i++) {
        find(*i, argv[2]);
    }
    return 0;
}
