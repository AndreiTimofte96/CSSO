#include <Windows.h>
#include <stdio.h>

int main() {
  /*
   * Pentru a creea cheie de registri aventi nevoie de permisiuni de administrator.
   * Aveti doua modalitati:
   * 1. Rulati Visual Studio ca administrator.
   * 2. Rulati executabilul compilat din windows explorer cu click dreapta -> Run as administrator.
   * Observatie: Daca rulati pe configuratia de x86, exemplul de mai jos va creea de fapt cheia
   * HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\test. Explicatiile sunt in pdf-ul pentru laboratorul 2.
   */

  HKEY hKey;
  DWORD disposition = 0;
  if (RegCreateKeyEx(
    HKEY_LOCAL_MACHINE,
    "SOFTWARE\\test\\test2",
    0, 
    NULL, 
    REG_OPTION_NON_VOLATILE, 
    KEY_WRITE,
    NULL,
    &hKey, 
    &disposition) != ERROR_SUCCESS) {
    printf("Nu s-a putut creea cheia. Cod eroare: %d\n", GetLastError());    
    return 0;
  }

  DWORD size = 10; // dummy value
  RegSetValueEx(
    hKey, 
    "a.txt.asda.asas.exe", 
    0, 
    REG_DWORD,
    (const BYTE*)&size, 
    sizeof(DWORD));

  RegCloseKey(hKey);
  printf("DONE");

  return 0;
}