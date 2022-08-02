/*
  * Elliott Chimienti
  * 933-135-216
*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>  // Threading API
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

//declare shared mutex
pthread_mutex_t bufferMutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bufferMutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bufferMutex3 = PTHREAD_MUTEX_INITIALIZER;

// condition variables
pthread_cond_t bufferCond1  = PTHREAD_COND_INITIALIZER;
pthread_cond_t bufferCond2  = PTHREAD_COND_INITIALIZER;
pthread_cond_t bufferCond3  = PTHREAD_COND_INITIALIZER;

// Shared Buffers (50 lines 1000 chars)      (P)    (C)
char buffer1[50][1000];    // shared between T1 and T2
char buffer2[50][1000];    // shared bewteen T2 and T3
char buffer3[50][1000];    // shared between T3 and T4

// Current index for producers
int pro_idx1 = 0;
int pro_idx2 = 0;
int pro_idx3 = 0;
// Current index for consumers
int con_idx1 = 0;
int con_idx2 = 0;
int con_idx3 = 0;

// Current sizes of buffers
int size1 = 0;
int size2 = 0;
int size3 = 0;

/*
  Thread 1 (Input thread)
  Read character lines from stdin
*/
void* input_thread(){
  char *line_input;
  char *input;
  size_t maxChar = 1000;

  while(1){ // Loops until user enters "STOP\n"
    getline(&line_input, &maxChar, stdin);
    input = calloc(maxChar+1, sizeof(char)); // Allocate memory to valid input
    strcpy(input, line_input);  // Copy user input

    // ----- Put into Buffer1 ------
    // lock mutex1
    pthread_mutex_lock(&bufferMutex1);
    // Add input to buffer
    strcpy(buffer1[pro_idx1], line_input);
    pro_idx1++; // increase index by 1
    size1++;  // Size of buffer increased by 1
    // tell consumer buffer has data
    pthread_cond_signal(&bufferCond1);
    // unlock mutex
    pthread_mutex_unlock(&bufferMutex1);
    // communicate to consumer its unlocked
    if(strcmp(input, "STOP\n") == 0){ // If user enters stopping string
      break;  // Kill thread
    }
  }
}

/*
  Thread 2 (Line seperator)
  Replace line seperators with spaces
*/
void *line_seperator(){
  char *line_input;
  while(1){ // Loop until STOP detected

    // ------- Take from buffer1 ---------
    pthread_mutex_lock(&bufferMutex1);  // lock the buffer
    if(size1 == 0){ // Wait for new input
      pthread_cond_wait(&bufferCond1, &bufferMutex1);
    }
    line_input = buffer1[con_idx1];
    size1--;
    con_idx1++;
    pthread_mutex_unlock(&bufferMutex1);  // lock the buffer
    // ------ Convert new line into spaces -------
    for(int i = strlen(line_input)-1; i >= 0; i--){
      if(line_input[i] == '\n'){
        line_input[i] = ' ';
      }
    }
    // ------- Put into buffer2 --------
    pthread_mutex_lock(&bufferMutex2);
    strcpy(buffer2[pro_idx2], line_input);
    pro_idx2++;
    size2++;
    pthread_cond_signal(&bufferCond2);  // tell consumer data is read
    pthread_mutex_unlock(&bufferMutex2);

    if(strcmp(line_input, "STOP ") == 0){ // If stop command is detected
      return NULL; // Stop thread
    }
  }
}

/*
  Thread 3 (Plus sign)
  Replace "++" with "^"
*/
void *plus_sign(){
  char* line_input;
  int length;
  while(1){
    // ------- Take from buffer 2 --------
    pthread_mutex_lock(&bufferMutex2);  // lock buffer2
    if(size2 == 0){
    pthread_cond_wait(&bufferCond2, &bufferMutex2);
    }
    line_input = buffer2[con_idx2];  // Grab new line
    size2--;
    con_idx2++;
    pthread_mutex_unlock(&bufferMutex2);  // unlock buffer2

    // ------- plus signs conversion --------
    length = strlen(line_input);
    for(int i = 0; i < length; i++){  // Search current line
      if(line_input[i] == '+' && line_input[i+1] == '+'){  // If "++" is found
        line_input[i] = '^';
        for(int j = i+1; j < length; j++){  // repair string
          line_input[j] = line_input[j+1];
        }
      }
    }

    // ------- Put into buffer3 --------
    pthread_mutex_lock(&bufferMutex3);  // lock buffer3
    strcpy(buffer3[pro_idx3], line_input);
    pro_idx3++;
    size3++;
    pthread_cond_signal(&bufferCond3);  // Tell signal
    pthread_mutex_unlock(&bufferMutex3);  // unlock buffer3

    if(strcmp(line_input, "STOP ") == 0){
      return NULL; // Stop thread
    }
  }
}

/*
  Thread 4 (Output_thread)
  Stdout as lines of exactly 80 characters
*/
void *output_thread(){
  char* line_input;
  char output[80];  // Prints in lines of 80 characters
  int size = 0; // Size of line input
  while(1){
    // ------ Take from buffer3 -------
    pthread_mutex_lock(&bufferMutex3);  // lock buffer3
    if(size3 == 0){
      pthread_cond_wait(&bufferCond3, &bufferMutex3); // wait until buffer has new data
    }
    line_input = buffer3[con_idx3];
    con_idx3++;
    size3--;
    pthread_mutex_unlock(&bufferMutex3);

    if(strcmp(line_input, "STOP ") == 0){
      return NULL; // Stop thread
    }

    for(int i = 0; i < strlen(line_input); i++){
      output[size] = line_input[i];
      size++;
      if(size == 80){
        printf("%s\n",output);
        size = 0;
      }
    }
  }
}

int main(int argc, char *argv[]){

  pthread_mutex_init(&bufferMutex1, NULL);  // Initialize buffers mutexes
  pthread_mutex_init(&bufferMutex2, NULL);
  pthread_mutex_init(&bufferMutex3, NULL);

  pthread_t t_1, t_2, t_3, t_4;
  pthread_create(&t_1, NULL, input_thread, NULL);   // Create pthreads
  pthread_create(&t_2, NULL, line_seperator, NULL);
  pthread_create(&t_3, NULL, plus_sign, NULL);
  pthread_create(&t_4, NULL, output_thread, NULL);

  pthread_join(t_1, NULL); // Wait for Thread termination
  pthread_join(t_2, NULL);
  pthread_join(t_3, NULL);
  pthread_join(t_4, NULL);

  pthread_mutex_destroy(&bufferMutex1);  // Destroy mutexes
  pthread_mutex_destroy(&bufferMutex2);
  pthread_mutex_destroy(&bufferMutex3);

}
