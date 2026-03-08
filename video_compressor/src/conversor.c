#include <stdio.h>
#include <stdlib.h>
#include <Itypes.h>
#include <string.h>

typedef struct {
  
  FILE *file;
  size_t size;
  char *path;
  
} fileInfo;

typedef struct {
  u8 fps;
  u8 width;
  u8 height;
  u8 grayscale;
} metadata;


fileInfo accessFile(char *path){

  fileInfo fileI = {.path = path};

  fileI.file = fopen(path,"rb");
  if(fileI.file == NULL){
    exit(EXIT_FAILURE);
  }
  fseek(fileI.file, 0, SEEK_END);
  fileI.size = ftell(fileI.file);
  rewind(fileI.file);
  
  return fileI;
}

void set_bit(u8 *buffer, size_t pos){
  buffer[pos/8] |= (1 << (7 - (pos%8)));
}

void clear_bit(u8 *buffer, size_t pos){
  buffer[pos/8] &= ~(1 << (7 - (pos%8)));
}

int main(int argc, char *argv[]){
  
  metadata meta;

  if(argc != 5) return -1;

  fileInfo fileIn = accessFile(argv[1]);
  size_t path_size = strlen(fileIn.path);

  char pathout[path_size + 1];
  strcpy(pathout, fileIn.path);
  pathout[path_size - 3] = 'b';
  pathout[path_size - 2] = 'i';
  pathout[path_size - 1] = 't';

  size_t size_bits = (fileIn.size/24);

  fileInfo fileOut = {.path = pathout, .size = sizeof(metadata) + size_bits};

  meta.height = atoi(argv[2]);
  meta.width = atoi(argv[3]);
  meta.fps = atoi(argv[4]);
  meta.grayscale = 0;

  u8 *bufferIn = malloc(fileIn.size);
  u8 *bufferOut = malloc(fileOut.size);
  memset(bufferOut, 0, fileOut.size);
  fread(bufferIn, 1, fileIn.size, fileIn.file);
  //memset(&bufferOut, 0, fileOut.size);

  u16 sum;
  memcpy(bufferOut, &meta, sizeof(metadata));
  size_t size_in_bits = fileOut.size * 8;
  for(size_t i = ((int)sizeof(metadata) * 8), j = 0; i < size_in_bits; i++, j+=3){
    sum = (u16)(bufferIn[j] + bufferIn[j+1] +bufferIn[j+2])/3;
    
    if(sum < 128)
      clear_bit(bufferOut, i);
    else
      set_bit(bufferOut, i);
  }

  fileOut.file = fopen(fileOut.path, "wb");  
  fwrite(bufferOut, sizeof(u8), fileOut.size/sizeof(u8), fileOut.file);

  printf("%s, %u, %u, %u\n", fileIn.path, meta.height, meta.width, meta.fps);

  fclose(fileIn.file);
  fclose(fileOut.file);
  free(bufferIn);
  free(bufferOut);
  return 0;
}


