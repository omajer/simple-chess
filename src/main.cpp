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
    string tmpStr, address, port;
    char tmpChar, fromy, toy, buffer[100];
    int fromx, tox=1, connectionServSock = 0, cliSock = 0, listenServSock;
    static const int MOVE = 1, QUIT = 2, MOVE_LOAD = 4, RESIGN = 5;
    bool isLocal = true, isSrv, isCli;
    memset(buffer, 0, 100);
    cout<<"Welcome to chess!"<<endl<<"Would you like to play over the network? (y/n)"<<endl;
    tmpChar = charInput("yn");
    if(tmpChar == 'y'){                         //establish connection
        isLocal = false;
        while(true){
            isSrv = false;
            isCli = false;
            connectionServSock = 0;
            cliSock = 0;
            listenServSock = 0;
            if(establishConn(cliSock, listenServSock, connectionServSock, isSrv, isCli) == 0){
                break;
            }
            cout<<"Couldn't connect. Try again? (y/n)"<<endl;
            tmpChar = charInput("ynq");
            if(tmpChar != 'y'){
                cin.clear();
                return 0;
            }
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
            currSock = connectionServSock;
        else if(isCli){
            b.setColor('b');
            currSock = cliSock;
        }
        if(isLocal || isSrv) {
            b.startGame(state, loadedGame, isLocal);
            if(state == QUIT){
                quit(isLocal, isSrv, currSock, listenServSock);
                cin.clear();
                return 0;
            }
            cout<<b.print();
        }
        while(true){
            if(!isLocal && (isCli || (isSrv && !firstTime))){
                int c = b.receiveData(state, currSock, listenServSock, isLocal, isSrv, loadedGame);
                if(c == 2){
                    return 0;
                    cin.clear();
                }
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
                quit(isLocal, isSrv, currSock, listenServSock);
                cin.clear();
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
    cin.clear();
    return 0;
}
