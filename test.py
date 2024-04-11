import socket
from time import sleep


client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('127.0.0.1', 9434))
sleep(1)
client.send(b'_socket_controllisten,: ksp_2')
sleep(1)
#close
client.close()