#
#define _CRT_SECURE_NO_WARNINGS
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


bool setSEPrivilege(LPCSTR lpszPrivilege) {
    //https://support.microsoft.com/en-us/help/131065/how-to-obtain-a-handle-to-any-process-with-sedebugprivilege
    HANDLE hProcess;
    HANDLE hToken;
    LUID privilegeId;
    TOKEN_PRIVILEGES tp;

    if(!OpenProcessToken(
        GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
        &hToken)
    ) {
        if (GetLastError() == ERROR_NO_TOKEN) {
            printf("ERROR_2: OpenProcessToken_ERROR\n");
            return false;
         }
        printf("ERROR_3 \n");
        return false;
     }

    if (!LookupPrivilegeValue(
        NULL,
        lpszPrivilege,
        &privilegeId
    )){ 
        printf("ERROR_4 %d\n", GetLastError());
        return false;
    }

    //setam structura de privilegii ale tokenului;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = privilegeId;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(
        hToken, 
        FALSE, 
        &tp, 
        sizeof(TOKEN_PRIVILEGES), 
        (PTOKEN_PRIVILEGES) NULL, 
        (PDWORD) NULL) 
    ){ 
          printf("ERROR_5 AdjustTokenPrivileges error: %u\n", GetLastError() ); 
          return false; 
    } 

    // CloseHandle(hToken);
    printf("SE_DEBUG_NAME setat!\n");

    return true;
}

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


bool closeOneProcess(DWORD pid){
    HANDLE hProcess;
    if((hProcess = OpenProcess(
        PROCESS_ALL_ACCESS,
        FALSE,
        pid)) == NULL
    ){
        printf("OpenProcessERROR\n");
        return false;
    }

    //aici putem da disable la SE_DEBUG_MODE

    
    if(!TerminateProcess(hProcess, 0xffffffff)){
        printf("TerminateProcessERROR\n");
        return false;
    }

    CloseHandle(hProcess);
    printf("Procesul cu pid [%d] a fost terminat.\n", pid);
    return true;
}

void closeAllProcesses(DWORD _ppid){

    if (!closeOneProcess(_ppid)){
        printf("ERROR_7, nu s-a putut inchide procesul cu PID: %d\n", _ppid);
    }

    for (int index = 0; index < pList.count; index++){
        DWORD pid = pList.procese[index].pid;
        DWORD ppid = pList.procese[index].ppid;
        char exeName[256];
        strcpy(exeName, pList.procese[index].exeName);

        if (ppid == _ppid){
            closeAllProcesses(pid);
        }
    }
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
                fflush(stdout);
                break;
            }
            case 2: {
                int pid;
                printf("PID: ");
                fflush(stdout);
                scanf("%d", &pid);
                closeAllProcesses(pid);
                printf("\n");
                fflush(stdout);
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
    if (!setSEPrivilege((LPCSTR) "SeDebugPrivilege")){
        printf("ERROR_1\n");
        return 0;       
    }

    HANDLE hData;
    if (!getProcessListFromMemory(hData)){
        printf("ERROR_6\n");
        return 0;
    };
    CloseHandle(hData);
    
    showTree();

    printOptions();

    return 0;
}