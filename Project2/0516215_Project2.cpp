#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<json-c/json.h>
#include<list>
using namespace std;

#define MAX_LINE 200



struct user_token{
    char    id[50];
    char    token[100];
};

list<struct user_token> TokenList;



void JsonParser(char *receive_msg, char UserID){
    json_object *msg_obj, *status;       // essential column
    json_object *message, *token, *post, *invite, *friends;  // optional column
    msg_obj = json_tokener_parse(receive_msg);

    //printf("new_obj.to_string()=\n%s\n", json_object_to_json_string(new_obj));

    status  = json_object_object_get(msg_obj, "status");
    message = json_object_object_get(msg_obj, "message");
    token   = json_object_object_get(msg_obj, "token");

    // UPDATE TOKENLIST:
    // if the json contains "token" column(i.e. it is a login command), 
    // then update the token list 
    if(token){
        char *token_string = json_object_get_string(token);
        struct user_token CurrentUser = {UserID, token_string};
        TokenList.push_back(CurrentUser);
    }


    // If parse fail, object is NULL
    if (!status) {
        printf("parse failed.");
        json_object_put(msg_obj);
        return;
    }

    if(message)
        printf("%s\n", json_object_get_string(message));
    else if(friends){
        // do something (print the vector)    
    }
    return;
}






//server IP:    140.113.207.51
//server port:  8008

int main(int argc, char const *argv[]){
    //list<struct user_token> TokenList;

    while(1){

        //FILE    *fPtr;
        char    InputBuffer[MAX_LINE];
        char    ReceiveMsg[500];
        char    *arg[MAX_LINE/2];
        char    *CommandType = NULL;
        char    *UserID = NULL;

        //read input from stdin
        memset(InputBuffer, '\0', sizeof(InputBuffer));
        read(STDIN_FILENO, InputBuffer, MAX_LINE);
        strtok(InputBuffer, "\n");
        
        /*slice the input command*/
		arg[0] = strtok(InputBuffer," ");
        char *tmp = arg[0];
		for(int i = 1; tmp != NULL ;i++){
			arg[i] = strtok(NULL, " ");
			tmp = arg[i];
		}

        CommandType = arg[0];
        UserID = arg[1];

        printf("Type: %s\n", CommandType);
        // CHECK COMMAND TYPE: 
        // if the command is not a Login nor a Register command, 
        // then replace the "id" argument into the user's "token" 
        if((strcmp(CommandType, "login") != 0) && (strcmp(CommandType, "register") != 0) ){
            bool foundToken = 0;
            list<struct user_token>::iterator it;
            for(it = TokenList.begin(); it != TokenList.end(); it++){
                // the user's id is in the token list (i.e. the user has already login)
                if(it->id == arg[1]){     
                    foundToken = 1;
                    arg[1] = it->token;
                    break;
                }    
            }
            
            // the user hasn't login, then replace his id with an empty string
            if(foundToken == 0)
                arg[1] = NULL;
            
            // do something else?
        }

        int BufferLen = strlen(InputBuffer);
        char SendMsg[BufferLen];
        memset(SendMsg, '\0', sizeof(SendMsg));
        memcpy(SendMsg, InputBuffer, BufferLen); // copy data from old buf to new one
/*
        //create a socket
        int sockfd;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1){
            printf("Fail to create a socket.\n");
        }

        //create connection to server
        struct sockaddr_in serv;
        memset(&serv, 0, sizeof(serv)); 
        serv.sin_family = PF_INET;
        serv.sin_addr.s_addr = inet_addr("140.113.207.51");
        serv.sin_port = htons(8008);

        int con_err = connect(sockfd, (struct sockaddr *)&serv, sizeof(serv));
        if(con_err == -1){
            printf("Connection fail\n");
        }
        
        // deal with<user> : replace <id> with token





        send(sockfd, SendMsg, sizeof(SendMsg), 0);
        recv(sockfd, ReceiveMsg, sizeof(ReceiveMsg), 0);
        close(sockfd);


        JsonParser(ReceiveMsg, Userid);

*/
    }
    return 0;
}
