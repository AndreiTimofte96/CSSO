#include <sys/types.h>
#include <Winsock2.h>
#include <windows.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdbool.h>
#include <wininet.h>
#include <conio.h>
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
    {"listdir"},
    {"listdir_recursive"}
};
int numberOfCommands = 9;

using namespace std;

int sd;
struct sockaddr_in server;
struct sockaddr_in client;

char httpFileData[1000];
DWORD dwBytes;
int commandIndex;
char serverUrl[100];
char urlObject[100];
char directoryContent[100000];

struct ThreadArguments{
  struct sockaddr_in client;
  char command[100];
};

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

BOOL _createProcess(LPCSTR processPath){
  PROCESS_INFORMATION pi;
  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  return CreateProcess(processPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}

HINTERNET _internetOpen(){
    return InternetOpen(
            (LPCTSTR) "eu",
            INTERNET_OPEN_TYPE_PRECONFIG,
            NULL,
            NULL,
            0
        );
}

HINTERNET _internetConnect(HINTERNET hInternetOpen, LPCSTR serverUrl, DWORD port, LPCSTR user, LPCSTR password, DWORD dwService){
    return  InternetConnect(
        hInternetOpen,
        serverUrl,
        port,
        user,
        password,
        dwService,
        0,
        0
    );
}

HINTERNET _httpOpenRequest(HINTERNET hInternetConnect, LPCSTR urlObject) {
    const char *acceptTypes[] = {"text/*", NULL};
     return HttpOpenRequest(
        hInternetConnect,
        (LPCSTR) "GET",
        urlObject,
        (LPCSTR) "HTTP/1.1",
        NULL,
        acceptTypes,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_EXISTING_CONNECT,
        0
    );    
}

BOOL _httpSendRequest(HINTERNET hHttpOpenRequest){
      return HttpSendRequest(
        hHttpOpenRequest,
        NULL,
        0,
        NULL,
        0
    );
}

void extractServerObjectName(string url){
  size_t found = url.find_first_of(":");
  string host = url.substr(0,found);
  size_t found1 = url.find_first_of("/");
  string port = url.substr(found+1,found1-found-1);
  string resource = url.substr(found1);

  const char* _serverUrl = port.c_str();
  const char* _urlObject = resource.c_str();

  strcpy(serverUrl, _serverUrl);
  strcpy(urlObject, _urlObject);
}

bool readFileFromHTTPServer(const char* url){

  HINTERNET hInternetOpen, hInternetConnect, hHttpOpenRequest;
  BOOL bHttpSendRequest, bInternetFileRead;

  string stringUrl(url);
  extractServerObjectName(stringUrl);
  printf("111 %s\n", serverUrl);
  printf("222 %s\n", urlObject);

  if (!(hInternetOpen = _internetOpen())){
      printf("ERROR 1: %d\n", GetLastError());
      return false;
  }

  if (!(hInternetConnect = _internetConnect(hInternetOpen, serverUrl, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP))){
      printf("ERROR 2: %d\n", GetLastError());
      return false;
  }

  if (!(hHttpOpenRequest = _httpOpenRequest(hInternetConnect, urlObject))){
      printf("ERROR 3: %d\n", GetLastError());
      return false;
  }

  if (!(bHttpSendRequest = _httpSendRequest(hHttpOpenRequest))){
    printf("ERROR 4: %d\n", GetLastError());
    return false;
  }

  if (!(bInternetFileRead = InternetReadFile(hHttpOpenRequest, &httpFileData, sizeof(httpFileData), &dwBytes))){
      printf("ERROR 5: %d\n", GetLastError());
      return false;
  }
  
  InternetCloseHandle(hHttpOpenRequest);
  InternetCloseHandle(hInternetConnect);
  InternetCloseHandle(hInternetOpen);

  return true;
}

void listDirectory(char* path, bool recursive, int depth) {   
    printf("%s\n", path);
    WIN32_FIND_DATA find_data;
    char pathWithStar[50];
    strcpy(pathWithStar, path);
    strcat(pathWithStar, "\\*");
    HANDLE hDir = FindFirstFile(pathWithStar, &find_data);

    while(FindNextFile(hDir, &find_data)) {
      if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
          if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0){
              char spaces[10000];
              for (int index = 0; index < depth; index++){
                spaces[index] = ' ';
              }
              strcat(directoryContent, spaces);

              strcat(directoryContent, "Director: ");
              strcat(directoryContent, find_data.cFileName);
              strcat(directoryContent, "\n"); 
              
              if (recursive) {
                char newPath[50];
                strcpy(newPath, path);
                if (newPath[strlen(newPath) - 1] != '\\'){
                  strcat(newPath, "\\");
                }
                strcat(newPath, find_data.cFileName);
                strcat(newPath, "\\");
                listDirectory(newPath, recursive, depth + 5);
              }
          } 
      } else {
        char spaces[10000];
        for (int index = 0; index < depth; index++){
          spaces[index] = ' ';
        }
        strcat(directoryContent, spaces);

        strcat(directoryContent, "Fisier: ");
        strcat(directoryContent, find_data.cFileName);
        strcat(directoryContent, "\n");
      }
    }
    FindClose(hDir);
}

int executeCommand(char* command){
  char * pch;
  char tmp[100];
  char commandParams[100][100];
  int numberOfParams = 0;
  strcpy(tmp, command);
  pch = strtok (tmp, " ");

  int commandFound = false;
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

  char splitString[50];
  if(commandIndex == 1){
    strcpy(splitString, "\"");
  }else{
    strcpy(splitString, " ");
  }

  while (pch != NULL){
    strcpy(commandParams[numberOfParams++], pch);
    pch = strtok (NULL, splitString);
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

  if (commandIndex == 5){ //run command
    if (numberOfParams != 2) return 0;
    
    if(!_createProcess(commandParams[1])){
      printf("Cannot run command. Error code: %d", GetLastError());
      return -1;
    }
  }

  if (commandIndex == 6){ //http download command
    if (numberOfParams != 2) return 0;
      
    if (!readFileFromHTTPServer(commandParams[1])){
      printf("Cannot read HTPP file. Error code: %d", GetLastError());
      return -1;
    }
  }

  if(commandIndex == 7){ //listdir
    if (numberOfParams != 2) return 0;
    strcpy(directoryContent, "\n");
    listDirectory(commandParams[1], false, 0);
  }

  if(commandIndex == 8){ //listdir_recursive
    if (numberOfParams != 2) return 0;
    strcpy(directoryContent, "\n");
    listDirectory(commandParams[1], true, 0);
  }

  return 1;
}

DWORD WINAPI myThread(LPVOID lpParameter){
  struct ThreadArguments *threadArguments = (struct ThreadArguments *) lpParameter;
  char msgrasp[1000]=" ";

  int executedCommand = executeCommand(threadArguments->command);
  bzero(msgrasp, sizeof(msgrasp));
  if (executedCommand == 1){
    if (commandIndex == 6){
      strcat(msgrasp, httpFileData);
    }else{
      if (commandIndex == 7 || commandIndex == 8){
        strcat(msgrasp, directoryContent);
      }else{
        strcat(msgrasp, "Comanda a fost executata cu succes!");
      }
    }
  }

  if (executedCommand == -1){
    strcat(msgrasp, "Eroare! Comanda nu a putut fi executata!");
  } 

  if (executedCommand == 0){
    strcat(msgrasp, "Comanda \"");
    strcat(msgrasp, threadArguments->command);
    strcat(msgrasp, "\" este invalida!");
  }

  printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
  /* returnam mesajul clientului */
  int length = sizeof (threadArguments->client);
  if (sendto(sd, msgrasp, sizeof(msgrasp), 0, (struct sockaddr*) &threadArguments->client, length) <= 0){
    perror("[server]Eroare la sendto() catre client.\n");
    // continue;		/* continuam sa ascultam */
  } else {
    printf("[server]Mesajul a fost trasmis cu succes.\n");
  }
}

int main ()
{ 
  InitWinsock();

  char command[100];

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

    DWORD myThreadID;
    struct ThreadArguments *threadArguments;
    threadArguments->client = client;
    strcpy(threadArguments->command, command);
    HANDLE threadHandle = CreateThread(NULL, 0, myThread, threadArguments, 0, &myThreadID);

  }		
}		

// create_file 123.txt 456.txt 13213.txt
// create_regkey SOFTWARE\\CSSO\\Tema61
// delete_regkey SOFTWARE\\CSSO\\Tema61
 // run C:\Windows\system32\calc.exe
 // http_download students.info.uaic.ro/~teodor.timofte/CSSO/info1.txt
 // listdir C:\Users\antimofte\Documents\FAC\CSSO\Tema4
 // listdir_recursive C:\Users\antimofte\Documents\FAC\CSSO\Tema4