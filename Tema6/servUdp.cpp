#include <sys/types.h>
#include <Winsock2.h>
#include <windows.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)  
/* portul folosit */
#define PORT 3000
/* codul de eroare returnat de anumite apeluri */
extern int errno;

const char COMMANDS[100][100] = {
    {"create_file"},
    {"append"},
    {"delete_file"},
    {"create_regkey"},
    {"delete_regkey"},
    {"run"},
    {"http_download"},
    {"listdir"}
};
int numberOfCommands = 8;


int sd;
struct sockaddr_in server;
struct sockaddr_in client;

void InitWinsock(){
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

int openServer(){
   /* crearea unui socket */
  if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
    perror ("[server]Eroare la socket().\n");
    return 0;
  }
  /* pregatirea structurilor de date */
  bzero(&server, sizeof (server));
  bzero(&client, sizeof (client));
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;	
  server.sin_addr.s_addr = htonl(INADDR_ANY); /* acceptam orice adresa */
  server.sin_port = htons(PORT);  /* utilizam un port utilizator */
   
  /* atasam socketul */
  if (bind(sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1){
    perror("[server]Eroare la bind().\n");
    return 0;
  }
  return 1;
}

HANDLE _createFile(char* fileName, DWORD dwCreationDisposition){
  return CreateFile(
    fileName,               
    GENERIC_READ | GENERIC_WRITE,    
    0,                     
    NULL,                  
    dwCreationDisposition,
    FILE_ATTRIBUTE_NORMAL, 
    INVALID_HANDLE_VALUE
  ); 
}

BOOL _deleteFile(char* fileName){
  return DeleteFile(fileName); 
}

BOOL _writeFile(HANDLE hFile, PCSTR text, DWORD lpNumberOfBytesWritten){
  return WriteFile(
      hFile,
      text,
      (DWORD)strlen(text),
      &lpNumberOfBytesWritten,
      NULL
  );
}

DWORD _regCreateKeyEx(const char* regPath){
  DWORD disposition = 0;
  HKEY hKey;
  return RegCreateKeyEx(
    HKEY_LOCAL_MACHINE,
    regPath,
    0, 
    NULL, 
    REG_OPTION_NON_VOLATILE, 
    KEY_READ | KEY_WRITE,
    NULL,
    &hKey, 
    &disposition
  );
}

DWORD _regDeleteKeyEx(const char* regPath){
   HKEY hKey;
  if (RegOpenKeyEx(
      HKEY_LOCAL_MACHINE,
      0,
      0,
      KEY_READ,
      &hKey ) != 0
     ){
       printf("Cannot open reg key\n");
      return -1;
  }
  return RegDeleteKey(hKey, regPath);
}

int executeCommand(char* command){
  char * pch;
  char tmp[100];
  char commandParams[100][100];
  int numberOfParams = 0;
  strcpy(tmp, command);
  pch = strtok (tmp, " ");

  int commandFound = false;
  int commandIndex;
  for (int index = 0; index < numberOfCommands; index++){
    if (strcmp(COMMANDS[index], pch) == 0){
      commandFound = true;
      commandIndex = index;
      break;
    }
  }

  if (!commandFound){
    return 0; //command not found;
  }

  while (pch != NULL){
    strcpy(commandParams[numberOfParams++], pch);
    pch = strtok (NULL, " ");
  }

  if (commandIndex == 0){ // create file
    if (numberOfParams != 2) return 0;
    HANDLE hFile;
    if ((hFile = _createFile(commandParams[1], CREATE_ALWAYS)) == INVALID_HANDLE_VALUE) {
      printf("Cannot create file. Error code: %d", GetLastError());
      return -1;
    }
    CloseHandle(hFile);
  }

  if (commandIndex == 1){ // append to file
    if (numberOfParams != 3) return 0;
    HANDLE hFile;
    DWORD lpNumberOfBytesWritten;
    if ((hFile = _createFile(commandParams[1], OPEN_EXISTING)) == INVALID_HANDLE_VALUE) {
      printf("Cannot create_append file. Error code: %d", GetLastError());
      return -1;
    }


    // Set the file pointer to the end-of-file:
    DWORD dwMoved = SetFilePointer(hFile, 0l, nullptr, FILE_END);
    if ( dwMoved == INVALID_SET_FILE_POINTER ) {
        printf( "Terminal failure: Unable to set file pointer to end-of-file.\n" );
        return -1;
    }

    if (!_writeFile(hFile, commandParams[2], lpNumberOfBytesWritten)){
      printf("Cannot append file. Error code: %d", GetLastError());
      return -1;
    }

    CloseHandle(hFile);
  }

  if (commandIndex == 2){ // delete file
    if (numberOfParams != 2) return 0;
    
    if (!_deleteFile(commandParams[1])) {
      printf("Cannot delete file. Error code: %d", GetLastError());
      return -1;
    }
  }

  if (commandIndex == 3){ // create regex key
    if (numberOfParams != 2) return 0;
    
    if (_regCreateKeyEx(commandParams[1]) != 0) {
      printf("Cannot create reg key file. Error code: %d", GetLastError());
      return -1;
    }
  }

  if (commandIndex == 4){ // delete regex key
    if (numberOfParams != 2) return 0;
    
    if (_regDeleteKeyEx(commandParams[1]) != 0) {
      printf("Cannot create reg key file. Error code: %d", GetLastError());
      return -1;
    }
  }

  return 1;
}

int main ()
{ 
  InitWinsock();

  char command[100];
  char msgrasp[100]=" ";

 if(!openServer()) return errno;
  
  while (1){
    int msglen;
    int length = sizeof (client);

    printf ("[server]Astept la portul %d...\n",PORT);
    fflush(stdout);

    bzero(command, sizeof(command));

    /* citirea mesajului primit de la client */
    if ((msglen = recvfrom(sd, command, sizeof(command), 0, (struct sockaddr*) &client, &length)) <= 0){
      perror ("[server]Eroare la recvfrom() de la client.\n");
      return errno;
    }
    printf("[server]Mesajul a fost receptionat...%s\n", command);

    int executedCommand = executeCommand(command);
    bzero(msgrasp, sizeof(msgrasp));
    if (executedCommand == 1){
      strcat(msgrasp, "Comanda  a fost executata cu succes!");
    }
    if (executedCommand == -1){
      strcat(msgrasp, "Eroare! Comanda nu a putut fi executata!");
    } 
    if (executedCommand == 0){
      strcat(msgrasp, "Comanda \"");
      strcat(msgrasp, command);
      strcat(msgrasp, "\" este invalida!");
    }

    printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
    /* returnam mesajul clientului */
    if (sendto(sd, msgrasp, sizeof(msgrasp), 0, (struct sockaddr*) &client, length) <= 0){
      perror("[server]Eroare la sendto() catre client.\n");
      continue;		/* continuam sa ascultam */
    } else {
      printf("[server]Mesajul a fost trasmis cu succes.\n");
    }
  }		
}		

//create_file 123.txt 456.txt 13213.txt
//create_regkey SOFTWARE\\CSSO\\Tema61
//delete_regkey SOFTWARE\\CSSO\\Tema61