#include <windows.h>
#include <stdbool.h>
#include <stdio.h>

void recursiveFct(char* path) 
{
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
                char newPath[50];
                strcpy(newPath, path);
                strcat(newPath, find_data.cFileName);
                strcat(newPath, "/");
                recursiveFct(newPath);
            }
        }
        else {
            printf("FISIER: %s%s\n", path, find_data.cFileName);
        }
    }
    FindClose(hDir);
}

int main()
{   
    printf("-----------------------------------\n\n");

    LPSTR path = "C:/Users/iziwork/Desktop/123/";
    recursiveFct(path);

    printf("-----------------------------------\n\n");

    return 0;
}