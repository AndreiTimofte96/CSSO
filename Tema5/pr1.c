#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <aclapi.h>
#include <tchar.h>
#define SECURITY_MAX_SID_SIZE 68
#pragma comment(lib, "Advapi32.lib")
#define _CRT_SECURE_NO_WARNINGS

HKEY hKey;
const char* regPath = "SOFTWARE\\CSSO\\Tema52";

DWORD _RegCreateKeyEx(SECURITY_ATTRIBUTES lpSecurityAttributes){
    DWORD disposition = 0;
    return RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        regPath,
        0, 
        NULL, 
        REG_OPTION_NON_VOLATILE, 
        KEY_READ,
        &lpSecurityAttributes,
        // NULL,
        &hKey, 
        &disposition);
}

int main()
{    
    //GET SIDs
    BYTE sidSize[SECURITY_MAX_SID_SIZE];
    PSID pSid, pEveryoneSid = NULL;
    SID_IDENTIFIER_AUTHORITY creatorSAuth = SECURITY_CREATOR_SID_AUTHORITY;  
    SID_IDENTIFIER_AUTHORITY worldSAuth = SECURITY_WORLD_SID_AUTHORITY;
    if (!AllocateAndInitializeSid(&creatorSAuth, 1,
        SECURITY_CREATOR_GROUP_RID,
        0, 0, 0, 0, 0, 0, 0,
        &pSid
    )){  
        printf("ERROR 2. Cod eroare: %d\n", GetLastError());
        return 0;
    }

    if (!AllocateAndInitializeSid(&worldSAuth, 1,
        SECURITY_WORLD_RID,
        0, 0, 0, 0, 0, 0, 0,
        &pEveryoneSid
    )){  
        printf("ERROR 3. Cod eroare: %d\n", GetLastError());
        return 0;
    }


    // ADD ACL RIGHTS
    EXPLICIT_ACCESS ea[2];
    ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));
    ea[0].grfAccessPermissions = KEY_ALL_ACCESS;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pSid;

    ea[1].grfAccessPermissions = KEY_READ;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance= NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[1].Trustee.ptstrName  = (LPTSTR) pEveryoneSid;

    PACL pAcl = NULL;
    if (SetEntriesInAcl(2, ea, NULL, &pAcl) != ERROR_SUCCESS) {
        printf("ERROR 5. Cod eroare: %d\n", GetLastError());
        return 0;
    }


    //CREATE SECURITY DESCRIPTOR
    PSECURITY_DESCRIPTOR pSD = NULL;
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
     if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)){
          printf("ERROR 1. Cod eroare: %d\n", GetLastError());
         return 0;
     }


    //SET GROUP SIDs
    if (!SetSecurityDescriptorGroup(
        pSD,
        pSid,
        FALSE
    )){
         printf("ERROR 4. Cod eroare: %d\n", GetLastError());
         return 0;
    }


    //SET DACL
    if (!SetSecurityDescriptorDacl(
        pSD, 
        TRUE,
        pAcl, 
        FALSE
    )) {  
        printf("ERROR 6. Cod eroare: %d\n", GetLastError());
        return 0;
    }

    //CREATE REGEX KEY
    SECURITY_ATTRIBUTES sAttr;
    sAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    sAttr.lpSecurityDescriptor = pSD;
    sAttr.bInheritHandle = FALSE;
    if ( _RegCreateKeyEx(sAttr) != ERROR_SUCCESS) {
        printf("ERROR 7. Nu s-a putut creea cheia. Cod eroare: %d\n", GetLastError());
        return 0;
    } 


    FreeSid(pSid);
    FreeSid(pEveryoneSid);
    LocalFree(pAcl);
    LocalFree(pSD);
    RegCloseKey(hKey);
}