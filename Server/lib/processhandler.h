#ifndef _PROCESSHANDLER_H 
#define _PROCESSHANDLER_H  

typedef struct node {
	char * client;
	char * file;
	char * date;
	char * width;
	char * height;
	char * img;
	char * option;
	int id;
	struct node * next;
} * process_node;

void insertProcess(process_node * process, int id, char * client, char * file, char * date, char* option, char * width, char * height, char * img);
void deleteProcess(process_node * process, int id);
process_node getSmallestProcess(process_node * process);

#endif