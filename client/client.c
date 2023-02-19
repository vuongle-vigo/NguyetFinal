#define _GNU_SOURCE
#define TIMEOUT 500

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <errno.h>
# include <sys/poll.h>
# include <limits.h>
# include <sys/stat.h>
# include <time.h>
# include <stdint.h>
#include <sys/stat.h> 
#include <stdbool.h> 

# define HOST_NAME "127.0.0.1"

struct sockaddr* setsocketaddr(char *host, int port);
int setSock(char *host, int port);
int downloadFile(uint32_t ip, int port, char *filepath);
int recvData(int fd, char *buffer, int maxlenBuff);
int sendData(int fd, char *buffer, int lenBuff);
bool file_exists (char *filename);

int main(int argc, char* argv[]){
    if(argc < 4){
        fprintf(stderr, "Usage: %s <host> <port> <port_share_file>\n", argv[0]);
        exit(1);
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0){
        perror("socket");
        exit(1);
    }
    int sockfd_listen = setSock(HOST_NAME, atoi(argv[3]));
    listen(sockfd_listen, 1);
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(argv[1]);
    sa.sin_port = htons(atoi(argv[2]));
    int cfd = connect(sockfd, (struct sockaddr *)&sa, sizeof(sa));
    if(cfd<0){
        perror("connect");
        exit(1);
    }
    struct pollfd fds[4];
    fds[0].fd = 0;    
    fds[0].events = POLLIN;       
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;
    fds[2].fd = sockfd_listen;
    fds[2].events = POLLIN | POLLPRI;
    while(1){
        loop:
        int ret = poll(fds, 4, TIMEOUT*1000);
        if(-1 == ret){
            perror("poll");
            return -1;
        }
        if(ret == 0){
            printf("poll() timed out: %d\n", TIMEOUT);
            return 0;
        }
        if(fds[1].revents & POLLIN){
            char buf[1024] = {0};
            int recv_count = recvData(fds[1].fd, buf, sizeof(buf));
            if(strncmp(buf, "$$", 2)==0){
                char filepath[1024] = {0};
                uint32_t ip;
                char ipstr[10] = {0};
                int port;
                char portstr[10] = {0};
                char *p = strtok(buf+2, " ");
                strcpy(filepath, p);
                p = strtok(NULL, " ");
                strcpy(ipstr, p);
                ip = atoi(ipstr);
                p = strtok(NULL, " ");
                strcpy(portstr, p);
                port = atoi(portstr);
                printf("connecting: %s - %u - %d\n", buf, ip, port);
                downloadFile(ip, port, filepath);
            }
            else{
                printf("%s\n", buf);
            }
        }
        if(fds[0].revents & POLLIN){
            char buf[1024] = {0};
            gets(buf);
            if(strncmp(buf, "uploadfile", strlen("uploadfile"))==0){
                char filepath[PATH_MAX];
                char *filename = buf + strlen("uploadfile") + 1;
                char *res = realpath(filename, filepath);
                if(res==NULL){
                    printf("Filepath not found: %s\n", filename);
                    goto loop;
                }
                else{
                    char tmp[100];
                    char name[100];
                    strcpy(tmp, filepath);
                    char *p = strtok(tmp, "/");
                    while(p!= NULL){
                        p = strtok(NULL, "/");
                        if(p!=NULL){
                            strcpy(name, p);
                        }
                    }
                    filepath[strlen(filepath)-strlen(name)] = '\0';
                    strcpy(buf, "uploadfile ");
                    strcat(buf, filepath);
                    strcat(buf, " ");
                    strcat(buf, name);
                    char portStr[10];
                    strcpy(portStr, argv[3]);
                    int len = strlen(portStr);
                    for(int i = 0; i < 8 - len; i++){ 
                        char tmp[8] = {0};
                        strcat(tmp, " ");
                        strcat(tmp, portStr);
                        strcpy(portStr, tmp );
                    }        
                    printf("uploading: %s\n", buf+strlen("uploadfile "));                       
                    strcat(buf, portStr);
                }
            }
            int sent = sendData(fds[1].fd, buf, strlen(buf)+1);
        }
        if(fds[2].revents & POLLIN){
            struct sockaddr_in ca =  {0};
            int addrlen = sizeof(ca);
            fds[3].fd = accept(sockfd_listen, (struct sockaddr*)&ca, &addrlen);
            fds[3].events = POLLIN;
            printf("incoming connection: %d\n", ca.sin_port);
        }
        if(fds[3].fd != 0 && (fds[3].revents & POLLIN)){
            char buf[1024] = {0};
            int recv = recvData(fds[3].fd, buf, sizeof(buf));
            printf("%s-", buf);
            FILE *fp = fopen(buf, "rb");
            if(fp==NULL){
                char mss[] = "Can't find file to send file";
                sendData(fds[3].fd, mss, strlen(mss));
                goto done;
            }
            int count = 0;
            char ch;
            while(!feof(fp)){
                int read = fread(&ch, 1, 1, fp);
                int sent = send(fds[3].fd, &ch, 1, 0);
            }
            done:
            fds[3].fd = 0;
            fclose(fp);
        }
    }
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


int downloadFile(uint32_t ip, int port, char *filepath){
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0){
        perror("socket download() error...\n");
        exit(1);
    }
    struct sockaddr_in addr_download;
    addr_download.sin_family = AF_INET;
    addr_download.sin_addr.s_addr = ip;
    addr_download.sin_port = htons(port);
    int cfd = connect(sockfd, (struct sockaddr*)&addr_download, sizeof(struct sockaddr));
    if(cfd < 0){
        perror("can't connect to download...\n");
        close(sockfd);
        return -1;
    }
    char filepathtmp[100];
    char myfilename[100];
    strcpy(filepathtmp, filepath);
    char *p = strtok(filepathtmp, "/");
    strcpy(myfilename, p);
    while(p!=NULL){
        p = strtok(NULL, "/");
        if(p!=NULL){
            strcpy(myfilename, p);
        }
    }
    char tmp[100];
    strcpy(tmp, myfilename);
    int i = 1;
    while(file_exists(tmp)){
        strcpy(tmp, myfilename);
        char *p = strstr(tmp, ".");
        char tail[10];
        strcpy(tail, p);
        *p = '\0';
        char istr[10];
        sprintf(istr, "%d", i);
        strcat(tmp, istr);
        strcat(tmp, tail);
        i++;
    }
    strcpy(myfilename, tmp);
    FILE *fp = fopen(myfilename, "wb");
    if(!fp){
        perror("fopen() error...\n");
        fclose(fp);
        return -1;
    }
    sleep(1);
    sendData(sockfd, filepath, strlen(filepath));
    struct pollfd fds;
    fds.fd = sockfd;
    fds.events = POLLIN | POLLRDHUP;
    while(1){
        int ret = poll(&fds, 1, 1000);
        if(ret == -1){
            perror("poll() error...\n");
            break;
        }
        if(ret == 0){
            printf("Download complete\n");
            break;
        }
        if(fds.revents & POLLIN){
            char ch;
            recv(sockfd, &ch, 1, 0);
            fputc(ch, fp);
        }
        if(fds.revents & POLLRDHUP){
            printf("Connection download closed...\n");
        }
    }
    fclose(fp);
    close(sockfd);
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

bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}