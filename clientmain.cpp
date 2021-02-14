#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// Enable if you want debugging to be printed, see examble below.
// Alternative, pass 
#define DEBUG

#define BUFFERSIZE 100

// Included to get the support library
#include "calcLib.h"

int main(int argc, char *argv[]){

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */
  
  // Variables Declaration
  int errorStatus;
  int msgBytes;
  int socketFD;
  char msgBuffer[BUFFERSIZE];
  char adressBuffer[INET_ADDRSTRLEN];
  char* terminalIP;
  char* terminalPort;
  char delim[] = ":";
  struct addrinfo hints;
  struct addrinfo* server;
  struct addrinfo* adressPointer;

  if(argc != 2)
  {
    fprintf(stderr, "Program call was incorrect.\n");
    return 1;
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  terminalIP = strtok(argv[1], delim);
  terminalPort = strtok(NULL, delim);
  int portInt = atoi(terminalPort);

  errorStatus = getaddrinfo(terminalIP, terminalPort, &hints, &server);
  if(errorStatus != 0)
  {
    fprintf(stderr, "Program stopped at getaddrinfo: %s\n", gai_strerror(errorStatus));
    return 2;
  }

  for(adressPointer = server; server != NULL; server->ai_next)
  {
    socketFD = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
    if (socketFD == -1)
    {
      continue;
    }

    if(connect(socketFD, adressPointer->ai_addr, adressPointer->ai_addrlen) == -1)
    {
      close(socketFD);
      continue;
    }

    break;
  }

  if(adressPointer == NULL)
  {
    fprintf(stderr, "Failed to connect to a server.\n");
    return 3;
  }

  inet_ntop(adressPointer->ai_family, adressPointer->ai_addr, adressBuffer, sizeof adressBuffer);
  freeaddrinfo(server);

  msgBytes = recv(socketFD, msgBuffer, BUFFERSIZE-1, 0);
  if(msgBytes == -1)
  {
    return 4;
  }
  msgBuffer[msgBytes] = '\0';
  printf("%s\n", msgBuffer);

  // Implement Send


  close(socketFD);

#ifdef DEBUG 
  printf("Host %s, and port %d.\n",terminalIP,portInt);
#endif

  
}
