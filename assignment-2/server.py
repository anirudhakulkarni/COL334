import socket
# import thread module
from _thread import *
import threading
printlock=threading.Lock()

def threaded(c):
    while True:
  
        # data received from client
        data = c.recv(1024)
        if not data:
            print('Bye')
              
            # lock released on exit
            printlock.release()
            break
  
        # reverse the given string from client
        data = data[::-1]
  
        # send back reversed string to client
        c.send(data)
  
    # connection closed
    c.close()

# main
def main():
    # server socket
    server=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    # get local machine name
    host = ""
    port = 9999
    # https://www.geeksforgeeks.org/multithreading-in-python-set-2-synchronization/
    
    # bind to the port
    server.bind((host, port))

    # queue up to 5 requests
    server.listen(5)
    print("Listening")
    # create thread for each client
    thread = []
    while True:
        # establish a connection
        client, addr = server.accept()
        print("Got a connection from: ",addr)
        # send a message to the client
        printlock.aquire()

        client.send(("Hello, welcome to the server!").encode('utf-8'))
        
        start_new_thread(threaded,(client,))
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