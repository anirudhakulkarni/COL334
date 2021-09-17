# COL334 Assignment -2

## Anirudha Kulkarni (2019CS50421)

### Design decisions:

1. At client - 2 sockets and 1 thread per socket - thread functions are different 
2. At server - 2 sockets and 1 thread per socket per client - thread functions are same to handle multiple requests
3. Username - at least size of 3 at most 10. Alphanumeric only. usernames are unique. No clashes.
4. Broadcasting - stop and wait - send individually - if all succeed then sender receives confirmation
5. Max buffer size - A single line message will take at most 400 characters and headers take 56 bytes. Hence total capped to 512 bytes. Ideal internet traffic suggested 512 bytes size.
6. No space in \n\n
7. Close program - Ctr+C

### Error Handling:

1. If the size of message is not equal to header - not possible but can be made manually - ERROR103
2. Another client takes same username - assume never arises
3. fragmentation and race conditions - assume small messages
4. If incorrect header/field missing etc - ERROR103 and close the connection
5. For every ERROR103 close the connection - both server and client act similarly
6. If A sent message to server and forward to B but B reports error then B should close A should not
7. If no destination client is registered send ERROR102  
8. If A sends to B but B replies invalid header then Server closes connection between server and receiver with ERROR103 but for sender it replies with ERROR102 and keeps connection alive



### Error codes:

1. ERROR100 - Malformed username
2. ERROR101 - No user registered
3. ERROR102 - Unable to send
4. ERROR103 - Header incomplete - closes connection

### References:

- Most comprehensive tutorial on socket programming : https://medium.com/iothincvit/socket-programming-f7f476df4bc5
- https://users.encs.concordia.ca/~glitho/F09_Socket.pdf

### Running instructions

1. Start server with ```python server.py```
2. Start client with ```python client.py```
3. Enter the IP of server when asked in ```client.py```
4. Enter username to register
5. Send messages with @recipient_username message
6. Send broadcast messages with @ALL message
7. 
