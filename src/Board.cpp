#include<iostream>
#include<vector>
#include<algorithm>
#include<fstream>
#include<sstream>
#include<cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include"Piece.h"
#include"Rook.h"
#include"Knight.h"
#include"Pawn.h"
#include"King.h"
#include"Queen.h"
#include"Bishop.h"
#include"Board.h"
#include"Input.h"
using namespace std;

class Piece;

string Board::load (const char * filename){
    ifstream ifs (filename, ifstream::in);
    if(!ifs.is_open())
        return "";
    string state;
    ifs >> state;
    if(state.length() != 73)
        return "";
    deleteBoard();
    makeBoard(state);
    return state;
}
void Board::deleteBoard() {

     for(int i = 0; i < 64; i++)
        if(board[i/8][i%8] != NULL)
            delete board[i/8][i%8];
}
Board::~Board(){
    deleteBoard();
}
bool Board::endTurn(const bool isLocal){
    if(isCheck && isMate){
        cout<<"Checkmate!"<<endl;
        if(!isLocal)
            cout<<"You win!"<<endl;
        else if(color == 'w')
            cout<<"White wins!"<<endl;
        else
            cout<<"Black wins!"<<endl;
        return true;
    } else if(isCheck)
        cout<<"Check!"<<endl;
    else if(isMate){
        cout<<"Stalemate"<<endl<<"The game is drawn"<<endl;
        return true;
    }
    if(isLocal)
        color = other(color);
    return false;
}
string Board::saveToString () const {
    string state = "";
    for(int i = 0; i < 64; i++){
        if(board[i/8][i%8] == NULL)
            state += '0';
        else
        state += board[i/8][i%8]->print();
    }
    state += epSquare.first;
    state += (char)epSquare.second+48;
    if(board[0][0] != NULL && board[0][0]->getMoved())
        state += '1';
    else
        state += '0';
    if(board[0][4] != NULL && board[0][4]->getMoved())
        state += '1';
    else
        state += '0';
    if(board[0][7] != NULL && board[0][7]->getMoved())
        state += '1';
    else
        state += '0';
    if(board[7][0] != NULL && board[7][0]->getMoved())
        state += '1';
    else
        state += '0';
    if(board[7][4] != NULL && board[7][4]->getMoved())
        state += '1';
    else
        state += '0';
    if(board[7][7] != NULL && board[7][7]->getMoved())
        state += '1';
    else
        state += '0';
    state += color;
    return state;

}
bool Board::save (const char * filename) const{
    string state = saveToString();
    ofstream ofs (filename, ofstream::out);
    if(!ofs.is_open())
        return false;
    ofs << state;
    ofs.close();
    return true;
}
void Board::sendData(const int& state, const string& loadedGame, const char fromy, const int fromx, const char toy,
                     const int tox, char& promote, const int currSock) const{
    char buffer[100];
    memset(buffer, 0, 100);
    if(state == MOVE_LOAD || state == LOAD){
        send(currSock, &loadedGame[0], loadedGame.length(), 0);
    } else {
        if(state == MOVE){
            buffer[0] = fromy;
            buffer[1] = (char)fromx;
            buffer[2] = toy;
            buffer[3] = (char)tox;
            if(isCheck && isMate)
                buffer[4] = 'm';
            else if(isCheck)
                buffer[4] = 'c';
            else if(isMate)
                buffer[4] = 's';
            else
                buffer[4] = 'n';
            buffer[5] = promote;
            promote = 0;
        } else if(state == QUIT)
            buffer[0] = 'q';
        else if(state == RESIGN)
            buffer[0] = 'r';
        buffer[72] = 0;
        send(currSock, buffer, 74, 0);
    }
    return;
}
int Board::receiveData(int& state, const int currSock, const int listenSock, const bool isLocal, const bool isSrv,
                       string& loadedGame){
    char buffer[100];
    if(state == LOAD)
        state = 0;
    int a = recv(currSock, buffer, sizeof(buffer) - 1, 0);
    if(a <= 0){
        close(currSock);
        return 2;
    }
    if(buffer[0] == 'q'){
        cout<<"Your opponent has quit the game"<<endl;
        if(!isLocal){
            close(currSock);
            if(isSrv)
                close(listenSock);
        }
        return 2;
    }
    if(buffer[0] == 'r'){
        cout<<"Your opponent has resigned"<<endl<<"You win!"<<endl;
            return 1;
    }
    if(buffer[72]){                         //handles loading a game
        loadedGame.assign(buffer, 73);
        buffer[72] = 0;
        deleteBoard();
        makeBoard(loadedGame);
        cout<<print();
    }else {                                         //handles moves
        move(buffer[0],(int)buffer[1],buffer[2],(int)buffer[3],other(color));
        if(buffer[5])
            promotion(buffer[2], (int)buffer[3], buffer[5], other(color));
        cout<<print();
        if(buffer[4] == 'm'){
            cout<<"Checkmate!"<<endl<<"You lose!"<<endl;
            return 1;
        } else if(buffer[4] == 'c')
            cout<<"Check!"<<endl;
        else if(buffer[4] == 's'){
            cout<<"Stalemate"<<endl<<"The game is drawn"<<endl;
            return 1;
            }
    }
    return 0;
}

void Board::startGame(int& state, string& loadedGame, bool isLocal){
    char tmpChar;
    string tmpStr;
    while(true){
        cout<<"Would you like to load a previous game or start a new game? (l/n)"<<endl;
        tmpChar = charInput("ln");
        /*do{
            cin.clear();
            cin >> tmpChar;
        } while (!cin.good() || (tmpChar != 'l' && tmpChar != 'n'));*/
        if(tmpChar == 'n')
            return;
        else {                              //handles loading a saved game
            cout<<"Enter filename"<<endl;
            do{
            cin.clear();
                cin >> tmpStr;
            } while (!cin.good());
            loadedGame = load(&tmpStr[0]);
            if(loadedGame != ""){
                if(loadedGame[72] == 'b'){
                    cout<<"Black to move"<<endl;
                    if(isLocal)
                        setColor('b');
                    else{
                        state = LOAD;
                    }
                } else{
                    cout<<"White to move"<<endl;
                    state = MOVE_LOAD;
                }
                return;
            }
            cout<<"Loading failed"<<endl;
        }
    }
}

bool Board::mate(const char playerColor){
    int j;
    char i;
    bool mate = true;
    vector< pair<char, int> > validMoves;
    for(i = 'a'; i <= 'h'; i++){                                                            //this section checks if the player has any legal regular moves
        for(j = 1; j <= 8; j++){
            if(at(i,j)!= NULL && at(i,j)->getColor() == playerColor){
                validMoves = at(i,j)->validMoves(i,j,*this);
                for(unsigned int k  = 0; k < validMoves.size(); k++){
                    Piece * tmp = at(validMoves[k].first, validMoves[k].second);
                    board[8-validMoves[k].second][validMoves[k].first-97] = at(i,j);
                    board[8-j][i-97]  = NULL;

                    if(!check(playerColor))
                        mate = false;
                    board[8-j][i-97] = at(validMoves[k].first, validMoves[k].second);
                    board[8-validMoves[k].second][validMoves[k].first-97] = tmp;
                    if(!mate)
                        return false;

                }

            }
        }
    }
    if(epSquare.first){                                            //this section checks if the player can perform en passant capture
        if(playerColor == 'w' && epSquare.second == 6){
            if(at(epSquare.first+1,5) != NULL && at(epSquare.first+1,5)->print() == 'p'){
                board[2][epSquare.first-97] = at(epSquare.first+1,5);
                board[3][epSquare.first-96] = NULL;
                Piece * tmp = board[3][epSquare.first-97];
                board[3][epSquare.first-97] = NULL;
                if(!check(playerColor))
                    mate = false;
                board[3][epSquare.first-96] = at(epSquare.first,6);
                board[2][epSquare.first-97] = NULL;
                board[3][epSquare.first-97] = tmp;
            }
            else if(at(epSquare.first-1,5) != NULL && at(epSquare.first-1,5)->print() == 'p'){
                board[2][epSquare.first-97] = at(epSquare.first-1,5);
                board[3][epSquare.first-98] = NULL;
                Piece * tmp = board[3][epSquare.first-97];
                board[3][epSquare.first-97] = NULL;
                if(!check(playerColor))
                    mate = false;
                board[3][epSquare.first-98] = at(epSquare.first,6);
                board[2][epSquare.first-97] = NULL;
                board[3][epSquare.first-97] = tmp;
            }
            if(!mate)
                return false;
        }
        else if(playerColor == 'b' && epSquare.second == 3){
            if(at(epSquare.first+1,4) != NULL && at(epSquare.first+1,4)->print() == 'P'){
                board[5][epSquare.first-97] = at(epSquare.first+1,4);
                board[4][epSquare.first-96] = NULL;
                Piece * tmp = board[4][epSquare.first-97];
                board[4][epSquare.first-97] = NULL;
                if(!check(playerColor))
                    mate = false;
                board[4][epSquare.first-96] = at(epSquare.first,3);
                board[5][epSquare.first-97] = NULL;
                board[4][epSquare.first-97] = tmp;
            }
            else if(at(epSquare.first-1,4) != NULL && at(epSquare.first-1,4)->print() == 'P'){
                board[5][epSquare.first-97] = at(epSquare.first-1,4);
                board[4][epSquare.first-98] = NULL;
                Piece * tmp = board[4][epSquare.first-97];
                board[4][epSquare.first-97] = NULL;
                if(!check(playerColor))
                    mate = false;
                board[4][epSquare.first-98] = at(epSquare.first,3);
                board[5][epSquare.first-97] = NULL;
                board[4][epSquare.first-97] = tmp;
            }
            if(!mate)
                return false;
        }
    }
    return true;
}


bool Board::move (const char fromy, const int fromx, const char toy, const int tox, const char playerColor){
    if(at(fromy, fromx) == NULL || at(fromy, fromx)->getColor() != playerColor || toy < 97 || toy > 104 || tox < 1 || tox > 8 || (at(toy, tox) != NULL && at(toy, tox)->getColor() == playerColor))
        return false;
    if(fromy == 'e' &&((fromx == 1 && tox == 1) || (fromx == 8 && tox == 8)) && (toy == 'c' || toy == 'g') && at(fromy, fromx)->getType() == 'k'){          //this branch handles castling
        if(at(fromy, fromx)->getColor() == playerColor && !at(fromy, fromx)->getMoved() && !check(playerColor)) {
            if(toy == 'g' && at('h',fromx) != NULL && at('h',fromx)->getColor() == playerColor && !at('h',fromx)->getMoved() &&  at('f',fromx) == NULL && at('g',fromx) == NULL ){
                board[8-fromx][5] = board[8-fromx][4];
                board[8-fromx][4] = NULL;
                if(check(playerColor)){
                    board[8-fromx][4] = board[8-fromx][5];
                    board[8-fromx][5] = NULL;
                    return false;
                }
                board[8-fromx][6] = board[8-fromx][5];
                board[8-fromx][5] = board[8-fromx][7];
                board[8-fromx][7] = NULL;
                if(check(playerColor)){
                    board[8-fromx][4] = board[8-fromx][6];
                    board[8-fromx][6] = NULL;
                    board[8-fromx][7] = board[8-fromx][5];
                    board[8-fromx][5] = NULL;
                    return false;
                }
            }

            else if(toy == 'c' && at('a',fromx) != NULL && at('a',fromx)->getColor() == playerColor && !at('a',fromx)->getMoved() && at('b',fromx) == NULL && at('c',fromx)==NULL&& at('d',fromx)==NULL){
                board[8-fromx][3] = board[8-fromx][4];
                board[8-fromx][4] = NULL;
                if(check(playerColor)){
                    board[8-fromx][4] = board[8-fromx][3];
                    board[8-fromx][3] = NULL;
                    return false;
                }
                board[8-fromx][2] = board[8-fromx][3];
                board[8-fromx][3] = board[8-fromx][0];
                board[8-fromx][0] = NULL;
                if(check(playerColor)){
                    board[8-fromx][4] = board[8-fromx][2];
                    board[8-fromx][2] = NULL;
                    board[8-fromx][0] = board[8-fromx][3];
                    board[8-fromx][3] = NULL;
                    return false;
                }
            }
            else
                return false;
            at(toy, tox)->setMoved();
        }
        else
            return false;
    }
    else if(   epSquare.first == toy && epSquare.second == tox && (toy == fromy - 1 || toy == fromy + 1) &&  (( at(fromy, fromx)->print() == 'P' && fromx == 4 && tox == 3 ) ||
        ( at(fromy, fromx)->print() == 'p' && fromx == 5 && tox == 6 ) ) ){                                                     //this branch handles en passant capture
        board[8-tox][toy-97] = at(fromy, fromx);
        board[8-fromx][fromy-97]  = NULL;
        Piece * tmp = board[8-fromx][toy-97];
        board[8-fromx][toy-97]  = NULL;
        if(check(playerColor)){
            board[8-fromx][fromy-97] = at(toy, tox);
            board[8-tox][toy-97] = NULL;
            board[8-fromx][toy-97] = tmp;
            return false;
        }
        at(toy, tox)->setMoved();
        delete tmp;
       }
    else {                                                                                              //this branch handles regular moves
        vector< pair<char, int> > validMoves = at(fromy, fromx)->validMoves(fromy, fromx, *this);
        vector< pair<char, int> >::iterator it = find(validMoves.begin(), validMoves.end(), make_pair(toy, tox));
        if(it == validMoves.end())
            return false;
        Piece * tmp = at(toy, tox);
        board[8-tox][toy-97] = at(fromy, fromx);
        board[8-fromx][fromy-97]  = NULL;
        if(check(playerColor)){
            board[8-fromx][fromy-97] = at(toy, tox);
            board[8-tox][toy-97] = tmp;
            return false;
        }
        at(toy, tox)->setMoved();
        delete tmp;
    }
    if( at(toy,tox)->getType() == 'p'  && (fromx == 2 || fromx == 7) && (tox == fromx - 2 || tox == fromx + 2 ))
        epSquare = make_pair(toy, (tox + fromx) / 2);
    else
        epSquare = make_pair(0,0);
    return true;
}

char Board::other (const char playerColor){
    if(playerColor == 'w')
        return 'b';
    if(playerColor == 'b')
        return 'w';
    return 0;
}

string Board::print() const{
    stringstream str;
    string result = "";
    char c;
    str<<"   _______________________________________________ "<<'\n'<<"  |█████|     |█████|     |█████|     |█████|     |"<<'\n';
    for(int i = 0; i < 8; i++){
        str<<8 - i;
        if(i%2)
            str<<" |  ";
        else
            str<<" |██";
        for(int j = 0; j < 8; j++){
            if(board[i][j] == NULL){
                if(!((i+j) % 2))
                    str<<"█";
                else
                    str<<" ";
            } else {
                str<<board[i][j]->print();
            }

                if((i+j)%2){
                    if(j<7)
                        str<<"  |██";
                    else
                        str<<"  |  ";
                    }

                else
                    str<<"██|  ";

        }
        if(i%2)
            str<<'\n'<<"  |_____|█████|_____|█████|_____|█████|_____|█████|"<<'\n';
        else
            str<<'\n'<<"  |█████|_____|█████|_____|█████|_____|█████|_____|"<<'\n';
        if(i < 7){
            if(i%2)
                str<<"  |█████|     |█████|     |█████|     |█████|     |";
            else
                str<<"  |     |█████|     |█████|     |█████|     |█████|";
            }
        str<<'\n';
    }
    str<<"     a     b     c     d     e     f     g     h"<<'\n';
    while(str.get(c))
        result += c;
    return result;
}


Piece * Board::at (const char y, const int x) const {
    if(y < 97 || y > 104 || x < 1 || x > 8)
        return NULL;
    return board[8 - x][y - 97];
}
void Board::getInput(int& state, char& promote, string& loadedGame, char& fromy, int& fromx, char& toy, int& tox, const bool firstTime){
    string tmpStr;
    while(state != LOAD){
        string input;
        getline(cin, input);
        if(!cin.good() || input.length() < 1){
            cin.clear();
            continue;
        }
        if(input[0] >= 'a' && input[0] <= 'h') {            //performs moves
            istringstream iss(input);
            iss>>fromy>>fromx>>toy>>tox;
            if(iss.fail())
                continue;
            if(move(fromy,fromx,toy,tox,color)){
                if((at(toy, tox)->print() == 'P' && tox == 1) || (at(toy, tox)->print() == 'p' && tox == 8)){       //handles promotion
                    cout<<"Promote to queen (q), knight (n), bishop (b) or rook (r)?"<<endl;
                    do{
                        cin>>promote;
                    } while(!cin.good() || (promote != 'q' && promote != 'r' && promote != 'n' && promote != 'b'));
                    promotion(toy, tox, promote, color);
                }
                isCheck = check(other(color));
                isMate = mate(other(color));
                if(state != MOVE_LOAD || !firstTime)
                    state = MOVE;
                if(state == MOVE_LOAD)
                    loadedGame = saveToString();
                break;
            }
        } else if(input.length() > 1)
            continue;
        else if(input[0] == 'q'){
            state = QUIT;
            break;
        } else if(input[0] == 'r'){
            state = RESIGN;
            break;
        }
        else if(input[0] == 's'){               //handles saving the game
            cout<<"Enter filename"<<endl;
            do{
            cin.clear();
                cin >> tmpStr;
            } while (!cin.good());
            if(!save(&tmpStr[0]))
                cout<<"Cannot save game"<<endl;
            else
                cout<<"Game saved"<<endl;
            continue;
        }
    }
    return;
}

bool Board::check(const char playerColor) const{
    int i, x=0;
    char j, y='\0';
    for(j = 'a'; j <= 'h'; j++){
        for(i = 1; i <= 8; i++){
            if(at(j,i) != NULL && at(j,i)->getType() == 'k'  && at(j,i)->getColor() == playerColor) {
                x = i;
                y = j;
            }
        }
    }
    if(playerColor == 'w' && at(y+1,x+1) != NULL && at(y+1,x+1)->print() == 'P')            //checks if the king is threatened by a pawn
        return true;
    if(playerColor == 'w' && at(y-1,x+1) != NULL && at(y-1,x+1)->print() == 'P')
        return true;
    if(playerColor == 'b' && at(y+1,x-1) != NULL && at(y+1,x-1)->print() == 'p')
        return true;
    if(playerColor == 'b' && at(y-1,x-1) != NULL && at(y-1,x-1)->print() == 'p')
        return true;

    if(at(y+1,x+1) != NULL && (at(y+1,x+1)->getType() == 'k' ) && at(y+1,x+1)->getColor() != playerColor)            //checks if the king is threatened by the other king (in  hypothetical positions)
        return true;
    if(at(y+1,x) != NULL && (at(y+1,x)->getType() == 'k' ) && at(y+1,x)->getColor() != playerColor)
        return true;
    if(at(y+1,x-1) != NULL && (at(y+1,x-1)->getType() == 'k' ) && at(y+1,x-1)->getColor() != playerColor)
        return true;
    if(at(y,x+1) != NULL && (at(y,x+1)->getType() == 'k' ) && at(y,x+1)->getColor() != playerColor)
        return true;
    if(at(y,x-1) != NULL && (at(y,x-1)->getType() == 'k' ) && at(y,x-1)->getColor() != playerColor)
        return true;
    if(at(y-1,x+1) != NULL && (at(y-1,x+1)->getType() == 'k' ) && at(y-1,x+1)->getColor() != playerColor)
        return true;
    if(at(y-1,x) != NULL && (at(y-1,x)->getType() == 'k' ) && at(y-1,x)->getColor() != playerColor)
        return true;
    if(at(y-1,x-1) != NULL && (at(y-1,x-1)->getType() == 'k' ) && at(y-1,x-1)->getColor() != playerColor)
        return true;
    if(at(y+1,x+2) != NULL && (at(y+1,x+2)->getType() == 'n') && at(y+1,x+2)->getColor() != playerColor)            //checks if the king is threatened by a knight
        return true;
    if(at(y+1,x-2) != NULL && (at(y+1,x-2)->getType() == 'n') && at(y+1,x-2)->getColor() != playerColor)
        return true;
    if(at(y-1,x+2) != NULL && (at(y-1,x+2)->getType() == 'n') && at(y-1,x+2)->getColor() != playerColor)
        return true;
    if(at(y-1,x-2) != NULL && (at(y-1,x-2)->getType() == 'n') && at(y-1,x-2)->getColor() != playerColor)
        return true;

    if(at(y+2,x+1) != NULL && (at(y+2,x+1)->getType() == 'n') && at(y+2,x+1)->getColor() != playerColor)
        return true;

    if(at(y+2,x-1) != NULL && (at(y+2,x-1)->getType() == 'n') && at(y+2,x-1)->getColor() != playerColor)
        return true;
    if(at(y-2,x+1) != NULL && (at(y-2,x+1)->getType() == 'n') && at(y-2,x+1)->getColor() != playerColor)
        return true;
    if(at(y-2,x-1) != NULL && (at(y-2,x-1)->getType() == 'n') && at(y-2,x-1)->getColor() != playerColor)
        return true;
    for(i = x+1; i <=8; i++) {
        if(at(y,i) != NULL){
           if( at(y,i)->getColor() == playerColor || ( at(y,i)->getType()!= 'r' &&  at(y,i)->getType()!= 'q' ))            //checks if the king is threatened by a rook or a queen on the same rank or file
                break;
           if( at(y,i)->getColor() != playerColor && ( at(y,i)->getType() == 'r' || at(y,i)->getType() == 'q'))
                return true;
        }
    }
    for(i = x-1; i >= 1; i--) {
        if(at(y,i) != NULL){
           if( at(y,i)->getColor() == playerColor || ( at(y,i)->getType()!= 'r' &&  at(y,i)->getType()!= 'q' ))
                break;
           if( at(y,i)->getColor() != playerColor && ( at(y,i)->getType() == 'r' || at(y,i)->getType() == 'q'))
                return true;
        }
    }
    for(j = y+1; j <='h'; j++) {
        if(at(j,x) != NULL){
           if( at(j,x)->getColor() == playerColor || ( at(j,x)->getType()!= 'r'  && at(j,x)->getType()!= 'q' ))
                break;
           if( at(j,x)->getColor() != playerColor && ( at(j,x)->getType() == 'r'  || at(j,x)->getType() == 'q'))
                return true;
        }
    }
    for(j = y-1; j >='a'; j--) {
        if(at(j,x) != NULL){
           if( at(j,x)->getColor() == playerColor || ( at(j,x)->getType()!= 'r'  && at(j,x)->getType()!= 'q' ))
                break;
           if( at(j,x)->getColor() != playerColor && ( at(j,x)->getType() == 'r'  || at(j,x)->getType() == 'q'))
                return true;
        }
    }
    j = y + 1;
    i = x + 1;
    while( j <='h' && i <= 8) {
        if(at(j,i) != NULL){
           if( at(j,i)->getColor() == playerColor || ( at(j,i)->getType()!= 'b' && at(j,i)->getType()!= 'q'))            //checks if the king is threatened by a bishop or a queen on the same diagonal
               break;
           if( at(j,i)->getColor() != playerColor && ( at(j,i)->getType() == 'b' || at(j,i)->getType() == 'q'))
                return true;
        }
        j++;
        i++;
    }
    j = y - 1;
    i = x + 1;
    while( j >='a' && i <= 8) {
        if(at(j,i) != NULL){
           if( at(j,i)->getColor() == playerColor || ( at(j,i)->getType()!= 'b' && at(j,i)->getType()!= 'q'))
                break;
           if( at(j,i)->getColor() != playerColor && ( at(j,i)->getType() == 'b'  || at(j,i)->getType() == 'q' ))
                return true;
        }
        j--;
        i++;
    }
    j = y + 1;
    i = x - 1;
    while( j <='h' && i >= 1) {
        if(at(j,i) != NULL){
           if( at(j,i)->getColor() == playerColor || ( at(j,i)->getType()!= 'b' && at(j,i)->getType()!= 'q'))
                break;
           if( at(j,i)->getColor() != playerColor && ( at(j,i)->getType() == 'b' || at(j,i)->getType() == 'q' ))
                return true;
        }
        j++;
        i--;
    }

    j = y - 1;
    i = x - 1;
    while( j >='a' && i >= 1) {
        if(at(j,i) != NULL){
           if( at(j,i)->getColor() == playerColor || ( at(j,i)->getType()!= 'b' && at(j,i)->getType()!= 'q'))
                break;
           if( at(j,i)->getColor() != playerColor && ( at(j,i)->getType() == 'b'  || at(j,i)->getType() == 'q' ))
                return true;
        }
        j--;
        i--;
    }
    return false;
}

void Board::promotion(const char toy, const int tox, const char promoteTo, const char playerColor){
    delete at(toy, tox);
    switch(promoteTo){
        case 'q':
            board[8-tox][toy-97] = new Queen(playerColor);
            break;
        case 'r':
            board[8-tox][toy-97] = new Rook(playerColor);
            break;
        case 'n':
            board[8-tox][toy-97] = new Knight(playerColor);
            break;
        case 'b':
            board[8-tox][toy-97] = new Bishop(playerColor);
            break;
    }

}

void Board::makeBoard(const string state){

    for(int i = 0; i < 64; i++){
        switch(state[i]){
        case '0':
            board[i/8][i%8] = NULL;
            break;
        case 'p':
            board[i/8][i%8] = new Pawn('w');
            break;
        case 'P':
            board[i/8][i%8] = new Pawn('b');
            break;
        case 'r':
            board[i/8][i%8] = new Rook('w');
            break;
        case 'R':
            board[i/8][i%8] = new Rook('b');
            break;
        case 'n':
            board[i/8][i%8] = new Knight('w');
            break;
        case 'N':
            board[i/8][i%8] = new Knight('b');
            break;
        case 'b':
            board[i/8][i%8] = new Bishop('w');
            break;
        case 'B':
            board[i/8][i%8] = new Bishop('b');
            break;
        case 'q':
            board[i/8][i%8] = new Queen('w');
            break;
        case 'Q':
            board[i/8][i%8] = new Queen('b');
            break;
        case 'k':
            board[i/8][i%8] = new King('w');
            break;
        case 'K':
            board[i/8][i%8] = new King('b');
            break;
        }
    }
    epSquare = make_pair(state[64],(int)state[65]-48);
    if(state[66] != '0' && board[0][0] != NULL)
        board[0][0]->setMoved();
    if(state[67] != '0' && board[0][4] != NULL)
        board[0][4]->setMoved();
    if(state[68] != '0' && board[0][7] != NULL)
        board[0][7]->setMoved();
    if(state[69] != '0' && board[7][0] != NULL)
        board[7][0]->setMoved();
    if(state[70] != '0' && board[7][4] != NULL)
        board[7][4]->setMoved();
    if(state[71] != '0' && board[7][7] != NULL)
        board[7][7]->setMoved();
}

Board::Board() {
    epSquare = make_pair(0,0);
    color = 'w';
    isCheck = false;
    isMate = false;
    board[0][0] = new Rook('b');
    board[0][7] = new Rook('b');
    board[0][1] = new Knight('b');
    board[0][6] = new Knight('b');
    board[0][2] = new Bishop('b');
    board[0][5] = new Bishop('b');
    board[0][3] = new Queen('b');
    board[0][4] = new King('b');
    for(int i = 0; i < 8; i++){
        board[1][i] = new Pawn('b');
    }
    for(int i = 2; i < 6; i++){
        for(int j = 0; j < 8; j++){
            board[i][j] = NULL;
        }
    }
    board[7][0] = new Rook('w');
    board[7][7] = new Rook('w');
    board[7][1] = new Knight('w');
    board[7][6] = new Knight('w');
    board[7][2] = new Bishop('w');
    board[7][5] = new Bishop('w');
    board[7][3] = new Queen('w');
    board[7][4] = new King('w');
    for(int i = 0; i < 8; i++){
        board[6][i] = new Pawn('w');
    }
}
