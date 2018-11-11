import sys
import socket
import json
import peewee as pw
import os
import hashlib
import time
#from peewee import	*


#def Connect_DB():
sqlite_db = pw.SqliteDatabase('0516215_Project3.db')

def json_generate(str):
    pass


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
            #data = [{'status':0, 'message': 'Success!'}]
            json_file = json.dumps({'status':0, 'message': 'Success!'})
            return json_file

        except pw.IntegrityError as e:
            if e.args[0] == "UNIQUE constraint failed: User.username":
                print("%s is already used" %account)

                json_file = json.dumps({'status':1, 'message': "%s is already used" %account})
                return json_file
    
    @classmethod
    def Login_User(self, account, password):
        try:
            #(1) do query
            query = User.select(User.id).where( (User.username == account) & (User.password == password) )
            if len(query) == 1:
                
                #print("%s %s" %(query[0].username, query[0].password))
                
                #(2) generate token (maybe use username + timestamp + hash function)
                token = hashlib.sha256((str(time.time())+account).encode('utf-8')).hexdigest()

                #(3) then update token list

                
                TokenList_3.create(user_id=query[0], token=token)
                
                
                
                
                json_file = json.dumps({'status':0, 'token':token, 'message': 'Success!'})
                return json_file

            elif len(query) == 0:
                print("No such user or password error")
                json_file = json.dumps({'status':1, 'message': "No such user or password error"})               
                return json_file

        except pw.IntegrityError as e:
            print(e)
    
    @classmethod
    def Delete_User(self, token):
        # do it later
        # need to do cascade delete, so wait for me to design other tables
        pass


class TokenList_3(SQLiteModel):
    #username = pw.CharField(unique=True)
    user_id = pw.IntegerField()
    token = pw.CharField()

    @classmethod
    def Logout_User(self, token):
        try:
            query = TokenList_3.delete().where(TokenList_3.token == token)
            query.execute()

            json_file = json.dumps({'status':0, 'message': 'Bye!'})
            return json_file

        except pw.IntegrityError as e:
            print(e)

    @classmethod
    def Search_Token(self, account):
        pass



class Invite_2(SQLiteModel):
    from_id = pw.IntegerField()
    to_id   = pw.IntegerField()
    
    @classmethod
    def InviteFriend(self, from_token, to_username):
        try:

            
            print(from_token)
            t1 = TokenList_3.select(TokenList_3.user_id)
            for itr in t1:
                #print(itr.user_id)
                print(itr.user_id)
                #print(itr.token == from_token)
                    

            q1 = TokenList_3.select(TokenList_3.user_id).where(TokenList_3.token == from_token)
            for itr in t1:
                #print(itr.user_id)
                print(itr.user_id)
            
            from_id = q1[0].user_id
            print(from_id) # query[0] is user id
            
            print(q1)
            print(len(q1))

            q2 = User.select(User.id).where(User.username == to_username)
            if q2.count() == 1:
                to_id = q2[0].id
                print(to_id)

                
                if from_id == to_id:
                    return json.dumps({'status':1, 'message': 'You cannot invite_2 yourself'})                

                q3 = Invite_2.select().where((Invite_2.from_id == from_id) & (Invite_2.to_id == to_id))
                #print(q3.count())
                if q3.count() != 0:
                    return json.dumps({'status':1, 'message': '​Already invite_2d'})

                q4 = Invite_2.select().where((Invite_2.from_id == to_id) & (Invite_2.to_id == from_id))
                if q4.count() != 0:
                    return json.dumps({'status':1, 'message': '​<id> has invite_2d you'})

                # check: ​“<id>​is already your friend”
                q5 = Friend.select().where((Friend.friendA_id == from_id) & (Friend.friendB_id == to_id))
                if q5.count() != 0:
                    return json.dumps({'status':1, 'message': '<id>​is already your friend'})

                
                # if no problem, then insert into invite_2 list
                Invite_2.create(from_id=from_id, to_id=to_id)
                json_file =  json.dumps({'status':0, 'message': 'Success!'})
                return json_file

            
            elif q2.count() == 0:
                json_file = json.dumps({'status':1, 'message': '​<id>​does not exist'})
                return json_file

        except pw.IntegrityError as e:
            print(e)


class Friend(SQLiteModel):
    friendA_id = pw.ForeignKeyField(User, on_update='CASCADE', on_delete='CASCADE')
    friendB_id = pw.ForeignKeyField(User, on_update='CASCADE', on_delete='CASCADE')
    pass






def CreateTable():
    User.create_table()
    TokenList_3.create_table()


def TCP_Connect(TCP_IP, TCP_Port):
    ServerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    ServerSocket.bind((TCP_IP, TCP_Port))
    ServerSocket.listen(1)
    return ServerSocket




def main():
    server = TCP_Connect(sys.argv[1], int(sys.argv[2]))
    User.create_table()
    TokenList_3.create_table()
    Invite_2.create_table()
    Friend.create_table()
    TokenList_3.delete().execute()

    while True:
        (cSock, addr) = server.accept()

        print(addr)

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

            if command[0] == "register":
                if len(command) == 3:
                    json_file = User.Insert_User(command[1], command[2])
                else:
                    json_file = json.dumps({'status':1, 'message': "Usage: register​ <id>​​ <password>​​"})
            elif command[0] == "login":
                if len(command) == 3:
                    json_file = User.Login_User(command[1], command[2])
                else:
                    json_file = json.dumps({'status':1, 'message': "Usage: login​ <id>​​ <password>​"})
            
            elif command[0] == "logout":
                if len(command) == 2:
                    json_file = TokenList_3.Logout_User(command[1])
                elif len(command) < 2:
                    json_file = json.dumps({'status':1, 'message': "Not login yet"})
                else:
                    json_file = json.dumps({'status':1, 'message': "Usage: logout​<user>"})
            
            
            
            
            # not finished!!!
            elif command[0] == "delete":
                if len(command) == 2:
                    pass
                    #json_file = User.Delete_User(command[1])
                elif len(command) < 2:
                    json_file = json.dumps({'status':1, 'message': "Not login yet"})
                else:
                    json_file = json.dumps({'status':1, 'message': "Usage: delete​<user>"})             


            elif command[0] == "invite":
                if len(command) == 3:
                    json_file = Invite_2.InviteFriend(command[1], command[2])
                elif len(command) == 2:
                    json_file = json.dumps({'status':1, 'message': "Not login yet"})
                else:
                    json_file = json.dumps({'status':1, 'message': "Usage: invite_2​<user>​​<id>"})

            print(json_file)
            msg_byte = json_file.encode('utf-8')
            cSock.send(msg_byte)
            
        cSock.close()
   

if __name__ == '__main__':
    #Connect_DB()
    main()    