# socket based chat application
import socket

# 2 sockets to send and receive messages
# 1 socket for the server to send messages to the client
# 1 socket for the client to send messages to the server

# client to server registration for sending messages
def register(sock, username):
    data='REGISTER TOSEND'+username+'\n \n'
    sock.send(data.encode())

# Server to client, if username is well formed
def well_formed_username(username):
    if ' ' in username:
        return False
    else:
        return True

#  Client to server registration, for receiving messages
def register_client(sock, username):
    data='REGISTER TORECV'+username+'\n \n'
    sock.send(data.encode())

# Server to client, if username is well formed


# Server to client message in response to any communication until registration is complete


# Client to server message
def send_message(sock, username,message):
    data="SEND "+username+"\n"+"Content-length: "+str(len(message))+"\n\n"+message
    sock.send(data.encode())

# server to client acknowledge
def ack(sock,status):
    if status=="DELEIVERED":
        data="DELEIVERED"
    elif status=="ERROR102":
        data="ERROR102 Unable to send\n\n"
    elif status=="ERROR103":
        data="ERROR103 Header incomplete\n\n"
    sock.send(data.encode())


# message forwarding at server
def forward_message(sock,username,message):
    data="FORWARD "+username+"\n"+"Content-length: "+str(len(message))+"\n\n"+message
    sock.send(data.encode())


# main
def main():
    # get username
    username = input("Enter your username: ")
    # get server ip
    host = input("Enter the server ip: ")

    # create socket
    send_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    recv_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # connect to server
    send_socket.connect((host, 9999))
    recv_socket.connect((host, 9999))
    # register client
    register_client(send_socket, username)
    

    # server to client registration
    while True:
        client, addr = server.accept()
        data = client.recv(1024)
        data = data.decode()
        if data[:4] == 'REGISTER':
            username = data[5:]
            if well_formed_username(username):
                register(client, username)
                register_client(client, username)
                print("Registered: "+username)
                break
            else:
                print("Error: Invalid username")
        else:
            print("Error: Invalid request")

    # client to server registration
    while True:
        client, addr = server.accept()
        data = client.recv(1024)
        data = data.decode()
        if data[:4] == 'REGISTER':
            username = data[5:]
            if well_formed_username(username):
                register_client(client, username)
                print("Registered: "+username)
                break
            else:
                print("Error: Invalid username")
        else:
            print("Error: Invalid request")

    # server to client registration
    while True:
        client, addr = server.accept()
        data = client.recv(1024)
        data = data.decode()
        if data[:4] == 'REGISTER':
            username = data[5:]
            if well_formed_username(username):
                register(client, username)
                print("Registered: "+username)
                break
            else:
                print("Error: Invalid username")
        else:
            print("Error: Invalid request")

    # server to client registration
    while True:
        client, addr = server.accept()
        data = client.recv(1024)
        data = data.decode()

main()