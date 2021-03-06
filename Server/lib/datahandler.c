#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_FILE_SIZE 10485760

// Struct used for storing an configuring information abstraction.
struct info {
   int port;
   char * red_path;
   char * green_path;
   char * blue_path;
   char * equa_path;
   char * log_path;
};
typedef struct info conf;

time_t current_time;
struct tm *info;

// This function returns the right side of a text since a dividing string. This dividing string can be more than once inside the text, so the occurrence parameter will demand which of them is.
char * findRight(char * string, char * text, int occurrence){
    int string_counter = 0;
    for(int text_counter = 0; text_counter < strlen(text); text_counter ++){
        if(*(string + string_counter) == tolower(*(text + text_counter)) || *(string + string_counter) == toupper(*(text + text_counter))){
            string_counter ++;
            if(string_counter == strlen(string)){
                if((occurrence--) == 0){
                    return (text + text_counter + 1);
                }
            }
        }else{
            string_counter = 0;
        }       
    }
    return NULL;
}

// This function returns the left side of a text since a dividing string. This dividing string can be more than once inside the text, so the occurrence parameter will demand which of them is.
char * findLeft(char * string, char * text, int occurrence){
    char * text_end = findRight(string, text, occurrence);
    if(text_end != NULL){
        char * split_string = (char *) calloc((text_end - text) + 1, sizeof(char));
        for(int c = 0; c < (text_end - text) - 1; c++) {
            *(split_string + c) = *(text + c);
        }
        return split_string;
    }
    return NULL;
}

// This fucntion returns the middle side between two dividing strings inside a text. These dividing strings can be more than once inside the text, so the occurrence parameter will demand which of them are.
char * findBetween(char * start, char * end, char * text, int occurrence){
    char * start_text, * end_text;
    if((start_text = findRight(start, text, occurrence)) != NULL){
        if((end_text = findLeft(end, start_text, occurrence)) != NULL){
            return end_text;
        } else {
            return start_text;
        }
    } else {
        return NULL;
    }
}

// This function merges two strings.
char * mergeString(char * string1, char * string2){
    int len1 = strlen(string1), len2 = strlen(string2);
    char * merge_string = (char *) calloc(len1 + len2 + 1, sizeof(char));
    for(int c1 = 0; c1 < len1; c1++){
        * (merge_string + c1) = * (string1 + c1);
    }
    for(int c2 = 0; c2 < len2; c2++)
    {
        * ((merge_string + len1) + c2) = * (string2 + c2);
    }
    return merge_string;
}

// This function creates a directory using mkdir system call.
void createDirectory(char * path){
    printf("The directory %s does not exist. Creating it.\n", path);
    if(mkdir(path, 0777) == -1) {
        printf("Error creating %s dir.\n", path);
        exit(EXIT_FAILURE);
    }
}

  
//  C faster way to check if a directory exists
void checkDirectory(char * path){
    // Using stat system call for getting directory information
    struct stat s;
    // If stat system call returns -1 the directory does not exists. If it is not -1 the directory exists. 
    if(stat(path, &s) == -1) {
        createDirectory(path);
    } else {
        if (1 - S_ISDIR(s.st_mode))
            createDirectory(path);
    }
}

// This function checks if a given path exists. If the path does not exist, it will be created.
void checkPath(char * path){
    int occurrence_counter = 0;
    char * temp_path;
    if(*path == '/'){
        occurrence_counter = 1;
    }
    while((temp_path = findLeft("/", path, occurrence_counter++)) != NULL){
        checkDirectory(temp_path);
    }
    checkDirectory(path);
}

// This function reads a file. If the file does not exist, it will be created. 
char * readFile(char * name){
    // Reserving memory to read the file.
    char * file_memory = (char *) calloc(MAX_FILE_SIZE, sizeof(char));
    // Openning the html document in read-only mode.
    int fd = open(name, O_CREAT | O_RDONLY, 0644); 
    // Reding the whole file.
    read(fd, file_memory, MAX_FILE_SIZE); 
    // Closing read file.
    close(fd);
    // Getting file lenght.
    int file_size = strlen(file_memory);
    // Reserving the necessary memory.
    char * file_string = (char *) calloc(file_size, sizeof(char));
    // Coping file_memory to file_string.
    for (int c = 0; c < file_size; c++){
        * (file_string + c) = * (file_memory + c);
    }
    // Setting free the unnecessary memory.
    free(file_memory);
    return file_string;
}

// This function write a file. If the file does not exist, it will be created. 
char * writeFile(char* name, char * data){
    // Openning file in read-only mode. If file does not exist, it will be generated.
    int fd = open(name, O_CREAT | O_WRONLY, 0644);
    // Writing the data.
    write(fd, data, strlen(data));
    // Closing written file.
    close(fd);
}

// This was meant for getting http Post requests properties, but it works for every "key: value" format.
char * getProperty(char * property, char * data){
    char * temp_property = mergeString(property, ": ");
    temp_property = findBetween(temp_property, "\n", data, 0);
    return temp_property;
}

// This function will read the existing configuration file for creating or checking the needed directories.
void setConfigurationFileData(conf * info, char * config_path){
    char * config_file = readFile(config_path);
    printf("%s\n", config_file);
    printf("Configuration file successsfuly read. Everything has been configured.\n");
    // Setting all conf struct values for being used by the server.
    info->port = atoi(getProperty("PORT",config_file));
    info->red_path = mergeString(getProperty("COLOR_DIR", config_file),"/red/");
    info->green_path = mergeString(getProperty("COLOR_DIR", config_file),"/green/");
    info->blue_path = mergeString(getProperty("COLOR_DIR", config_file),"/blue/");
    info->equa_path = mergeString(getProperty("FILTER_DIR", config_file),"/equa/");
    info->log_path = getProperty("LOG_DIR", config_file);
    checkPath(info->red_path);
    checkPath(info->green_path);
    checkPath(info->blue_path);
    checkPath(info->equa_path);
    checkPath(info->log_path);
}

// It converts an integer into a String.
char * intoString(int x){
    int length = snprintf(NULL, 0, "%d", x);
    char* str = malloc(length + 1);
    snprintf(str, length + 1, "%d", x);
    return str;
}

// This is used for writting at the log file with relevant process information.
void writeLog(char * path, char * client, char * file, char * time, char * status){
    char * current_data = readFile(path);
    char * info = "\nclient: ";
    info = mergeString(info, client);
    info = mergeString(info, "\ndate: ");
    info = mergeString(info, time);
    info = mergeString(info, "\nstatus: ");
    info = mergeString(info, status);
    info = mergeString(info, "\nfile: ");
    info = mergeString(info, file);
    info = mergeString(info, "\n");
    printf("------------------------------------------------------------\n%s\n------------------------------------------------------------\nThis has been added at %s\n------------------------------------------------------------\n", info, path);
    if(strcmp(current_data, "") != 0){
        info = mergeString(current_data, info);
    }
    writeFile(path, info);
    free(info);
    free(current_data);
}


//  C library function - strftime()
void getCurrentTime(char * time_S){
    time(&current_time);
    info = localtime(&current_time);
    strftime(time_S, 30, "%x - %I:%M:%S%p", info);
}

// This function returns a new JSON converted into a better format.
char * getResponseData(char * response){
    return findRight("\n\n", response, 0);
}


