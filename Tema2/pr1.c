#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <tlhelp32.h>


struct InfoProces {
  DWORD pid;
  DWORD ppid;
  char exeName[256];
};

struct ProcessList {
  int         count;
  InfoProces  procese[2048];
};

int main()
{
    HANDLE hProcess, hProcessSnap;
    PROCESSENTRY32 pe32;
    ProcessList pList;

    //cer un snapshot la procese
    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( hProcessSnap == INVALID_HANDLE_VALUE ){
        printf("CreateToolhelp32Snapshot failed.err = %d \n",GetLastError());
        return( -1 );
    }

    //initializez dwSize cu dimensiunea structurii.
    pe32.dwSize = sizeof( PROCESSENTRY32 );

    //obtin informatii despre primul proces
    if( !Process32First( hProcessSnap, &pe32 ) ){
        printf("Process32First failed. err = %d \n", GetLastError() );
        CloseHandle( hProcessSnap ); //inchidem snapshot-ul
        return( -1 );
    }

    do{
        //afisez pid-ul si executabilul
        printf("Process [%d] with parent [%d]: %s \n",
        pe32.th32ProcessID,  pe32.th32ParentProcessID, pe32.szExeFile);

        InfoProces ip;
        ip.pid = pe32.th32ProcessID;
        ip.ppid = pe32.th32ParentProcessID;
        strcpy(ip.exeName, pe32.szExeFile);
        pList.procese[pList.count] = ip;
        pList.count += 1;
    }
    while(Process32Next(hProcessSnap, &pe32 )); //trec la urmatorul proces
    printf("Process Counter: %d\n", pList.count);

    
    //inched handle-ul catre snapshot
    CloseHandle(hProcess);



    // WRITE TO FILE pList
    HANDLE hData = CreateFileMapping(
        INVALID_HANDLE_VALUE, 
        NULL, 
        PAGE_READWRITE, 
        0, 
        sizeof(ProcessList), 
        "pr1"
    );

    if (hData == NULL) {
        printf("Cannot create file mapping. Error code: %d", GetLastError());
        return 0;
    }

    unsigned char* pData = (unsigned char*) MapViewOfFile(
        hData, 
        FILE_MAP_WRITE, 
        0, 0, 0);

    if (pData == NULL) {
        printf("Cannot get pointer to file mapping. Error code: %d", GetLastError());
        CloseHandle(hData);
        return 0;
    }

    memcpy(pData, &pList, sizeof(ProcessList));
    

    char ch;
    printf("Enter any character \n");
    scanf("%c", &ch);
    CloseHandle(hData);

    return 0;
}