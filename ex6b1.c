
/*
======================================================================
File: ex6b1.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
    Mahmood Jazmawy, ID = 211345277

 * This program is our server that checks for prime numbers.
 * it receives a string through the socket and returns a string that
 * contains only the prime numbers that where in it
 * if there were no prime numbers in the string it returns 0
 */

 //==============-INCLUDES-===============
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h> // for memset
#include <sys/socket.h>
#include <netdb.h>

//================-DEFINE-================
#define BUF_LEN 100
int const ARR_SIZE = 102;
#define MY_PORT "1234"

//===============-PROTOTYPES-=============
void catch_sigint(int sig_num);

bool is_prime(int num);

void check_string(char buffer[BUF_LEN]);

//============-GLOBLAS-================
struct addrinfo* addr_info_res;
int main_socket;

//==============-MAIN-=====================
int main() {

    signal(SIGINT, catch_sigint);

    //----------------variables--------------
    int rc; // return code
    int serving_socket;
    int fd;
    fd_set rfd;
    fd_set c_rfd;
    char buffer[BUF_LEN];
    struct addrinfo con_kind;

    //------------INIT----------------------
    memset(&con_kind, 0, sizeof(con_kind));
    con_kind.ai_family = AF_UNSPEC;
    con_kind.ai_socktype = SOCK_STREAM;
    con_kind.ai_flags = AI_PASSIVE; // system will fill my IP

    if ((rc = getaddrinfo(NULL, // NULL = you set IP address //TODO: we set it
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
                rc = read(fd, buffer, BUF_LEN);
                if (rc == 0) {
                    close(fd);
                    FD_CLR(fd, &rfd);
                }
                else if (rc > 0) {
                    check_string(buffer);
                    write(fd, buffer, strlen(buffer) + 1);
                }
                else {
                    perror("read() failed");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}


/**
 * checks the string. it creates a new string with only the prime
 * numbers that were in the original string. if there were no prime numbers
 * it only puts zero in the string.
 */
void check_string(char buffer[BUF_LEN]) {
    char ret_txt[BUF_LEN] = { '\0' };
    char* token;
    int tmp;
    token = strtok(buffer, " ");
    while (token != NULL) {
        tmp = atoi(token);
        /*
         * if the current token(number) is a prime.
         * add it to the existing string and add a space after it.
         */
        if (is_prime(tmp)) {
            strcat(ret_txt, token);
            if (strlen(ret_txt) < BUF_LEN) {
                ret_txt[strlen(ret_txt)] = ' ';
            }
        }
        token = strtok(NULL, " ");
    }

    if (strlen(ret_txt) > 0) {
        strcpy(buffer, ret_txt);
    }
    else {
        strcpy(buffer, "0");
    }
}

/**
 * This function checks if a number is prime or not
 */
bool is_prime(int num) {
    int mid = num / 2;
    if (num < 2) {
        return false;
    }
    for (int i = 2; i <= mid; i++) {
        if (num % i == 0) {
            return false;
        }
    }
    return true;
}


/**
 * catch SIGINT and free the socket
 */
void catch_sigint(int sig_num) {
    close(main_socket);
    freeaddrinfo(addr_info_res);
    exit(EXIT_SUCCESS);
}

