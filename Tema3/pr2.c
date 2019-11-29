#include <Windows.h>
#include <stdio.h>

#define _CRT_SECURE_NO_WARNINGS
#define NO_OF_ITT 100
#define MAPPED_FILE_NAME "tema3"
#define GENERATE_EVENT_NAME "GenerateNumbers"
#define VERIFY_EVENT_NAME "VerifyNumbers"

struct Numbers {
  int randomA;
  int computedB;
} nToRead;
HANDLE hVerify, hGenerate, hData, hSetSignal;

bool readNumbers(){
    // READ FROM p1 MAPPED FILE
    HANDLE hData = OpenFileMapping(
        FILE_MAP_ALL_ACCESS, 
        FALSE, 
        MAPPED_FILE_NAME
    );

    if (hData == NULL) {
        printf("[PR2]: Cannot create file mapping. Error code: %d", GetLastError());
        return false;
    }

    unsigned char* pData = (unsigned char*) MapViewOfFile(
        hData, 
        FILE_MAP_READ, 
        0, 0, 0);

    if (pData == NULL) {
        printf("[PR2]: Cannot get pointer to file mapping. Error code: %d\n", GetLastError());
        CloseHandle(hData);
        return false;
    }

    memcpy(&nToRead, pData, sizeof(Numbers));

    return true;
}

void printVerifyResult(){
    if (nToRead.randomA * 2 == nToRead.computedB){
        printf("[PR2]: CORECT\n\n");
    } else{
         printf("[PR2]: INCORECT\n\n");
    }
    fflush(stdout);
}

bool setSignalEvent(LPCSTR eventName){
    hVerify = OpenEvent(
        EVENT_MODIFY_STATE,
        FALSE,
        eventName
    );

    if (!SetEvent(hVerify)) {
        printf("[PR1]: SetEvent failed (%d)\n", GetLastError());
        return false;
    }
    return true;
}

void waitSignalEvent(LPCSTR eventName){
    hGenerate = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        eventName
    );
    WaitForSingleObject(hGenerate, INFINITE);
}


HANDLE openEvent(LPCSTR eventName){
    return OpenEvent(
        EVENT_MODIFY_STATE,
        FALSE,
        eventName
    );
}


int main()
{    
    if (!(hVerify = openEvent((LPCSTR) VERIFY_EVENT_NAME))){
        printf("[PR1]: Cannot create event %d.\n", GetLastError());
        return 0;
    }
    if(!(hGenerate = openEvent((LPCSTR) GENERATE_EVENT_NAME))){
        printf("[PR1]: Cannot create event %d.\n", GetLastError());
        return 0;
    }

    for (int index = 0; index < NO_OF_ITT; index++){
        waitSignalEvent((LPCSTR) VERIFY_EVENT_NAME);
                
        if(!readNumbers()){
            return 0;
        }

        printVerifyResult();
        if(!setSignalEvent((LPCSTR) GENERATE_EVENT_NAME)){
            return 0;
        }
    }
    
    CloseHandle(hVerify);
    CloseHandle(hGenerate);
    CloseHandle(hData);

    return 0;
}