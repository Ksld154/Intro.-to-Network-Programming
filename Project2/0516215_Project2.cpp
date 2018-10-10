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
    char    *id;
    char    *token;
};

list<user_token> TokenList;


void JsonParser(char *receive_msg, char *UserID){
    json_object *msg_obj, *status;                            // essential column
    json_object *message, *token, *invite, *friends,  *post;  // optional column
    msg_obj = json_tokener_parse(receive_msg);
    
    int     len = strlen(UserID);
    char    *user = new char[len];
    strcpy(user, UserID);

    //printf("msg:\n%s\n", json_object_to_json_string(msg_obj));

    status  = json_object_object_get(msg_obj, "status");
    message = json_object_object_get(msg_obj, "message");
    token   = json_object_object_get(msg_obj, "token");
    invite  = json_object_object_get(msg_obj, "invite");
    friends = json_object_object_get(msg_obj, "friend");
    post    = json_object_object_get(msg_obj, "post");

    // If parse fail, object is NULL
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
    
    // output the corresponding thing to stdout
    if(message)
        printf("%s\n", json_object_get_string(message));
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
    //list<struct user_token> TokenList;

    while(1){

        char    InputBuffer[MAX_LINE];
        char    ReceiveMsg[500];
        char    *arg[MAX_LINE/2];
        char    *CommandType = NULL;
        char    *UserID = NULL;

        //read input from stdin
        memset(InputBuffer, '\0', sizeof(InputBuffer));
        read(STDIN_FILENO, InputBuffer, MAX_LINE);
        strtok(InputBuffer, "\n");

        if(strcmp(InputBuffer, "exit") == 0){
            //clear the TokenList???
            if(TokenList.size() != 0)
                TokenList.clear();
            break;
        } 


        // slice the input command
        int arg_cnt = 0;
		arg[0] = strtok(InputBuffer," ");
        char *tmp = arg[0];
		for(int i = 1; tmp != NULL ;i++){
			arg[i] = strtok(NULL, " ");
			tmp = arg[i];
            arg_cnt++;
		}
        CommandType = arg[0];
        UserID = arg[1];


        // CHECK COMMAND TYPE: 
        // if the command is not a Login nor a Register command, 
        // then replace the "id" argument into the user's "token" 
        if((strcmp(CommandType, "login") != 0) && (strcmp(CommandType, "register") != 0) ){
            bool foundToken = 0;

            list<user_token>::iterator it;
            for(it = TokenList.begin(); it != TokenList.end(); it++){
                // the user's id is in the token list (i.e. the user has already login)
                if(strcmp(it->id, arg[1]) == 0){     
                    foundToken = 1;
                    arg[1] = it->token;
                    break;
                }    
            }
            // the user hasn't login, then replace his id with an empty string
            if(foundToken == 0)
                arg[1] = NULL;
        }
        

        // store the modified input (id argument is modified on non-login and non-register commands)
        char InputModified[300];
        memset(InputModified, '\0', sizeof(InputModified));
        for(int i = 0; i < arg_cnt; i++){
            strcat(InputModified, arg[i]);
            if(i != arg_cnt-1)
                strcat(InputModified, " ");
        }

        // copy data from old buf to new one, in order to remove the trailing '\0's.
        int InputModified_len = strlen(InputModified);
        char SendMsg[InputModified_len];
        memset(SendMsg, '\0', sizeof(SendMsg));
        memcpy(SendMsg, InputModified, InputModified_len); 

        /*Create a TCPconnect function??*/
        
        //create a socket
        int sockfd;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1){
            printf("Fail to create a socket.\n");
        }
        
        //create connection to server
        char    IPAddr[20];
        int     Port;
        strcpy(IPAddr, argv[1]);
        Port = atoi(argv[2]);

        struct sockaddr_in serv;
        memset(&serv, 0, sizeof(serv)); 
        serv.sin_family = PF_INET;
        serv.sin_addr.s_addr = inet_addr(IPAddr);
        serv.sin_port = htons(Port);

        int con_err = connect(sockfd, (struct sockaddr *)&serv, sizeof(serv));
        if(con_err == -1){
            printf("Connection fail\n");
        }
        

        //send and receive socket
        send(sockfd, SendMsg, sizeof(SendMsg), 0);
        recv(sockfd, ReceiveMsg, sizeof(ReceiveMsg), 0);
        close(sockfd);

        JsonParser(ReceiveMsg, UserID);
        
    }
    return 0;
}
