#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char * msg) {

    perror(msg);
    exit(0);
}

void * mythread1(void * param) {

    int sockfd = * (int * ) param;

    char buffer[256];

    while (1) {

        bzero(buffer, 256);
        buffer[0] = "";
        fgets(buffer, 255, stdin);
        write(sockfd, buffer, strlen(buffer));
    }
}

void * mythread2(void * param) {

    int sockfd = * (int * ) param;
    char buffer[256];

    while (1) {

        read(sockfd, buffer, 255);
        printf("%s\n", buffer);
        bzero(buffer, 256);
        buffer[0] = "";
    }
}

int main(int argc, char * argv[]) {

    pthread_t thrd1, thrd2;

    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent * server;

    if (argc < 3) {

        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);

    if (server == NULL) {

        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char * ) & serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char * ) server -> h_addr, (char * ) & serv_addr.sin_addr.s_addr, server -> h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    pthread_create( & thrd1, NULL, mythread1, (void * ) & sockfd);
    pthread_create( & thrd2, NULL, mythread2, (void * ) & sockfd);

    pthread_join(thrd1, NULL);
    pthread_join(thrd2, NULL);

    close(sockfd);
    return 0;
}