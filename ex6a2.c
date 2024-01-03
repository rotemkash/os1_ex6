/*
======================================================================
File: ex6a2.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
Mahmood Jazmawy, ID = 211345277

* This is the "prime number creators" program.
* when they are ran they create a pipe with the filler process.
* when all 3 creators are up and running they receive a signal to start drawing random
* prime number and sending them to the Filler.
* if the number the process draws enters the array im the Filler it adds one
* to a counter. after the Filler is finished and sent the signal to the process here
* to finish this process prints how much numbers it managed to insert in the array.
*/

//==================-INCLUDES-=======================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for read/write/close
#include <sys/socket.h> /* socket interface functions */
#include <netdb.h> /* host to IP resolution */
#include <math.h>
#include <stdbool.h>


//================-CONSTS-==================

#define BUF_LEN 1024
#define CAN_RUN 1
const int ARR_SIZE = 100;
const int END = -1;
const int ADDED = 1;
const int NOT_ADDED = 0;

//==========-PROTOTYPES-=======================
bool is_prime(int num);


//=============-MAIN-==============================
int main(int argc, char *argv[]) {
    int rc; /* system calls return value storage */
    int my_socket;
    int nums_found = 0, prime_num, server_ans;
    struct addrinfo con_kind,
            *addr_info_res;

    if (argc != 4) {
        fprintf(stderr, "Invalid Arguments numbers");
        exit(EXIT_FAILURE);
    }

    srand(atoi(argv[1]));

    memset(&con_kind, 0, sizeof(con_kind));
    con_kind.ai_family = AF_UNSPEC; // AF_INET, AF_INET6
    con_kind.ai_socktype = SOCK_STREAM;


    if ((rc = getaddrinfo("127.0.0.1", argv[3],
                          &con_kind,
                          &addr_info_res) != 0)) {
        fprintf(stderr, "(getaddrinfo() failed %s\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    my_socket = socket(addr_info_res->ai_family, // PF_INET
                       addr_info_res->ai_socktype, // SOCK_STREAM
                       addr_info_res->ai_protocol);

    if (my_socket < 0) {
        perror("socket: allocation failed");
        exit(EXIT_FAILURE);
    }

    rc = connect(my_socket,
                 addr_info_res->ai_addr, // addr of server: IP+PORT
                 addr_info_res->ai_addrlen);

    if (rc) {
        perror("connect failed:");
        exit(EXIT_FAILURE);
    }

    // send ok
    prime_num = 1;
    write(my_socket, &prime_num, sizeof(int));
    rc = read(my_socket, &server_ans, sizeof(int));
    if (rc > 0) {
        if (server_ans != CAN_RUN) {
            exit(EXIT_FAILURE);
        }
    } else {
        perror("read() failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        prime_num = rand() % ((int) (pow(10, 6)) - 1) + 2;

        if (is_prime(prime_num)) {
            //write the number to the server
            write(my_socket, &prime_num, sizeof(int));
            rc = read(my_socket, &server_ans, sizeof(int));

            if (rc > 0) {
                if (server_ans == -1) { // if received to terminate
                    break;
                }
                // if the answer is 1 add if it's zero we "add" zero
                nums_found += server_ans;
            } else {
                perror("read() failed here");
                exit(EXIT_FAILURE);
            }
        }
    }

    printf("nums found %d.\n", nums_found);

    close(my_socket);
    freeaddrinfo(addr_info_res);

    return EXIT_SUCCESS;
}


/**
 * checks if the number is prime
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
