#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <iostream>
#include <netdb.h>
#include"Input.h"
using namespace std;


int prepareCliSocket(const char * listenAddr, const char * port){

    struct addrinfo hints, *serv_info;
    int opt=1, cli_fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(listenAddr, port, &hints, &serv_info) != 0){
        cout<<"error\n";
        return -1;
    }

    cli_fd = socket(serv_info->ai_family, serv_info->ai_socktype, 0);
    if(cli_fd == -1){
        cout<<"error\n";
        freeaddrinfo(serv_info);
        return -1;
    }

    if(setsockopt(cli_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof opt) != 0){
        cout<<"error\n";
        freeaddrinfo(serv_info);
        return -1;
    }

    if(connect(cli_fd, serv_info->ai_addr, serv_info->ai_addrlen) != 0){
        cout<<"error\n";
        freeaddrinfo(serv_info);
        return -1;
    }
/*
    char msg[100] = {};
    while(true){
        fgets(msg, 100, stdin);
        send(cli_fd, msg, strlen(msg)+1, 0);
    }*/



    freeaddrinfo(serv_info);
    return cli_fd;


}






int prepareSrvSocket(const char * listenAddr, const char * port){

    struct addrinfo hints, *serv_info;
    int opt=1, server_fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(listenAddr, port, &hints, &serv_info) != 0){
        cout<<"error1\n";
        return -1;
    }

    server_fd = socket(serv_info->ai_family, serv_info->ai_socktype, 0);
    if(server_fd == -1){
        cout<<"error2\n";
        freeaddrinfo(serv_info);
        return -1;
    }

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof opt) != 0){
        cout<<"error3\n";
        freeaddrinfo(serv_info);
        return -1;
    }

    if(bind(server_fd,serv_info->ai_addr, serv_info->ai_addrlen) != 0){
        cout<<"error4\n";
        freeaddrinfo(serv_info);
        return -1;
    }

    freeaddrinfo(serv_info);

    if(listen(server_fd, 10) != 0){
        cout<<"error5\n";
        freeaddrinfo(serv_info);
        return -1;
    }
/*
    new_fd = accept(server_fd, (struct sockaddr*)&their_addr, &addr_size);

    char buffer[100];
    while(true){
        if(recv(new_fd, buffer, 100, 0) == 0){
            freeaddrinfo(serv_info);
            return 0;
        }
        cout<<buffer<<'\n';
    }
*/
    return server_fd;
}

int establishConn(int& cliSock, int& listenSock, int& clientSock, bool& isSrv, bool& isCli){
    string address, port;
    cout<<"Run as server or client? (s/c)"<<endl;
    char tmpChar = charInput("sc");
    cout<<"Enter address"<<endl;
    do{
        cin.clear();
        getline(cin, address);
    } while (!cin.good());
    cout<<"Enter port number"<<endl;
    do{
        cin.clear();
        getline(cin, port);
    } while (!cin.good());
    if(tmpChar == 'c'){
        isCli = true;
        cliSock = prepareCliSocket(&address[0], &port[0]);
        if (cliSock == -1){
            return 1;
        }
    }
    else {
        isSrv = true;
        struct sockaddr_storage their_addr;
        socklen_t addr_size = sizeof their_addr;
        listenSock = prepareSrvSocket(&address[0], &port[0]);
        if (listenSock == -1){
            return 1;
        }
        cout<<"Waiting for your opponent..."<<endl;
        clientSock = accept(listenSock, (struct sockaddr*)&their_addr, &addr_size);
    }
    return 0;
}
