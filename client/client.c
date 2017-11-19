//David Mellitt
//Brian Byrne
//

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

void private_message();
void broadcast();
void *handle_messages(void *);

int main(int argc, char *argv[]) { 
  int s; 
  struct hostent *hp;
  struct sockaddr_in sin;

  if (argc != 4){
    fprintf(stderr, "usage: ./client server port username");
    exit(1);
  }

  char* host = argv[1];
  int port = atoi(argv[2]);
  char* username = argv[3];

  // translate host name into IP address
  hp = gethostbyname(host);
  if(!hp) {
    fprintf(stderr, "client: unknown host: %s\n", host);
    exit(1);
  }

  // build address
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(port);

  // active open
  if ((s=socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("client: socket error");
    exit(1);
  }

  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
    perror("client: connect failed");
    close(s);
    exit(1);
  } 

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
