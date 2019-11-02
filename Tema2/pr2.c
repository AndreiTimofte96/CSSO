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

void showTree(){
    int noOfTree = 1;
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
    fflush(stdout);
}

int getNumberOfDescendets(DWORD _ppid){
    int counter = 1;
    for (int index = 0; index < pList.count; index++){
        DWORD pid = pList.procese[index].pid;
        DWORD ppid = pList.procese[index].ppid;
        char exeName[256];
        strcpy(exeName, pList.procese[index].exeName);

        if (ppid == _ppid){
            counter += getNumberOfDescendets(pid);
            // printf("%s %d\n", exeName, counter);
        }
    }
    return counter;
}

void showNumberOfDescendants(char* processName){
    for (int index = 0; index < pList.count; index++){
        DWORD pid = pList.procese[index].pid;
        char exeName[256];
        strcpy(exeName, pList.procese[index].exeName);

        if (strcmp(exeName, processName) == 0){
            printf("%s [%d] Nr descendeti: %d\n", 
                exeName, 
                pid, 
                getNumberOfDescendets(pid) - 1
            );
        }
    }
    fflush(stdout);
}

void closeAllProcessesWithRoot(DWORD _ppid){
    
}

void printOptions(){
    int inputOption = -1;
    // Sleep(1000);
    while(inputOption != 3){
        printf("\nOptiuni: \n");
        printf("1) Nume proces - Afisare nr de descendeti (1)\n");
        printf("2) PID proces - Inchide procese subarbore PID (2)\n");
        printf("3) Exit (3)\n\n");
        printf("Alege optiune: ");
        fflush(stdout);
        scanf("%d", &inputOption);

        printf("Optiune aleasa: %d\n", inputOption);

        switch(inputOption){
            case 1: {
                char processName[100];
                printf("Nume proces: ");
                fflush(stdout);
                scanf("%s", &processName);
                showNumberOfDescendants(processName);
                break;
            }
            case 2: {
                int pid;
                printf("PID: ");
                fflush(stdout);
                scanf("%d", &pid);
                closeAllProcessesWithRoot(pid);
                break;
            }
            case 3: {
                break;
            }
            default: 
                printf("Optiune invalida!\n");
        
        }
    }
}


int main()
{   
    HANDLE hData;
    if (!getProcessListFromMemory(hData)){
        return 0;
    };
    CloseHandle(hData);
    
    showTree();

    printOptions();

    return 0;
}