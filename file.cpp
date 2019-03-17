#include <iostream>     //cout
#include <fstream>      //file
#include <string>
#include <regex>
#include <new>

#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>  //dir
#include <unistd.h>     //chdir
#include <dirent.h>     //dir

typedef struct {
    std::string name;
    char *pattern;
    pthread_mutex_t output_queue;
} thread_arg;

/*
    This function will look recursively for all files .txt inside dirr_name and put their names on name_list.
*/
void getfiles (std::string dirr_name, std::vector <std::string> &name_list) {
    DIR *current = opendir(dirr_name.c_str());  
    std::regex pattern ("\\.txt"); //will only look for .txt for now.    

    if (current == NULL) {
        std::cerr << "Error when trying to open " << dirr_name << std::endl;
        std::exit (EXIT_FAILURE);
    }
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
    This function will look for all lines with the regex pattern pat in the file name. 
    All lines that have a match will be printed to the stdout.
*/
void *find (void *args) {
    thread_arg *arg = (thread_arg *) args;
    std::ifstream file;
    std::string line, output;
    std::regex pattern;
    int line_number = 0;
    pattern = arg->pattern;
    file.open(arg->name.c_str());
    if (file.fail()) {
        pthread_mutex_lock(&arg->output_queue);
        std::cerr << "There was an error while opening the file " << arg->name << std::endl;
        pthread_mutex_unlock(&arg->output_queue);
    }
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
    return NULL;
}

int main (int argc, char **argv) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments: it's " << argc << " , should be 4\n";
        return 1;
    }

    std::vector <std::string> names_list;
    std::vector <thread_arg *> args;
    std::vector <pthread_t> threads;
    pthread_mutex_t output_queue;
    pthread_mutex_init(&output_queue, NULL);
    
    getfiles(argv[3], names_list);
    for (auto i = names_list.begin(); i != names_list.end(); i++) {        
        auto j = i - names_list.begin();
        args.push_back(new thread_arg());
        args[j]->name = *i;
        args[j]->pattern = argv[2];
        args[j]->output_queue = output_queue;
        threads.push_back(pthread_t());
        pthread_create(&threads[j], NULL, &find, args[j]);
    }
    
    for (auto i = threads.begin(); i != threads.end(); i++) {
        pthread_join(*i, NULL);
    }
    return 0;
}
