#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

double f(double x){
   if (4 - x * x < 0) {return 0;}    
   else {return sqrt(4 - x * x);}
}

int main()
{
    int sockfd, clilen, n; 
    int arr[3]; 
    int N, start_i, end_i;
    long long i;
    double h, x_i, x_i_plus_1;
    struct sockaddr_in servaddr, cliaddr;    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(52014);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Can\'t create socket, errno = %d\n", errno);
        exit(1);
    }
    if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        printf("Can\'t bind socket, errno = %d\n", errno);
        close(sockfd);
        exit(1);
    }
    while(1) {     
        double S_i=0;  
        clilen = sizeof(cliaddr);       
        if((n = recvfrom(sockfd, arr, sizeof(arr), 0, (struct sockaddr *) &cliaddr, 
        &clilen)) < 0){
            printf("Can\'t receive request, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
        N = arr[0]; start_i=arr[1]; end_i=arr[2];
        //printf("from %s recieved message: N=%d, start_i=%d, end_i=%d\n", inet_ntoa(cliaddr.sin_addr), N, start_i, end_i);
        h = 2.0 / (double)N;
        for (i = (long long)start_i; i <= (long long)end_i; i++) {
            x_i = i * h;           
            x_i_plus_1 = (i + 1) * h;
            S_i += 0.5 * h * (f(x_i) + f(x_i_plus_1));
        }
        if(sendto(sockfd, &S_i, sizeof(double), 0, (struct sockaddr *) &cliaddr, 
        clilen) < 0){
            printf("Can\'t send answer, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
    }
    return 0;
}
