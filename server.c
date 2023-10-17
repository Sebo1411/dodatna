#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

//primjer 1 - jedno spajanje i preinaka poruke

int main(){

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
        puts(WSAGetLastError());//funkcija za hendlanje
        return -1;
    }
    
    if (bind(sock,(SOCKADDR_STORAGE *)&addressFamily,sizeof(addressFamily))==SOCKET_ERROR){
        puts(WSAGetLastError());//funkcija za hendlanje
        return -1;
    }


}