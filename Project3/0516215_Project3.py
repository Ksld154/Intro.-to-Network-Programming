import sys
import socket
import json
import peewee as pw
import os
import hashlib
import time
#from peewee import	*


#def Connect_DB():
sqlite_db = pw.SqliteDatabase('0516215_Project3.db', pragmas={'foreign_keys': 1})
#sqlite_db.pragma('foreign_keys', 1, permanent=True)


class SQLiteModel(pw.Model):
    """A base model that will use our SQLite database"""
    class Meta:
        database = sqlite_db

class User(SQLiteModel):
    id = pw.IntegerField(primary_key=True)
    username = pw.CharField(unique=True)
    password = pw.CharField()
    
    @classmethod
    def Insert_User(self, account, password):
        try:
            User.create(username=account, password=password)
            json_file = json.dumps({'status':0, 'message': 'Success!'}, indent=2)
            return json_file

        except pw.PeeweeException as e:
            if e.args[0] == "UNIQUE constraint failed: User.username":
                print(e)

                json_file = json.dumps({'status':1, 'message': "%s is already used" %account}, indent=2)
                return json_file
    
    @classmethod
    def Login_User(self, account, password):
        try:
            #(1) do query
            query = User.select(User.id).where( (User.username == account) & (User.password == password) )
            if query.count() == 1:
                #print(query[0].id)
                #print("%s %s" %(query[0].username, query[0].password))
                
                #(2) generate token (maybe use username + timestamp + hash function)
                #(3) then update token list
                q2 = TokenList.select(TokenList.token).where(TokenList.user_id == query[0].id)
                
                if q2.count() == 0:
                    token = hashlib.sha256((str(time.time())+account).encode('utf-8')).hexdigest()
                    TokenList.create(user_id=query[0], token=token)
                elif q2.count() == 1:
                    token = q2[0].token
                          
                json_file = json.dumps({'status':0, 'token':token, 'message': 'Success!'}, indent=2)
                return json_file

            elif query.count() == 0:
                print("No such user or password error")
                json_file = json.dumps({'status':1, 'message': "No such user or password error"}, indent=2)               
                return json_file

        except pw.PeeweeException as e:
            print(e)
    
    
    # def ListUser(self):
    #     try:
    #         query = User.select(User.username)
    #         tmp1 = list(query)
    #         for itr in tmp1:
    #             print(itr.username)


    #     except pw.IntegrityError as e:
    #         print(e)

    @classmethod
    def Delete_User(self, token, arg_num):
        try:
            if arg_num > 2:
                q1 = TokenList.select().where(TokenList.token == token)
                if q1.count() == 1: 
                    return json.dumps({'status':1, 'message': 'Usage: delete​<user>'}, indent=2)
                elif q1.count() == 0:
                    return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)            # not valid token
           

            q1 = TokenList.select(TokenList.user_id).where(TokenList.token == token) 
            if q1.count() == 1: 
                delete_id = q1[0].user_id
                            
                q2 = User.delete().where(User.id == delete_id).execute()
                return json.dumps({'status':0, 'message': 'Success!'}, indent=2)
                
            elif q1.count() == 0:
                return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)            # not valid token

        except pw.PeeweeException as e:
            print(e)





class TokenList(SQLiteModel):
    #username = pw.CharField(unique=True)
    #user_id = pw.IntegerField()
    user_id = pw.ForeignKeyField(User, on_update='CASCADE', on_delete='CASCADE')
    token = pw.CharField()

    @classmethod
    def Logout_User(self, token , arg_num):
        try:
            # if arg_num < 2:
            #     return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)
            if arg_num > 2:
                q1 = TokenList.select().where(TokenList.token == token)
                if q1.count() == 1: 
                    return json.dumps({'status':1, 'message': 'Usage: logout​<user>'}, indent=2)
                elif q1.count() == 0:
                    return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)            # not valid token
            
            #elif arg_num == 2:
            q1 = TokenList.select().where(TokenList.token == token)
            if q1.count() == 1: 
                TokenList.delete().where(TokenList.token == token).execute()
                return json.dumps({'status':0, 'message': 'Bye!'}, indent=2)
            elif q1.count() == 0:
                return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)            # not valid token
            

        except pw.PeeweeException as e:
            print(e)




class Invite(SQLiteModel):
    # from_id = pw.IntegerField()
    # to_id   = pw.IntegerField()
    from_id = pw.ForeignKeyField(User, on_update='CASCADE', on_delete='CASCADE')
    to_id   = pw.ForeignKeyField(User, on_update='CASCADE', on_delete='CASCADE')
    
    @classmethod
    def InviteFriend(self, from_token, to_username, arg_num):
        try:
            if arg_num != 3:  #arg_num == 2 | arg_num > 3
                q1 = TokenList.select().where(TokenList.token == from_token)
                if q1.count() == 1: 
                    return json.dumps({'status':1, 'message': 'Usage: invite​<user>​​<id>'}, indent=2)
                elif q1.count() == 0:
                    return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)  

            # arg_num == 3

            q1 = TokenList.select(TokenList.user_id).where(TokenList.token == from_token)
            if q1.count() == 1: 
                from_id = q1[0].user_id
            elif q1.count() == 0:
                return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)            # not valid token

            
            #print(from_id) # query[0] is user id
            #print(q1)
            #print(len(q1))

            # Handling when input arguments are one less than expected, might be: 
            # 1. Usage problem  
            # 2. Not login yet (from_token is 2nd user's username)
            # MIGHT CHANGE, cause client still replace logged out username with the token
            # i.e. client side didnt erase the token of the deleted user
            print(q1.count())
            print(to_username)

            if (q1.count() == 1) & (to_username == None):
                return json.dumps({'status':1, 'message': "Usage: invite​<user>​​<id>"}, indent=2)  # format problem
            elif to_username == None:
                return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)            # not valid token


            q2 = User.select(User.id).where(User.username == to_username)
            if q2.count() == 1:
                to_id = q2[0].id
                
                if from_id == to_id:
                    return json.dumps({'status':1, 'message': 'You cannot invite yourself'}, indent=2)                

                q3 = Invite.select().where((Invite.from_id == from_id) & (Invite.to_id == to_id))
                if q3.count() != 0:
                    return json.dumps({'status':1, 'message': 'Already invited'}, indent=2)

                q4 = Invite.select().where((Invite.from_id == to_id) & (Invite.to_id == from_id))
                if q4.count() != 0:
                    return json.dumps({'status':1, 'message': '​<id> has invited you'}, indent=2)

                q5 = Friend.select().where((Friend.friendA_id == from_id) & (Friend.friendB_id == to_id))
                if q5.count() != 0:
                    return json.dumps({'status':1, 'message': '<id>​is already your friend'}, indent=2)

                # if no problem, then insert into invite list
                Invite.create(from_id=from_id, to_id=to_id)
                json_file =  json.dumps({'status':0, 'message': 'Success!'}, indent=2)
                return json_file

            elif q2.count() == 0:
                json_file = json.dumps({'status':1, 'message': '​<id>​does not exist'}, indent=2)
                return json_file
        except pw.PeeweeException as e:
            print(e)

    @classmethod    
    def ListInvite(self, token, arg_num):
        try:

            if arg_num > 2:
                q1 = TokenList.select().where(TokenList.token == token)
                if q1.count() == 1: 
                    return json.dumps({'status':1, 'message': 'Usage: list-invite​<user>'}, indent=2)
                elif q1.count() == 0:
                    return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)            # not valid token
            
            #elif arg_num == 2:
            q1 = TokenList.select(TokenList.user_id).where(TokenList.token == token)
            if q1.count() == 1: 
                target_id = q1[0].user_id
                
                # do list-invite
                #join???
                q2 = (User
                .select(User.username)
                .join(Invite, pw.JOIN.INNER, on=(User.id == Invite.from_id))
                .where(Invite.to_id == target_id))

                result_list = []
                for itr in q2:
                    print(itr.username)
                    result_list.append(itr.username)

                return json.dumps({'status':0, 'invite': result_list}, indent=2)
            elif q1.count() == 0:
                return json.dumps({'status':1, 'message': "Not login yet"}, indent=2)            # not valid token
            
        except Exception as e:
            print(e)

class Friend(SQLiteModel):
    friendA_id = pw.ForeignKeyField(User, on_update='CASCADE', on_delete='CASCADE')
    friendB_id = pw.ForeignKeyField(User, on_update='CASCADE', on_delete='CASCADE')
    pass






def CreateTable():
    User.create_table()
    TokenList.create_table()


def TCP_Connect(TCP_IP, TCP_Port):
    ServerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    ServerSocket.bind((TCP_IP, TCP_Port))
    ServerSocket.listen(1)
    return ServerSocket




def main():
    server = TCP_Connect(sys.argv[1], int(sys.argv[2]))
    User.create_table()
    TokenList.create_table()
    Invite.create_table()
    Friend.create_table()
    TokenList.delete().execute()

    while True:
        (cSock, addr) = server.accept()

        #print(addr)

        msg = cSock.recv(1024)
        ClientMsg = msg.decode('utf-8').rstrip()
        print(ClientMsg)

        if not msg:
            pass
        elif ClientMsg == "exit" + os.linesep:
            print("exit")
            cSock.shutdown(1)
            cSock.close()
            break
        elif ClientMsg == "closeserv":  #just for convenience
            print("Shutdown Server")
            os._exit(0) 
        else:
            command = ClientMsg.split()
            arg_num = len(command)

            if command[0] == "register":
                if len(command) == 3:
                    json_file = User.Insert_User(command[1], command[2])
                else:
                    json_file = json.dumps({'status':1, 'message': "Usage: register​ <id>​​ <password>​​"}, indent=2)
            elif command[0] == "login":
                if len(command) == 3:
                    json_file = User.Login_User(command[1], command[2])
                else:
                    json_file = json.dumps({'status':1, 'message': "Usage: login​ <id>​​ <password>​"}, indent=2)
            
            elif command[0] == "logout":

                if len(command) < 2:
                    json_file = json.dumps({'status':1, 'message': "Not login yet"}, indent=2)
                else:
                    json_file = TokenList.Logout_User(command[1], len(command))
                # else:
                #     #json_file = TokenList.Logout_User(command[1])
                #     json_file = json.dumps({'status':1, 'message': "Usage: logout​<user>"}, indent=2)
            
            
            
            # not finished!!!
            elif command[0] == "delete":
                if len(command) < 2:
                    json_file = json.dumps({'status':1, 'message': "Not login yet"}, indent=2)
                else:
                    json_file = User.Delete_User(command[1], len(command))
                    #json_file = json.dumps({'status':1, 'message': "Usage: delete​<user>"}, indent=2)             


            elif command[0] == "invite":
                if len(command) == 3:
                    json_file = Invite.InviteFriend(command[1], command[2], arg_num)
                elif len(command) == 2:
                    json_file = Invite.InviteFriend(command[1], None, arg_num)
                elif len(command) == 1:
                    json_file = json.dumps({'status':1, 'message': "Not login yet"}, indent=2)
                else:
                    json_file = Invite.InviteFriend(command[1], command[2], arg_num)
                    #0json_file = json.dumps({'status':1, 'message': "Usage: invite​<user>​​<id>"}, indent=2)
            
            elif command[0] == 'list-invite':
                if len(command) < 2:
                    json_file = json.dumps({'status':1, 'message': "Not login yet"}, indent=2)
                else:
                    json_file = Invite.ListInvite(command[1], len(command))
            
            # elif command[0] == 'listuser':
            #     User.ListUser()
            

            else:
                json_file = json.dumps({'status':1, 'message': "Invaild command"}, indent=2)
            print(json_file)
            msg_byte = json_file.encode('utf-8')
            cSock.send(msg_byte)
            
        cSock.close()
   

if __name__ == '__main__':
    #Connect_DB()
    main()    