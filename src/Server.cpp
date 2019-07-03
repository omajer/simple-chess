/** @file
* Contains functions that establish the connection when playing over the network.
* The functions implemented in this file were written by Ing. Ladislav Vagner, Ph.D.
* and copied from https://edux.fit.cvut.cz/courses/BI-PA2/_media/net_2015.tgz.
*/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
using namespace std;


int prepareCliSocket ( const char * listenAddr, int port )
 {
   struct addrinfo * ai;
   char portTx[20];

   snprintf ( portTx, sizeof ( portTx ), "%d", port );
   if ( getaddrinfo ( listenAddr, portTx, NULL, &ai ) != 0 ) return -1;

   int sock = socket ( ai -> ai_family, SOCK_STREAM, 0 );
   if ( sock == -1 )
    {
      freeaddrinfo ( ai );
      return -1;
    }

   if ( connect ( sock, ai -> ai_addr, ai -> ai_addrlen ) != 0 )
    {
      close ( sock );
      freeaddrinfo ( ai );
      return -1;
    }
   freeaddrinfo ( ai );
   return sock;
 }

int prepareSrvSocket ( const char * listenAddr, const char * service )
 {
   struct addrinfo * ai;

   if ( getaddrinfo ( listenAddr, service, NULL, &ai ) != 0 ) return -1;

   int sock = socket ( ai -> ai_family, SOCK_STREAM, 0 );

   if ( sock == -1 )
    {
      freeaddrinfo ( ai );
      return -1;
    }

   if ( bind ( sock, ai -> ai_addr, ai -> ai_addrlen ) != 0 )
    {
      close ( sock );
      freeaddrinfo ( ai );
      return -1;
    }

   if ( listen ( sock, 10 ) != 0 )
    {
      close ( sock );
      freeaddrinfo ( ai );
      return -1;
    }
   freeaddrinfo ( ai );
   return sock;
 }

