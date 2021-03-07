#include <stdio.h>
#include <pthread.h> 
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "processhandler.h"
#include "image_processing.h"
#include "datahandler.h"

#define TRUE 1
#define FALSE 0
#define MAX_RESPONSE_SIZE 41943040

// Variables for all file descriptors given by the operating system.
int server_fd, new_socket, html_fd;
// Struct with all network setting data.
struct sockaddr_in address;
// Size of the struct in bytes.
int addrlen = sizeof(address);
// Process counter
int process_id = 0;
// Main server thread.
pthread_t server_thread, process_thread;
// Mutex struct.
pthread_mutex_t lock;
// Client response memory variables. 
char * response;
char * temp_response;
// Configuration file string data.
char * conf_data;
// Configuration struct. This has all configuration file usefull data.
conf info;
// List for process management.
process_node process_list;
char * time_s;

void init(char * config_path){
    // Reading configuration file.
    setConfigurationFileData(&info, config_path);
    // Creating socket file descriptor. If the syscall socket returns 0 there is an error.
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    //This is to prevent the "In bind: Address already in use" error.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int)) < 0){
        perror("setsockopt(SO_REUSEPORT) failed");
        exit(EXIT_FAILURE);
    }
    // Setting all network values in address struct.
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(info.port);
    // Set all memory spaces with \0 value in sin_zero struct attribute.
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    // Stablishing socket connection with bind system call. If the syscall bind returns less than 0 there is an error.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    // Telling operating system to listen new connection requests for the socket. If the syscall listen returns less than 0 there is an error.
    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    // Reserving memory to avoid segmentation fault when the server is suddenly closed.
    response = (char *) calloc(1, sizeof(char));
    temp_response = (char *) calloc(1, sizeof(char));
    time_s = (char *) calloc(30, sizeof(char));
}

void * run(void *ptr){
    printf("\n------------------ Waiting for new connection ------------------\n\n");
    // Server endless while. It continues forever while there is no error at accepting the client connection request. If the syscall accept returns less than 0 there is an error.
    while((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) > 0) {
        // Releasing response memory to avoid segmentation fault.
        free(response);
        free(temp_response);
        // Reserving memory for client response.
        response = (char *) calloc(MAX_RESPONSE_SIZE, sizeof(char));
        // This response helps to read the socket more than once because the data is not being completely read.
        temp_response = (char *) calloc(MAX_RESPONSE_SIZE/4, sizeof(char));
        // Reading client response and storing that data into the response.
        read(new_socket, response, MAX_RESPONSE_SIZE);
        // Using the function getResponseProperty we get the content length from the response
        char * request_size_str = getProperty("Content-Length", response);
        // Check if content length property exists
        if(request_size_str != NULL){
            // Turning content length to integer
            int request_size = atoi(request_size_str);
            // If the content read by the response is less than the content lenght, we read again the socket until we get the whole response
            while(strlen(response) <= request_size){
                read(new_socket, temp_response, MAX_RESPONSE_SIZE/4);
                strcat(response, temp_response);
                memset(temp_response, 0, MAX_RESPONSE_SIZE/4);
            }
        }

        printf("%s\n", response);
        // Getting all response properties to be used.
        char * client = getProperty("User-Agent", response);
        char * file = getProperty("name", response);
        char * img = getProperty("img", response);
        char * option = getProperty("option", response);
        char * width = getProperty("width", response);
        char * height = getProperty("height", response);
        // Writing information of the new process at log.file.
        writeLog(mergeString(info.log_path, "/log.file"), client, file, time_s, "pending");
        // Inserting process information at process list.
        insertProcess(&process_list, ++process_id, client, file, time_s, option, width, height, img);

        printf("\n------------------ Client connection finished -------------------\n\n");
        printf("\n------------------ Waiting for new s ------------------\n\n");
        // Closing the socket.
        close(new_socket);
    }
}

void * processing(void *ptr){
    // Current process to work with.
    process_node current_process;
    while(TRUE){
        // Getting the current system date.
        getCurrentTime(time_s);
        // If process list is not empty.
        if(process_list != NULL){
            // Getting the smallest img data from process node.
            current_process = getSmallestProcess(&process_list);
            int width = atoi(current_process->width);
            int height = atoi(current_process->height);
            int img_color_type;
            unsigned char *** img = stringToImage(&height, &width, current_process->img);
            unsigned char *** output_img = allocateMemorySpaceForImage(&height, &width);
            // Using option information to fiter o classify the image.
            if(strcmp(current_process->option, "0") == 0){
                // Converting image to gray scale. This is for applying mean and median filter.
                convertImageToGrayscale(&height, &width, img);
                // Setting the string path for storing the filtered median image.
                char * equalizeImageFileName = mergeString(info.equa_path, current_process->file);
                // Applying median filter and storing images.
                histogramEqualisation(&height, &width, img, output_img);  
                generateBitmapImage(height, width, output_img, mergeString(equalizeImageFileName, ".bmp"));     //Save the image

                printf("-> Image equalized and saved\n");
            }else{
                // This function gives
                classifyImageByColor(&height, &width, img, &img_color_type);
                char * classifyColorImageFileName;
                // Color path selection
                if (img_color_type == 0){
                    classifyColorImageFileName = mergeString(info.red_path, current_process->file);
                    generateBitmapImage(height, width, img, mergeString(classifyColorImageFileName, ".bmp"));   //Save the image
                }else if (img_color_type == 1){
                    classifyColorImageFileName = mergeString(info.green_path, current_process->file);
                    generateBitmapImage(height, width, img, mergeString(classifyColorImageFileName, ".bmp"));  //Save the image
                }else{
                    classifyColorImageFileName = mergeString(info.blue_path, current_process->file);
                    generateBitmapImage(height, width, img, mergeString(classifyColorImageFileName, ".bmp"));  //Save the image
                }
            }
            // Freeing all image memory arrays.
            deallocateMemorySpaceOfImage(&height, &width, img);
            deallocateMemorySpaceOfImage(&height, &width, output_img);

            // Writing the completed process information at log.file.
            writeLog(mergeString(info.log_path, "/log.file"), current_process->client, current_process->file, time_s, "completed");
            // Deleting the completed process from process list.
            deleteProcess(&process_list, current_process->id);
        }
    }
}


void start(char * config_path){
    // Initializing all server variables
    init(config_path);
    // Main server thread initialization
    pthread_create(&server_thread,  NULL, run, NULL);
    pthread_create(&process_thread, NULL, processing, NULL);
    pthread_join(server_thread, NULL);
}

void stop(){
    // Close the sever thread
    pthread_cancel(server_thread);
    pthread_cancel(process_thread);
    // Closing the used sockets.
    close(new_socket);
    close(server_fd);
    free(time_s);
}

void startServer(char * config_path){
    start(config_path);
    printf("Server running on port %d\n", info.port);
    printf("Http Server has started\n");
}

void stopServer(){
    stop();
    printf("Http Server has stoped\n");
}
