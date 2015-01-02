/*====================================================================================================
	Chatroom project for CO318 network programming lab 2
====================================================================================================*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
/*==================================================================================================*/
#define MAXCLIENTS 8  /* maximum number of clients that can connect at a time */
#define MAXMSG 256    /* maximum message length */
#define PORTNO 12345  /* server port number */
/*==================================================================================================*/
typedef struct {
	int index;      /* position in the clients array */
	int sd;			/* socket descriptor for this client */
	pthread_t tid;  /* ID of the thread handling this client */
	char *name;     /* hostname/IP of the client */
} client_t;
/*==================================================================================================*/
static client_t *clients[MAXCLIENTS];   /* array to hold client informations */
static volatile sig_atomic_t quit = 0;   

pthread_mutex_t broadcast_mutex = PTHREAD_MUTEX_INITIALIZER;  /* mutex for broadcasting */
/*==================================================================================================*/
void broadcast_msg(char *message);
void * handle_client (void *index);
int setup_server(void);
int next_free(void);
/*void cleanup (int signal);*/
/*==================================================================================================*/
void broadcast_msg(char *message)
{
	int i;
	char message2[MAXMSG];
	sprintf(message2,">> %s",message);
	for(i=0;i<MAXCLIENTS;i++){
		if(clients[i]!=NULL){
			send(clients[i]->sd,message2,strlen(message2),0);
			message[strlen(message)-1]='\0';
			printf(" \"%s\" broadcasted\n",message);
			}
		}
}
/*==================================================================================================*/
void * handle_client (void *index)
{
	char buffer[MAXMSG];
	int n; 

	while(1)
	{
		/* recieve the message from client */
		n = recv(clients[*((int*)index)]->sd,buffer,MAXMSG,0);
		
		/* n>0 means client sent something */
		if(n>0){
			
			/* add terminating character to the end of the message */
			buffer[n]='\0';
			
			/* mutex to avoid collisions when broadcasting */
			pthread_mutex_lock(&broadcast_mutex);
			broadcast_msg(buffer);
			pthread_mutex_unlock(&broadcast_mutex);
		}
		
		/* n<=0 means client disconnected or error occured. we terminate the client thread */
		else{
			printf("Client disconnected\n");
			break;
			}	
	}
	
	free(clients[*(int*)index]);
	clients[*(int*)index]=NULL;
		
	return NULL;
}
/*==================================================================================================*/
int setup_server(void)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* Initialize socket structure */
	 
	/* Now bind the host address using bind() call*/
	
	return sockfd; 
}
/*==================================================================================================*/
int next_free(void)
{
	int i=0;
	while(1){
		if(clients[i]==NULL){
			return i;
			}
		if(i==MAXCLIENTS-1){
			printf("client array is full..\n");
			i=0;
			}
		else{
			i++;
		}
	}
}
/*==================================================================================================
void cleanup (int signal)
{
	puts("Caught interrupt. Exiting...\n");
	quit = 1;
}
==================================================================================================*/
int main( int argc, char *argv[] )
{
	int sockfd,current_index=0,i;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t clilen=sizeof(cliaddr);
	
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(PORTNO);

	bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	listen(sockfd,0);
	
	printf("Starting server...\n");
	
	while(!quit)
	{
		current_index=next_free();
		/* Allocate and set up new client_t struct in clients array */
		clients[current_index]=malloc(sizeof(client_t));
		clients[current_index]->index=current_index;
		
		printf("waiting for new connection.\n");
		
		/* Accept an incoming connection */
		clients[current_index]->sd=accept(sockfd, (struct sockaddr *) &cliaddr, &clilen);
		
		printf("new client connected...\n");
		
		/* Create a DETACHED thread to handle the new client until the quit is set */
		if(pthread_create(&(clients[current_index]->tid), NULL, handle_client,(void*)(&(clients[current_index]->index)))){
			printf("error creating thread.");
			abort();
		}
		
	}
	
	puts("Shutting down client connections...\n");
	
	for(i=0;i<MAXCLIENTS;i++){
		free(clients[i]);
		}
		
	/* close(serversock); */
	
	return 0;
}
/*==================================================================================================*/
