// ./cliUdp 127.0.0.1 3000 --help
#include <sys/types.h>
#include <Winsock2.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)  

extern int errno;
int port;

const char HELPCOMMAND[50][50] = {
    {"create_file [FILE]"},
    {"append [FILE] [TEXT]"},
    {"delete_file [FILE]"},
    {"create_regkey [REG_KEY]"},
    {"delete_regkey [REG_KEY]"},
    {"run [FILE_PATH]"},
    {"http_download [URL]"},
    {"listdir [DIR_PATH]"},
    {"listdir_recursive [DIR_PATH]"}
};
int numberOfCommands = 9;

int sd;			// descriptorul de socket
struct sockaddr_in server;	// structura folosita pentru conectare 
struct sockaddr_in client;

void InitWinsock(){
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

using namespace std;

int connectToServer(char *argv[]){
   /* stabilim portul */
  port = atoi (argv[2]);
  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror ("Eroare la socket().\n");
    return 0;
  }
  /* umplem structura folosita pentru realizarea dialogului cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]); /* adresa IP a serverului */
  server.sin_port = htons(port);/* portul de conectare */
  return 1;
}

int main (int argc, char *argv[])
{
  InitWinsock();

  char command[100];		// mesajul trimis
  char serverRes[100];
  int commandLen = 0, serverResLen = 0, serverLength = 0;

  if (argc < 3){
    printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }
  if (argc == 4 && strcmp(argv[3], "--help") == 0){
    for (int index = 0; index < numberOfCommands; index++){
      printf("%s\n", HELPCOMMAND[index]);
    }
    printf("\n");
  }

  if(!connectToServer(argv)) return errno;

  while(1){
      /* citirea mesajului */
      bzero(command, sizeof(command));
      bzero(serverRes, sizeof(serverRes));
      printf("[client]Introduceti o comanda: ");
      fflush(stdout);
      fgets(command, sizeof(command), stdin);
      command[strlen(command) - 1] = '\0';
      
      /* trimiterea mesajului la server */
      serverLength = sizeof(server);
      // printf("%d len %s\n", strlen(command), command);
      if (sendto(sd, command, strlen(command), 0, (struct sockaddr*)&server, serverLength) <= 0){
        perror("[client]Eroare la sendto() spre server.\n");
        return errno;
      }

      /* citirea raspunsului dat de server 
        (apel blocant pina cind serverul raspunde) */
      if ((serverResLen = recvfrom(sd, serverRes, sizeof(serverRes), 0, (struct sockaddr*)&server, &serverLength)) < 0){
        perror("[client]Eroare la recvfrom() de la server.\n");
        return errno;
      }
      /* afisam mesajul primit */
      printf("[client]Mesajul primit este: %s\n", serverRes);
  }
  /* inchidem conexiunea, am terminat */
  close(sd);
}