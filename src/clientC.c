#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

#include "defaults.h"

void handleSockErr(const long int err);

int main(int argc,char** argv){
    int err;
    WSADATA wsaData; 
    err=WSAStartup(MAKEWORD(2,2),&wsaData);
    if (err){
        handleSockErr(err);
        WSACleanup();
        return -1;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2){
        puts("Nema zadovoljene verzije");
        return -1;
    }

    struct addrinfo* rezultat=NULL, hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_protocol=IPPROTO_TCP;

    err=getaddrinfo(NULL,DEFAULT_PORT,&hints,&rezultat);
    if (err){
        handleSockErr(WSAGetLastError());
        WSACleanup();
        return -1;
    }

    SOCKET sock=socket(rezultat->ai_family,rezultat->ai_socktype,rezultat->ai_protocol);
    if (sock==INVALID_SOCKET){
        handleSockErr(WSAGetLastError());
        freeaddrinfo(rezultat);
        WSACleanup();
        return -1;
    }

    if (connect(sock,rezultat->ai_addr,(int) rezultat->ai_addrlen)==SOCKET_ERROR){
        handleSockErr(WSAGetLastError());
        closesocket(sock);
        freeaddrinfo(rezultat);
        WSACleanup();
        return -1;
    }

    freeaddrinfo(rezultat);

    char sendBuff[DEFAULT_BUFLEN];
    char recvBuff[DEFAULT_BUFLEN];

    fgets(sendBuff,sizeof(sendBuff),stdin);

    err=send(sock,sendBuff,sizeof(sendBuff),0);
    if (err==SOCKET_ERROR){
        closesocket(sock);
        handleSockErr(err);
        WSACleanup();
        return -1;
    }

    err=recv(sock,sendBuff,sizeof(sendBuff),0);
    if (err==SOCKET_ERROR){
        closesocket(sock);
        handleSockErr(err);
        WSACleanup();
        return -1;
    }
    puts(sendBuff);

    puts("Pritisni bilo koji gumb da izades");
    getc(stdin);
    closesocket(sock);
    WSACleanup();
}


void handleSockErr(const long int err){
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