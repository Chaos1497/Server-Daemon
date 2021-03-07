#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h> 

#define COLOR_CHANNELS 3 //RED,GREEN,BLUE
#define R_CHANNEL 2
#define G_CHANNEL 1
#define B_CHANNEL 0
#define WINDOW_FILTER_SIZE 3
#define fileHeaderSize 14
#define infoHeaderSize 40

unsigned char *** allocateMemorySpaceForImage(const int * ptr_height, const  int * ptr_width);
void deallocateMemorySpaceOfImage(const int * ptr_height, const  int * ptr_width, unsigned char *** img);
unsigned char *** stringToImage(const int * ptr_height, const int * ptr_width, char * str_pixels);
void convertImageToGrayscale(const int * ptr_height, const int * ptr_width, unsigned char *** img);
void classifyImageByColor(const int * ptr_height, const int * ptr_width, unsigned char *** img, int * ptr_color_type);
void incrementColorCounter(const unsigned char * ptr_R_value, const unsigned char * ptr_G_value, const unsigned char * ptr_B_value, int * ptr_R_counter, int * ptr_G_counter, int * ptr_B_Counter);
void classifyImage(const int * ptr_R_counter, const int * ptr_G_counter, const int * ptr_B_counter, int * ptr_color_type);
void histogramEqualisation(int cols, int rows, char* input_file_name, char* output_file_name);
void generateBitmapImage(int height, int width, unsigned char *** image, char * imageFileName);
unsigned char * createBitmapFileHeader(int height, int width, int paddingSize);
unsigned char * createBitmapInfoHeader(int height, int width);



//----------------------------------------------------------------------------------------------------------------------
// FUNCTIONS FOR HANDLE THE IMAGE
//----------------------------------------------------------------------------------------------------------------------
unsigned char *** stringToImage(const int * height, const int * width, char * str_pixels){
    unsigned char *** ptr_image = allocateMemorySpaceForImage(height, width);
    char sep[2] = ",";
    char * token = strtok(str_pixels, sep);
    for (int i = *height - 1; i >= 0 ; i--)
        for (int j = *width - 1; j >= 0 ; j--)
            for (int k = COLOR_CHANNELS - 1; k >= 0; k--){
                ptr_image[i][j][k] = (unsigned char)atoi(token);
                token = strtok(NULL, sep);
            }
    return ptr_image;
}

void convertImageToGrayscale(const int * ptr_height, const int * ptr_width, unsigned char *** img){
    for (int i = 0; i < *ptr_height; i++)
        for (int j = 0; j < *ptr_width; j++){
            int grayscale_value = (int)((float)(0.30 * img[i][j][R_CHANNEL]) +
                                        (float)(0.59 * img[i][j][G_CHANNEL]) +
                                        (float)(0.11 * img[i][j][B_CHANNEL]));
            img[i][j][R_CHANNEL] = grayscale_value;
            img[i][j][G_CHANNEL] = grayscale_value;
            img[i][j][B_CHANNEL] = grayscale_value;
        }
}

unsigned char *** allocateMemorySpaceForImage(const int * ptr_height, const  int * ptr_width){
    unsigned char *** img = (unsigned char ***)calloc(*ptr_height, sizeof(unsigned char **));

    if (img == NULL){
        fprintf(stderr, "Out of memory");
        exit(0);
    }

    for (int i = 0; i < *ptr_height; i++){
        img[i] = (unsigned char **)calloc(*ptr_width, sizeof(unsigned char*));

        if (img[i] == NULL){
            fprintf(stderr, "Out of memory");
            exit(0);
        }

        for (int j = 0; j < *ptr_width; j++){
            img[i][j] = (unsigned char *)calloc(COLOR_CHANNELS, sizeof(unsigned char));

            if (img[i][j] == NULL){
                fprintf(stderr, "Out of memory");
                exit(0);
            }
        }
    }
    return img;
}

void deallocateMemorySpaceOfImage(const int * ptr_height, const  int * ptr_width, unsigned char *** img){
    for (int i = 0; i < *ptr_height; i++){
        for (int j = 0; j < *ptr_width; j++)
            free(img[i][j]);
        free(img[i]);
    }
    free(img);
}

//----------------------------------------------------------------------------------------------------------------------
// FUNCTIONS FOR IMAGE CLASSIFICATION
//----------------------------------------------------------------------------------------------------------------------
void classifyImageByColor(const int * ptr_height, const int * ptr_width, unsigned char *** rgb_img, int * ptr_color_type){
    int R_counter = 0, G_counter = 0, B_counter = 0;

    for (int i = 0; i < *ptr_height; i++)
        for (int j = 0; j < *ptr_width; j++){
            incrementColorCounter(&rgb_img[i][j][R_CHANNEL],
                                  &rgb_img[i][j][G_CHANNEL],
                                  &rgb_img[i][j][B_CHANNEL],
                                  &R_counter, &G_counter, &B_counter);
        }
    classifyImage(&R_counter, &G_counter, &B_counter, ptr_color_type);
}


void incrementColorCounter(const unsigned char * ptr_R_value, const unsigned char * ptr_G_value, const unsigned char * ptr_B_value, int * ptr_R_counter, int * ptr_G_counter, int * ptr_B_Counter){
    //RED value predominates or RED by default if (R = G = B)
    if (*ptr_R_value >= *ptr_G_value && *ptr_R_value >= *ptr_B_value)
        (*ptr_R_counter)++;
    //GREEN value predominates or GREEN by default if (R < G = B)
    else if (*ptr_G_value > *ptr_R_value && *ptr_G_value >= *ptr_B_value)
        (*ptr_G_counter)++;
    //BLUE value predominates
    else
        (*ptr_B_Counter)++;
}


void classifyImage(const int * ptr_R_counter, const int * ptr_G_counter, const int * ptr_B_counter, int * ptr_color_type){
    //RED image
    if (*ptr_R_counter >= *ptr_G_counter && *ptr_R_counter >= *ptr_B_counter)
        *ptr_color_type = 0;
    //GREEN image
    else if (*ptr_G_counter > *ptr_R_counter && *ptr_G_counter >= *ptr_B_counter)
        *ptr_color_type = 1;
    //BLUE image
    else
        *ptr_color_type = 2;
}

//----------------------------------------------------------------------------------------------------------------------
// FUNCTIONS FOR IMAGE FILTERING
//----------------------------------------------------------------------------------------------------------------------
void histogramEqualisation(int cols, int rows, 
                           char* input_file_name, char* output_file_name){ 
    // creating image pointer 
    unsigned char* image; 
  
    // Declaring 2 arrays for storing histogram values (frequencies) and 
    // new gray level values (newly mapped pixel values as per algorithm) 
    int hist[256] = { 0 }; 
    int new_gray_level[256] = { 0 }; 
  
    // Declaring other important variables 
    int input_file, output_file, col, row, total, curr, i; 
  
    // allocating image array the size equivalent to number of columns 
    // of the image to read one row of an image at a time 
    image = (unsigned char*)calloc(cols, sizeof(unsigned char)); 
  
    // opening input file in Read Only Mode 
    input_file = open(input_file_name, O_RDONLY); 
    if (input_file < 0) { 
        printf("Error opening input file\n"); 
        exit(1); 
    } 
  
    // creating output file that has write and read access 
    output_file = creat(output_file_name, 0666); 
    if (output_file < 0) { 
        printf("Error creating output file\n"); 
        exit(1); 
    } 
  
    // Calculating frequency of occurrence for all pixel values 
    for (row = 0; row < rows; row++) { 
        // reading a row of image 
        read(input_file, &image[0], cols * sizeof(unsigned char)); 
  
        // logic for calculating histogram 
        for (col = 0; col < cols; col++) 
            hist[(int)image[col]]++; 
    } 
  
    // calculating total number of pixels 
    total = cols * rows; 
  
    curr = 0; 
  
    // calculating cumulative frequency and new gray levels 
    for (i = 0; i < 256; i++) { 
        // cumulative frequency 
        curr += hist[i]; 
  
        // calculating new gray level after multiplying by 
        // maximum gray count which is 255 and dividing by 
        // total number of pixels 
        new_gray_level[i] = round((((float)curr) * 255) / total); 
    } 
  
    // closing file 
    close(input_file); 
  
    // reopening file in Read Only Mode 
    input_file = open(input_file_name, O_RDONLY); 
  
    // performing histogram equalisation by mapping new gray levels 
    for (row = 0; row < rows; row++) { 
        // reading a row of image 
        read(input_file, &image[0], cols * sizeof(unsigned char)); 
  
        // mapping to new gray level values 
        for (col = 0; col < cols; col++) 
            image[col] = (unsigned char)new_gray_level[image[col]]; 
  
        // reading new gray level mapped row of image 
        write(output_file, &image[0], cols * sizeof(unsigned char)); 
    } 
  
    // freeing dynamically allocated memory 
    free(image); 
  
    // closing input and output files 
    close(input_file); 
    close(output_file); 
} 



//----------------------------------------------------------------------------------------------------------------------
// FUNCTIONS FOR SAVE BITMAP IMAGE
//----------------------------------------------------------------------------------------------------------------------
void generateBitmapImage(int height, int width, unsigned char *** image, char * imageFileName){
    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (width * COLOR_CHANNELS) % 4) % 4;
    unsigned char* fileHeader = createBitmapFileHeader(height, width, paddingSize);
    unsigned char* infoHeader = createBitmapInfoHeader(height, width);
    FILE* imageFile = fopen(imageFileName, "wb");
    fwrite(fileHeader, 1, fileHeaderSize, imageFile);
    fwrite(infoHeader, 1, infoHeaderSize, imageFile);

    //------------------------------------------------------------------------------------------------------------------
    unsigned char img_temp[height][width][COLOR_CHANNELS];
    for (int k = 0; k < 3; k++)
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                img_temp[i][j][k] = image[i][j][k];
    unsigned char * img = (unsigned char *)img_temp;
    //------------------------------------------------------------------------------------------------------------------

    for(int i = 0; i < height; i++){
        fwrite(img+(i * width * COLOR_CHANNELS), COLOR_CHANNELS, width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }
    fclose(imageFile);
}

unsigned char * createBitmapFileHeader(int height, int width, int paddingSize){
    int fileSize = fileHeaderSize + infoHeaderSize + (COLOR_CHANNELS * width + paddingSize) * height;
    static unsigned char fileHeader[] = {
            0,0,     /// signature
            0,0,0,0, /// image file size in bytes
            0,0,0,0, /// reserved
            0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize    );
    fileHeader[ 3] = (unsigned char)(fileSize>> 8);
    fileHeader[ 4] = (unsigned char)(fileSize>>16);
    fileHeader[ 5] = (unsigned char)(fileSize>>24);
    fileHeader[10] = (unsigned char)(fileHeaderSize + infoHeaderSize);

    return fileHeader;
}

unsigned char * createBitmapInfoHeader(int height, int width){
    static unsigned char infoHeader[] = {
            0,0,0,0, /// header size
            0,0,0,0, /// image width
            0,0,0,0, /// image height
            0,0,     /// number of color planes
            0,0,     /// bits per pixel
            0,0,0,0, /// compression
            0,0,0,0, /// image size
            0,0,0,0, /// horizontal resolution
            0,0,0,0, /// vertical resolution
            0,0,0,0, /// colors in color table
            0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(infoHeaderSize);
    infoHeader[ 4] = (unsigned char)(width    );
    infoHeader[ 5] = (unsigned char)(width>> 8);
    infoHeader[ 6] = (unsigned char)(width>>16);
    infoHeader[ 7] = (unsigned char)(width>>24);
    infoHeader[ 8] = (unsigned char)(height    );
    infoHeader[ 9] = (unsigned char)(height>> 8);
    infoHeader[10] = (unsigned char)(height>>16);
    infoHeader[11] = (unsigned char)(height>>24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(COLOR_CHANNELS * 8);

    return infoHeader;
}
