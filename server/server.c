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
int broadcast(int sock);
int private(int sock);
int end(int sock);

void del(struct node * current)
{
  if(current->next)
    del(current->next);

  free(current);
}

struct node root;

int main(int argc , char *argv[])
{
  root.socket = -1;
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

  //Listen
  listen(socket_desc , 3);

  //Accept and incoming connection
  c = sizeof(struct sockaddr_in);
  pthread_t thread_id;

  while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
  {
    struct node * current = &root;
    while(current->next)
      current = current->next;

    current->next = malloc(sizeof(struct node));
    if(current->next == NULL){
      printf("Error allocating user node\n");
      return 1;
    }
    current = current->next;
    current->socket = client_sock;
    current->next = NULL;

    if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
    {
      perror("could not create thread");
      return 1;
    }

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
  int read_size;
  char * username, buf[10];

  if(login(sock) < 0)
    return 0;

  while(1) {
    if(recv(sock , buf, 10 , 0) < 0)
    {
      printf("Error reading\n");
      return 0;
    }

    if(buf[0] == 'B') {
      if(broadcast(sock) < 0)
        return 0;
    }
    else if(buf[0] == 'P') {
      if(private(sock) < 0)
        return 0;
    }
    else if (buf[0] == 'E') {
      end(sock);
      return 0;
    }
  }
}

int broadcast(int sock)
{
  int read_size;
  char buf[BUFSIZE];
  char * message = "0";

  if(send(sock , message , strlen(message)+1, 0) < 0) {
    printf("Error writing\n");
    return -1;
  }
  if(send(sock , message , strlen(message)+1, 0) < 0) {
    printf("Error writing\n");
    return -1;
  }

  buf[0] = 'D';
  struct node * current = root.next;
  while(current->socket != sock)
    current = current->next;

  int i;
  for(i = 0; i < strlen(current->username); i++)
    buf[i+1] = current->username[i];

  buf[i+1] = ':';
  buf[i+2] = ' ';
  buf[i+2] = '\0';
  int pre = strlen(buf);
  if( (read_size = recv(sock , buf+pre, BUFSIZE-pre, 0)) < 0 )
  {
    printf("Error reading\n");
    return -1;
  }
  buf[read_size+pre] = '\0';

  current = root.next;
  while(current) {
    if(write(current->socket, buf, strlen(buf)+1) < 0) {
      printf("Error writing\n");
      return -1;
    }
    current = current->next;
  }
    
  message = "Broadcast sent\n";
  if(write(sock , message , strlen(message)+1) < 0) {
    printf("Error writing\n");
    return -1;
  }

  return 0;
}

int private(int sock)
{
  char * buf;

  buf = malloc(BUFSIZE);
  memset(buf, '\0', BUFSIZE);
  if(buf == NULL) {
    printf("Error allocating list space\n");
    return -1;
  }

  struct node * current = root.next;
  while(current) {
    strcat(buf,current->username);
    strcat(buf,"\n");
    current = current->next;
  }

  // send list
  int i;
  for(i = 0; i < 2; i++)
  if(write(sock , buf , strlen(buf)+1) < 0) {
    printf("Error writing\n");
    free(buf);
    return -1;
  }

  // get target user
  int read_size; 
  if( (read_size = recv(sock , buf, BUFSIZE, 0)) < 0 )
  {
    printf("Error reading\n");
    return -1;
  }
  buf[read_size] = '\0';
  
  current = root.next;
  while(current && strcmp(current->username, buf))
    current = current->next;

  int target = NULL;
  if(current)
    target = current->socket;

  // append sending user's name to buffer
  current = root.next;
  while(current->socket != sock)
    current = current->next;

  buf[0] = 'D';
  for(i = 0; i < strlen(current->username); i++)
    buf[i+1] = current->username[i];

  buf[i+1] = ':';
  buf[i+2] = ' ';
  buf[i+2] = '\0';
  int pre = strlen(buf);

  // read message
  if( (read_size = recv(sock , buf+pre, BUFSIZE-pre, 0)) < 0 )
  {
    printf("Error reading\n");
    return -1;
  }
  buf[read_size+pre] = '\0';

  char * message = "Not sent";
  if(target) {
    if(write(target, buf, strlen(buf)+1) < 0) {
      printf("Error writing\n");
      return -1;
    }
    message = "Sent";
  }
  
  for(i=0;i<2;i++)
  if(write(sock , message , strlen(message)+1) < 0) {
    printf("Error writing\n");
    return -1;
  }

  free(buf);
  return 0;
}

int end(int sock)
{
  close(sock);
  struct node * current = &root;

  while(current->next->socket != sock)
    current = current->next;

  struct node * temp = current->next->next;
  free(current->next);
  current->next = temp;

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
    if(!strcmp(line, username)) {
      exists = 1;

      // associated password
      getline(&line, &len, fp);
      line[strlen(line)-1] = '\0';
      break;
    }
    // skip password
    getline(&line, &len, fp);
  }

  if(exists)
    message = "Welcome Back! Enter Password >> ";
  else
    message = "New user? Create password >> ";

  if(write(sock , message , strlen(message)+1) < 0) {
    printf("Error writing\n");
    return -1;
  } 

  //password
  if((read_size = recv(sock , password , 20, 0)) < 0 )
  {
    printf("Error reading\n");
    return -1;
  }
	password[read_size] = '\0';

  if(exists) {
    while(strcmp(line, password)) {
      message = "Invalid password.\nPlease enter again >> ";
      if(write(sock , message , strlen(message)+1) < 0) {
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
    if(write(sock , message , strlen(message)+1) < 0) {
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
    if(write(sock , message , strlen(message)+1) < 0) {
      printf("Error writing\n");
      return -1;
    } 
  }

  struct node * current = root.next;
  if(current == NULL)
    printf("NULL!\n");

  while(current->socket != sock)
    current = current->next;
  strcpy(current->username, username);
  
  fclose(fp);
  if(line)
    free(line);

  return 0;
} 
