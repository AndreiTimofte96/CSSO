#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define _CRT_SECURE_NO_WARNINGS
#define MIN_N 1
#define MAX_N 1000
#define NO_OF_ITT 100
#define MAPPED_FILE_NAME "tema3"
#define GENERATE_EVENT_NAME "GenerateNumbers"
#define VERIFY_EVENT_NAME "VerifyNumbers"



struct Numbers {
  int randomA;
  int computedB;
};

Numbers generateNumbers(){
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
        MAPPED_FILE_NAME
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
    // CloseHandle(hData);
    return true;
}

void printNumbers(Numbers nToWrite, int index){
    printf("IT: %d [PR1]: a=%d b=%d ", index,  nToWrite.randomA, nToWrite.computedB);
    fflush(stdout);
}


bool setSignalEvent(LPCSTR eventName){
    HANDLE hSetSignal = OpenEvent(
        EVENT_MODIFY_STATE,
        FALSE,
        eventName
    );

    if (!SetEvent(hSetSignal)) {
        printf("[PR1]: SetEvent failed (%d)\n", GetLastError());
        return false;
    }
    
    CloseHandle(hSetSignal);
    return true;
}

void waitSignalEvent(LPCSTR eventName){
    HANDLE hGenerate = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        eventName
    );
    WaitForSingleObject(hGenerate, INFINITE);

    //ACUM TRECEM MAI DEPARTE
    CloseHandle(hGenerate);
}

bool createProcess(LPCSTR processPath){
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

     if (!CreateProcess(processPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("[PR1]: Cannot create process %d.\n", GetLastError());
        return false;
    }
    // printf("[PR1]: PROCESS PR2 CREAT!\n");
    fflush(stdout);
    sleep(1);
    return true;
}

int main()
{   
    srand(time(NULL)); 

    if(!createProcess((LPCSTR) "./pr2.exe")){
        printf("[PR1]: ERROR_2\n");
        fflush(stdout);
        return 0;
    }

    for (int index = 0; index < NO_OF_ITT; index++){
        Numbers nToWrite = generateNumbers();
        if (!writeNumbers(nToWrite)){
            printf("[PR1]: ERROR_1\n");
            fflush(stdout);
            return 0;
        }
        printNumbers(nToWrite, index + 1);        

        if(!setSignalEvent((LPCSTR) VERIFY_EVENT_NAME)){
            printf("[PR1]: ERROR_3\n");
            return 0;
        }

        waitSignalEvent((LPCSTR) GENERATE_EVENT_NAME);
    }

    return 0;
}