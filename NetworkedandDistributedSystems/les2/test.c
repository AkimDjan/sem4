
/* A simple echo UDP client */

#include <sys/types.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    char buffer[100];
    int a, b;
    char extra;
    printf("Введите два числа через пробел: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%d %d %c", &a, &b, &extra);
    }
    
    return 0;
}