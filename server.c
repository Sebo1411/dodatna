#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//primjer 1 - jedno spajanje i preinaka poruke

void handleErr(const long int);

int main(int argc,const char** argv){
    //Podatci o socket implementaciji
    WSADATA wsaData; 
    int err = WSAStartup(MAKEWORD(2,2),&wsaData);
    if (err){
        handleErr(err);
        return -1;
    }


    SOCKADDR_STORAGE addressFamily;
    addressFamily.ss_family=(ADDRESS_FAMILY) AF_INET;

    /*
     *  IPv4
     *  sequenced, reliable, two-way, connection-based byte streams with an OOB data transmission mechanism 
     *  TCP
     *
     */
    SOCKET sock=socket(addressFamily.ss_family,SOCK_STREAM,IPPROTO_TCP);
    if (sock==INVALID_SOCKET){
        handleErr(WSAGetLastError());
        return -1;
    }
    
    if (bind(sock,(SOCKADDR_STORAGE *)&addressFamily,sizeof(addressFamily))==SOCKET_ERROR){
        handleErr(WSAGetLastError());
        return -1;
    }



    WSACleanup();
    return 0;
}






/*
 * 
 * Argument: rezultat funkcije WSAGetLastError()
 * Ispisuje odgovarajucu poruku
 *
 */
void handleErr(const long int err){
    int maxLen=45;
    char msg[maxLen];
    switch (err){
        case WSA_INVALID_HANDLE:
            strncpy(msg,"Krivi 'handle' za objekt",maxLen);
            break;
        case WSA_NOT_ENOUGH_MEMORY:
            strncpy(msg,"Nema dovoljno memorije",maxLen);
            break;
        case WSA_INVALID_PARAMETER:
            strncpy(msg,"Krivi argumenti funkcije",maxLen);
            break;
        case WSASYSNOTREADY:
            strncpy(msg,"Sistem nije spreman za mreznu komunikaciju",maxLen);
            break;
        case WSAVERNOTSUPPORTED:
            strncpy(msg,"Trazena Windows socket verzija nije podrzana",maxLen);
            break;
        default:
            snprintf(msg,maxLen,"Error s kodom %ld",err);
    }
    fputs(msg,stderr);
}