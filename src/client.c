#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "defaults.h"
#include "handleErr.h"

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
        fputs("Nema zadovoljene verzije",stdout);
        _getch();
        return -1;
    }

    struct addrinfo* rezultat=NULL, hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_protocol=IPPROTO_TCP;

    err=getaddrinfo(NULL,DEFAULT_PORT,&hints,&rezultat); //nodename == NULL za localhost, drugo ime za ostalo
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

    fputs(">> Cekam spajanje...\n",stdout);
    for (int i=0;i<MAX_CONN_ATTEMPT;i++) {
        if((err=connect(sock,rezultat->ai_addr,(int) rezultat->ai_addrlen))==SOCKET_ERROR){
            printf(">> Nema servera... Pokusaj spajanja %d\r",i);
            usleep((useconds_t)MAX_TIMEOUT/(useconds_t)MAX_CONN_ATTEMPT);
        }
        else break;
    }
    if (err==SOCKET_ERROR){
        printf("                                                      \r");
        handleSockErr(WSAGetLastError());
        closesocket(sock);
        freeaddrinfo(rezultat);
        WSACleanup();
        return -1;
    }

    freeaddrinfo(rezultat);

    char sendBuff[DEFAULT_BUFLEN];
    char recvBuff[DEFAULT_BUFLEN];

    err=recv(sock,recvBuff,sizeof(recvBuff),0);
    if (err==SOCKET_ERROR){
        handleSockErr(WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    fputs(">> Spojeno\n>> Upisi broj: ",stdout);
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
    fputs(">> ",stdout);
    fputs(sendBuff,stdout);

    closesocket(sock);
    fputs("\n>> Pritisni bilo koji gumb da izades...",stdout);
    _getch();

    WSACleanup();
}
