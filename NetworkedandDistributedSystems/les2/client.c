#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int sockfd, n, len, err, zone;
    int sendmassive[3];
    double recvSi;
    struct sockaddr_in servaddr, cliaddr;
    unsigned short port;
    double pi=0;
    int start_i, end_i, chunk_size, remainder, N, k;
    char buffer[1000];

    if(argc < 2 || argc > 3){
        printf("Usage: a.out <IP address> <port - default 51000>\n");
        exit(1);
    }
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    if(argc == 3){
        err = sscanf(argv[2], "%d", &port);
        if(err != 1 || port == 0){
              printf("Invalid port\n");
              exit(-1);
       }
    } else {
        port = 51000;
    }

    printf("Port set to %d\n", port);

    servaddr.sin_port   = htons(port);
    
    if(inet_aton(argv[1], &servaddr.sin_addr) == 0){
        printf("Invalid IP address\n");
        exit(-1);
    }

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Can\'t create socket, errno = %d\n", errno);
        exit(1);
    }

    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family      = AF_INET;
    cliaddr.sin_port        = htons(0);
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0){
       printf("Can\'t bind socket, errno = %d\n", errno);
       close(sockfd);
       exit(1);
    }
    
    printf("Enter N and k separetd by a space => ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%d %d", &N, &k);
    }
    
    for(zone = 0; zone < k; zone++) {
        start_i = (N / k) * zone;
        end_i = (zone == k - 1) ? N - 1 : (N / k) * (zone + 1) - 1;
        sendmassive[0]=N;
        sendmassive[1]=start_i;
        sendmassive[2]=end_i;
        if(sendto(sockfd, sendmassive, sizeof(sendmassive), 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
            printf("Can\'t send request, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
        if((n = recvfrom(sockfd, &recvSi, sizeof(double), 0, (struct sockaddr *) NULL, NULL)) < 0){
            printf("Can\'t receive answer, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        } 
        pi += recvSi;
    }

    printf("%f\n", pi);
    close(sockfd);
    return 0;
}
