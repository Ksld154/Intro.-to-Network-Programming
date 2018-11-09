import socket, sys


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 2048))

msg = []
msg = "Hello I am Client."
msg_byte = msg.encode('utf-8')

exit_msg = "exit"
exit_byte = exit_msg.encode('utf-8')

print(msg)

sock.send(msg_byte)
print(sock.recv(1024).decode('utf-8'))
sock.close()
