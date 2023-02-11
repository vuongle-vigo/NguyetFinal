
#define _GNU_SOURCE
#define TIMEOUT 500 //timeout after 500 seconds

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <errno.h>
# include <sys/socket.h>
# include <signal.h>
# include <wait.h>
# include <pthread.h>
# include <sys/poll.h>


# include "FileInforStruct.h"
# include "DatabaseHandle.h"
# include "RequestHandle.h"


#define MAX_BUF_SIZE 1024
#define MAX_CONNECTIONS 1024
#define HOST_NAME "127.0.0.1"
#define PORT_NUMBER 8089

struct sockaddr* setsocketaddr(char *host, int port);
int setSock(char *host, int port);
int deleteConnect(int fd, struct pollfd fds[]);
int recvData(int fd, char *buffer, int maxlenBuff);
int sendData(int fd, char *buffer, int lenBuff);

int gcount = 1;

int main(int argc, char *argv[]){
    FileShareInfor fileShareInfor = readDataBase(fileShareInfor);
    struct sockaddr_in caList[MAX_CONNECTIONS];
    int sockfd = setSock(HOST_NAME, PORT_NUMBER);
    listen(sockfd, 5);
    struct pollfd pfd[MAX_CONNECTIONS];
    pfd[0].fd = sockfd;
    pfd[0].events = POLLIN | POLLPRI;
    while(1){
        int ret = -1;
        for(int i = 1;i < 100; i++){
            pfd[i].events = POLLRDHUP | POLLIN;
        }
        ret = poll(pfd, gcount, TIMEOUT * 1000);
        if(ret == -1){
            perror("poll() error!");
            return -1;
        }
        if (0 == ret){
            printf ("poll() timeout sau %d giay.\n", TIMEOUT);
            return 0;
        }
        if(pfd[0].revents & POLLIN){
            struct sockaddr_in ca =  {0};
            int addrlen = sizeof(ca);
            pfd[gcount].fd = accept(sockfd, (struct sockaddr*)&ca, &addrlen);
            caList[pfd[gcount].fd] = ca;
            printf("new connect: %d - %d -%d\n", pfd[gcount].fd, caList[pfd[gcount].fd].sin_port, caList[pfd[gcount].fd].sin_addr);
            gcount++;
        }
        for(int i = 1;i < gcount; i++){
            if(pfd[i].revents & POLLRDHUP){
                printf("disconnect:%d\n", pfd[i].fd);
                deleteConnect(pfd[i].fd, pfd);
            }
            else if(pfd[i].revents & POLLIN){ //incoming data from client
                char buffer[100] = {0};
                int recv = recvData(pfd[i].fd, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                requestHandler(buffer, pfd[i].fd, &fileShareInfor, caList, pfd);
            }
        }
    }
    return 0;
}



struct sockaddr* setsocketaddr(char *host, int port){
    if(port==0){return NULL;}
    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);
    addr_in.sin_addr.s_addr = inet_addr(host);
    memset(&addr_in.sin_zero, 0, sizeof(addr_in.sin_zero));
    struct sockaddr* addr = (struct sockaddr *)&addr_in; 
    return addr;
}

int setSock(char *host, int port){
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0){
        perror("socket");
        exit(1);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        perror("setsockopt() error...\n");
        exit(1);
    }   
    struct sockaddr* addr = setsocketaddr(host, port);
    if(port == 0) return sockfd;
    if(bind(sockfd, addr, sizeof(struct sockaddr)) < 0 ){perror("bind() error...\n");exit(1);}
    return sockfd;
}


int deleteConnect(int fd, struct pollfd pfd[]){
    for(int i = 1; i < gcount; i++){
        if(pfd[i].fd == fd){
            pfd[i].revents = 0;
            for(int j = i;j<gcount-1;j++){
                pfd[j] = pfd[j+1];
            }
            gcount--;
            break;
        }
    }
}

int recvData(int fd, char *buffer, int maxlenBuff){
    int recvCount = 0;
    int maxSizeRead = 1024;
    while (1){
        int size_read = recv(fd, buffer+recvCount, maxSizeRead, 0);
        recvCount += size_read;
        if(size_read <= 0 || recvCount >= maxlenBuff || size_read < maxSizeRead){
            break;
        }
    }
    return recvCount;
}

int sendData(int fd, char *buffer, int lenBuff){
    int sent = 0;
    while(1){
        int size_write = send(fd, buffer+sent, (lenBuff-sent), 0);
        if(size_write <= 0)
            break;
        sent += size_write;
    }
    return sent;
}