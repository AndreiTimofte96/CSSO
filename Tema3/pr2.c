#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>

struct Numbers {
  int randomA;
  int computedB;
} nToRead;


bool readNumbers(){
    // READ FROM p1 MAPPED FILE
    HANDLE hData = OpenFileMapping(
        FILE_MAP_ALL_ACCESS, 
        FALSE, 
        "tema3"
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
        printf("[PR2]: Cannot get pointer to file mapping. Error code: %d", GetLastError());
        CloseHandle(hData);
        return false;
    }

    memcpy(&nToRead, pData, sizeof(Numbers));

    return true;
}

void printResult(){

    printf("%d %d", nToRead.randomA, nToRead.computedB);
    if (nToRead.randomA == 2 * nToRead.computedB){
        printf("[PR2]: CORECT\n\n");
    } else{
         printf("[PR2]: INCORECT\n\n");
    }
    fflush(stdout);
}

bool setGenerateSignalEvent(){
    LPCSTR generateEventName = "GenerateNumbers";

    HANDLE hGenerate = OpenEvent(
        EVENT_MODIFY_STATE,
        FALSE,
        generateEventName
    );
    if (! SetEvent(hGenerate)) {
        printf("[PR2]: SetEvent failed (%d)\n", GetLastError());
        return false;
    }
    CloseHandle(hGenerate);
    return true;
}

void waitVerifySignalEvent(){
    LPCSTR verifyEventName = "VerifyNumbers";
    HANDLE hVerify = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        verifyEventName
    );
    WaitForSingleObject(hVerify, INFINITE);
    //ACUM TRECEM MAI DEPARTE SI CITIM NUMERELE DIN PR1;
    CloseHandle(hVerify);
}

int main()
{   

    printf("[PR2] INITIALIZARE\n");
    fflush(stdout);
    // while(1){
        waitVerifySignalEvent();
            
        if(!readNumbers()){
            return 0;
        }
        printResult();
        if(!setGenerateSignalEvent()){
            return 0;
        }
    // }

    return 0;
}