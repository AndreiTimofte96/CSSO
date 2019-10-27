#define _CRT_SECURE_NO_WARNINGS
#define SE_DEBUG_NAME
#include <Windows.h>
#include <stdio.h>
#include <tlhelp32.h>


struct InfoProces {
  DWORD pid;
  DWORD ppid;
  char  exeName[256];
};

struct ProcessList {
  int         count;
  InfoProces  procese[2048];
} pList;

int frecv[2048];

bool getProcessListFromMemory(HANDLE hData){
    // READ FROM p1 MAPPED FILE
    hData = OpenFileMapping(
        FILE_MAP_ALL_ACCESS, 
        FALSE, 
        "pr1"
    );

    if (hData == NULL) {
        printf("Cannot create file mapping. Error code: %d", GetLastError());
        return false;
    }

    unsigned char* pData = (unsigned char*) MapViewOfFile(
        hData, 
        FILE_MAP_READ, 
        0, 0, 0);

    if (pData == NULL) {
        printf("Cannot get pointer to file mapping. Error code: %d", GetLastError());
        CloseHandle(hData);
        return false;
    }

    memcpy(&pList, pData, sizeof(ProcessList));
    // printf("%d\n", pList.count);
    // for (int index = 0; index < pList.count; index++){
    //     printf("%d %d %s\n", pList.procese[index].pid, pList.procese[index].ppid, pList.procese[index].exeName);
    // }

    return true;
}

void printProcess(DWORD pid, DWORD ppid, char* exeName, int depth){
    for (int index = 0; index < depth; index++){
        printf(" ");
    }
    // printf("%s: P:[%d] PP:[%d]\n", exeName, pid, ppid);
    printf("%s: [%d]\n", exeName, pid);
}

void showRecursiveTree(DWORD _ppid, int depth){
    for (int index = 0; index < pList.count; index++){
        DWORD pid = pList.procese[index].pid;
        DWORD ppid = pList.procese[index].ppid;
        char exeName[256];
        strcpy(exeName, pList.procese[index].exeName);

        if (ppid == _ppid && !frecv[index]){
            frecv[index] = 1;
            printProcess(pid, ppid, exeName, depth);
            showRecursiveTree(pid, depth + 5);
        }
    }
    return;
}

int main()
{   
    HANDLE hData;
    int noOfTree = 1;
    if (!getProcessListFromMemory(hData)){
        return 0;
    };
    CloseHandle(hData);
 
    for (int index = 0; index < pList.count; index++){
        if (!frecv[index]){
            frecv[index] = 1;

            DWORD pid = pList.procese[index].pid;
            DWORD ppid = pList.procese[index].ppid;
            char exeName[256];
            strcpy(exeName, pList.procese[index].exeName);

            printf("\n\n----------------[arbore nr. %d]----------------\n", noOfTree++);
            printProcess(pid, ppid, exeName, 0);
            showRecursiveTree(pid, 5);
        }
    }
    return 0;
}