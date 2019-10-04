#include <windows.h>
#include <stdbool.h>
#include <stdio.h>

void f(DWORD* dw) {

}

void g(LPDWORD dw) {

}

int main() {
  BYTE b;     // unsigned char b;
  WORD w;     // unsigned short w;
  DWORD dw;   // unsigned int dw;
  TCHAR c;    // char c;
  
  LPSTR s = "adasdads";     // char*
  LPCSTR s2 = "sdadsdasd";  // const char*
  
  HANDLE hFile = CreateFile(
    "a.txt",                // lpFileName
    GENERIC_READ | GENERIC_WRITE,           // dwDesiredAccess
    0,                      // dwShareMode
    NULL,                   // lpSecurityAttributes
    OPEN_EXISTING,          // dwCreationDisposition
    FILE_ATTRIBUTE_NORMAL,  // dwFlagsAndAttributes
    INVALID_HANDLE_VALUE);  // hTemplateFile
    
  if (hFile == INVALID_HANDLE_VALUE) {
    printf("Cannot open file. Error code: %d", GetLastError());
  } else {
    char text[10];
    DWORD citite;
    while (true) {
      ReadFile(hFile, text, 1, &citite, NULL);
      if (citite == 0) {
        break;
      }
      text[citite] = '\0';
      printf("%s", text);
    }

   PCSTR toWriteFLAG = "\nANA ARE MERE";
   DWORD bytesToWriteNo = (DWORD)strlen(toWriteFLAG);
   DWORD scrise;
   BOOL hWrite = WriteFile(
      hFile,
      toWriteFLAG,
      bytesToWriteNo,
      &scrise,
      NULL
   );

   if (!toWriteFLAG){
      printf("Cannot write in file. Error code: %d", GetLastError());
   }

    CloseHandle(hFile);
    printf("\n");
  }

  WIN32_FIND_DATA find_data;
  HANDLE hDir = FindFirstFile("C:/Users/iziwork/Desktop/CSSO/*", &find_data); 
  printf("%s %d\n", find_data.cFileName, find_data.dwFileAttributes);

  while (FindNextFile(hDir, &find_data)) {

    printf("%s %d", find_data.cFileName, find_data.dwFileAttributes);
    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY != 0){
       printf("DIRECTOR\n");
    } else{
       printf("FISIER\n");
    }
  }

  FindClose(hDir);

  return 0;
}