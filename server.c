#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <stdio.h>

//primjer 1 - jedno spajanje i preinaka poruke

int main(){

    /*
     *  IPv4
     *  sequenced, reliable, two-way, connection-based byte streams with an OOB data transmission mechanism 
     *  TCP
     *
     */
    SOCKET socFileDesc=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (socFileDesc==INVALID_SOCKET){
        puts(WSAGetLastError());//funkcija za hendlanje
        return -1;
    }

}