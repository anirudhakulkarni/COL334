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
# check headers corresponding to table 2
def check_msg_header(msg):
    rec=""
    msg_len=0
    msg_content=""
    if(len(msg)<20):
        return False,rec,msg_len,msg_content
    if(msg[0:4]!="SEND"):
        return False,rec,msg_len,msg_content
    back_pos=msg.find("\n")
    d_back_pos=msg.find("\n\n")
    cont_len_pos=msg.find("Content-length:")
    if( cont_len_pos<7 or back_pos<6 or d_back_pos<23):
        return False,rec,msg_len,msg_content
    rec=msg[5:back_pos]
    msg_len=msg[cont_len_pos+16:d_back_pos]
    msg_content=msg[d_back_pos+2:]
    if((not msg_len.isdigit() )or int(msg_len)!=len(msg_content)):
        print("message length is not given length. Given length is:",msg_len,len(msg_content))
        return False,rec,msg_len,msg_content
    return True,rec,int(msg_len),msg_content
def forward_to_recp(msg_response,conn,sender_username):
    check,receipient,msg_length,msg_content=check_msg_header(msg_response)
    if(not check):
        return "ERROR103"
    print("message received is formatted.")
    if(receipient not in hash_table):
        return "ERROR102"
    if(msg_length!=len(msg_content)):
        print("message length not equal to given length")
        return "ERROR103" 
    print("Forwarding the message")
    data="FORWARD "+sender_username+"\nContent-length: "+str(msg_length)+"\n\n"+msg_content
    receipient_conn_send=hash_table[receipient][1]
    receipient_conn_send.sendall(data.encode())
    print("Message Forwarded waiting for confirmation")
    receipient_conn_recv=hash_table[receipient][0]
    receipient_ack=receipient_conn_recv.recv(1024).decode()
    print("Confirmation received")
    if(receipient_ack[0:8]=="RECEIVED"):
        return "SEND"
    elif (receipient_ack[0:9]=="ERROR 103"):
        return "ERROR103"
    else:
        return "ERROR102"

def send_error102(conn):
    data="ERROR 102 Unable to send\n\n"
    conn.sendall(data.encode())
def send_error103(conn):
    data='ERROR 103 Header Incomplete\n\n'
    conn.send(data.encode())
def send_delivered(conn,username):
    data="SEND "+username+"\n\n"
    conn.sendall(data.encode())

def client_thread(conn, add):
    first_response = (conn.recv(1024)).decode() 
    op_code,username=register_user(first_response,conn)
    while True:
        msg_response = (conn.recv(1024)).decode()
        ack=forward_to_recp(msg_response,conn,username)
        if(ack=="ERROR102"):
            send_error102(conn)
        if(ack=="ERROR103"):
            send_error103(conn)
        if(ack=="SEND"):
            send_delivered(conn,username)
        print(ack)

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