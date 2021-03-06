#ifndef _IMAGE_PROCESSING_H
#define _IMAGE_PROCESSING_H

unsigned char *** allocateMemorySpaceForImage(const int * ptr_height, const  int * ptr_width);
void deallocateMemorySpaceOfImage(const int * ptr_height, const  int * ptr_width, unsigned char *** img);
unsigned char *** stringToImage(const int * ptr_height, const int * ptr_width, char * str_pixels);
void convertImageToGrayscale(const int * ptr_height, const int * ptr_width, unsigned char *** img);
void classifyImageByColor(const int * ptr_height, const int * ptr_width, unsigned char *** img, int * ptr_color_type);
void incrementColorCounter(const unsigned char * ptr_R_value, const unsigned char * ptr_G_value, const unsigned char * ptr_B_value,
                           int * ptr_R_counter, int * ptr_G_counter, int * ptr_B_Counter);
void classifyImage(const int * ptr_R_counter, const int * ptr_G_counter, const int * ptr_B_counter, int * ptr_color_type);
void histogramEqualisation(int cols, int rows, char* input_file_name, char* output_file_name)
void generateBitmapImage(int height, int width, unsigned char *** image, char * imageFileName);
unsigned char * createBitmapFileHeader(int height, int width, int paddingSize);
unsigned char * createBitmapInfoHeader(int height, int width);

#endif //IMG_PROCESSING_IMAGE_PROCESSING_H
