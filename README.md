# lets-talk
Want to talk to your friends in a programmer's way on the terminal of your PC.</br>
_Welcome to **Lets Talk**_ </br>

## Project Description
Lets Talk is a multi-threaded chat server. It is written in c++ and uses the pthreads library for implementing the multithreading.
It uses socket programming for connecting server and client, both of which are written in c++. It is a terminal based 
application which helps in communtication between various clients which join the server.</br>
The idea behind this project is to implement parallel processing of data using threads.

## HOW TO RUN
To run the project the following requirements are to be fufilled:
1. Install a Linux distribution(preferably Ubuntu) -- How to install
2. Install G++ compiler for Linux(Ubuntu) -- How to install

In the terminal, go to the directory in which the server file is present</br>
```
-g++ -o server server.cpp -lpthread
./server
```
>This compiles and runs the server
</br>
Open another terminal and go to the directory where client file is present </br>

```
g++ -o client client.cpp -lpthread
./client
```

>This runs the client and hence the client-server system is up and running.
</br>
To add more clients run the client file in new terminal windows.

#### Server Log
To view the server log of the last server run or the server log of the current server run till now, run:

`echo -ne $(cat serverlog.txt | sed  's/$/\\n/' | sed 's/ /\\a /g')`

## Project Details

The project uses threads library, which is an OS dependent library(Linux). The server uses socket programming to
connect to the client. The server first binds itself, i.e the bind function binds the socket to the address 
and port number specified in addr.Then the listen mode is activated i.e It puts the server socket in a passive mode, where it waits for the 
client to approach the server to make a connection.

Both the server and clients follow TCP protocol and use IPV4 address format. 

In server.cpp, a socket for the server is created and then bind to an address inclusive of the port number. After this, the `listen()` method makes the server socket ready to accept client connections, with a backlog set to 10. After this, the server keeps on looking for client connections in an infinite while loop. 

The server.cpp file contains an array `client_list` which stores the username, address and socket field descriptor of the clients that are connected to the server at a time. As soon as a client socket connects with the server socket, its details, i.e username, address and socket field descriptor values are added to the first empty location in the `clients_list`. This addition is done using the method `add_to list()`. Also another method, `remove_from_list()` is present to remove a client from the `client_list` once the client has disconnected from the server. Both these methods modify a data structure that has to be shared between the various threads and hence contain mutex locks to ensure that only one thread is able to modify `clients_list` at a time. The maximum length of `clients_list` is set as 70 (the variable `MAX_CLIENTS`) and so, the the maximum clients that can chat on the server at a time are 70. Once a client has been added to the list, the server must communicate with it and for this purpose, the `handle_connection()` method has been made. As all clients must be handled parallely, a thread is assigned to each client and this thread runs each client's `handle_connection()` method, thus ensuring concurrent handling of each client by the server.

In each `handle_connection()` method that runs parallely, first, the username is received from the client. It is then checked for in the `clients_list` to make sure the username is unique. In case it is, the client is given permission to start sending and receiving messages from the server. The `handle_connection()` method then keeps on receiving instructions from the client. If it receives a message from the client, it calls the `broadcast()` method to broadcast the message sent to all clients connected to the server. Once the client has left the server, its socket is closed and the client's details are removed from the `clients_list`.

As each clients' handling is taken up by a thread, any message sent by a client is sent to all other clients and any analogously, each client receives any message sent by any other client. Thus, the chat server works perfectly.

Any statement that the server prints also gets stored in the file `serverlog.txt` and can be viewed at any time. The more beneficial use of this file is that once a server has been shut down and the console on which it was run has been closed, the completye log of this latest server session can be viewed on this text file. Since the log in the text file might be difficult to understand due to it containing several ANSI colour escape sequence, the exact log can be viewed on the console using the command shown in **How to Run the Project** section.

In client.cpp, first, a username is asked from the client to make sure it follows a set length guideline (between 2 and 32 characters) and then the client socket is created and is made to connect to the server using server address and port number. After this, the client's username is sent to the server to verify for unique username. Once the client is given permission to start chatting, two threads are created, one for holding the `handle_recv()` function and the other to hold the `handle_send()` function. These two functions handle the client's sending messages to the server and receiving messages from the server. Since these two tasks must occur concurrently as we don't know when a client might want to send a message and when the server will be sending a message to the client, we run these two methods parallely using threads.

The `handle_recv()` method keeps on looking for messages from the server. This is done using the `recv()` method. In case this method receives any message from the server, the message is print on the client's console. The `handle_send()` works in a similar way in that it also keeps on looking for any message that the client has typed in its console. Once it gets a message to be sent to the server, the `write()` method is called upon to perform this task. In case the client types `E` (for exit) in the console, the `handle_send()` method makes sure the client program terminates and the client gets exited from the server. 

In this way, each client can send a message independant of receiving messages from the client and can receive messages from the client independant of seding messages to the client using threads.

