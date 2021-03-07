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
#include <sys/time.h>

using namespace std;

#define SERVPROTOCOL "TEXT TCP 1.0\n\n"
#define BUFFERSIZE 1450
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
  int reUse = 1;
  struct addrinfo hints;
  struct addrinfo* server;
  struct addrinfo* p;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  rv = getaddrinfo(servPort, servPort, &hints, &server);
  if(rv != 0)
  {
    fprintf(stderr, "Error: getaddrinfo: %s\n", gai_strerror(rv));
    return 2;
  }

  for(p = server; p != NULL; p = p->ai_next)
  {
    // Create Socket
    sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(sockFD == -1)
    {
      fprintf(stderr, "Error Socket Creation.\n");
      return 3;
    }

    // Socket options REUSE
    rv = setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &reUse, sizeof(reUse));
    if(rv == -1)
    {
      perror("Error SO_OPTIONS");
      exit(1);
    }
  }
  
  freeaddrinfo(server);

  // Create Server
  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(servPortInt);
  inet_aton(servHost, &servAddr.sin_addr);

  // Bind Socket and Server
  rv = bind(sockFD, (struct sockaddr*)&servAddr, sizeof(servAddr));
  if(rv == -1)
  {
    perror("Error Bind");
    return 4;
  }

  // Listen to incoming connections on IP:PORT
  #ifdef DEBUG
  printf("Listening to %s:%d\n\n", inet_ntoa(servAddr.sin_addr), servPortInt);
  #endif

  int clientBacklog = 5;
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
  char buff[BUFFERSIZE];

  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  // Loop to accept incoming client calls
  while(1)
  {
    printf("Waiting for new client...\n");

    clientSockFD = accept(sockFD, (sockaddr*)&clientAddr, &clientLen);
    if(clientSockFD == -1)
    {
      perror("Error Accepting client");
      continue;
    }

    #ifdef DEBUG
    printf("Client Accepted\n");
    #endif

    if(setsockopt(clientSockFD, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
      perror("sockopt");
      break;
    }

    // If client is accepted, enter a nested loop for communication
    while(1)
    { 
      memset(&buff, 0, sizeof(buff));
      sprintf(buff, SERVPROTOCOL);
      printf("%s", buff);
      
      // Send to Client
      sv = send(clientSockFD, &buff, sizeof(buff), 0);
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

      // Recieve from client
      memset(&buff, 0, sizeof(buff));
      rv = recv(clientSockFD, &buff, sizeof(buff), 0);
      if(rv == -1)
      {
        perror("Error Recieving");
        memset(&buff, 0, sizeof(buff));
        sprintf(buff, "ERROR TO\n");

        sv = send(clientSockFD, &buff, sizeof(buff), 0);
        break;
      }
      else if(rv == 0)
      {
        printf("Recieved 0 Bytes\n");
        break;
      }
      else 
      {
        printf("%s\n", buff);

        if(strcmp(buff, "OK\n") != 0)
        {
          printf("No matching protocol, disconnect client.\n");
          break;
        }
      }

      // Clear Buffer for new entries and Generate Numbers for Operation
      memset(&buff, 0, sizeof(buff));
      initCalcLib();
      char* typeHolder;
      typeHolder = randomType();

      int fInt;
      int sInt;
      int rInt;

      double fDouble;
      double sDouble;
      double rDouble;
      
      if(typeHolder[0] == 'f')
      {
        fDouble = randomFloat();
        sDouble = randomFloat();

        if(strcmp(typeHolder, "fadd") == 0)
        {
          rDouble = fDouble + sDouble;
        }
        else if(strcmp(typeHolder, "fsub") == 0)
        {
          rDouble = fDouble - sDouble;
        }
        else if(strcmp(typeHolder, "fmul") == 0)
        {
          rDouble = fDouble * sDouble;
        }
        else if(strcmp(typeHolder, "fdiv") == 0)
        {
          rDouble = fDouble / sDouble;
        }
        else 
        {
          printf("Error with typeHolder DOUBLE\n");
        }

        // Print Math operation on server side
        printf("%s %8.8g %8.8g\n", typeHolder, fDouble, sDouble);

        // Fill buffer with the operation for sending to client
        sprintf(buff, "%s %8.8g %8.8g\n", typeHolder, fDouble, sDouble);
      }
      else
      {
        fInt = randomInt();
        sInt = randomInt();

        if(strcmp(typeHolder, "add") == 0)
        {
          rInt = fInt + sInt;
        }
        else if(strcmp(typeHolder, "sub") == 0)
        {
          rInt = fInt - sInt;
        }
        else if(strcmp(typeHolder, "mul") == 0)
        {
          rInt = fInt * sInt;
        }
        else if(strcmp(typeHolder, "div") == 0)
        {
          rInt = fInt / sInt;
        }
        else 
        {
          printf("Error with typeHolder INT\n");
        }

        // Print Math operation on server side
        printf("%s %d %d\n", typeHolder, fInt, sInt);

        // Fill buffer with the operation for sending to client
        sprintf(buff, "%s %d %d\n", typeHolder, fInt, sInt);
      }

      sv = send(clientSockFD, &buff, sizeof(buff), 0);
      if(sv == -1)
      {
        perror("Error Sending");
        break;
      }
      else if(sv == 0)
      {
        printf("Sent 0 Bytes.\n");
        break;
      }

      memset(&buff, 0, sizeof(buff));
      rv = recv(clientSockFD, &buff, sizeof(buff), 0);
      if(rv == -1)
      {
        perror("Second Timeout");
        memset(&buff, 0, sizeof(buff));
        sprintf(buff, "ERROR TO\n");

        sv = send(clientSockFD, &buff, sizeof(buff), 0);
        break;
      }
      else if(rv == 0)
      {
        printf("Recv 0 bytes.\n");
        break;
      }
      else 
      {
        bool equal = false;
        
        if(typeHolder[0] == 'f')
        {
          double diff;
          double clientDouble = atof(buff);
          
          #ifdef DEBUG
          printf("client answer: %8.8g | server answer: %8.8g\n", clientDouble, rDouble);
          #endif

          diff = abs(clientDouble - rDouble);

          if(diff < 0.0001)
          {
            equal = true;
          }
          else 
          {
            equal = false;
          }
        }
        else
        {
          int clientInt = atoi(buff);
          
          #ifdef DEBUG
          printf("client answer: %d | server answer: %d\n", clientInt, rInt);
          #endif
          
          if(clientInt == rInt)
          {
            equal = true;
          }
          else 
          {
            equal = false;
          }
        }

        memset(&buff, 0, sizeof(buff));
        if(equal)
        {
          char validate[BUFFERSIZE] = "OK\n";
          sprintf(buff, "%s", validate);
        }
        else
        {
          char unvalidate[BUFFERSIZE] = "ERROR\n";
          sprintf(buff, "%s", unvalidate);
        }
        
        sv = send(clientSockFD, &buff, sizeof(buff), 0);
        if(sv == -1)
        {
          perror("Server Calc Response");
          break;
        }
        else if(sv == 0)
        {
          fprintf(stderr, "Sent 0 bytes\n");
          break;
        }
        else
        {
          printf("%s\n", buff);
        } 

        #ifdef DEBUG
        printf("Terminating Client.\n");
        #endif

        break;
      }
    }
    close(clientSockFD);
  }
  close(sockFD); 
  return 0;
}