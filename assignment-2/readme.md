# COL334 Assignment -2

## Anirudha Kulkarni (2019CS50421)

### Design decisions:

1. At client - 2 sockets and 1 thread per socket - thread functions are different 
2. At server - 2 sockets and 1 thread per socket per client - thread functions are same to handle multiple requests
3. Username - at least size of 3 at most 10. Alphanumeric only. If a client disconnects he can rejoin and take the same username again - (what if someone else registers by same username?)
4. Broadcasting - stop and wait - send individually - if all succeed then sender receives confirmation
5. Max buffer size - ?
6. No space in \n\n
7. Close program - Ctr+C

### Error Handling:

1. If the size of message is not equal to header - not possible but can be made manually - ERROR103
2. Another client takes same username - assume never arises
3. fragmentation and race conditions - assume small messages
4. If incorrect header/field missing etc - ERROR103 and close the connection
5. For every ERROR103 close the connection - both server and client act similarly
6. If A sent message to server and forward to B but B reports error then B should close A should not
7. (?) If no destination client is registered send ERROR102  
8. If A sends to B but B replies invalid header then Server closes connection between server and receiver with ERROR103 but for sender it replies with ERROR102 and keeps connection alive



### Error codes:

1. ERROR100 - Malformed username
2. ERROR101 - No user registered
3. ERROR102 - Unable to send
4. ERROR103 - Header incomplete - closes connection

- users to send plain text messages
- single server and multiple clients
- tcp sockets
- 

TODO:
- server - registration
- client - registration
- client - send message
- server - forward message

For other errors such as ERROR 103, we mentioned in the assignment statement that we need to close the socket connection.

case sensitive usernames a-z,A-Z,0-9

 we would need to use some thread safe hash table implementation and also maintain a per socket lock so that two threads cannot send messages to the same socket at the same time

 you can choose a max-size and add it to your report.

 https://piazza.com/class/ksadh5klx166sx?cid=39_f19

 Most comprehensive tutorial on socket programming : https://medium.com/iothincvit/socket-programming-f7f476df4bc5
 https://users.encs.concordia.ca/~glitho/F09_Socket.pdf
