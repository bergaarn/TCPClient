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
#include <errno.h>
// Included to get the support library
#include <calcLib.h>

#define DEBUG
#define BUFFERSIZE 1450
#define PROTOCOL "TEXT TCP 1.0"


int main(int argc, char *argv[]){

  int errorStatus;
  int recvBytes;
  int socketFD;
  char recvBuffer[BUFFERSIZE];
  char addressBuffer[INET_ADDRSTRLEN];
  char* terminalIP;
  char* terminalPort;
  char delim[] = ":";
  struct addrinfo hints;
  struct addrinfo* server;
  struct addrinfo* addressPointer;
  
  if(argc != 2)
  {
    fprintf(stderr, "Program call was incorrect.\n");
    return 1;
  }

  if(strlen(argv[1]) < 12)
  {
    fprintf(stderr, "No Port Found.\n");
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

  for(addressPointer = server; addressPointer != NULL; addressPointer = addressPointer->ai_next)
  {
    socketFD = socket(addressPointer->ai_family, addressPointer->ai_socktype, addressPointer->ai_protocol);
    if (socketFD == -1)
    {
      continue;
    }

    if(connect(socketFD, addressPointer->ai_addr, addressPointer->ai_addrlen) == -1)
    {
      close(socketFD);
      continue;
    }

    break;
  }

  if(addressPointer == NULL)
  {
    fprintf(stderr, "Failed to connect to a server.\n");
    return 3;
  }

  else
  {
    char localIP[INET_ADDRSTRLEN];
    struct sockaddr_in myAddress;
    memset(&myAddress, 0, sizeof(myAddress));
    socklen_t len = sizeof myAddress;
    getsockname(socketFD, (struct sockaddr*)&myAddress, &len);
    inet_ntop(AF_INET, &myAddress, localIP, sizeof(localIP));
    

    #ifdef DEBUG 
    printf("Host %s, and port %d.\n",terminalIP,portInt);
    #endif

    printf("Connected to Server. My Local IP is: %s:%d\n", localIP, ntohs(myAddress.sin_port));
  }

  inet_ntop(addressPointer->ai_family, addressPointer->ai_addr, addressBuffer, sizeof addressBuffer);
  freeaddrinfo(server);

  recvBytes = recv(socketFD, &recvBuffer, sizeof(recvBuffer), 0);
  if(recvBytes == -1)
  {
    return 4;
  }
  printf("%s", recvBuffer);

  char delimProtocol[] = "\n";
  char* tcpOne = strtok(recvBuffer, delimProtocol);

  if(strcmp(tcpOne, PROTOCOL) == 0)
  { 
    char sendBuffer[BUFFERSIZE] = "OK\n";
    printf("%s\n", sendBuffer);
    int length = strlen(sendBuffer);

    int rv = send(socketFD, &sendBuffer, length, 0);
    if(rv == -1)
    {
      perror("send error");
    }
  }
  else 
  {
    fprintf(stderr, "Incompatible Protocol.\n");
    return 5;
  }

  memset(&recvBuffer, 0, sizeof(recvBuffer));
  recvBytes = recv(socketFD, &recvBuffer, BUFFERSIZE, 0);
  if (recvBytes == -1)
  {
    return 6;
  }
  else if(recvBytes == 0)
  {
    fprintf(stderr, "Recieved 0 bytes\n");
    return 6;
  }
  else 
  {
    #ifdef DEBUG 
    printf("Recieved: %d bytes\n", recvBytes);
    #endif 
  }
  printf("%s\n", recvBuffer);
  char* operand;
  char* firstValue;
  char* secondValue;
  char delimValues[] = " ";

  operand = strtok(recvBuffer, delimValues);
  firstValue = strtok(NULL, delimValues);
  secondValue = strtok(NULL, "\n"); 
 
  if(operand[0] == 'f')
  {
    double firstFloatValue = atof(firstValue);
    double secondFloatValue = atof(secondValue);
    double floatResult;
    
    if(operand[1] == 'a')
    {
      floatResult = firstFloatValue + secondFloatValue;
      printf("%8.8g\n", floatResult);
    }
    else if(operand[1] == 'd')
    {
      floatResult = firstFloatValue / secondFloatValue;
      printf("%8.8g\n", floatResult);
    }

    else if(operand[1] == 'm')
    {
      floatResult = firstFloatValue * secondFloatValue;
      printf("%8.8g\n", floatResult);
    }

    else if(operand[1] == 's')
    {
      floatResult = firstFloatValue - secondFloatValue;
      printf("%8.8g\n", floatResult);
    }
    else
    {
      fprintf(stderr, "Operand not Applicable");
      return 7;
    }

    char floatBuffer[BUFFERSIZE];
    sprintf(floatBuffer, "%8.8g\n", floatResult);
    int floatLength = strlen(floatBuffer);
    send(socketFD, &floatBuffer, floatLength, 0);
  }
  else 
  {
    int firstIntValue = atoi(firstValue);
    int secondIntValue = atoi(secondValue);
    int intResult;
    

    if(operand[0] == 'a')
    {
      intResult = firstIntValue + secondIntValue;
      printf("%d\n", intResult);
    }
    else if(operand[0] == 's')
    {
      intResult = firstIntValue - secondIntValue;
      printf("%d\n", intResult);
    }

    else if(operand[0] == 'm')
    {
      intResult = firstIntValue * secondIntValue;
      printf("%d\n", intResult);
    }

    else if(operand[0] == 'd')
    {
      intResult = firstIntValue / secondIntValue;
      printf("%d\n", intResult);
    }
    else
    {
      fprintf(stderr, "Operand not Applicable");
      return 8;
    }
    
    char intBuffer[BUFFERSIZE];
    sprintf(intBuffer, "%d\n", intResult);
    int intLength = strlen(intBuffer);
    send(socketFD, &intBuffer, intLength, 0);
  }

  
  memset(&recvBuffer, 0, sizeof(recvBuffer));
  recvBytes = recv(socketFD, &recvBuffer, BUFFERSIZE, 0);
  if (recvBytes == -1)
  {
    return 6;
  }
  else if(recvBytes == 0)
  {
    fprintf(stderr, "Recieved 0 bytes\n");
    return 6;
  }
  else if(recvBytes == 1)
  {
    #ifdef DEBUG 
    printf("Recieved: %d bytes\n", recvBytes);
    #endif 
  }
  else
  {
    printf("%s\n", recvBuffer);
  }
  
  close(socketFD);

  return 0;
}