// teodor.timofte@fenrir.info.uaic.ro
// http://students.info.uaic.ro/~teodor.timofte/CSSO/info.txt
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <wininet.h>

#define CMD_PUT "PUT"
#define CMD_RUN "RUN"
#define _CRT_SECURE_NO_WARNINGS

HINTERNET hInternetOpen, hInternetConnect, hHttpOpenRequest;
HINTERNET hFTPInternetConnect;
BOOL bHttpSendRequest, bInternetFileRead;
BOOL bFtpPutFile;
char fileData[1000];
DWORD dwBytes;

struct {
    int noOfInstructions;
    char* serverUrl;
    char* user;
    char* password;
    char* commands[100];
} parsedData;

HINTERNET _internetOpen(){
    return InternetOpen(
            (LPCTSTR) "eu",
            INTERNET_OPEN_TYPE_PRECONFIG,
            NULL,
            NULL,
            0
        );
}

LPCSTR serverUrl = "students.info.uaic.ro" ;
// LPCSTR urlObject = "~teodor.timofte/CSSO/info1.txt";
LPCSTR urlObject = "~teodor.timofte/CSSO/info.txt";
LPCSTR ftpPath = "C:\\Users\\antimofte\\Documents\\FAC\\CSSO\\Tema4\\ftpServer\\demo";


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

bool readFileFromHTTPServer(LPCSTR serverUrl, LPCSTR urlObject){

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

     if (!(bInternetFileRead = InternetReadFile(hHttpOpenRequest, &fileData, sizeof(fileData), &dwBytes))){
        printf("ERROR 5: %d\n", GetLastError());
        return false;
    }
   
    return true;
}

void parseFileData(){
    char * pch;
    parsedData.noOfInstructions = atoi(strtok (fileData, "\n"));
    parsedData.serverUrl = strtok (NULL, "\n");
    parsedData.user = strtok (NULL, "\n");
    parsedData.password = strtok (NULL, "\n");
    for (int index = 0; index < parsedData.noOfInstructions; index++){
        parsedData.commands[index] = strtok (NULL, "\n");
    }

    printf("%d\n", parsedData.noOfInstructions);
    printf("%s\n", parsedData.serverUrl);
    printf("%s\n", parsedData.user);
    printf("%s\n", parsedData.password);
    for (int index = 0; index < parsedData.noOfInstructions; index++){
        printf("%s\n", parsedData.commands[index]);
    }
}

BOOL _ftpPutFile(HINTERNET hFTPInternetConnect, LPCSTR filePath, LPCSTR fileName){
    return FtpPutFileA(
        hFTPInternetConnect,
        filePath,
        fileName,
        FTP_TRANSFER_TYPE_BINARY,
        12
    );
}

BOOL _ftpGetFile(HINTERNET hFTPInternetConnect, LPCSTR ftpFilePath, LPCSTR localPath){
    return FtpGetFile(
        hFTPInternetConnect,
        ftpFilePath,
        localPath,
        FALSE, 
        FILE_ATTRIBUTE_NORMAL,
        FTP_TRANSFER_TYPE_ASCII,
        0
    );
}

bool _createProcess(LPCSTR processPath){
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

     if (!CreateProcess(processPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("[PR1]: Cannot create process %d.\n", GetLastError());
        return false;
    }
    fflush(stdout);
    return true;
}

void _internetGetLastResponseInfo(){
        DWORD error = GetLastError();
        printf("Error: %d\n", error);
        DWORD dwInetError;
        DWORD dwExtLength = 1000;
        TCHAR *szExtErrMsg = NULL;
        TCHAR errmsg[1000];
        szExtErrMsg = errmsg;
        int returned = InternetGetLastResponseInfo( &dwInetError, szExtErrMsg, &dwExtLength );
        printf("dwInetError: %d  Returned: %d\n %s\n", dwInetError, returned, szExtErrMsg);
}

bool uploadFilesToFTPServer(){
    if (!(hFTPInternetConnect = _internetConnect(hInternetOpen, parsedData.serverUrl, 21, parsedData.user, parsedData.password, INTERNET_SERVICE_FTP))){
        printf("ERROR 6: %d\n", GetLastError());
        return false;
    }
    for (int index = 0; index < parsedData.noOfInstructions; index++){
        char *cmdType, *cmdVal;
        cmdType = strtok (parsedData.commands[index], " ");
        cmdVal = strtok (NULL, " ");
        if (strcmp(cmdType, CMD_PUT) == 0){ //PUT
            char *fileName, *pch;
            char cmdValCopy[100];
            strcpy(cmdValCopy, cmdVal);
            pch = strtok (cmdValCopy, "\\");
             while (pch != NULL){
                fileName = pch;
                pch = strtok (NULL, "\\");
            } // get file name

             if (!(bFtpPutFile = _ftpPutFile(hFTPInternetConnect, cmdVal, fileName))){
                printf("ERROR 7: %d\n %d", GetLastError());
                _internetGetLastResponseInfo();
                return false;
            }
        } 

        if (strcmp(cmdType, CMD_RUN) == 0){ //RUN
            char filePath[100];
            strcpy(filePath, ftpPath);
            strcat(filePath, "\\");
            strcat(filePath, cmdVal);
            printf("%s\n", filePath);
            
            //  if (!(bFtpPutFile = _ftpGetFile(hFTPInternetConnect, filePath, "./"))){
            //     printf("ERROR 8: %d\n %d", GetLastError());
            //     _internetGetLastResponseInfo();
            //     return false;
            // }

             if(!_createProcess(filePath)){
               printf("ERROR 9: %d\n %d", GetLastError());
                return false;
            }
        } 
    }
    return true;
}

void closeAllHandles(){
    InternetCloseHandle(hFTPInternetConnect);
    InternetCloseHandle(hHttpOpenRequest);
    InternetCloseHandle(hInternetConnect);
    InternetCloseHandle(hInternetOpen);
}

int main()
{   
    if (!readFileFromHTTPServer(serverUrl, urlObject)){
         closeAllHandles();
        return 0;
    }
    parseFileData();

    if (!uploadFilesToFTPServer()){
        closeAllHandles();
        return 0;
    };
    closeAllHandles();
    return 0;
}