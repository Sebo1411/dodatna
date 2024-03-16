#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
    #endif 
    #include <winsock2.h>
    #include <ws2tcpip.h>
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
         // iOS, tvOS, or watchOS Simulator
    #elif TARGET_OS_MACCATALYST
         // Mac's Catalyst (ports iOS API into Mac, like UIKit).
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
    #elif TARGET_OS_MAC
        // Other kinds of Apple platforms
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __ANDROID__
    // Below __linux__ check should be enough to handle Android,
    // but something may be unique to Android.
#elif __linux__
    #include <sys/types.h>          /* See NOTES */
    #include <sys/socket.h>
    typedef struct sock SOCKET
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "defaults.h" //Zadane konstante, npr. max broj konkurentnih veza i max broj veza na cekanju
#include "handleErr.h"

int radi=TRUE;

void* handleClientThreads(void* arg);
void handleClient(SOCKET* sock);
void* handleClientNum(void* arg);

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t uvjet=PTHREAD_COND_INITIALIZER;

pthread_mutex_t stringMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stringUvjet=PTHREAD_COND_INITIALIZER;

//red cekanja
SOCKET queue[Q_SIZE];
_Atomic int atm_qHeadInd=-1, atm_qTailInd=-1;

void enQueue(SOCKET* cSock);
SOCKET deQueue();

int prost(long long int n);

_Atomic int atm_bSpojenih;
int bCeka;

int main(int argc,const char** argv){
    //Podatci o socket implementaciji, trazimo zadanu verziju 2.2
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

    //hints je addrinfo struktura kojom zadajemo zeljeni socket, a rezultat ta socket implementacija
    /*
     *  Ovaj socket:
     *
     *  IPv4
     *  sequenced, reliable, two-way, connection-based byte streams with an OOB data transmission mechanism
     *  TCP
     *
     */
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

    //thread pool, svaki thread jedan klijent
    pthread_t threadArr[MAX_CONN];
    for (int i=0;i<MAX_CONN;i++) pthread_create(&(threadArr[i]),NULL,handleClientThreads,NULL);

    pthread_t bKlijenata;
    pthread_create(&bKlijenata,NULL,handleClientNum,NULL);

    fputs("Cekam spajanje...\n",stdout);

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
        pthread_cond_signal(&uvjet);
        enQueue(&clientSock);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&stringUvjet);
    }

    for (int i=0;i<MAX_CONN;i++) pthread_join(threadArr[i],NULL);

    closesocket(sock);
    WSACleanup();
    return 0;
}

void* handleClientNum(void* arg){
    while (radi){
        pthread_cond_wait(&stringUvjet,&stringMutex);
        if (atm_qHeadInd==-1) bCeka=0;
        else if (atm_qHeadInd>atm_qTailInd) bCeka=Q_SIZE-atm_qHeadInd+atm_qTailInd+1;
        else bCeka=atm_qTailInd-atm_qHeadInd+1;
        printf(">> Broj klijenata:%10d, klijenti na cekanju:%10d(?)\r",atm_bSpojenih,bCeka);
        fflush(stdout);
    }
    return NULL;
}

/*
 *  threadovi u ovoj funkciji cekaju vezu
 *  nakon dodavanja klijenta u red, signalizira se uvjet
 *  preuzimaju klijenta ak trenutno nemaju
 *  salju u handleClient
 *  vraca NULL
 *
 */
void* handleClientThreads(void* arg){
    SOCKET client;
    while (radi){

        pthread_mutex_lock(&mutex);
        if ((client=deQueue())==0){
            pthread_cond_wait(&uvjet,&mutex);
            client=deQueue();
        }
        pthread_mutex_unlock(&mutex);

        ++atm_bSpojenih;
        if (client!=0) handleClient(&client);
        --atm_bSpojenih;
        pthread_cond_signal(&stringUvjet);
    }
    return NULL;
}

/*
 *  uzima klijent socket ko argument
 *  prima poruku, pretvara u long long
 *  za -1 gasi server, <2 gasi vezu s klijentom
 *  salje poruku je li broj prost
 *
 */
void handleClient(SOCKET* sock){
    int gotovo=FALSE;
    int status;
    long long int n;

    char prostMsg[]="Broj je prost";
    char nijeProstMsg[]="Broj nije prost";

    char recvBuf[DEFAULT_BUFLEN]="1";
    status=send(*sock,recvBuf,sizeof(recvBuf),0);
    if (status==SOCKET_ERROR) gotovo=TRUE;

    status=recv(*sock,recvBuf,sizeof(recvBuf),0);
    while (!gotovo){
        while(status>0){
            n=atoll(recvBuf);
            if (n==-1) radi=FALSE;
            if (n<2) gotovo=TRUE;
            else if (prost(n)) status=send(*sock,prostMsg,sizeof(prostMsg),0);
            else status=send(*sock,nijeProstMsg,sizeof(nijeProstMsg),0);
            if (status==SOCKET_ERROR) break;
            status=recv(*sock,recvBuf,sizeof(recvBuf),0);
        }
        if (status==SOCKET_ERROR || status==0) gotovo=TRUE;
    }
}

int prost(long long int n){
    if (n<2) return FALSE;
    for (long long i=2;i<llround(sqrt((double) n))+1;i++)
        if (n%i==0 && n!=i) return FALSE;
    return TRUE;
}

// stavlja klijent u na cekanje u red
void enQueue(SOCKET* cSock){
    //queue je pun
    if ((atm_qHeadInd==atm_qTailInd+1) || (atm_qHeadInd==0 && atm_qTailInd==Q_SIZE-1)){
        perror("Previse prometa");
        return;
    }
    if (atm_qHeadInd==-1) atm_qHeadInd=0;
    atm_qTailInd=(atm_qTailInd+1)%Q_SIZE;
    queue[atm_qTailInd]=*cSock;
}

// vraca jednog klijenta iz reda cekanja
SOCKET deQueue(){
    //queue je prazan
    if (atm_qHeadInd==-1) return 0;
    SOCKET ret=queue[atm_qHeadInd];
    //samo 1 element
    if (atm_qHeadInd==atm_qTailInd){
        atm_qHeadInd=-1;
        atm_qTailInd=-1;
    }else atm_qHeadInd=(atm_qHeadInd+1)%Q_SIZE;
    return ret;
}
