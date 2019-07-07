#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

int prepareSrvSocket (const char * listenAddr, const char * port);
int prepareCliSocket (const char * listenAddr, const char * port);
int establishConn(int& cliSock, int& listenSock, int& clientSock, bool& isSrv, bool& isCli);

#endif // SERVER_H_INCLUDED
