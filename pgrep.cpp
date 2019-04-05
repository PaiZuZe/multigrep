/*
 *      pgrep.cpp
 *
 *      pgrep does a recursive search in the DIRECTORY_PATH searching for a PATTERN in each 
 *      FILE using at most MAX_THREADS. pgrep prints which lines of a file matched the PATTERN.
 *
 *      by Guilherme Vieira and Victor Gramuglia
 *
 */

#include <iostream>     //cout
#include <fstream>      //file
#include <string>
#include <regex>
#include <pthread.h>
#include <dirent.h>     //dir
#include <queue>
#include <time.h>

bool finished;
std::regex pattern;
std::queue<std::string> file_queue; // Files to grep
pthread_mutex_t file_queue_mutex;
pthread_mutex_t output_mutex;

#define DIE(...) { \
        pthread_mutex_lock(&output_mutex); \
        std::cerr << __VA_ARGS__; \
        pthread_mutex_unlock(&output_mutex); \
        std::exit (EXIT_FAILURE); \
}

/*
    Looks through the files graph using a iteractive breath first search, all files
    that have a .txt will be put in file_queue.
*/
void *getfiles(void *dirr) {
    std::string dirr_name = static_cast<char *>(dirr);
    DIR *current;
    std::string active;
    std::queue<std::string> frontier;
    frontier.push(dirr_name);
    std::regex pattern ("\\.txt");
    
    while (!frontier.empty()) {
        active = frontier.front();
        frontier.pop();
        current = opendir(active.c_str());
        if (current == NULL) {
            DIE("Error when trying to open " << active << std::endl);
        }   
        for (dirent *curr_file = readdir(current); curr_file != NULL; curr_file = readdir(current)) {
            if (curr_file->d_type == DT_DIR) {            
                if (std::strcmp(curr_file->d_name, ".") && std::strcmp(curr_file->d_name, "..")) {
                    frontier.push(((std::string("").append(active)).append("/")).append(curr_file->d_name));
                }
            }
            else if (std::regex_search(curr_file->d_name, pattern)) {
                pthread_mutex_lock(&file_queue_mutex);
                file_queue.push(((std::string("").append(active)).append("/")).append(curr_file->d_name));
                pthread_mutex_unlock(&file_queue_mutex);
            }
        }
        closedir(current);
    }
    finished = true;
    return NULL;
}

/*
    This thread will look for all lines with the regex pattern in the files that are left in 
    file_queue. All lines that have a match will be printed to the stdout.
*/
void *find (void *) {
    std::string name;
    std::ifstream file;
    struct timespec time;
    time.tv_sec = 0;
    time.tv_nsec = 1000;

    while (1) {
        // Looks for available files to process in the global queue.
        pthread_mutex_lock(&file_queue_mutex);
        if (file_queue.empty()) {
            if (finished) {
                pthread_mutex_unlock(&file_queue_mutex);
                return NULL;
            }
            else {
                pthread_mutex_unlock(&file_queue_mutex);
                nanosleep(&time, NULL);
                continue;
            }
        }
        else {
            name = file_queue.front();
            file_queue.pop();
        }
        pthread_mutex_unlock(&file_queue_mutex);
        
        // Process grep in the file removed from the queue.
        std::string line, output;
        int line_number = 0;
        file.open(name.c_str());
        if (file.fail()) {
            pthread_mutex_lock(&output_mutex);
            std::cerr << "There was an error while opening the file " << name << std::endl;
            pthread_mutex_unlock(&output_mutex);
            continue;
        }
        while (getline(file, line)) {
            if (std::regex_search(line, pattern)) {
                output.append(name + ": " + std::to_string(line_number) + "\n");
            }
            line_number++;
        }
        pthread_mutex_lock(&output_mutex);
        std::cout << output;
        pthread_mutex_unlock(&output_mutex);
        file.close();
    }
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
    finished = false;
    int max_threads = std::stoi(argv[1]);
    pattern = argv[2];
    pthread_t producer;
    std::vector <pthread_t> threads;

    if (max_threads < 3) {
        DIE("This program needs at least 3 threads to run\n");
    }
    if (pthread_mutex_init(&output_mutex, NULL)) {
        DIE("Unable to create output_mutex, terminating\n");
    }
    if (pthread_mutex_init(&file_queue_mutex, NULL)) {
        DIE("Unable to create file_queue_mutex, terminating\n");
    }
    
    // Removes unnecessary slash from directory name.
    char *last_char = &argv[3][static_cast<int>(strlen(argv[3])) -1];
    if (*last_char == '/') {
        *last_char = '\0';
    }
    if (pthread_create(&producer, NULL, getfiles, static_cast<void *>(argv[3]))) {
        DIE("Failed to create producer thread\n");
    }
    for (auto i = 0; i != max_threads - 2; i++) {
        threads.push_back(pthread_t());
        if (pthread_create(&threads[i], NULL, &find, NULL)) {
            DIE("Failed to create thread number " << i << " \n");
        }
    }
    
    for (auto i = threads.begin(); i != threads.end(); i++) {
        if (pthread_join(*i, NULL)) {
            DIE("Failed to join thread number " << *i << " \n");
        }
    }
    pthread_mutex_destroy(&output_mutex);
    pthread_mutex_destroy(&file_queue_mutex);
    
    return 0;
}
