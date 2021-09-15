import time, socket, sys
from _thread import *
# global hash_table
from collections import defaultdict
hash_table = defaultdict(list)


def isvalid_username(username):
    if(len(username)>=3 and len(username)<=10):
        for i in username:
            if(i.isalpha() or i.isdigit()):
                continue
            else:
                return False
        return True
    return False

def register_user(first_response,conn):
    end=first_response.find("\n\n")
    print(first_response)
    if(len(first_response)<19 or end==-1):
        conn.sendall('ERROR 101 No user registered\n\n'.encode())
        return "ERROR101",""
    if(first_response[0:15]=="REGISTER TOSEND"):
        username=first_response[16:end]
        if (isvalid_username(username)):
            print(username,"THIS is username")
            hash_table[username]+=[conn]
            data="REGISTERED TOSEND "+username+"\n\n"
            conn.sendall(data.encode())
            return "REGISTER_TOSEND",username
        else:
            conn.sendall('ERROR 100 Malformed username\n\n'.encode())
            return "ERROR100",""
    elif(first_response[0:15]=="REGISTER TORECV"):
        username=first_response[16:end]
        if (isvalid_username(username)):
            hash_table[username]+=[conn]
            print(hash_table)
            data="REGISTERED TORECV "+username+"\n\n"
            conn.sendall(data.encode())
            return "REGISTER_TORECV",username
        else:
            conn.sendall('ERROR 100 Malformed username\n\n'.encode())
            return "ERROR100",""
    conn.sendall('ERROR 101 No user registered\n\n'.encode())
    return "ERROR101",""

def client_thread(conn, add):
    first_response = (conn.recv(1024)).decode() 
    op_code,username=register_user(first_response,conn)
    while True:
        msg_response = (conn.recv(1024)).decode()
        
    print(username + ' has connected.')


# create socket
new_socket = socket.socket()
host_name = socket.gethostname()
s_ip = socket.gethostbyname(host_name)
port = 8080
new_socket.bind((host_name, port))
print( 'Binding successful!')
print('This is your IP: ', s_ip)

thread_cnt = 0

# connection
while True:
    print('Waiting for connection...')
    new_socket.listen(10)               # Number of connections to queue
    conn, add = new_socket.accept()
    print("Connected to: ", add[0], ':', add[1])
    start_new_thread(client_thread, (conn, add))
    thread_cnt += 1
    print('Thread count: ', thread_cnt)
    print('Received connection from ', add[0])
    print('Connection Established. Connected From: ',add[0])
    
new_socket.close()

conn.send(name.encode())

# conversation
while True:
    message = input('Me : ')
    conn.send(message.encode())
    message = conn.recv(1024)
    message = message.decode()
    print(client, ':', message)