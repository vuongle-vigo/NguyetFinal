
int idFile = 1;

typedef struct fileInfor{
    int id;
    char file_path[100];
    char file_name[50];
    uint32_t ip_address;
    int port_share;
    struct fileInfor *next;
}*FileInfor;
/// @brief 
/// @param file_path 
/// @param file_name 
/// @param ip_address 
/// @param port_share 
/// @return 
FileInfor file_init(char *file_path, char *file_name, uint32_t ip_address, int port_share);
FileInfor add_file(FileInfor head, FileInfor newFileInfor);
FileInfor delete_file(FileInfor head, int id);

FileInfor file_init(char *file_path, char *file_name, uint32_t ip_address, int port_share){
    FileInfor file_infor;
    file_infor = (FileInfor)malloc(sizeof(struct fileInfor));
    file_infor->id = idFile;
    idFile++;
    strcpy(file_infor->file_path, file_path);
    strcpy(file_infor->file_name, file_name);
    file_infor->ip_address = ip_address;
    file_infor->port_share = port_share;
    file_infor->next = NULL;
    return file_infor;
}

FileInfor add_file(FileInfor head, FileInfor newFileInfor){
    FileInfor temp;
    if(head == NULL){
        head = newFileInfor;
    }
    else{
        temp = head;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = newFileInfor;
    }
    return head;
}

FileInfor delete_head(FileInfor head){
    printf("delete_head\n");
    if(head==NULL){
        return head;
    }
    head = head->next;
    return head;
}

FileInfor delete_tail(FileInfor head){
    if(head==NULL || head->next==NULL){
        head = delete_head(head);
        return head;
    }
    FileInfor tmp = head;
    while(tmp->next->next!=NULL){
        tmp = tmp->next;
    }
    tmp->next = NULL;
    return head;
}

FileInfor delete_file(FileInfor head, int id){
    FileInfor temp;
    if(head->id == id){
        head = delete_head(head);
    }
    else{
        temp = head;
        if(temp->next == NULL) return head;
        while(temp->next->id != id && temp->next->next!=NULL){
            temp = temp->next;
        }
        if(temp->next->next == NULL && temp->next->id == id){
            return delete_tail(head);
        }
        else if(temp->next->id == id){
            temp->next = temp->next->next;
        }
    }
    return head;
}