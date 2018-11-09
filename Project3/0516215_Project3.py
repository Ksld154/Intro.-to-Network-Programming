import sys
import socket
import json


def TCP_Connect(TCP_IP, TCP_Port):
    ServerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    ServerSocket.bind((TCP_IP, TCP_Port))
    ServerSocket.listen(1)
    return ServerSocket


def main():
    server = TCP_Connect(sys.argv[1], int(sys.argv[2]))

    while True:
        (cSock, addr) = server.accept()
        print("Client info:  " + addr)

        msg = cSock.recv(1024)
        ClientMsg = msg.decode('utf-8')
        if not msg:
            pass
        elif ClientMsg == "exit":
            print("exit")
            cSock.shutdown(1)
            cSock.close()
            break 
        else:
            print("Client sent msg:  " + msg.decode('utf-8'))
            ServMsg = "Server received."
            print(ServMsg)
            msg_byte = ServMsg.encode('utf-8')
            cSock.send(msg_byte)
            
        cSock.close()    

if __name__ == '__main__':
    main()    