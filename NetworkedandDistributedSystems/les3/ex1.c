/* A simple echo TCP server */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int    sockfd, newsockfd, clilen, n;
    char   line[1000];
    char   sendline[1000];
    struct sockaddr_in servaddr, cliaddr;
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(51444);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
       printf("Can\'t create socket, errno = %d\n", errno);
       exit(1);
    }

    if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
       printf("Can\'t bind socket, errno = %d\n", errno);
       close(sockfd);
       exit(1);
    }

    if(listen(sockfd, 5) < 0){
       printf("Can\'t change state of socket to listen state, errno = %d\n", errno);
       close(sockfd);
       exit(1);
    }

    while(1){
       clilen = sizeof(cliaddr);
        
       if( (newsockfd = accept(sockfd, (struct sockaddr *) &cliaddr, &clilen)) < 0){
          printf("Can\'t accept connection, errno = %d\n", errno);
          close(sockfd);
          exit(1);
       }
       
       printf("Connection with %s\n", inet_ntoa(cliaddr.sin_addr));

       bzero(line,1000);
       while( (n = read(newsockfd, line, 1000)) > 0){
          printf("Client => %s\n",line);
          if(strcmp(line, "stop") == 0){
             break;
          }

          printf("Server => ");
          if(fgets(sendline, 999, stdin) == NULL){
             break;
          }
          sendline[strlen(sendline)-1] = 0;

          if( (n = write(newsockfd, sendline, strlen(sendline)+1)) < 0){
             printf("Can\'t write, errno = %d\n", errno);
             close(sockfd);
             close(newsockfd);
             exit(1);
          }
          if(strcmp(sendline, "stop") == 0){
             break;
          }
          bzero(line,1000);
       } 
       
       close(newsockfd);
       if(n == 0){
          printf("Client disconnected\n");
       } else if(n < 0){
          printf("Can\'t read, errno = %d\n", errno);
       }
       printf("Connection closed\n");
    }   
    return 0;
}
