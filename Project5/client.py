import sys
import socket
import json
import os
import stomp
import time

class MyListener(stomp.ConnectionListener):
    def on_error(self, headers, message):
        print('received an error "%s"' % message)
    def on_message(self, headers, message):
        print(message)

def ConnectActiveMQ():
    conn = stomp.Connection([('54.221.15.231', 61613)])  #modify 'localhost' to '54.221.15.231' ????
    conn.set_listener('', MyListener())
    conn.start()
    conn.connect('admin', 'admin', wait=True)
    return conn

class Client(object):
    def __init__(self, ip, port):
        try:
            socket.inet_aton(ip)
            if 0 < int(port) < 65535:
                self.ip = ip
                self.port = int(port)
            else:
                raise Exception('Port value should between 1~65535')
            self.cookie = {}
            self.subscribed = {}
            
            self.login_ip = ip
            self.login_port = port
            self.app_ip = {}
            self.app_port = {}

        except Exception as e:
            print(e, file=sys.stderr)
            sys.exit(1)

    def run(self):
        conn = ConnectActiveMQ()
        while True:
            cmd = sys.stdin.readline()
            if cmd.rstrip() == 'exit':
                return
            if cmd != os.linesep:
                try:
                    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                       
                        cmd = self.__attach_token(cmd)
                        self.__connect_destination(cmd)

                        s.connect((self.ip, self.port))    # should modify this
                        
                        s.send(cmd.encode())
                        resp = s.recv(4096).decode()
                        self.__show_result(json.loads(resp), cmd, conn)
                except Exception as e:
                    print("error")
                    print(e)
                    print(e, file=sys.stderr)



    def __connect_destination(self, cmd=None):
        if cmd:
            command = cmd.split()
            if len(command) > 1:
                
                if command[0] == 'logout' or command[0] == 'delete' or command[0] == 'login' or command[0] == 'register':
                    self.ip = self.login_ip
                    self.login_port = self.login_port
                else:
                    for owner,token in self.cookie.items():
                        if token ==  command[1]:            # command[1] is a token!!!
                            user = owner                    # user is a username        

                    if self.app_ip[user] and self.app_port[user]:
                        self.ip = self.app_ip[user]
                        self.port = self.app_port[user]
                    # deal with unknown command or no allocated app server
                    else:
                        self.ip = self.login_ip
                        self.port = self.login_port

        else:
            return cmd

    def __attach_token(self, cmd=None):
        if cmd:
            command = cmd.split()
            if len(command) > 1:
                if command[0] != 'register' and command[0] != 'login':
                    if command[1] in self.cookie:
                        command[1] = self.cookie[command[1]]
                    else:
                        command.pop(1)
            return ' '.join(command)
        else:
            return cmd

    def __show_result(self, resp, cmd=None, conn=None):
        if 'message' in resp:
            print(resp['message'])

        if 'invite' in resp:
            if len(resp['invite']) > 0:
                for l in resp['invite']:
                    print(l)
            else:
                print('No invitations')

        if 'friend' in resp:
            if len(resp['friend']) > 0:
                for l in resp['friend']:
                    print(l)
            else:
                print('No friends')

        if 'post' in resp:
            if len(resp['post']) > 0:
                for p in resp['post']:
                    print('{}: {}'.format(p['id'], p['message']))
            else:
                print('No posts')
        
        if 'groups' in resp:
            if len(resp['groups']) > 0:
                for g in resp['groups']:
                    print(g)
            else:
                print('No groups')

        if 'joined' in resp:
            if len(resp['joined']) > 0:
                for j in resp['joined']:
                    print(j)
            else:
                print('No groups')

        # print(resp)

        if cmd:
            command = cmd.split()
            if resp['status'] == 0 and command[0] == 'login':   # login successful
                user = command[1]
                self.cookie[user] = resp['token']
                
                ########### CONNECT to ACTIVEMQ server and SUBSCRIBE #############          
                # Subscribe to the joined groups(topics)
                if 'subscribed' in resp:
                    self.subscribed[user] = list()
                    self.subscribed[user] = resp['subscribed']

                    for topic in resp['subscribed']:
                        path = '/topic/' + topic
                        conn.subscribe(destination=path, id=resp['token']+topic, ack='auto')
                
                # Subscribe to personal channel(queue)
                conn.subscribe(destination='/queue/'+user, id=user, ack='auto')      

                ########### Save app server's ip address and port number ###############
                if 'ip' in resp:
                    app_server_ip = resp['ip']
                    self.app_ip[user]   = app_server_ip
                    self.app_port[user] = 2048

            
            ########### SUBSCRIBE ############# 
            if 'gotta_subscribe' in resp:
                for owner,token in self.cookie.items():
                    if token ==  command[1]:            # command[1] is a token!!!
                        user = owner
                topic = resp['gotta_subscribe']
                self.subscribed[user].append(topic)     

                conn.subscribe(destination='/topic/'+topic, id=command[1]+topic, ack='auto')      # should modify id       

            ########### UNSUBSCRIBE when logout or delete_user############# 
            if (resp['status'] == 0) and ((command[0] == 'logout') | (command[0] == 'delete')):
                
                for owner,token in self.cookie.items():
                    if token ==  command[1]:            
                        user = owner                

                for topic in self.subscribed[user]:
                    unsub = command[1]+topic
                    conn.unsubscribe(unsub)               # should modify id  
                conn.unsubscribe(user)                    # should modify id  
                self.subscribed[user].clear()

                ########### Discard app server's ip address and port number ###############
                self.app_ip[user]   = None
                self.app_port[user] = None



def launch_client(ip, port):
    c = Client(ip, port)
    c.run()

if __name__ == '__main__':
    if len(sys.argv) == 3:
        launch_client(sys.argv[1], sys.argv[2])
    else:
        print('Usage: python3 {} IP PORT'.format(sys.argv[0]))
