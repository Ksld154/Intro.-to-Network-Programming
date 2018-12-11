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
        print('received a message "%s"' % message)


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
        except Exception as e:
            print(e, file=sys.stderr)
            sys.exit(1)

    def run(self):
        while True:
            cmd = sys.stdin.readline()
            if cmd.rstrip() == 'exit':
                return
            if cmd != os.linesep:
                try:
                    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                        s.connect((self.ip, self.port))
                        cmd = self.__attach_token(cmd)
                        s.send(cmd.encode())
                        resp = s.recv(4096).decode()
                        self.__show_result(json.loads(resp), cmd)
                except Exception as e:
                    print("a")
                    print(e)
                    print(e, file=sys.stderr)

    def __show_result(self, resp, cmd=None):
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

        if cmd:
            command = cmd.split()
            if resp['status'] == 0 and command[0] == 'login':   # login successful
                user = command[1]
                self.cookie[user] = resp['token']
                
                ########### CONNECT to ACTIVEMQ server and SUBSCRIBE #############
                conn = stomp.Connection([('localhost', 61613)])
                conn.set_listener('', stomp.listener.PrintingListener())
                conn.start()
                conn.connect('admin', 'admin', wait=True)
                
                # Subscribe to the joined groups(topics)
                if 'subscribed_topics' in resp and len(resp['subscribed_topics']) > 0:
                    self.subscribed[user] = resp['subscribed_topics']
                    for topic in resp['subscribed_topics']:
                        path = '/topic/' + topic
                        print(topic)
                        conn.subscribe(destination=path, id=topic, ack='auto')

                # Subscribe to personal channel(queue)
                conn.subscribe(destination='/queue/'+user, id=user, ack='auto')                
                
                


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


def launch_client(ip, port):
    c = Client(ip, port)
    c.run()

if __name__ == '__main__':
    if len(sys.argv) == 3:
        launch_client(sys.argv[1], sys.argv[2])
    else:
        print('Usage: python3 {} IP PORT'.format(sys.argv[0]))
