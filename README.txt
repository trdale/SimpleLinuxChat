Tom Dale
Prog 1
CS 372

Compile:

gcc -o client chatclient.c
g++ -o server chatserv.cpp

Usage:

$ server [port num]
$ client [hostname] [port num]

The server takes a port number to listen to on, while the client will take the hostname of the server as well as
the port to connect to the server

A client will provide a handle to be distinguish themselves from the server

Either the client or the server can terminate the chat by typing "\quit"

Max message size is 500 Characters
Max handle size is 10 Characters

Messages must alternate between server and client

Testing:

Tested on OSU flip3 server using [hostname] as 127.0.0.1 and abatrary high ports not in use

Example Test:
./server 4123
./client 127.0.0.1 4123


Resources:

Code taken from Beej's Guide to Network Programming  http://beej.us/guide/bgnet/