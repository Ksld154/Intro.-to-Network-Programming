# pylint: disable=E1111
import sys
import socket
import time
from model import * # pylint: disable=W0614
import json
import uuid
import stomp
import boto3

class DBControl(object):
    def __auth(func):
        def validate_token(self, token=None, *args):
            if token:
                t = Token.get_or_none(Token.token == token)
                if t:
                    return func(self, t, *args)
            return {
                'status': 1,
                'message': 'Not login yet'
            }
        return validate_token

    def register(self, username=None, password=None, *args):
        if not username or not password or args:
            return {
                'status': 1,
                'message': 'Usage: register <username> <password>'
            }
        if User.get_or_none(User.username == username):
            return {
                'status': 1,
                'message': '{} is already used'.format(username)
            }
        res = User.create(username=username, password=password)
        if res:
            return {
                'status': 0,
                'message': 'Success!'
            }
        else:
            return {
                'status': 1,
                'message': 'Register failed due to unknown reason'
            }

    @__auth
    def delete(self, token, *args):
        if args:
            return {
                'status': 1,
                'message': 'Usage: delete <user>'
            }
        token.owner.delete_instance()
        return {
            'status': 0,
            'message': 'Success!'
        }

    def login(self, username=None, password=None, *args):
        if not username or not password or args:
            return {
                'status': 1,
                'message': 'Usage: login <id> <password>'
            }
        res = User.get_or_none((User.username == username) & (User.password == password))
        if res:
            t = Token.get_or_none(Token.owner == res)
            if not t:
                t = Token.create(token=str(uuid.uuid4()), owner=res)

            # search Subscribe table and return all subscribed topics by the user 
            subscribed_topics = Subscribe.select().where(Subscribe.user == t.owner)
            topics = []
            for s in subscribed_topics:
                topics.append(s.group.group_name)





            # also do app server ALLOCATION!!!!
            # STEP1 : check whether there are available app_servers (inuse < 10)
            available_server = None
            try:
                available_server = (ServerAllocation
                .select()
                .group_by(ServerAllocation.server)
                .having(fn.Count(ServerAllocation.server) < 10)
                .get()
                )

            except ServerAllocation.DoesNotExist:
                available_server = None

            # maybe try get_or_none????


            # STEP2: if YES, then return that app_server's ip and port
            if available_server:
                print(available_server.server.ip)
                target_ip = available_server.server.ip                
                print(target_ip)
                
                # target_ip = (AppServer
                # .select(AppServer.ip)
                # .where(AppServer == available_server)
                # )
                
                ServerAllocation.create(server=available_server, user=t.owner)


            # STEP3: if NO, then create a new EC2 instance, and return it's ip and port
            else:
                # create EC2 instance, sleep, get EC2 instance ip
                # create a row in Server table and ServerAllocation table

                client = boto3.client('ec2')

                user_data = '''#!/bin/bash
                cd /home/ubuntu
                echo 'test' > /home/ubuntu/commandline-output.txt
                python3.6 ./hello_world.py > /home/ubuntu/python.txt
                python3.6 ./server_app.py'''

                resp = client.run_instances(
                    ImageId='ami-0b95686768d9b1890',      # my AMI(with trying sys.path.append to import)
                    InstanceType='t2.micro',
                    MinCount=1,
                    MaxCount=1,
                    SecurityGroupIds=['sg-0100b5a9d0143c2fd'],
                    KeyName='test1',
                    UserData=user_data
                )

                for instance in resp['Instances']:
                    created_instance_id = instance['InstanceId']
                print(created_instance_id)

                time.sleep(5)

                # Connect to EC2
                ec2 = boto3.client('ec2')
                target_ip = ec2.describe_instances(InstanceIds=[created_instance_id])['Reservations'][0]['Instances'][0]['PublicIpAddress']
                print(target_ip)



                # create a row in Server table and ServerAllocation table
                NewServer = AppServer.create(ip=target_ip, instance_id=created_instance_id)
                ServerAllocation.create(server=NewServer, user=t.owner)





            return {
                'status': 0,
                'token': t.token,
                'message': 'Success!',
                'subscribed': topics,
                'ip':target_ip
            }
        else:
            return {
                'status': 1,
                'message': 'No such user or password error'
            }

    @__auth
    def logout(self, token, *args):
        if args:
            return {
                'status': 1,
                'message': 'Usage: logout <user>'
            }
        token.delete_instance()
        return {
            'status': 0,
            'message': 'Bye!'
        }

    @__auth
    def invite(self, token, username=None, *args):
        if not username or args:
            return {
                'status': 1,
                'message': 'Usage: invite <user> <id>'
            }
        if username == token.owner.username:
            return {
                'status': 1,
                'message': 'You cannot invite yourself'
            }
        friend = User.get_or_none(User.username == username)
        if friend:
            res1 = Friend.get_or_none((Friend.user == token.owner) & (Friend.friend == friend))
            res2 = Friend.get_or_none((Friend.friend == token.owner) & (Friend.user == friend))
            if res1 or res2:
                return {
                    'status': 1,
                    'message': '{} is already your friend'.format(username)
                }
            else:
                invite1 = Invitation.get_or_none((Invitation.inviter == token.owner) & (Invitation.invitee == friend))
                invite2 = Invitation.get_or_none((Invitation.inviter == friend) & (Invitation.invitee == token.owner))
                if invite1:
                    return {
                        'status': 1,
                        'message': 'Already invited'
                    }
                elif invite2:
                    return {
                        'status': 1,
                        'message': '{} has invited you'.format(username)
                    }
                else:
                    Invitation.create(inviter=token.owner, invitee=friend)
                    return {
                        'status': 0,
                        'message': 'Success!'
                    }
        else:
            return {
                'status': 1,
                'message': '{} does not exist'.format(username)
            }
        pass

    @__auth
    def list_invite(self, token, *args):
        if args:
            return {
                'status': 1,
                'message': 'Usage: list-invite <user>'
            }
        res = Invitation.select().where(Invitation.invitee == token.owner)
        invite = []
        for r in res:
            invite.append(r.inviter.username)
        return {
            'status': 0,
            'invite': invite
        }

    @__auth
    def accept_invite(self, token, username=None, *args):
        if not username or args:
            return {
                'status': 1,
                'message': 'Usage: accept-invite <user> <id>'
            }
        inviter = User.get_or_none(User.username == username)
        invite = Invitation.get_or_none((Invitation.inviter == inviter) & (Invitation.invitee == token.owner))
        if invite:
            Friend.create(user=token.owner, friend=inviter)
            invite.delete_instance()
            return {
                'status': 0,
                'message': 'Success!'
            }
        else:
            return {
                'status': 1,
                'message': '{} did not invite you'.format(username)
            }
        pass

    @__auth
    def list_friend(self, token, *args):
        if args:
            return {
                'status': 1,
                'message': 'Usage: list-friend <user>'
            }
        friends = Friend.select().where((Friend.user == token.owner) | (Friend.friend == token.owner))
        res = []
        for f in friends:
            if f.user == token.owner:
                res.append(f.friend.username)
            else:
                res.append(f.user.username)
        return {
            'status': 0,
            'friend': res
        }

    @__auth
    def post(self, token, *args):
        if len(args) <= 0:
            return {
                'status': 1,
                'message': 'Usage: post <user> <message>'
            }
        Post.create(user=token.owner, message=' '.join(args))
        return {
            'status': 0,
            'message': 'Success!'
        }

    @__auth
    def receive_post(self, token, *args):
        if args:
            return {
                'status': 1,
                'message': 'Usage: receive-post <user>'
            }
        res = Post.select().where(Post.user != token.owner).join(Friend, on=((Post.user == Friend.user) | (Post.user == Friend.friend))).where((Friend.user == token.owner) | (Friend.friend == token.owner))
        post = []
        for r in res:
            post.append({
                'id': r.user.username,
                'message': r.message
            })
        return {
            'status': 0,
            'post': post
        }


    @__auth
    def send(self, token, username=None, *args):

        if not username or len(args) <= 0:
            return {
                'status': 1,
                'message': 'Usage: send <user> <friend> <message>'
            }
        
        message = ' '.join(args)
        receiver = User.get_or_none(User.username == username)       
        
        if receiver:
            res1 = Friend.get_or_none((Friend.user == token.owner) & (Friend.friend == receiver))
            res2 = Friend.get_or_none((Friend.user == receiver) & (Friend.friend == token.owner))
            if res1 or res2:

                online = Token.get_or_none(Token.owner == receiver)
                if online:

                    # connect to ActiveMQ
                    conn = stomp.Connection([('localhost', 61613)])
                    conn.start()
                    conn.connect('admin', 'admin', wait=True)

                    dest = '/queue/' + str(username)
                    # <<<USER_A->USER_B: HELLO WORLD>>>
                    msg_formatted = '<<<{USER_A}->{USER_B}: {msg}>>>'.format(USER_A=token.owner.username, USER_B=username, msg=message)

                    conn.send(body=msg_formatted, destination=dest)

                    conn.disconnect()
                    return {
                        'status': 0,
                        'message': 'Success!'  #activemq message
                    }
                else:
                    return{
                        'status': 1,
                        'message': '{} is not online'.format(username)
                    }
            
            else:
                return {
                    'status': 1,
                    'message': '{} is not your friend'.format(username)
                }

        else:
            return {
                'status': 1,
                'message': 'No such user exist'
            }
        pass

    @__auth
    def list_group(self, token, *args):
        if args:
            return {
                'status': 1,
                'message': 'Usage: list-group <user>'
            }
        groups = Group.select()
        res = []
        for g in groups:
            res.append(g.group_name)
        return {
            'status': 0,
            'groups': res
        }

    @__auth
    def list_joined(self, token, *args):
        if args:
            return {
                'status': 1,
                'message': 'Usage: list-joined <user>'
            }
        
        res = Subscribe.select().where(Subscribe.user == token.owner)
        joined_groups = []
        for r in res:
            joined_groups.append(r.group.group_name)
        
        return {
            'status': 0,
            'joined': joined_groups
        }

    @__auth
    def create_group(self, token, group=None, *args):
        if not group or args:
            return {
                'status': 1,
                'message': 'Usage: create-group <user> <group>'
            }            
        
        res = Group.get_or_none(Group.group_name == group)
        if res:
            return {
                'status': 1,
                'message': '{} already exist'.format(group)
            }
        else:
            
            # create the group in DB
            new_group = Group.create(group_name=group)
            Subscribe.create(user=token.owner, group=new_group)
            return {
                'status': 0,
                'message': 'Success!',
                'gotta_subscribe': group
            }

    @__auth
    def join_group(self, token, group=None, *args):
        if not group or args:
            return {
                'status': 1,
                'message': 'Usage: join-group <user> <group>'
            }
        
        gotta_join = Group.get_or_none(Group.group_name == group)
        if gotta_join:
            double_join = Subscribe.get_or_none((Subscribe.user == token.owner) & (Subscribe.group == gotta_join))
            
            if double_join:
                return {
                    'status': 1,
                    'message': 'Already a member of {}'.format(group)
                }                
            else:
                # join the group
                Subscribe.create(user=token.owner, group=gotta_join)
                return {
                    'status': 0,
                    'message': 'Success!',
                    'gotta_subscribe': group
                }
        else:
            return {
                'status': 1,
                'message': '{} does not exist'.format(group)
            }         

    @__auth
    def send_group(self, token, group=None, *args):
        if not group or len(args) <= 0:
            return {
                'status': 1,
                'message': 'Usage: send-group <user> <group> <message>'
            }
        
        message = ' '.join(args)
        receive_group = Group.get_or_none(Group.group_name == group)       
        
        if receive_group:
            inside_group = Subscribe.get_or_none((Subscribe.user == token.owner) & (Subscribe.group == receive_group))

            if inside_group:
                # connect to ActiveMQ
                conn = stomp.Connection([('localhost', 61613)])
                conn.start()
                conn.connect('admin', 'admin', wait=True)

                dest = '/topic/' + str(group)
                # <<<USER_A->GROUP<GROUP_A>: HELLO WORLD>>>
                msg_formatted = '<<<{USER_A}->GROUP<{GROUP_A}>: {msg}>>>'.format(USER_A=token.owner.username, GROUP_A=group, msg=message)
                conn.send(body=msg_formatted, destination=dest)
                conn.disconnect()
                return {
                    'status': 0,
                    'message': 'Success!'
                }

            else:
                return {
                    'status': 1,
                    'message': 'You are not the member of {}'.format(group)
                }
        else:
            return {
                'status': 1,
                'message': 'No such group exist'
            }

            

class Server(object):
    def __init__(self, ip, port):
        try:
            socket.inet_aton(ip)
            if 0 < int(port) < 65535:
                self.ip = ip
                self.port = int(port)
            else:
                raise Exception('Port value should between 1~65535')
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.db = DBControl()
        except Exception as e:
            print(e, file=sys.stderr)
            sys.exit(1)

    def run(self):
        self.sock.bind((self.ip, self.port))
        self.sock.listen(100)
        socket.setdefaulttimeout(0.1)
        # Token.delete().execute()
        while True:
            try:
                conn, addr = self.sock.accept()
                with conn:
                    cmd = conn.recv(4096).decode()
                    resp = self.__process_command(cmd)
                    conn.send(resp.encode())
            except Exception as e:
                print(e, file=sys.stderr)

    def __process_command(self, cmd):
        command = cmd.split()
        if len(command) > 0:
            command_exec = getattr(self.db, command[0].replace('-', '_'), None)
            if command_exec:
                return json.dumps(command_exec(*command[1:]))
        return self.__command_not_found(command[0])

    def __command_not_found(self, cmd):
        return json.dumps({
            'status': 1,
            'message': 'Unknown command {}'.format(cmd)
        })


def launch_server(ip, port):
    c = Server(ip, port)
    c.run()

if __name__ == '__main__':
    if sys.argv[1] and sys.argv[2]:
        launch_server(sys.argv[1], sys.argv[2])
