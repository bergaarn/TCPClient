#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <calcLib.h>

using namespace std;

#define SERVPROTOCOL "TEXT TCP 1.0"
#define DEBUG


int main(int argc, char *argv[]){
 
  if(argc < 2)
  {
    fprintf(stderr, "Program call was incorrect.\n");
    return 1;
  }

  if(strlen(argv[1]) < 12)
  {
    fprintf(stderr, "No Port Found.\n");
    return 1;
  }

  char delim[] = ":";
  char* servHost = strtok(argv[1],delim);
  char* servPort = strtok(NULL,delim);

  #ifdef DEBUG  
  printf("Host %s, and port %s.\n",servHost, servPort);
  #endif

  int rv;
  int sockFD;
  int servPortInt = atoi(servPort);
  int clientBacklog = 5;
  
  // Create Socket
  sockFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sockFD == -1)
  {
    fprintf(stderr, "Error Socket Creation.\n");
    return 3;
  }

  // Create Server
  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(servPortInt);
  inet_aton(servHost, &servAddr.sin_addr);

  // Socket options REUSE
  int reUse = 1;
  rv = setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &reUse, sizeof(reUse));
  if(rv == -1)
  {
    perror("Error SO_OPTIONS");
    exit(1);
  }

  // Bind Socket and Server
  rv = bind(sockFD, (struct sockaddr*)&servAddr, sizeof(servAddr));
  if(rv == -1)
  {
    perror("Error Bind");
    return 4;
  }

  // Listen to incoming connections on IP:PORT
  printf("Listening to %s:%d\n",inet_ntoa(servAddr.sin_addr), servPortInt);

  rv = listen(sockFD, clientBacklog);
  if(rv == -1)
  {
    perror("Unable to Listen");
    exit(1);
  }

  int sv;
  int clientSockFD;
  struct sockaddr_in clientAddr;
  memset(&clientAddr, 0, sizeof(clientAddr));
  socklen_t clientLen = sizeof(clientAddr);
  char buff[1495];


  // Loop to accept incoming client calls
  while(1)
  {
    clientSockFD = accept(sockFD, (sockaddr*)&clientAddr, &clientLen);
    if(clientSockFD == -1)
    {
      perror("Error Accepting client");
      continue;
    }

    printf("[Client Accepted]\n");

    // If client is accepted, enter a nested loop for communication
    while(1)
    {

      // Recieve from client
      memset(&buff, 0, sizeof(buff));
      rv = recv(clientSockFD, &buff, sizeof(buff), 0);
      if(rv == -1)
      {
        perror("Error Recieving");
        break;
      }
      else if(rv == 0)
      {
        printf("Recieved 0 Bytes\n");
      }
      else 
      {
        printf("Client: <%d bytes> %s", rv, buff);
      }

      // Send to Client
      sv = send(clientSockFD, &buff, rv, 0);
      if(sv == -1)
      {
        perror("Error Sending");
        break;
      }
      else if(sv == 0)
      {
        printf("Sent 0 Bytes\n");
        break;
      }
      else if(rv != sv)
      {
        printf("Expected to send %d, actually sent %d\n", rv, sv);
      }
      else 
      {
        //printf("Server: <%d bytes> %s\n", sv, buff);
      }
    }
    close(clientSockFD);
  }
 
  close(sockFD);
  return 0;
}
