#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <fstream>

using namespace std;

ofstream file("serverlog.txt");

#define SERVER_BACKLOG 10
#define MAX_CLIENTS 70
#define USERNAME_LENGTH 32
#define BUFFER_LENGTH 2048

const char* join_msg[]={"has joined","has arrived","hopped in","is here","just landed","just slid in"};
unsigned int clients_number=0;
pthread_mutex_t lock_mutex=PTHREAD_MUTEX_INITIALIZER;


struct client_struct{
    struct sockaddr_in address;
    int socket_fd;
    char username[USERNAME_LENGTH];
};
client_struct* list[MAX_CLIENTS];


//Method to remove newline character '\n' from end of message
void remove_newline(char* msg, int length){
    for(int i=0;i<length;i++){
        if(msg[i]=='\n'){
            msg[i]='\0';
            break;
        }
    }
}


//Method to add connected client to clients list
void add_to_list(client_struct* cls){
    pthread_mutex_lock(&lock_mutex);
    for(int i=0;i<MAX_CLIENTS;i++){
        if(list[i]==NULL){
            list[i]=cls;
            break;
        }
    }
    pthread_mutex_unlock(&lock_mutex);
}


//Method to remove connected client from clients list based on username 
void remove_from_list(char* uname){
    pthread_mutex_lock(&lock_mutex);
    for(int i=0;i<MAX_CLIENTS;i++){
        if(list[i]->username==uname){
            list[i]=NULL;
            break;
        }
    }
    pthread_mutex_unlock(&lock_mutex);
}


//Method to send message to all clients connected to server
void broadcast(char* msg, string uname){
    pthread_mutex_lock(&lock_mutex);
    for(int i=0;i<MAX_CLIENTS;i++){
        if(list[i]!=NULL && list[i]->username!=uname){
            if(write(list[i]->socket_fd, msg, strlen(msg))==-1){
                cout<<"ERROR: Failed to send message to client "<<list[i]->username<<endl;
                file<<"ERROR: Failed to send message to client "<<list[i]->username<<endl;
                // break;
            }
        }
    }
    pthread_mutex_unlock(&lock_mutex);
}


/* 
This is the method to handle client connections to server. Each 
client thread will run this method till termination of client program.
*/
void* handle_connection(void* arg){
    char buffer[BUFFER_LENGTH];
    char uname[USERNAME_LENGTH];
    clients_number++;
    int flag=0;    //This flag variable will be used for closing of socket in case of some error
    client_struct* cls=(client_struct*) arg;

    //Recieve username from client
    if(recv(cls->socket_fd, uname, USERNAME_LENGTH,0)==-1){
        cout<<"ERROR: Not received username"<<endl;
        file<<"ERROR: Not received username"<<endl;
        flag=1;
    }
    else{
        int check=1;

        //For loop to check if username is unique or already has joined the server
        pthread_mutex_lock(&lock_mutex);
        for(int i=0;i<MAX_CLIENTS;i++){
            if(list[i]!=NULL && strcmp(uname,list[i]->username)==0){
                check=0;
                break;
            }
        }
        pthread_mutex_unlock(&lock_mutex);
        
        char permission[3];
        if(check==0){
            flag=1;
            permission[0]='N';
            write(cls->socket_fd, permission, 3);
        }
        else{
            permission[0]='Y';
            permission[1]=(clients_number-1)/10+48;
            permission[2]=(clients_number-1)%10+48;
            write(cls->socket_fd, permission, 3);
            strcpy(cls->username, uname);
            srand(time(0));
            int join=rand()%6;
            sprintf(buffer,"\033[1;32m\033[3;32m%s %s\033[0m\033[0m\n",cls->username, join_msg[join]);
            cout<<buffer;
            file<<buffer;
            broadcast(buffer, uname);
        }
    }

    bzero(buffer, BUFFER_LENGTH);

    while(true){
        if(flag==1){
            break;
        }

        int receive=recv(cls->socket_fd, buffer, BUFFER_LENGTH,0);
        if(receive>0){
            if(strlen(buffer)>0){
                broadcast(buffer, cls->username);
                remove_newline(buffer,strlen(buffer));
                cout<<buffer<<endl;
                file<<buffer<<endl;
            }
        }
        else if(receive==0){
            sprintf(buffer,"\033[1;31m\033[3;31m%s has left\033[0m\033[0m\n", cls->username);
            cout<<buffer;
            file<<buffer;
            broadcast(buffer,cls->username);
            flag=1;
        }
        else{
            cout<<"ERROR: recv() error"<<endl;
            file<<"ERROR: recv() error"<<endl;
            flag=1;
        }
        bzero(buffer,BUFFER_LENGTH);
    }
    
    /* 
    Once the client has exited, we close the client socket, remove the 
    client from list of clients and detatch the thread that was was running
    this instance of handle_connection() method.
    */

    close(cls->socket_fd);
    remove_from_list(cls->username);
    free(cls);
    clients_number--;
    pthread_detach(pthread_self());
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
        file<<"ERROR: Port Number in "<<arg[0]<<endl;
        return EXIT_FAILURE;
    }
    
    pthread_t t;
    int yes=1;

    //Creating the server socket
    int server_socket;
    server_socket=socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket==-1){
        cout<<"ERROR: Socket Creation Failed"<<endl;
        file<<"ERROR: Socket Creation Failed"<<endl;
        return EXIT_FAILURE;
    }

    //Defining the server address
    struct sockaddr_in server_address;
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(port_number); 
    server_address.sin_addr.s_addr=INADDR_ANY;

    //Binding the socket
    if (setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes)==-1){  //Reuse port in case port already in use
        cout<<"ERROR: setsockopt"<<endl;
        file<<"ERROR: setsockopt"<<endl;
        return EXIT_FAILURE;
    }
    if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address))==-1){
        cout<<"ERROR: Bind Failed"<<endl;
        file<<"ERROR: Bind Failed"<<endl;
        return EXIT_FAILURE;
    }

    //Listening for client to make connection
    if(listen(server_socket, SERVER_BACKLOG)==-1){
        cout<<"ERROR: Listen Failed"<<endl;
        file<<"ERROR: Listen Failed"<<endl;
        return EXIT_FAILURE;
    }

    /* 
    Once socket(), bind() and listen() have been performed successfully,
    the chat server is ready for clients to enter and chat.
    */
    
    cout<<"\033[1;34m-----------CHAT SERVER-----------\033[0m"<<endl;
    cout<<"Welcome!!!"<<endl;
    file<<"\033[1;34m-----------CHAT SERVER-----------\033[0m"<<endl;
    file<<"Welcome!!!"<<endl;

    while(true){
        //Accept the connection
        int client_socket;
        struct sockaddr_in client_address;
        int clen=sizeof(client_address);
        client_socket=accept(server_socket, (struct sockaddr*) &client_address, (socklen_t*) &clen);

        //Checking if maximum number of clients are already present in server
        if(clients_number==MAX_CLIENTS){
            cout<<"ERROR: Maximum number of clients already connected to server"<<endl;
            file<<"ERROR: Maximum number of clients already connected to server"<<endl;
            close(client_socket);
            continue;
        }

        client_struct *cls=(client_struct *) malloc(sizeof(client_struct));
        cls->address=client_address;
        cls->socket_fd=client_socket;

        add_to_list(cls);

        /*
        Multiple clients can join the server. A method handle_connection() handles
        the server's interactions with each client. To ensure proper functioning of
        the server, each client must be handled by a different instance of 
        handle_connection() method all of whom run parallely. This is where we use 
        threads, one for each client's handle_connection() method. Since both the
        main() method and thread methods access and modify common data (the clients list)
        we will also use mutex locks to ensure synchronisation.
        */

        if(pthread_create(&t, NULL, handle_connection, (void*)cls)!=0){
            cout<<"ERROR: pthread creation failed"<<endl;
            file<<"ERROR: pthread creation failed"<<endl;
            return EXIT_FAILURE;
        };
    }
    file<<endl;
    return EXIT_SUCCESS;
}
