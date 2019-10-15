
// make run ARG="C:/Users/iziwork/Desktop/CSSO/Tema1/"
// # C:\Users\iziwork\Desktop\CSSO\Tema1\main.exe ${ARG}

#include <windows.h>
#include <stdbool.h>
#include <stdio.h>

void recursiveFct(char* path, const char* regPath) 
{   
    HKEY hKey;
    DWORD disposition = 0;
    if (RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        regPath,
        0, 
        NULL, 
        REG_OPTION_NON_VOLATILE, 
        KEY_WRITE,
        NULL,
        &hKey, 
        &disposition) != ERROR_SUCCESS) {
        printf("Nu s-a putut creea cheia. Cod eroare: %d\n", GetLastError());  
        return;
    }


    WIN32_FIND_DATA find_data;
    char pathWithStar[50];
    strcpy(pathWithStar, path);
    strcat(pathWithStar, "*");
    HANDLE hDir = FindFirstFile(pathWithStar, &find_data);
    // printf("DIRECTOR: %s\n", find_data.cFileName);

    while(FindNextFile(hDir, &find_data)) {
        
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            
            if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0){
                printf("DIRECTOR: %s%s\n", path, find_data.cFileName);    

                char newRegPath[50];
                strcpy(newRegPath, regPath);
                strcat(newRegPath, "\\");
                strcat(newRegPath, find_data.cFileName);

                char newPath[50];
                strcpy(newPath, path);
                strcat(newPath, find_data.cFileName);
                strcat(newPath, "/");


                recursiveFct(newPath, newRegPath);
            }
        }
        else {
             RegSetValueEx(
                hKey, 
                find_data.cFileName, 
                0, 
                REG_DWORD,
                (const BYTE*)&find_data.nFileSizeLow, 
                sizeof(DWORD)
            );
            printf("FISIER: %s%s %d\n", path, find_data.cFileName, find_data.nFileSizeLow);
        }
    }

    RegCloseKey(hKey);
    FindClose(hDir);
}

int main(int argc, char* argv[]) {   
    if (argc != 2){
        printf("Numar invalid de argumente!\n");
        return 0;    
    }
    LPSTR path = argv[1];
    const char* regPath = "SOFTWARE\\CSSO";
    if (path[strlen(path) - 1] != '/'){
        strcat(path, "/");
    }

    printf("-----------------------------------\n\n");

    recursiveFct(path, regPath);

    printf("-----------------------------------\n\n");

    return 0;
}



