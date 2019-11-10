#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define MIN_N 1
#define MAX_N 1000
#define NO_OF_ITT 1


struct Numbers {
  int randomA;
  int computedB;
};

Numbers generateTheNumbers(){
    Numbers n;
    n.randomA = rand() % MAX_N + MIN_N;
    n.computedB = 2 * n.randomA;
    return n;
}


bool writeNumbers(Numbers nToWrite){
    // WRITE TO FILE the two numbers;
    HANDLE hData = CreateFileMapping(
        INVALID_HANDLE_VALUE, 
        NULL, 
        PAGE_READWRITE, 
        0, 
        sizeof(Numbers), 
        "tema3"
    );

    if (hData == NULL) {
        printf("[PR1]: Cannot create file mapping. Error code: %d", GetLastError());
        return false;
    }

    unsigned char* pData = (unsigned char*) 
        MapViewOfFile(hData,  FILE_MAP_WRITE, 0, 0, 0);

    if (pData == NULL) {
        printf("[PR1]: Cannot get pointer to file mapping. Error code: %d", GetLastError());
        CloseHandle(hData);
        return false;
    }
    memcpy(pData, &nToWrite, sizeof(Numbers));

    fflush(stdout);
    CloseHandle(hData);
    return true;
}

void printNumbers(Numbers nToWrite){
    printf("[PR1]: a=%d b=%d\n", nToWrite.randomA, nToWrite.computedB);
    fflush(stdout);
}

void setVerifySignalEvent(){
    LPCSTR verifyEventName = "VerifyNumbers";

    HANDLE hVerify = OpenEvent(
        EVENT_MODIFY_STATE,
        FALSE,
        verifyEventName
    );

    if (! SetEvent(hVerify)) {
        printf("[PR1]: SetEvent failed (%d)\n", GetLastError());
        return;
    }
    CloseHandle(hVerify);

}

void waitGenerateSignalEvent(){
    LPCSTR generateEventName = "GenerateNumbers";
    HANDLE hGenerate = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        generateEventName
    );
    WaitForSingleObject(hGenerate, INFINITE);
    //ACUM TRECEM MAI DEPARTE SI GENERAM UN NOU SET DE NUMERE;
    CloseHandle(hGenerate);
}

int main()
{   
    srand(time(NULL)); 

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

     if (!CreateProcess("./pr2.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("[PR1]: Cannot create process %d.\n", GetLastError());
        return 0;
    }
    printf("[PR1]: PROCESS PR2 CREAT!\n");
    fflush(stdout);
    sleep(1);


    for (int index = 0; index < NO_OF_ITT; index++){
        Numbers nToWrite;
        nToWrite = generateTheNumbers();
        if (!writeNumbers(nToWrite)){
            printf("[PR1]: ERROR_1\n");
            return 0;
        }
        printNumbers(nToWrite);

        setVerifySignalEvent();

        waitGenerateSignalEvent();
    }

    return 0;
}