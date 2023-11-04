#include <winsock2.h>
//#include <ws2tcpip.h>

#include <stdio.h>

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
            perror("Krivi 'handle' za objekt");
            break;
        case WSA_NOT_ENOUGH_MEMORY:
            perror("Nema dovoljno memorije");
            break;
        case WSA_INVALID_PARAMETER:
            perror("Krivi argumenti funkcije");
            break;
        case WSASYSNOTREADY:
            perror("Sistem nije spreman za mreznu komunikaciju");
            break;
        case WSAVERNOTSUPPORTED:
            perror("Trazena Windows socket verzija nije podrzana");
            break;
        default:
            snprintf(msg,maxLen,"Error s kodom %ld",err);
            perror(msg);
    }
    WSACleanup();
}