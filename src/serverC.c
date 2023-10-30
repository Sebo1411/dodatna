#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "defaults.h"

int gotovo=0;

//primjer 1 - jedno spajanje i preinaka poruke

void handleSockErr(const long int err);
void* handleClient(void* arg);


int main(int argc,const char** argv){
    //Podatci o socket implementaciji
    WSADATA wsaData; 
    int err=WSAStartup(MAKEWORD(2,2),&wsaData);
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
    hints.ai_flags=AI_PASSIVE;
    hints.ai_family=AF_INET;
    hints.ai_protocol=SOCK_STREAM;
    hints.ai_protocol=IPPROTO_TCP;

    err=getaddrinfo(NULL,DEFAULT_PORT,&hints,&rezultat);
    if (err){
        handleSockErr(WSAGetLastError());
        WSACleanup();
        return -1;
    }

    /*
     *  IPv4
     *  sequenced, reliable, two-way, connection-based byte streams with an OOB data transmission mechanism 
     *  TCP
     *
     */
    SOCKET sock=socket(rezultat->ai_family,rezultat->ai_socktype,rezultat->ai_protocol);
    if (sock==INVALID_SOCKET){
        handleSockErr(WSAGetLastError());
        freeaddrinfo(rezultat);
        WSACleanup();
        return -1;
    }
    
    if (bind(sock,rezultat->ai_addr,(int) rezultat->ai_addrlen)==SOCKET_ERROR){
        handleSockErr(WSAGetLastError());
        freeaddrinfo(rezultat);
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    //podatci nam vise ne trebaju
    freeaddrinfo(rezultat);
    
    if (listen(sock,MAX_CONN)==SOCKET_ERROR){
        handleSockErr(WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    pthread_t threadArr[MAX_CONN];
    int clientCounter;
    SOCKET clientSock;
    while (!gotovo){
        clientSock=accept(sock,NULL,NULL);
        if (clientSock==INVALID_SOCKET){
            handleSockErr(WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            return -1;
        }
        pthread_create(&(threadArr[clientCounter % MAX_CONN]),NULL,handleClient,&clientSock);
    }
    if (clientCounter>5){
        for (int i=0;i<5;i++) pthread_detach(threadArr[i]);
    }
    else{
        for (int i=0;i<clientCounter;i++) pthread_detach(threadArr[i]);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

void *handleClient(void *arg){
    puts("got into handle client");
    SOCKET* sock=arg;
    int status;
    char recvBuf[DEFAULT_BUFLEN];
    status=recv(*sock,recvBuf,sizeof(recvBuf),0);
    while(status>0){
        send(*sock,recvBuf,sizeof(recvBuf),0);
        status=recv(*sock,recvBuf,sizeof(recvBuf),0);
    }
    if (status==SOCKET_ERROR){
        
    }
    return NULL;
}

int prost(const long int n){
    for (long i=2;i<lround(sqrt((double) n))+1;i++)
        if (n%i==0 && n!=i) return 0;
    return 1;
}



/*
 * 
 * Argument: rezultat funkcije WSAGetLastError()
 * Ispisuje odgovarajucu poruku
 *
 */
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
    WSACleanup();
}