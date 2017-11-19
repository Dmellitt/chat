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

//the thread function
void *connection_handler(void *);

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
  char *message, username[20], password[20], buf[BUFSIZE];

  //username
  if( (read_size = recv(sock , username , 20 , 0)) < 0 )
  {
    printf("Error reading\n");
    return 0;
  }

  //end of string marker
	username[read_size] = '\0';

  //open file
  FILE * fp = fopen("./users", "a+");

  if(fp == NULL) {
    printf("Error opening file.\n");
    return 0;
  }

  int exists = 0;
  char * line = NULL;
  size_t len = 0;
  //check name
  while(getline(&line, &len, fp) != -1) {
    if(!strcmp(buf, username)) {
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

  write(sock , message , strlen(message));

  //password
  if( (read_size = recv(sock , password , 20, 0)) < 0 )
  {
    printf("Error reading\n");
    return 0;
  }

  if(exists) {
    while(strcmp(line, password)) {
      message = "Invalid password.\nPlease enter again >> ";
      write(sock , message , strlen(message));

      if( (read_size = recv(sock , password , 20, 0)) < 0 )
      {
        printf("Error reading\n");
        return 0;
      }
    }
  }
  else {
    fprintf(fp, username);
    fprintf(fp, "\n");
    fprintf(fp, password);
    fprintf(fp, "\n");
  }

  fclose(fp);
  if(line)
    free(line);

	//clear the message buffer
  //memset(client_message, 0, BUFSIZE);



  /*
  //Send some messages to the client
  message = "Greetings! I am your connection handler\n";
  write(sock , message , strlen(message));

  message = "Now type something and i shall repeat what you type \n";
  write(sock , message , strlen(message));

  //Receive a message from client
  while( (read_size = recv(sock , client_message , BUFSIZE , 0)) > 0 )
  {
    //end of string marker
	  client_message[read_size] = '\0';

	  //Send the message back to client
    write(sock , client_message , strlen(client_message));

	  //clear the message buffer
	  memset(client_message, 0, 2000);
  }

  if(read_size == 0)
  {
    puts("Client disconnected");
    fflush(stdout);
  }
  else if(read_size == -1)
  {
    perror("recv failed");
  }
  */ 
  return 0;
} 
