/*
    Copyright (C) HWPORT.COM
    All rights reserved.
    Author: JAEHYUK CHO <mailto:minzkn@minzkn.com>
*/

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
# include <winsock2.h>
# include <ws2tcpip.h>
# include <windows.h>
# include <sys/types.h>
# include <io.h>
# include <direct.h> 
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <memory.h>
# include <string.h>
# include <time.h>
# include <fcntl.h>
# include <errno.h>
# include <signal.h>
# define ssize_t signed long int
# define def_windows
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
#endif

int main(int s_argc, char **s_argv);

int main(int s_argc, char **s_argv)
{
    unsigned int s_mac_array[6];
    unsigned char s_target_mac[6];

#if defined(def_windows)
    SOCKET s_socket;
#else
    int s_socket;
#endif    
    struct sockaddr_in s_sockaddr_in;
    unsigned char s_magic_packet[ 6 + (6 * 16) ];
    int s_repeat;
    ssize_t s_send_bytes;
    int s_value;

    if(s_argc < 2) {
        (void)fprintf(stdout, "Usage: %s <mac>\n", s_argv[0]);
        return(EXIT_FAILURE);
    }
    
    (void)memset((void *)(&s_mac_array[0]), 0, sizeof(s_mac_array));
    if(sscanf(s_argv[1], "%02x:%02x:%02x:%02x:%02x:%02x",
        &s_mac_array[0], &s_mac_array[1], &s_mac_array[2],
        &s_mac_array[3], &s_mac_array[4], &s_mac_array[5]) < 6) {
        if(sscanf(s_argv[1], "%02x%02x%02x%02x%02x%02x",
            &s_mac_array[0], &s_mac_array[1], &s_mac_array[2],
            &s_mac_array[3], &s_mac_array[4], &s_mac_array[5]) < 6) {
            (void)fprintf(stderr, "invalid mac address !\n");
            return(EXIT_FAILURE);
        }
    }
    s_target_mac[0] = (unsigned char)s_mac_array[0];
    s_target_mac[1] = (unsigned char)s_mac_array[1];
    s_target_mac[2] = (unsigned char)s_mac_array[2];
    s_target_mac[3] = (unsigned char)s_mac_array[3];
    s_target_mac[4] = (unsigned char)s_mac_array[4];
    s_target_mac[5] = (unsigned char)s_mac_array[5];
        
#if defined(def_windows)
    do { /* initialize windows socket v2.2 (required DLL resource : "WS2_32.DLL") */
        BYTE s_require_major_version;
        BYTE s_require_minor_version;
        WORD s_require_version;
        WSADATA s_wsadata;

        s_require_major_version = (BYTE)2;
        s_require_minor_version = (BYTE)2;
        s_require_version = MAKEWORD(/* low */ s_require_major_version, /* high */ s_require_minor_version);
       
        /* int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData); */
        (void)WSAStartup(s_require_version, (LPWSADATA)(&s_wsadata));
    }while(0);
#endif    

    /* UDP socket open */
    s_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(s_socket == (-1)) {
        perror("socket");
        return(EXIT_FAILURE);
    }

    /* bind */
    (void)memset((void *)(&s_sockaddr_in), 0, sizeof(s_sockaddr_in));
    s_sockaddr_in.sin_family = AF_INET;
    s_sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
    s_sockaddr_in.sin_port = htons(0);
    if(bind(s_socket, (const struct sockaddr *)(&s_sockaddr_in), (socklen_t)sizeof(s_sockaddr_in)) == (-1)) {
        perror("bind");
        
#if defined(def_windows)
        closesocket(s_socket);
#else
        (void)close(s_socket);
#endif        
        
        return(EXIT_FAILURE);
    }

    /* broadcast socket option enable */
    s_value = 1;
    (void)setsockopt(s_socket, SOL_SOCKET, SO_BROADCAST, (const void *)(&s_value), (socklen_t)sizeof(s_value));

    /* WOL packet build (repeat 6) */
    (void)memset((void *)(&s_magic_packet[0]), 0xff, (size_t)6u);
    for(s_repeat = 0;s_repeat < 16;s_repeat++) {
        (void)memcpy((void *)(&s_magic_packet[6 + (s_repeat * 6)]), (const void *)(&s_target_mac[0]), sizeof(s_target_mac));
    }
    
    /* broadcast socket address structure set */
    (void)memset((void *)(&s_sockaddr_in), 0, sizeof(s_sockaddr_in));
    s_sockaddr_in.sin_family = AF_INET;
    (void)inet_pton(s_sockaddr_in.sin_family, "255.255.255.255", (void *)(&s_sockaddr_in.sin_addr));
    s_sockaddr_in.sin_port = htons(2304); /* any port ... */

    /* send */
    s_send_bytes = sendto(
        s_socket,
        (const void *)(&s_magic_packet[0]),
        sizeof(s_magic_packet),
        0,
        (const struct sockaddr *)(&s_sockaddr_in),
        (socklen_t)sizeof(s_sockaddr_in)
    );
    if(s_send_bytes == ((ssize_t)(-1))) {
        perror("sendto");
        
#if defined(def_windows)
        closesocket(s_socket);
#else
        (void)close(s_socket);
#endif        
        
        return(EXIT_FAILURE);
    }

    (void)fprintf(stdout,
        "WOL packet : %ld bytes (Target %02X:%02X:%02X:%02X:%02X:%02X)\n",
        (long)s_send_bytes,
        (unsigned int)s_target_mac[0], (unsigned int)s_target_mac[1], (unsigned int)s_target_mac[2],
        (unsigned int)s_target_mac[3], (unsigned int)s_target_mac[4], (unsigned int)s_target_mac[5]
    );

#if defined(def_windows)
        closesocket(s_socket);
#else
        (void)close(s_socket);
#endif        
    
    return(EXIT_SUCCESS);
}

/* vim: set expandtab: */
/* End of source */
