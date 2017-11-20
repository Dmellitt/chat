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
#include <unistd.h>
#include <libgen.h>

#define MAXDATASIZE 4096

//functions
int private_message();
int broadcast();
void *handle_messages(void *);
int login(char*);
int quit(pthread_t);
void usage();

//global vars
int EXIT = 0;
int ACTIVE = 1;
int s;

//main thread
int main(int argc, char *argv[]) { 
  struct hostent *hp;
  struct sockaddr_in sin;

  if (argc != 4){
    fprintf(stderr, "usage: ./client server port username\n");
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

  //MAIN LOOP//
  while (!EXIT) {
    //Log in 
    if(login(username))
      exit(1);
    
    pthread_t thread;
    int rc = pthread_create(&thread, NULL, handle_messages, NULL);

    //interact //
    while (1) {
      if (rc) {
        printf("Error: unable to create thread\n");
        exit(-1);
      }
      char* op;
      usage();
      scanf("%s", op);

      if (op[0] == 'P')
        private_message();
      else if (op[0] == 'B')
        broadcast();
      else if (op[0] == 'E'){
        quit(thread);
        break;
      }
      else {
        printf("Invalid Entry\n");
      }
    }
    close(s);
  }
  return 0;
}

//helper threads for messages
void *handle_messages(void * args){
  char buf[MAXDATASIZE];
  while(ACTIVE) { 
    char* message;
    if(recv(s, buf, MAXDATASIZE, 0) == -1) {
      perror("client: receive error 2\n");
      return 0;
    }
    if (buf[0] == 'D'){ //check if data message
      printf("%s\n", buf+1);
      fflush(stdout);
    }
    else {
      //handle command message?
    }
  }
  return 0;
}

int login(char* username){
  char buf[MAXDATASIZE];
  char password[MAXDATASIZE];
  int cont = 0;
  if(send(s, username, strlen(username), 0) == -1) {
    perror("client: send error\n");
    return 1;
  }

  if(recv(s, buf, MAXDATASIZE, 0) == -1) {
    perror("client: receive error 1\n");
    return 1;
  }

  while(!cont){
    printf("%s", buf);
    scanf("%s", password);
    
    if (send(s, password, strlen(password), 0) == -1) {
      perror("client: send error\n");
      return 1;
    }

    //gets message back about password
    if(recv(s, buf, MAXDATASIZE, 0) == -1) {
      perror("client: receive error 1\n");
      return 1;
    }

    if(buf[0] != 'I')
      cont = 1;
  }

  return 0;
}

int broadcast(){
  char buf[MAXDATASIZE];
  

  if (send(s, "B", 2, 0) == -1) {
    perror("client: send error\n");
    return 1;
  }
  //acknowledgment 
  if(recv(s, buf, MAXDATASIZE, 0) == -1) {
    perror("client: receive error 1\n");
    return 1;
  }
  //the actual message
  if(buf[0] == '0'){
    printf("Enter broadcast message >> ");
    fflush(stdout);
    scanf("%s", buf);
    if (send(s, buf, strlen(buf), 0) == -1) {
      perror("client: send error\n");
      return 1;
    }
  }
  else
    return 1;
  //confirmation it sent
  if(recv(s, buf, MAXDATASIZE, 0) == -1) {
    perror("client: receive error 1\n");
    return 1;
  }
  printf("%s\n", buf);
  fflush(stdout);
  return 0;
}

int private_message(){
  char buf[MAXDATASIZE];

  //Sending private message
  if (send(s, "P", 2, 0) == -1) {
    perror("client: send error\n");
    return 1;
  }
  //acknowledgment 
  if(recv(s, buf, MAXDATASIZE, 0) == -1) {
    perror("client: receive error 1\n");
    return 1;
  }
  //print users
  printf("%s\n", buf);
  printf("Which user do you want to send to? >> ");
  fflush(stdout);
  scanf("%s", buf);
  
  //choose a user
  if (send(s, buf, strlen(buf), 0) == -1) {
    perror("client: send error\n");
    return 1;
  }

  //write and send message
  printf("Enter private message >> ");
  scanf("%s", buf);

  if (send(s, buf, strlen(buf), 0) == -1) {
    perror("client: send error\n");
    return 1;
  }

  //confirmation
  if(recv(s, buf, MAXDATASIZE, 0) == -1) {
    perror("client: receive error 1\n");
    return 1;
  }

  printf("%s\n", buf);
  fflush(stdout);
  return 0;
}

void usage(){
  printf("Enter P for private conversation.\nEnter B for message broadcasting.\nEnter E for exit.\n>> ");
}

int quit(pthread_t thread){
  if (send(s, "E", 2, 0) == -1) {
    perror("client: send error\n");
    return 1;
  }
  ACTIVE = 0;
  pthread_join(thread, NULL);
  EXIT = 1;
  close(s);

  return 0;
}
