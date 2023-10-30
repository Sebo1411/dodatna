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

#define DEFAULT_PORT "10000"
#define DEFAULT_BUFLEN 1024
#define MAX_CONN 5

int gotovo=0;

//primjer 1 - jedno spajanje i preinaka poruke

void handleSockErr(const long int);
void *handleClient(void *arg);


int main(int argc,const char** argv){
    //Podatci o socket implementaciji
    WSADATA wsaData; 
    int err = WSAStartup(MAKEWORD(2,2),&wsaData);
    if (err){
        handleSockErr(err);
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
        freeaddrinfo(rezultat);
        handleSockErr(WSAGetLastError());
        return -1;
    }
    
    if (bind(sock,rezultat->ai_addr,(int) rezultat->ai_addrlen)==SOCKET_ERROR){
        freeaddrinfo(rezultat);
        closesocket(sock);
        handleSockErr(WSAGetLastError());
        return -1;
    }

    //podatci nam vise ne trebaju
    freeaddrinfo(rezultat);
    
    if (listen(sock,MAX_CONN)==SOCKET_ERROR){
        closesocket(sock);
        handleSockErr(WSAGetLastError());
        return -1;
    }

    pthread_t threadArr[MAX_CONN];

    SOCKET clientSock;
    while (!gotovo){
        clientSock=accept(sock,NULL,NULL);
        if (clientSock==INVALID_SOCKET){
            closesocket(sock);
            handleSockErr(WSAGetLastError());
            return -1;
        }
        pthread_create(&(threadArr[1]),NULL,&handleClient,&clientSock);
    }


    closesocket(sock);
    WSACleanup();
    return 0;
}

void *handleClient(void *arg){
    puts("got into handle client");
    SOCKET* sock=arg;
    int recvStatus;
    char recvBuf[DEFAULT_BUFLEN];
    recvStatus=recv(*sock,recvBuf,sizeof(recvBuf),0);
    while(recvStatus>0){

        recvStatus=recv(*sock,recvBuf,sizeof(recvBuf),0);
    }
    if (recvStatus==SOCKET_ERROR){

    }

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
 * Poziva WSACleanup()
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