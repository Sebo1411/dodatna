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

int radi=TRUE;

void handleSockErr(const long int err);
void* handleThreads(void* arg);
void handleClient(SOCKET* sock);

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t uvjet=PTHREAD_COND_INITIALIZER;

SOCKET queue[Q_SIZE];
int qHeadInd=-1, qTailInd=-1;

void enQueue(SOCKET* cSock);
SOCKET deQueue();

int prost(long long int n);

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
    for (int i=0;i<5;i++) pthread_create(&(threadArr[i]),NULL,handleThreads,NULL);

    SOCKET clientSock;
    while (radi){
        clientSock=accept(sock,NULL,NULL);
        if (clientSock==INVALID_SOCKET){
            handleSockErr(WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            return -1;
        }
        pthread_mutex_lock(&mutex);
        enQueue(&clientSock);
        pthread_cond_signal(&uvjet);
        pthread_mutex_unlock(&mutex);
    }

    for (int i=0;i<5;i++) pthread_join(threadArr[i],NULL);

    closesocket(sock);
    WSACleanup();
    return 0;
}

void* handleThreads(void* arg){
    SOCKET client;
    while (radi){
        pthread_mutex_lock(&mutex);
        if ((client=deQueue())==0){
            pthread_cond_wait(&uvjet,&mutex);
            client=deQueue();
        }
        pthread_mutex_unlock(&mutex);

        if (client!=0) handleClient(&client);
    }
}

void handleClient(SOCKET* sock){
    int gotovo=FALSE;
    int status;
    long long int n;

    char prostMsg[]="Broj je prost";
    char nijeProstMsg[]="Broj nije prost";

    char recvBuf[DEFAULT_BUFLEN];
    while (!gotovo){
        status=recv(*sock,recvBuf,sizeof(recvBuf),0);
        while(status>0){
            n=atoll(recvBuf);
            if (n==-1) radi=FALSE;
            if (n<2) gotovo=TRUE;
            else if (prost(n)) status=send(*sock,prostMsg,sizeof(prostMsg),0);
            else status=send(*sock,nijeProstMsg,sizeof(nijeProstMsg),0);
            if (status==SOCKET_ERROR) break;
            status=recv(*sock,recvBuf,sizeof(recvBuf),0);
        }
        if (status==SOCKET_ERROR) gotovo=TRUE;
    }
}

int prost(long long int n){
    if (n<2) return FALSE;
    for (long long i=2;i<llround(sqrt((double) n))+1;i++)
        if (n%i==0 && n!=i) return FALSE;
    return TRUE;
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

void enQueue(SOCKET* cSock){
    //queue je pun
    if ((qHeadInd==qTailInd+1) || (qHeadInd==0 && qTailInd==Q_SIZE-1)){
        perror("Previse prometa");
        return;
    }
    if (qHeadInd==-1) qHeadInd=0;
    qTailInd=(qTailInd+1)%Q_SIZE;
    queue[qTailInd]=*cSock;
}

SOCKET deQueue(){
    //queue je prazan
    if (qHeadInd==-1) return 0;
    SOCKET ret=queue[qHeadInd];
    //samo 1 element
    if (qHeadInd==qTailInd){
        qHeadInd=-1;
        qTailInd=-1;
    }else qHeadInd=(qHeadInd+1)%Q_SIZE;
    return ret;
}