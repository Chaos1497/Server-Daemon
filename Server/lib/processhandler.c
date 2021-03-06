#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This struct stores all process relevant data.
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

// This function inserts a new process into the process list.
void insertProcess(process_node * process, int id, char * client, char * file, char * date, char* option, char * width, char * height, char * img){
	process_node current = (*process), new_process = (process_node) malloc(sizeof(struct node));
	new_process->client = client;
	new_process->file = file;
	new_process->date = date;
	new_process->width = width;
	new_process->height = height;
	new_process->img = img;
	new_process->option = option;
	new_process->id = id;
	new_process->next = NULL;
	if((*process) == NULL){
		(*process) = new_process;
	}else{
		while (current->next != NULL) current = current->next; 
		current->next = new_process;
	}
}

// Giving a unique ID, in this case the date data, this function deletes a process from list.
void deleteProcess(process_node * process, int id){
	process_node to_delete = (* process), previous = NULL;
	while(to_delete->id != id){
		previous = to_delete;
		to_delete = to_delete->next;
	}

	if(previous == NULL){
		(* process) = NULL;
		free(to_delete);
	}else if(to_delete->next == NULL){
		previous->next = NULL;
		free(to_delete);
	}else if(previous == NULL){
		(* process) = to_delete->next;
		free(to_delete);
	}else{
		previous->next = to_delete->next;
		free(to_delete);
 	}
}

// This function returns the smallest data process node from list.
process_node getSmallestProcess(process_node * process){
	int data_size = 2147483647;
	process_node aux = (* process), smallest_process;
	while(aux != NULL){
		if(strlen(aux->img) < data_size){
			smallest_process = aux;
			data_size = strlen(aux->img);
		}
		aux = aux->next;
	}
	return smallest_process;
}