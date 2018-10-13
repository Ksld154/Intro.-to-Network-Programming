#include<cstdio>
#include<cstdlib>
#include<iostream>
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
    char    *id;
    char    *token;
};

list<user_token> TokenList;

int TCPconnect(const char *HostIP, const char *port){
    
    //create a socket
    int     PortNum = atoi(port);
    int     sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        printf("Fail to create a socket.\n");
    }
            
    //setup connection to server
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv)); 
    serv.sin_family = PF_INET;
    serv.sin_addr.s_addr = inet_addr(HostIP);
    serv.sin_port = htons(PortNum);

    //create connection to server
    int con_err = connect(sockfd, (struct sockaddr *)&serv, sizeof(serv));
    if(con_err == -1){
        printf("Connection fail\n");
    }

    return sockfd;
}

void JsonParser(char *receive_msg, char *UserID, char *CommandType){
    json_object *msg_obj, *status;                            // essential column
    json_object *message, *token, *invite, *friends,  *post;  // optional column
    
    /* allocate memory for two pointer argument, (cuz pointers are passed by reference) */
    int     ID_len = strlen(UserID);
    int     com_len = strlen(CommandType);
    char    *user = new char[ID_len];
    char    *Command = new char[com_len];
    strcpy(user, UserID);
    strcpy(Command, CommandType);

    msg_obj = json_tokener_parse(receive_msg);                // parse the received JSON file

    //got json objects from JSON file 
    status  = json_object_object_get(msg_obj, "status");
    message = json_object_object_get(msg_obj, "message");
    token   = json_object_object_get(msg_obj, "token");
    invite  = json_object_object_get(msg_obj, "invite");
    friends = json_object_object_get(msg_obj, "friend");
    post    = json_object_object_get(msg_obj, "post");

    // If parse fail, object is NULL, then do nothing and return
    if (!status) {
        printf("Parse failed.");
        json_object_put(msg_obj);
        return;
    }

    // UPDATE TOKENLIST:
    // if the json contains "token" column(i.e. it is a login command), 
    // then update the token list 
    if(token){
        const char *token_string = json_object_get_string(token);
        struct user_token CurrentUser = {user, (char *)token_string};
        TokenList.push_back(CurrentUser);
    }
    

    // output the corresponding thing to stdout ("message", "invite", "friend", "post")
    if(message){
        printf("%s\n", json_object_get_string(message));

        /* 1. LOGOUT: Has to delete user from TokenList! Otherwise the next time when the user login,
         *            we might use the old token to replace the user's id, which would cause login error.
         * 2. DELETE USER: Also need to remove user from user list. Otherwise when the id is RE-Registered,
         *                 it will also cause the similiar token-duplicate problem. */
        const char *msg_text = json_object_get_string(message);
        if( (strcmp(msg_text, "Bye!") == 0) || (strcmp(msg_text, "Success!") == 0 && strcmp(Command, "delete") == 0) ){
            list<user_token>::iterator it;
            for(it = TokenList.begin(); it != TokenList.end(); it++){
                // if the user's id is in the token list (i.e. the user has already login), then remove his token.
                if(strcmp(it->id, UserID) == 0){     
                    TokenList.erase(it);
                    //printf("remove user's token\n");
                    break;
                }    
            }
        }
    }
    else if(invite){
        int array_len = json_object_array_length(invite);
        json_object *ArrayItem;
        // print out each item in invite array(seperate by a space)
        for(int i = 0; i < array_len; i++){
            ArrayItem = json_object_array_get_idx(invite, i);
            printf("%s\n", json_object_get_string(ArrayItem));
        }
        if(array_len == 0)
            printf("No invitations.\n");
    }
    else if(friends){  //similar as print invite
        int array_len = json_object_array_length(friends);
        json_object *ArrayItem;
        for(int i = 0; i < array_len; i++){
            ArrayItem = json_object_array_get_idx(friends, i);
            printf("%s\n", json_object_get_string(ArrayItem));
        }
        if(array_len == 0)
            printf("No friends. QQ\n");
    }
    else if(post){
        int array_len = json_object_array_length(post);
        json_object *ArrayItem, *id_tmp, *msg_tmp;
        // print out each item in post array(seperate by a space)
        for(int i = 0; i < array_len; i++){
            ArrayItem = json_object_array_get_idx(post, i);
            id_tmp    = json_object_object_get(ArrayItem, "id");
            msg_tmp   = json_object_object_get(ArrayItem, "message");
            printf("%s: %s\n", json_object_get_string(id_tmp), json_object_get_string(msg_tmp));
        }
        if(array_len == 0)
            printf("No posts.\n");
    }
    return;
}

//server IP:    140.113.207.51
//server port:  8008

int main(int argc, char const *argv[]){

    while(1){

        char    InputBuffer[MAX_LINE];        // input command 
        char    InputModified[MAX_LINE+100];  // the result that after doing (1)split, (2)replace id, (3)concate, to the input command
        char    ReceiveMsg[500];              // the received message(i.e. JSON file)
        char    *arg[MAX_LINE/2];             // the arguments in input command
        char    *CommandType = NULL;
        char    *UserID = NULL;
        memset(InputBuffer,   '\0', sizeof(InputBuffer));
        memset(InputModified, '\0', sizeof(InputModified));
        memset(arg,           '\0', sizeof(InputModified));
        
        //read input from stdin
        read(STDIN_FILENO, InputBuffer, MAX_LINE);
        strtok(InputBuffer, "\n");                          // remove the '\n' from stdin.

        //exit the program when the input command is "exit"
        if(strcmp(InputBuffer, "exit") == 0){  
            if(TokenList.size() != 0)
                TokenList.clear();
            break;
        } 

        // (1) SPLIT the input command
        int arg_cnt = 0;
        arg[0] = strtok(InputBuffer," ");
        while(arg[arg_cnt] != NULL){
            arg_cnt++;
            arg[arg_cnt] = strtok(NULL, " ");
        }
        arg_cnt--;     // cuz it should't increment at the last iteration


        // if there are more than 1 argument(i.e. arg_cnt > 0), 
        // then set the CommendType and UserID to the first two arguments, respectively.
        if(arg_cnt > 0){
            CommandType = arg[0];
            UserID = arg[1];
        }
        // if contains only one argument => MUST be a WRONG command, 
        // so doesn't need to replace token nor do concatenation, just pack it into InputModified.
        else{                              
            strcpy(InputModified, arg[0]);
            CommandType = (char *)"";
            UserID = (char *)"";
        }
        
        
        if(arg_cnt){  // the command contains more than one argument

            // (2) REPLECE ID with TOKEN: 
            //     if the command is not a Login nor a Register command, 
            //     then replace the "id" argument into the user's "token" 
            if((strcmp(CommandType, "login") != 0) && (strcmp(CommandType, "register") != 0)){
                bool foundToken = 0;
                list<user_token>::iterator it;
                for(it = TokenList.begin(); it != TokenList.end(); it++){
                    // if the user's id is in the token list (i.e. the user has already login), then replace it.
                    if(strcmp(it->id, arg[1]) == 0){     
                        foundToken = 1;
                        arg[1] = it->token;
                        break;
                    }    
                }
                // if the user hasn't login, then replace his id with an empty string
                if(foundToken == 0)
                    arg[1] = (char *)"\0";
            }

            // (3) CONCATE: 
            //     Concate all the modified input arguments (id argument is modified on non-login and non-register commands)
            for(int i = 0; i <= arg_cnt; i++){
                strcat(InputModified, arg[i]);
                if(i != arg_cnt)
                    strcat(InputModified, " ");
            }
        }
        
        // copy data from old buf to new one, in order to remove the trailing '\0's.
        int InputModified_len = strlen(InputModified);   // strlen() ignore the '\0's automatically
        char SendMsg[InputModified_len];                 // so SendMsg doesn't contain any '\0'
        memset(SendMsg, '\0', sizeof(SendMsg));
        strcpy(SendMsg, InputModified);


        /* Connect to the server */
        const char *IPAddr = argv[1];
        const char *host   = argv[2];
        int sockfd = TCPconnect(IPAddr, host);  

        //send and receive socket
        send(sockfd, SendMsg, sizeof(SendMsg), 0);
        recv(sockfd, ReceiveMsg, sizeof(ReceiveMsg), 0);
        close(sockfd);

        // Parse the received JSON file
        JsonParser(ReceiveMsg, UserID, CommandType);
    }
    return 0;
}
