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
            data = [{'status':0, 'message': 'Success!'}]
            json_file = json.dumps(data)
            #print(json_file)

        except pw.IntegrityError as e:
            if e.args[0] == "UNIQUE constraint failed: User.username":
                print("%s is already used" %account)

                data = [{'status':1, 'message': "%s is already used" %account}]
                json_file = json.dumps(data)
                #print(json_file)
    
    @classmethod
    def Login_User(self, account, password):
        try:
            #(1) do query
            query = User.select().where( (User.username == account) & (User.password == password) )
            if len(query) == 1:
                print("Login success!!\n\n")
                print("%s %s" %(query[0].username, query[0].password))
            elif len(query) == 0:
                print("No such user or password error")

            #q2 = User.select().where(User.username == account)
            #for itr in q2:
            #    print("%s %s" %(itr.username, itr.password))
            
            
            #(2) generate token (maybe use username + timestamp + hash function)
            token = hashlib.sha256((str(time.time())+account).encode('utf-8')).hexdigest()
            print(token)

            #(3) then update token list
            TokenList.create(username=account, token=token)

        except pw.IntegrityError as e:
            pass
            

class TokenList(SQLiteModel):
    username = pw.CharField(unique=True)
    token = pw.CharField()


    def Insert_Token(self, account, token):
        pass
    #def Search_Token(self, account):










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
                    print("try to insert user\n")
                    User.Insert_User(command[1], command[2])
                else:
                    print("Usage: register​ <id>​​ <password>​")
            elif command[0] == "login":
                if len(command) == 3:
                    #print("login user\n")
                    User.Login_User(command[1], command[2])
                else:
                    print("Usage: login​ <id>​​ <password>​")






            ServMsg = "Server received."
            print(ServMsg)
            msg_byte = ServMsg.encode('utf-8')
            cSock.send(msg_byte)
            
        cSock.close()
   

if __name__ == '__main__':
    #Connect_DB()
    main()    