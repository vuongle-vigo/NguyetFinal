const char* inforallfiles = "inforallfiles";
const char* myfiles = "myfiles";
const char* delfile = "delfile";
const char* uploadfile = "uploadfile";
const char* downloadfile = "downloadfile";

int recvData(int fd, char *buffer, int maxlenBuff);
int sendData(int fd, char *buffer, int lenBuff);
int requestHandler(char *request, int fd, FileInfor *fileShareInfor, struct sockaddr_in caList[], struct pollfd pfd[]);
int checkOnline(uint32_t ip_address, struct sockaddr_in caList[], struct pollfd fds[]); 
int requestHandler(char *request, int fd, FileInfor *fileShareInfor, struct sockaddr_in caList[], struct pollfd pfd[]){
    if(strcmp(request, inforallfiles) == 0){
        FileInfor tmp = *fileShareInfor;
        while(tmp!=NULL){
            if(checkOnline(tmp->ip_address, caList, pfd)){
                char buf[1024];
                char idstr[10];
                char ipstr[10];
                char portstr[10];
                sprintf(idstr, "%d", tmp->id);
                sprintf(ipstr, "%d", tmp->ip_address);
                sprintf(portstr, "%d", tmp->port_share);
                sprintf(buf, "%s %s %s %s %s\n", idstr, tmp->file_path, tmp->file_name, ipstr, portstr);
                sendData(fd, buf, strlen(buf));
            }  
            tmp = tmp->next;
        }
    }
    else if(strcmp(request, myfiles) == 0){
        FileInfor tmp = *fileShareInfor;
        while(tmp!=NULL){
            if(tmp->ip_address == caList[fd].sin_addr.s_addr){
                char buf[1024];
                char idstr[10];
                char ipstr[10];
                char portstr[10];
                sprintf(idstr, "%d", tmp->id);
                sprintf(ipstr, "%d", tmp->ip_address);
                sprintf(portstr, "%d", tmp->port_share);
                sprintf(buf, "%s %s %s %s %s\n", idstr, tmp->file_path, tmp->file_name, ipstr, portstr);
                sendData(fd, buf, strlen(buf));
            } 
            tmp = tmp->next;
        }
    }
    else if(strncmp(request, delfile, strlen(delfile))==0){
        char mss[] = "DELETE FILE COMPLETE";
        char mss2[] = "DELETE FILE FAIL";
        char *p = strtok(request, " ");
        char idstr[10];
        while(p!=NULL){
            p = strtok(NULL, " ");
            if(p!=NULL){
                strcpy(idstr, p);
            }
        }
        int id = atoi(idstr);
        FileInfor tmp = *fileShareInfor;
        int check = 0;
        while(tmp!=NULL){
            if(tmp->id == id && tmp->ip_address==caList[fd].sin_addr.s_addr){
                check = 1;
                break;
            }
            tmp = tmp->next;
        }
        if(check == 1){
            *fileShareInfor = delete_file(*fileShareInfor, id);
            if(write_data(*fileShareInfor)){
                sendData(fd, mss, strlen(mss));
            }
            else{
                sendData(fd, mss2, strlen(mss2));
            }
        }
        else{
            sendData(fd, "File does not belong to you, use *myfiles* command to check", strlen("File does not belong to you, use *myfiles* command to check"));
        }
        
    }
    else if(strncmp(request, uploadfile, strlen(uploadfile))==0){
        char portStr[8];
        strcpy(portStr, request + strlen(request) - 8); // //sizeof portStr = 8 bytes
        request[strlen(request) - 8] = '\0';
        char *p = strtok(request, " ");
        char file_name[50];
        char filepath[100];
        char tmp[150];
        p = strtok(NULL, " ");
        strcpy(filepath, p);
        p = strtok(NULL, " ");
        strcpy(file_name, p);
        FileInfor newFileInfor = file_init(filepath, file_name, caList[fd].sin_addr.s_addr, atoi(portStr));
        *fileShareInfor = add_file(*fileShareInfor, newFileInfor);
        sleep(1);
        if(write_data(*fileShareInfor)==1){
            sendData(fd, "Upload file success", strlen("Upload file success"));
        }
        else{
            sendData(fd, "Upload file failed", strlen("Upload file failed"));
        }
    }
    else if(strncmp(request, downloadfile, strlen(downloadfile))==0){
        char *p = strtok(request, " ");
        char idstr[10];
        while(p!=NULL){
            p = strtok(NULL, " ");
            if(p!=NULL){
                strcpy(idstr, p);
            }
        }  
        int id = atoi(idstr);
        FileInfor tmp = *fileShareInfor;
        while(tmp!=NULL){
            if(tmp->id==id){
                char buf[1024];
                char ipstr[10];
                char portstr[10];
                sprintf(idstr, "%d", tmp->id);
                sprintf(ipstr, "%d", tmp->ip_address);
                sprintf(portstr, "%d", tmp->port_share);
                sprintf(buf, "$$%s%s %s %s", tmp->file_path, tmp->file_name, ipstr, portstr);
                sendData(fd, buf, strlen(buf));
                break;
            }
            tmp = tmp->next;
        }
        if(tmp==NULL){
            char mss[] = "Don't find this file share";
            sendData(fd, mss, strlen(mss));
        }
    }
    else{
        char mss[] = "Invalid command";
        sendData(fd, mss, strlen(mss));
    }
}


int checkOnline(uint32_t ip_address, struct sockaddr_in caList[], struct pollfd fds[]){
    int i = 1;
    while(fds[i].fd != 0){
        if(caList[fds[i].fd].sin_addr.s_addr == ip_address){
            return 1;
        }
        i++;
    }
    return 0;
}