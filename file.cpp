#include <iostream>     //cout
#include <fstream>      //file
#include <string>
#include <regex>

#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>  //dir
#include <unistd.h>     //chdir
#include <dirent.h>     //dir

typedef struct {
    std::string name;
    char *pat;
    pthread_mutex_t output_queue;
} thread_arg;

/*
    This function will look recursively for all files .txt inside dirr_name and put their names on name_list.
*/
void getfiles (std::string dirr_name, std::vector <std::string> &name_list) {
    DIR *current = opendir(dirr_name.c_str());  
    std::regex pattern ("\\.txt"); //will only look for .txt for now.    

    for (dirent *curr_file = readdir(current); curr_file != NULL; curr_file = readdir(current)) {
        if (curr_file->d_type == DT_DIR) {            
            if (std::strcmp(curr_file->d_name, ".") && std::strcmp(curr_file->d_name, "..")) {
                getfiles(dirr_name + "/" + curr_file->d_name, name_list);
            }
        }
        else if (std::regex_search(curr_file->d_name, pattern)) {
            name_list.push_back(dirr_name + "/" + curr_file->d_name);
        }
    }
    
    closedir(current);
    return;
}

/*
    This function will look for all lines with the regex pattern pat in the file name. All lines that have a match will be printed to de stdout.
*/
int find (void *args) {
    thread_arg *arg = (thread_arg *) args;
    std::ifstream file;
    std::string line, output;
    std::regex pattern;
    int line_number = 0;
    pattern = arg->pat;
    file.open(arg->name.c_str());
    
    while (getline(file, line)) {
        if (std::regex_search(line, pattern)) {
            output.append(arg->name + ": " + std::to_string(line_number) + "\n");
        }
        line_number++;
    }
    pthread_mutex_lock(&arg->output_queue);
    std::cout << output;
    pthread_mutex_unlock(&arg->output_queue);
    file.close();
    return 0;
}

int main (int argc, char **argv) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments: it's " << argc << " , should be 4\n";
        return 1;
    }
    
    std::vector <std::string> names_list;
    pthread_mutex_t output_queue;
    pthread_mutex_init(&output_queue, NULL);
    
    getfiles(argv[2], names_list);
    thread_arg *args = (thread_arg *) malloc(names_list.size() * sizeof(thread_arg));
    
    for (auto i = names_list.begin(); i != names_list.end(); i++) {        
        args[i - names_list.begin()].pat = argv[3];
        args[i - names_list.begin()].name = *i;
        args[i - names_list.begin()].output_queue = output_queue;
        find(&args[i - names_list.begin()]);
    }
    return 0;
}
