import time, socket, sys, threading

def register_to_send(sock, username):
    data='REGISTER TOSEND '+username+'\n\n'
    sock.send(data.encode())

def well_formed_username(username):
    if ' ' in username:
        return False
    else:
        return True

#  Client to server registration, for receiving messages
def register_to_receive(sock, username):
    data='REGISTER TORECV '+username+'\n\n'
    sock.send(data.encode())

def get_ack(response):
    print(response)
    end=response.find('\n\n')
    if response[0:9]=='ERROR 100':
        return "error", ""
    if response[0:10] == 'REGISTERED':
        return "success", response[18:end]
    else:
        return "error",""
def get_ack_sent(response):
    if response[0:9]=='ERROR 102':
        return "unable to send"
    if response[0:9] == 'ERROR 103':
        return "header incomplete"
    if response[0:4] == 'SEND':
        return "sent"
def get_rec_msg(in_line):
    space=in_line.find(' ')
    if in_line[0]!='@' or space==-1 or space==len(in_line)-1 or in_line[space+1]=='\n':
        return False,"",""
    recepient = in_line[1:space]
    message = in_line[space+1:]
    return True,recepient, message

# Client to server message
def send_message(sock, username,message):
    data="SEND "+username+"\n"+"Content-length: "+str(len(message))+"\n\n"+message
    print("sending data:",data)
    sock.send(data.encode())


def get_forwarded_message(sock ,response):
    print(response)
    if response[0:7]=='FORWARD':
        isformated,sender,message=is_formated(response)
        if isformated:
            # send ack
            send_received_ack(sock,sender)
            return "sent",message
        else:
            send_error103(sock)
            return "error", ""
    
    else:
        send_error103(sock)
        return "error", ""

def send_error103(sock):
    data='ERROR 103 Header Incomplete\n\n'
    sock.send(data.encode())
def send_received_ack(sock,username):
    data='RECEIVED '+username+'\n\n'
    sock.send(data.encode())
def is_formated(response):
    if response[0:7]=='FORWARD':
        space=response.find(' ')
        end=response.find('\n')
        if space==-1:
            return False,"",""
        sender=response[space:end]
        len_message=response[response.find('Content-length:')+16:response.find('\n\n')]
        if not len_message.isdigit():
            return False,"",""
        message=response[response.find('\n\n')+2:response.find('\n\n')+int(len_message)+2]
        return True, sender, message
    else:
        return False,"",""

def send_message_thread(socket_server_send, username):
# 1.2 send message
    while(True):
        in_line = input('Enter your message: ')
        ok,recipient, message = get_rec_msg(in_line)
        if not ok:
            print("Message format is wrong. Please retype in correct format")
            continue
        send_message(socket_server_send, recipient, message)
        # receive acknowledgement from server
        sent=get_ack_sent(socket_server_send.recv(1024).decode())
        if sent != 'sent':
            print(sent)
            continue

def receive_message_thread(socket_server_receive, username):
    # 1.3 receive message
    while(True):
        response=socket_server_receive.recv(1024).decode()
        ack,msg=get_forwarded_message(socket_server_receive,response)
        if ack=="error":
            print("Error in receiving message")
            continue
        break


# socket creation
socket_server_send = socket.socket()
socket_server_receive = socket.socket()
server_host = socket.gethostname()
ip = socket.gethostbyname(server_host)
s_port = 8080
print('This is your IP address: ',ip)
# server_host = input('Enter friend\'s IP address:')
server_host = "192.168.137.1"
socket_server_send.connect((server_host, s_port))
socket_server_receive.connect((server_host, s_port))

while(True):
    username = input('Enter your Username: ')
    # register_username(username, socket_server_send, socket_server_receive)
    register_to_send(socket_server_send, username)
    register_to_receive(socket_server_receive, username)

    # receive acknowledgement from server
    ack_tosend,ack_username_tosend=get_ack(socket_server_send.recv(1024).decode())
    ack_torecv,ack_username_toreceive=get_ack(socket_server_receive.recv(1024).decode())
    print(ack_tosend,ack_username_tosend,00)
    print(ack_torecv,ack_username_toreceive,0)
    if ack_tosend!= "success" or ack_torecv!= "success" or ack_username_tosend!=username or ack_username_toreceive!=username:
        print("Error in registration")
        continue
    break
print("Registration Successful")
# create thread for sending and receiving messages
thread_send = threading.Thread(target=send_message_thread, args=(socket_server_send, username,))
thread_receive = threading.Thread(target=receive_message_thread, args=(socket_server_receive, username,))
thread_send.start()
thread_receive.start()

