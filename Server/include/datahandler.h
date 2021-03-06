#ifndef _DATAHANDLER_H 
#define _DATAHANDLER_H  

struct info {
   int port;
   char * red_path;
   char * green_path;
   char * blue_path;
   char * equa_path;
   char * log_path;
};

typedef struct info conf;

void   checkPath(char * path);
char * getProperty(char * property, char * data);
void   getCurrentTime(char * time_S);
void   setConfigurationFileData(conf * info, char * config_path);
char * mergeString(char * string1, char * string2);
char * getResponseData(char * response);
void   writeLog(char * path, char * client, char * file, char * time, char * status);
char * writeFile(char* name, char * data);

#endif
