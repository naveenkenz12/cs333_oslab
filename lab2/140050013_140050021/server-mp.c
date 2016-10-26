#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define DATAPACKETS 2048

void sigchld_handler(int s)
{
  while(wait(NULL) > 0){};
}

int main(int argc, char* argv[])
{
    int port = atoi(argv[1]);
    //printf("Starting Server on Port %d \n", port);
  
    int sockfd;
    int new_fd;

    int sin_size; 
    int total_data = 0; 
    
    struct sockaddr_in my_addr; 
    struct sockaddr_in their_addr; 
    struct sigaction sig; 
    
    int x = 1; 
    int numbytes;
    
    char message[100];
    char buffer[DATAPACKETS];
    FILE* fp;
    char file_name[100];

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&x,sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    my_addr.sin_family = AF_INET; 
    my_addr.sin_port = htons(port); 
    my_addr.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(my_addr.sin_zero), 8); 

    if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind error");
        exit(1);
    }

    if(listen(sockfd, 50) == -1)
    {
        perror("listen error");
        exit(1);
    }

    sig.sa_handler = sigchld_handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = SA_RESTART;
    
    if(sigaction(SIGCHLD, &sig, NULL) == -1)
    {
        perror("sigaction error");
        exit(1);
    }

    while(1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }
        
        //printf("server: %s \n", inet_ntoa(their_addr.sin_addr));
        int ret = fork();
        
        if(ret==0)
        {

            if((numbytes = recv(new_fd,message,100,0))== -1)
            {
                perror("recv error");
                exit(1);
            }

            bzero(file_name,100);
            strcpy(file_name,message+4);
            close(sockfd);

            fp = fopen(file_name,"rb");

            //if(fp == NULL)printf("Error in Sending %s \n",file_name);
            //else printf("Sending %s \n",file_name);
            
            numbytes=fread(buffer,1,DATAPACKETS,fp);
            buffer[numbytes-1] = '\0';
            
            while(numbytes>0) 
            {
                total_data = total_data + numbytes;
                send(new_fd,buffer,numbytes,0);
                numbytes=fread(buffer,1,DATAPACKETS,fp);
                buffer[numbytes-1] = '\0';
            }

            //printf("%d bytes sent",total_data);
      
            close(new_fd);
            exit(1);
        }

        close(new_fd);
    }
    return 0;
}
