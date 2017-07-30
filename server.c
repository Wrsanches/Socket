#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_MAX 3
#define VALUE_MAX 30
#define DIRECTION_MAX 35

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define POUT1 2
#define POUT2 3

#define PIN1 24 
#define PIN2 23 

static int GPIOExport(int pin) {

    int fd;
    ssize_t bytes_written;
    char buffer[BUFFER_MAX];

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {

        fprintf(stderr, "Falha ao abrir Export para escrita\n");
        return (-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}

static int GPIOUnexport(int pin) {

    int fd;
    ssize_t bytes_written;
    char buffer[BUFFER_MAX];

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1) {

        fprintf(stderr, "Falha ao abrir Unexport para escrita\n");
        return (-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}

static int GPIODirection(int pin, int dir) {

    static const char s_directions_str[] = "in\0out";

    int fd;
    char path[DIRECTION_MAX];

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);

    fd = open(path, O_WRONLY);
    if (fd == -1) {

        fprintf(stderr, "Falha ao abrir GPIO Direction para escrita\n");
        return (-1);
    }

    if (write(fd, & s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3) == -1) {

        fprintf(stderr, "Falha ao atribuir valor a Direction\n");
        return (-1);
    }

    close(fd);
    return (0);
}

static int GPIORead(int pin) {

    int fd;
    char value_str[3];
    char path[VALUE_MAX];

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);

    fd = open(path, O_RDONLY);
    if (fd == -1) {

        fprintf(stderr, "Falha ao abrir GPIO Value para leitura\n");
        return (-1);
    }

    if (read(fd, value_str, 3) == -1) {

        fprintf(stderr, "Falha na leitura\n");
        return (-1);
    }

    close(fd);
    return (atoi(value_str));
}

static int GPIOWrite(int pin, int value) {

    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);

    fd = open(path, O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "Falha ao abrir GPIO Value para escrita\n");
        return (-1);
    }

    if (1 != write(fd, & s_values_str[LOW == value ? 0 : 1], 1)) {

        fprintf(stderr, "Falha na escrita\n");
        return (-1);
    }

    close(fd);
    return (0);
}

void error(const char * msg) {

    perror(msg);
    exit(1);
}

void * mythread1(void * param) {

    int n;
    FILE * fp;
    char buffer[256], ch[256];

    int newsockfd = * (int * ) param;

    while (1) {

        n = read(newsockfd, buffer, 255);
        fp = fopen("Log.txt", "a+");

        switch (buffer[0]) {

        case 'a':
            fprintf(fp, "LED 1: Ligado. -%s -- %s \n", __DATE__, __TIME__);
            GPIOExport(POUT1);
            usleep(1000000);
            GPIODirection(POUT1, OUT);
            GPIOWrite(POUT1, HIGH);
            n = write(newsockfd, "LED 1: Ligado.", 13);
            bzero(buffer, 256);
            buffer[0] = "";
            break;

        case 'b':
            fprintf(fp, "LED 1: Desligado. -- %s - %s \n", __DATE__, __TIME__);
            GPIOExport(POUT1);
            usleep(1000000);
            GPIODirection(POUT1, OUT);
            GPIOWrite(POUT1, LOW);
            n = write(newsockfd, "LED 1: Desligado.", 16);
            bzero(buffer, 256);
            buffer[0] = "";
            break;

        case 'c':
            fprintf(fp, "LED 2: Ligado. -- %s - %s \n", __DATE__, __TIME__);
            GPIOExport(POUT2);
            usleep(1000000);
            GPIODirection(POUT2, OUT);
            GPIOWrite(POUT2, HIGH);
            n = write(newsockfd, "LED 2: Ligado.", 13);
            bzero(buffer, 256);
            buffer[0] = "";
            break;

        case 'd':
            fprintf(fp, "LED 2: Desligado. -- %s - %s \n", __DATE__, __TIME__);
            GPIOExport(POUT2);
            usleep(1000000);
            GPIODirection(POUT2, OUT);
            GPIOWrite(POUT2, LOW);
            n = write(newsockfd, "LED 2: Desligado.", 16);
            bzero(buffer, 256);
            buffer[0] = "";
            break;

        case 'e':
            GPIOExport(PIN1);
            usleep(1000000);
            GPIODirection(PIN1, IN);

            if (GPIORead(PIN1) == 1) {

                fprintf(fp, "Status da chave: Ligada -- %s - %s\n", __DATE__, __TIME__);
                write(newsockfd, "Chave 1 Ligada.", 15);

            } else if (GPIORead(PIN1) == 0) {

                fprintf(fp, "Status da chave: Desligada -- %s - %s\n", __DATE__, __TIME__);
                write(newsockfd, "Chave 1 Desligada.", 18);
            }

            break;

        case 'f':
            GPIOExport(PIN2);
            usleep(1000000);
            GPIODirection(PIN2, IN);

            if (GPIORead(PIN2) == 1) {

                fprintf(fp, "Status da chave: Ligada -- %s - %s\n", __DATE__, __TIME__);
                write(newsockfd, "Chave 2 Ligada.", 15);

            } else if (GPIORead(PIN2) == 0) {

                fprintf(fp, "Status da chave: Desligada -- %s - %s\n", __DATE__, __TIME__);
                write(newsockfd, "Chave 2 Desligada.", 18);
            }

            break;

        case 'g':
            system("rm -rf Log.txt");
            write(newsockfd, "Arquivo excluido", 16);
            break;

        default:
            buffer[0] = "";
            write(newsockfd, "Comando Inválido", 16);
            break;
        }

        fclose(fp);
    }
}

void * mythread2(void * param) {

    int cont = 0, cont1 = 0;
    int newsockfd = * (int * ) param;

    GPIOExport(PIN1);
    usleep(1000000);
    GPIODirection(PIN1, IN);

    while (1) {

        if (GPIORead(PIN1) == 1 && cont == 0) {

            cont = 1;
            write(newsockfd, "Chave 1: HIGH", 12);
            usleep(500000);

        } else if (GPIORead(PIN1) == 0 && cont == 1) {

            cont = 0;
            write(newsockfd, "Chave 1: LOW", 11);
            usleep(500000);
        }
        if (GPIORead(PIN2) == 1 && cont1 == 0) {

            cont1 = 1;
            write(newsockfd, "Chave 2: HIGH", 12);
            usleep(500000);
        } else if (GPIORead(PIN2) == 0 && cont1 == 1) {

            cont1 = 0;
            write(newsockfd, "Chave 2: LOW", 11);
            usleep(500000);
        }

    }
}

int main(int argc, char * argv[]) {

    socklen_t clilen;
    pthread_t thrd1, thrd2;
    int newsockfd, thr, sockfd, portno, cont;

    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {

        fprintf(stderr, "Forneça uma porta\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("Erro ao abrir o Socket");

    bzero((char * ) & serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0)
        error("Erro na ligação");

    listen(sockfd, 5);

    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr * ) & cli_addr, & clilen);

    if (newsockfd < 0)
        error("Erro ao aceitar nova conexão");

    pthread_create( & thrd1, NULL, mythread1, (void * ) & newsockfd);
    pthread_create( & thrd2, NULL, mythread2, (void * ) & newsockfd);
    pthread_join(thrd1, NULL);
    pthread_join(thrd2, NULL);

    close(newsockfd);
    close(sockfd);
    return 0;
}