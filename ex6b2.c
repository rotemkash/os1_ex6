/*
 * ======================================================================
File: ex6b2.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
    Mahmood Jazmawy, ID = 211345277

 * This program is our server that checks for palindrome series.
 * it receives a string containing a series of number. it checks
 * if the series is a palindrome and returns to the client through it's socket
 * 1 if it's a palindrome and 0 otherwise
 */

//====================-INCLUDES-=================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h> // for memset
#include <sys/socket.h>
#include <netdb.h>

//==============-DEFINE-=======================
#define MY_PORT "5678"
int const IS_PALI = 1;
int const NOT_PALI = 0;
int const ARR_SIZE = 100;


//=======================-PROTOTYPES-===========================
void catch_sigint(int sig_num);

int is_palindrome(const char *ptr);

//=======-GLOBALS-==================
int main_socket;
struct addrinfo *addr_info_res;

//===========-MAIN-==================
int main() {

    signal(SIGINT, catch_sigint);

    //----------------variables--------------
    int rc; // return code
    int serving_socket;
    int fd;
    fd_set rfd;
    fd_set c_rfd;
    char arr[ARR_SIZE];
    struct addrinfo con_kind;


    //------------INIT----------------------
    memset(&con_kind, 0, sizeof(con_kind));
    con_kind.ai_family = AF_UNSPEC;
    con_kind.ai_socktype = SOCK_STREAM;
    con_kind.ai_flags = AI_PASSIVE; // system will fill my IP

    if ((rc = getaddrinfo(NULL, // NULL = you set IP address
                          MY_PORT,
                          &con_kind,
                          &addr_info_res)) != 0) {
        fprintf(stderr, "(getaddrinfo() failed %s\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    main_socket = socket(addr_info_res->ai_family,
                         addr_info_res->ai_socktype,
                         addr_info_res->ai_protocol);
    if (main_socket < 0) {
        perror("socket: allocation failed");
        exit(EXIT_FAILURE);
    }
    rc = bind(main_socket, addr_info_res->ai_addr,
              addr_info_res->ai_addrlen);
    if (rc) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    rc = listen(main_socket, 5); 
    if (rc) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&rfd);
    FD_SET(main_socket, &rfd);

    while (1) {
        c_rfd = rfd;
        rc = select(getdtablesize(), &c_rfd, NULL, NULL, NULL);
        if (FD_ISSET(main_socket, &c_rfd)) {
            serving_socket = accept(main_socket, NULL, NULL);
            if (serving_socket >= 0) {
                FD_SET(serving_socket, &rfd);
            }
        }

        for (fd = main_socket + 1; fd < getdtablesize(); fd++) {
            if (FD_ISSET(fd, &c_rfd)) {
                rc = read(fd, arr, ARR_SIZE);
                if (rc == 0) {
                    close(fd);
                    FD_CLR(fd, &rfd);
                } else if (rc > 0) {
                    if (is_palindrome(arr)) {
                        write(fd, &IS_PALI, sizeof(int));
                    } else {
                        write(fd, &NOT_PALI, sizeof(int));
                    }

                    // check that write went okay...
                } else {
                    perror("read() failed");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}


/**
 * checks if the string is a palindrome
 */
int is_palindrome(const char *ptr) {
    int i, j = strlen(ptr) - 2;

    for (i = 0; i < j; ++i, --j) {
        if (ptr[i] != ptr[j]) {
            return 0;
        }
    }
    return 1;
}

/**
 * catch SIGINT, free socket
 */
void catch_sigint(int sig_num) {
    close(main_socket);
    freeaddrinfo(addr_info_res);
    exit(EXIT_SUCCESS);
}

