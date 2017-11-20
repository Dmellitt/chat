/*
    C socket server example, handles multiple clients using threads
    Compile

    Based on oleksiiBobko's code
    https://gist.github.com/oleksiiBobko/43d33b3c25c03bcc9b2b

    gcc server.c -lpthread -o server
*/

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

#define BUFSIZE 4096

struct node {
  int socket;
  char username[20];
  struct node * next;
};

//the thread function
void *connection_handler(void *);
int login(int sock);

void del(struct node * current)
{
  if(current->next)
    del(current->next);

  free(current);
}

struct node root;
struct node * newest = &root;

int main(int argc , char *argv[])
{
  int socket_desc , client_sock , c, port;
  struct sockaddr_in server , client;

  if (argc != 2) {
      fprintf(stderr, "Usage: ./chatserver port\n");
      return 1;
  }

  port = atoi(argv[1]);

  //Create socket
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
  {
    printf("Could not create socket");
  }
  puts("Socket created");

  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

  //Bind
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
  {
    //print the error message
    perror("bind failed. Error");
    return 1;
  }
  puts("bind done");

  //Listen
  listen(socket_desc , 3);

  //Accept and incoming connection
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);
  pthread_t thread_id;

  while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
  {
    newest->socket = client_sock;
    newest->next = malloc(sizeof(struct node));
    if(newest->next == NULL){
      printf("Error allocating user node\n");
      return 1;
    }
    newest = newest->next;
    newest->socket = -1;
    newest->next = NULL;

    puts("Connection accepted");

    if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
    {
      perror("could not create thread");
      return 1;
    }

    //Now join the thread , so that we dont terminate before the thread
    //pthread_join( thread_id , NULL);
    puts("Handler assigned");
  }

  if (client_sock < 0)
  {
    perror("accept failed");
    return 1;
  }

  del(root.next);
  return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
  //Get the socket descriptor
  int sock = *(int*)socket_desc;
  char * username;

  if(login(sock) < 0)
    return 0;
}

int login(int sock)
{
  int read_size;
  char *message, password[20], username[20];

  //username
  if( (read_size = recv(sock , username , 20 , 0)) < 0 )
  {
    printf("Error reading\n");
    return -1;
  }

  //end of string marker
	username[read_size] = '\0';

  //open file
  FILE * fp = fopen("./users", "a+");

  if(fp == NULL) {
    printf("Error opening file.\n");
    return -1;
  }

  int exists = 0;
  char * line = NULL;
  size_t len = 0;
  //check name
  while(getline(&line, &len, fp) != -1) {
    line[strlen(line)-1] = '\0';
    printf("%s\n", line);
    if(!strcmp(line, username)) {
      exists = 1;

      // associated password
      getline(&line, &len, fp);
      break;
    }
    // skip password
    getline(&line, &len, fp);
  }

  if(exists)
    message = "Welcome Back! Enter Password >> ";
  else
    message = "New user? Create password >> ";

  if(write(sock , message , strlen(message)) < 0) {
    printf("Error writing\n");
    return -1;
  } 

  //password
  if( read_size = recv(sock , password , 20, 0) < 0 )
  {
    printf("Error reading\n");
    return -1;
  }
	password[read_size] = '\0';

  if(exists) {
    while(strcmp(line, password)) {
      message = "Invalid password.\nPlease enter again >> ";
      if(write(sock , message , strlen(message)) < 0) {
        printf("Error writing\n");
        return -1;
      } 

      if( (read_size = recv(sock , password, 20, 0)) < 0 )
      {
        printf("Error reading\n");
        return -1;
      }
	    password[read_size] = '\0';
    }
    message = "Welcome!\n";
    if(write(sock , message , strlen(message)) < 0) {
      printf("Error writing\n");
      return -1;
    } 
  }
  else {
    fprintf(fp, username);
    fprintf(fp, "\n");
    fprintf(fp, password);
    fprintf(fp, "\n");

    message = "Created new user. Welcome!\n";
    if(write(sock , message , strlen(message)) < 0) {
      printf("Error writing\n");
      return -1;
    } 
  }

  struct node * current = &root;
  while(current->socket != sock)
    current = current->next;
  strcpy(current->username, username);
  
  fclose(fp);
  if(line)
    free(line);

  return 0;
} 
