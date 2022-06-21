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

## Project Details

The project uses pthreads library, which is an OS dependent library(Linux). The server uses socket programming to
connect to the client. The server first binds itself, i.e the bind function binds the socket to the address 
and port number specified in addr.Then the listen mode is activated i.e It puts the server socket in a passive mode, where it waits for the 
client to approach the server to make a connection.

