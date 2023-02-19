

const char* datafile = "data.txt";
FileInfor read_data(FileInfor file_infor);
int write_data(FileInfor file_infor);


FileInfor read_data(FileInfor file_infor){
    FILE *fp = fopen(datafile, "r");
    if(!fp){
        perror("Couldn't open database file to read\n");
        return NULL;
    }
    while(!feof(fp)){
        FileInfor tmp;
        int id;
        char file_path[100];
        char file_name[50];
        long ip_address;
        int port;
        fscanf(fp, "%d %s %s %u %d\n", &id, file_path, file_name, &ip_address, &port);
        tmp = file_init(file_path, file_name, ip_address, port);
        file_infor = add_file(file_infor, tmp);
    }
    fclose(fp);
    return file_infor;
}


int write_data(FileInfor file_infor){
    FileInfor temp;
    FILE *fp = fopen(datafile, "w");
    if(!fp){
        perror("Couldn't open database file to write\n");
        return 0;
    }
    temp = file_infor;
    while(temp!=NULL){
        fprintf(fp, "%d %s %s %u %d\n", temp->id, temp->file_path, temp->file_name, temp->ip_address, temp->port_share);
        temp = temp->next;
    }
    fclose(fp);
    return 1;
}