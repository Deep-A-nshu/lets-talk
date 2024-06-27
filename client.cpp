#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <csignal>

using namespace std;

#define MAX_CLIENTS 70
#define USERNAME_LENGTH 32
#define BUFFER_LENGTH 2048

const char* colour_list[]={"30","33","34","35","36","37","90","93","94","95","96","97"};
volatile int clr;
volatile sig_atomic_t flag=0;
char username[USERNAME_LENGTH];


void catch_exit(){
    flag=1;
}


//Method to add > before any new message seen by client
void output_designer(){
    cout<<"\r>";
    cout.flush();
}


//Method to remove newline character '\n' from end of message
void remove_newline(char* msg, int length){
    for(int i=0;i<length;i++){
        if(msg[i]=='\n'){
            msg[i]='\0';
            break;
        }
    }
}


//Method to handle receive messages from server to client
void* handle_recv(void* arg){
    int* client_socket=(int*) arg;
    char buffer[BUFFER_LENGTH];
    while(true){
        int receive=recv(*client_socket, buffer, BUFFER_LENGTH,0);
        if(receive>0){
            cout<<buffer;
            output_designer();
        }
        else if(receive==0){
            break;
        }
        bzero(buffer,BUFFER_LENGTH);
    }
    catch_exit();
    return NULL;
}


//Method to handle sending of messages from client to server
void* handle_send(void* arg){
    int* client_socket=(int*) arg;
    char buffer[BUFFER_LENGTH];
    char message[BUFFER_LENGTH];
    while(true){
        output_designer();
        fgets(buffer, BUFFER_LENGTH, stdin);
        remove_newline(buffer,BUFFER_LENGTH);
        if(strcmp(buffer,"E")==0){  
            break;
        }
        else{
            sprintf(message, "\033[1;%sm%s\033[0m: %s\n",colour_list[clr], username, buffer);
            send(*client_socket, message, strlen(message),0);
        }
        bzero(buffer,BUFFER_LENGTH);
        bzero(message,BUFFER_LENGTH+USERNAME_LENGTH);
    }
    flag=1;
    return NULL;
}


int main(int argc, char **arg){

    /* 
    Port number can be specified as command-line argument for main(), 
    in which case the value of argc will be 2, one for the name of the 
    program and one for the specified port number on which the server 
    has connected. In case no port number has been specified, default 
    port number 9002 is taken.
    */
    
    int port_number;  

    if(argc==1){
        port_number=9002;
    }
    else if(argc==2){
        port_number=stoi(arg[1]);
    }
    else{
        cout<<"ERROR: Port Number in "<<arg[0]<<endl;
        return EXIT_FAILURE;
    }

    //Asking for username
    cout<<"Enter username:";
    fgets(username, USERNAME_LENGTH, stdin);
    remove_newline(username, strlen(username));
    if(strlen(username)>USERNAME_LENGTH || strlen(username)<2){
        cout<<"Enter username correctly. It must be between 2 and "<<USERNAME_LENGTH<<" characters long."<<endl;
        return EXIT_FAILURE;
    }

    //Creating the client socket
    int client_socket;
    client_socket=socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket==-1){
        cout<<"ERROR: Socket Creation Failed"<<endl;
        return EXIT_FAILURE;
    }

    //Specify server address for client socket to connect to
    struct sockaddr_in server_address;
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(port_number);
    server_address.sin_addr.s_addr=INADDR_ANY;

    //Connecting with server
    int connection_status=connect(client_socket,(struct sockaddr *) &server_address, sizeof(server_address));
    if(connection_status==-1){
        cout<<"ERROR: Connecting with server failed"<<endl;
        return EXIT_FAILURE;
    }

    //Sending username to server
    send(client_socket, username, USERNAME_LENGTH,0);

    /*
    Now, to ensure that each user has a unique username, the client sends
    the entered username to the server and waits for the server to
    send back permission to use the username. Character array permission[3]
    will store permission received back from the server. The first element  
    of the array 'Y' if permission given, 'N' if not given. The next two
    elements will be the number fo people already present and chatting 
    on the server.
    */

    char permission[3];      
    recv(client_socket, permission, 3, 0);

    if(permission[0]=='Y'){
        cout<<"\033[1;34m-----------CHAT SERVER-----------\033[0m"<<endl;
        cout<<"Welcome!!! Happy Chatting!\n"<<permission[1]<<permission[2]<<" people are already on the server";
        cout<<"\nTo leave server, enter E)"<<endl;

        //Giving the client a specific colour at random
        srand(time(0));
        clr=rand()%12;

        /*
        The client has to send messages to server and also receive messages sent
        by the server. These two tasks can occur at any time, i.e, there no definite order 
        in which these two tasks will be performed. Hence, we will run these 
        two tasks/functions parallely using threads.
        */

        pthread_t recv_thread;
        pthread_t send_thread;

         //Creating thread to handle receive of messages from server
        if(pthread_create(&recv_thread, NULL, handle_recv, &client_socket)!=0){
            cout<<"ERROR: pthread creation failed"<<endl;
            return EXIT_FAILURE;
        }

        //Creating thread to handle sending of messages to server from client
        if(pthread_create(&send_thread, NULL, handle_send, &client_socket)!=0){
            cout<<"ERROR: pthread creation failed"<<endl;
            return EXIT_FAILURE;
        }

        while(true){
            if(flag){
                cout<<"Bye! See you soon!!!"<<endl;
                break;
            }
        }
    }
    else{
        cout<<"Username already in use. Try again.";
    }

    //Closing client socket
    close(client_socket);

    return EXIT_SUCCESS;
}
