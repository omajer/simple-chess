/** @mainpage
* This program is an implementation of chess without AI. It is playable over the network or locally and supports saving games.
*/
#include<iostream>
#include<vector>
#include<algorithm>
#include<fstream>
#include<sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include<cstring>
#include"Piece.h"
#include"Rook.h"
#include"Knight.h"
#include"Pawn.h"
#include"King.h"
#include"Queen.h"
#include"Bishop.h"
#include"Board.h"
#include"Server.h"
#include"Input.h"
using namespace std;


int main() {
    string tmpStr, address, service;
    char tmpChar, fromy, toy, buffer[100];
    int port, fromx, tox=1, clientSock = 0, cliSock = 0, listenSock;
    static const int MOVE = 1, QUIT = 2, MOVE_LOAD = 4, RESIGN = 5;
    bool isLocal = true, isSrv = false, isCli = false;
    memset(buffer, 0, 100);
    cout<<"Welcome to chess!"<<endl<<"Would you like to play over the network? (y/n)"<<endl;
    tmpChar = charInput("yn");
    if(tmpChar == 'y'){
        isLocal = false;
        cout<<"Run as server or client? (s/c)"<<endl;
        tmpChar = charInput("sc");
        cout<<"Enter address"<<endl;
        do{
            cin.clear();
            getline(cin, address);
        } while (!cin.good());
        cout<<"Enter port number"<<endl;
        if(tmpChar == 'c'){
            isCli = true;
            do{
                cin.clear();
                cin >> port;
            } while (!cin.good());

            cliSock = prepareCliSocket ( &address[0], port );
            if (cliSock == -1){
                cout<<"Couldn't connect"<<endl;
                return 1;
            }
        }
        else {
            isSrv = true;
            do{
                cin.clear();
                cin >> service;
            } while (!cin.good());
            listenSock = prepareSrvSocket ( &address[0], &service[0]);
            if ( listenSock == -1 ){
                cout<<"Couldn't connect"<<endl;
                return 1;
            }
            struct sockaddr addr;
            socklen_t addrLen = sizeof ( addr );
            cout<<"Waiting for your opponent..."<<endl;
            clientSock = accept ( listenSock, &addr, &addrLen );
        }
    }

    cout<<endl<<"Instructions:"<<endl<<"To move a piece, simply type the name of the origin square";
    cout<<"and the destination square, e.g.:"<<endl<<"'a2 a4'"<<endl;
    cout<<"To resign, type 'r'. To save the game, type 's'. To quit the game, type 'q'"<<endl<<endl;

    while(true){
        Board b;
        char promote = 0;
        string loadedGame;
        int currSock, state = 0;
        bool firstTime = true;
        if(isSrv)
            currSock = clientSock;
        else if(isCli){
            b.setColor('b');
            currSock = cliSock;
        }
        if(isLocal || isSrv) {
            b.startGame(state, loadedGame, isLocal);
            if(state == QUIT){
                quit(isLocal, isSrv, currSock, listenSock);
                return 0;
            }
            cout<<b.print();
        }
        while(true){
            if(!isLocal && (isCli || (isSrv && !firstTime))){
                int c = b.receiveData(state, currSock, listenSock, isLocal, isSrv, loadedGame);
                if(c == 2)
                    return 0;
                else if(c == 1)
                    break;
            }
            b.getInput(state, promote, loadedGame, fromy, fromx, toy, tox, firstTime);
            if(!isLocal)
                b.sendData(state, loadedGame, fromy, fromx, toy, tox,  promote, currSock);
            if(state == MOVE || state == MOVE_LOAD){
                cout<<b.print();
            }
            if(state == QUIT){
                quit(isLocal, isSrv, currSock, listenSock);
                return 0;
            }
            if(state == RESIGN){
                if(isLocal){
                    cout<<b.getColorName()<<" wins!"<<endl;
                }
                break;
            }
            if(b.endTurn(isLocal)){
                break;
            }
            firstTime = false;
        }
    }
    return 0;
}
