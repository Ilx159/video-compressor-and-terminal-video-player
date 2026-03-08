#include <stdio.h>
#include <stdlib.h>
#include <Itypes.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

  
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

u8 test_bit(u8 *buffer, size_t pos){
  return (buffer[pos/8] & (1 << (7 - (pos%8))))?1:0;
};

int main(int argc, char *argv[]){



  struct timespec ts;
  initscr();
  curs_set(0);
  cbreak();
  keypad(stdscr, TRUE);
  noecho();
  //nodelay(stdscr, TRUE);
  //leaveok(stdscr, TRUE);


  metadata metaData;

  if(argc != 2) return -1;

  fileInfo fileIn = accessFile(argv[1]);
  u8 *buffer = malloc(fileIn.size);
  u8 *video = malloc(fileIn.size - sizeof(metaData));
  fread(buffer, 1, fileIn.size, fileIn.file);
  memcpy(&metaData, buffer, sizeof(metaData));
  memcpy(video, buffer + sizeof(metaData), fileIn.size - sizeof(metaData));
  

  if(metaData.fps == 0){
    endwin();
    free(video);
    free(buffer);
    printf("FPS inválido\n");
    return 1;
}
  long long frameT = 1000000000LL / (long long)metaData.fps;

  size_t video_bytes = fileIn.size - sizeof(metadata);
  size_t video_bits  = video_bytes * 8;
  size_t frame_bits  = metaData.width * metaData.height;
  size_t frames = video_bits / frame_bits;
  int frame_size = (frame_bits*3 + metaData.height);
  char *frame = malloc(frame_size);
 


  
  for(u16 i = metaData.width*3; i < frame_size - 1; i+= (metaData.width*3 + 1)) frame[i] = '\n';
  frame[frame_size - 1] = '\0';

   for(size_t i = 0; i < frames; i++){
      clock_gettime(CLOCK_MONOTONIC, &ts);
      size_t offset = i * frame_bits;
      long long inicio_ts = ts.tv_sec * 1000000000LL + ts.tv_nsec;
      for(u8 y = 0; y < metaData.height; y++){
        for(u8 x = 0; x < metaData.width; x++){
          if(test_bit(video,(y*metaData.width + x + offset))){
            frame[(y*metaData.width*3 + y) + x*3 + 2] = '@';
            frame[(y*metaData.width*3 + y) + x*3 + 1] = '@';
            frame[(y*metaData.width*3 + y) + x*3] = '@';
          }
          else{
            frame[(y*metaData.width*3 + y) + x*3 + 2] = ' ';
            frame[(y*metaData.width*3 + y) + x*3 + 1] = ' ';
            frame[(y*metaData.width*3 + y) + x*3] = ' ';
          }
        }
      }
      
      mvaddstr(0,0,frame);
      refresh();
    

      clock_gettime(CLOCK_MONOTONIC, &ts);
      long long fim_ts = ts.tv_sec * 1000000000LL + ts.tv_nsec;
      long long elapsed = fim_ts - inicio_ts; 
      
      if(elapsed < frameT){
        long long restante = frameT - elapsed;

        ts.tv_sec = 0;
        ts.tv_nsec = restante;

        nanosleep(&ts, NULL);
      }
   }
  endwin();
  fclose(fileIn.file);
  free(buffer);
  free(video);
  free(frame);
  return 0;
}


