#include <iostream>     //cout
#include <fstream>      //file
#include <string>
#include <regex>
#include <pthread.h>
#include <dirent.h>     //dir
#include <queue>

std::regex pattern;
std::queue<std::string> file_queue; // Files to grep
pthread_mutex_t sem_queue;
pthread_mutex_t output_queue;

/*
    This function will look recursively for all files .txt inside dirr_name and push their 
    location on file_queue.
*/
void getfiles (std::string dirr_name) {
    DIR *current = opendir(dirr_name.c_str());  
    std::regex pattern ("\\.txt"); // Will only look for .txt for now.    

    if (current == NULL) {
        std::cerr << "Error when trying to open " << dirr_name << std::endl;
        std::exit (EXIT_FAILURE);
    }
    for (dirent *curr_file = readdir(current); curr_file != NULL; curr_file = readdir(current)) {
        if (curr_file->d_type == DT_DIR) {            
            if (std::strcmp(curr_file->d_name, ".") && std::strcmp(curr_file->d_name, "..")) {
                getfiles(dirr_name + "/" + curr_file->d_name);
            }
        }
        else if (std::regex_search(curr_file->d_name, pattern)) {
            file_queue.push(dirr_name + "/" + curr_file->d_name);
        }
    }
    
    closedir(current);
    return;
}

/*
    This thread will look for all lines with the regex pattern in the files that are left in 
    file_queue. All lines that have a match will be printed to the stdout.
*/
void *find (void *) {
    std::ifstream file;
    
    while (1) {
        std::string name;
        
        // Looks for available files to process in the global queue.
        pthread_mutex_lock(&sem_queue);
        if (file_queue.empty()) {
            pthread_mutex_unlock(&sem_queue);
            return NULL;
        }
        else {
            name = file_queue.front();
            file_queue.pop();
        }
        pthread_mutex_unlock(&sem_queue);
        
        // Process grep in the file removed from the queue.
        std::string line, output;
        int line_number = 0;
        file.open(name.c_str());
        if (file.fail()) {
            pthread_mutex_lock(&output_queue);
            std::cerr << "There was an error while opening the file " << name << std::endl;
            pthread_mutex_unlock(&output_queue);
        }
        while (getline(file, line)) {
            if (std::regex_search(line, pattern)) {
                output.append(name + ": " + std::to_string(line_number) + "\n");
            }
            line_number++;
        }
        pthread_mutex_lock(&output_queue);
        std::cout << output;
        pthread_mutex_unlock(&output_queue);
        file.close();
    }
    return NULL;
}

/* 
    Process arguments, initialize semaphores and the global queue of files to grep. 
    Create threads to grep the files in the file_queue.
*/
int main (int argc, char **argv) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments: it's " << argc << " , should be 4\n";
        return 1;
    }

    int max_threads = std::stoi(argv[1]);
    pattern = argv[2];
    std::vector <pthread_t> threads;
    pthread_mutex_init(&output_queue, NULL);
    pthread_mutex_init(&sem_queue, NULL);
    
    getfiles(argv[3]);
    if (static_cast<int>(file_queue.size()) < max_threads) {
        max_threads = file_queue.size();
    }

    for (auto i = 0; i != max_threads; i++) {
        threads.push_back(pthread_t());
        pthread_create(&threads[i], NULL, &find, NULL);
    }
    
    for (auto i = threads.begin(); i != threads.end(); i++) {
        pthread_join(*i, NULL);
    }
    return 0;
}
