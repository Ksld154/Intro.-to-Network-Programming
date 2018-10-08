#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<json-c/json.h>

#define MAX_LINE 200

//server IP:    140.113.207.51
//server port:  8008

int main(int argc, char const *argv[]){
    FILE *fPtr;
    char InputBuffer[MAX_LINE];
    char ReceiveMsg[500];
    
    memset(InputBuffer, '\0', sizeof(InputBuffer));
    read(STDIN_FILENO, InputBuffer, MAX_LINE);


    /*create a socket*/
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        printf("Fail to create a socket.\n");
    }

    /*create connection to server*/
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv)); 
    serv.sin_family = PF_INET;
    serv.sin_addr.s_addr = inet_addr("140.113.207.51");
    serv.sin_port = htons(8008);

    int con_err = connect(sockfd, (struct sockaddr *)&serv, sizeof(serv));
    if(con_err == -1){
        printf("Connection fail\n");
    }

    send(sockfd, InputBuffer, sizeof(InputBuffer), 0);
    recv(sockfd, ReceiveMsg,  sizeof(ReceiveMsg),  0);
    close(sockfd);
    //printf("%s\n", ReceiveMsg);

    json_object *new_obj, *status, *message;

    new_obj = json_tokener_parse(ReceiveMsg);
    //printf("new_obj.to_string()=\n%s\n", json_object_to_json_string(new_obj));


    status  = json_object_object_get(new_obj, "status");
    message = json_object_object_get(new_obj, "message");
    // If parse fail, object is NULL
    if (!status || !message) {
        printf("parse failed.");
        json_object_put(new_obj);
        exit(-1);
    }

    // Fetch value
    //printf("name=%s", json_object_get_string(name));
    printf("%s\n", json_object_get_string(message));



/*
    fPtr = fopen("recv.json", "w");
    if (!fPtr) {
        printf("Fail to create a json file.\n");
        exit(1);
    }
    fprintf(fPtr, "%s", ReceiveMsg);
    fclose(fPtr);
*/     


    return 0;
}
