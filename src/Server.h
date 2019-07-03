#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED
int prepareSrvSocket ( const char * listenAddr, const char * service );
int prepareCliSocket ( const char * listenAddr, int port );



#endif // SERVER_H_INCLUDED
