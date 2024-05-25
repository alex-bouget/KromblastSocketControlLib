import socket
from time import sleep


client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('127.0.0.1', 9434))
sleep(1)
# client.send(b'_socket_controllisten,: ksp_2')
client.send(b'_socket_controlexecute,: "WTF"\0')
client.send(b'_socket_controlexecute,: document.write("<h1>WTF</h1>")\0')
sleep(1)
# close
data = client.recv(1024)
print(data)
client.close()
