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
