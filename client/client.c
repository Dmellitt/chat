//David Mellitt
//Brian Byrne
//

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

void private_message();
void broadcast();
void *handle_messages(void *);

int main(int argc, char *argv[]) { 
  while (!EXIT) {
    // Log in // 
    //login(username)
    
    pthread_t thread;
    int rc = pthread_create(&thread, NULL, handle_messages, NULL);

    //interact //
    while (1) {
      if (rc) {
        printf("Error: unable to create thread\n");
        exit(-1);
      }
      char* op;
      scanf("%s", op);

      if (strcmp(op, "P") == 0)
        private_message();
      else if (strcmp(op, "B") == 0)
        broadcast();
      else if (strcmp(op, "E") == 0)
        exit(-1);
      else {
        printf("Invalid Entry\n");
      }
    }
    s.close_socket();
  }
  return 0;
}

void *handle_messages(){
  while(ACTIVE) { 
    char* message;
    s.recv(message);
    if (message is Data Message){
      //handle data message
    }
    else {
      //handle command message
    }
  }
}
