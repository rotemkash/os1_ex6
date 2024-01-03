/*
======================================================================
File: ex6a1.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
Mahmood Jazmawy, ID = 211345277

* This if the "Filler" process and the owner of the array.
 * it initializes an array with 100 cells.
* it loops a 100 times getting data through a socket from 3 other processes
* if the number sent by the process is bigger/equal to the last number in the array
* it inserts that number in the array and send the matching process 1.
* if the number is not larger/equal it send 0.
* after 100 iterations, whether the array is full or not, it prints the data we
* were asked to show (array size, min value, max value).
* then it sends -1 to the process indicating to finish and printing
* how many number each process has entered o the array.
*/

//==================-INCLUDES-=======================
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//================-CONSTS-==================
#define NUM_OF_CLIENTS 3
#define MY_PORT "12121"

const int ARR_SIZE = 100;
const int END = -1;
const int ADDED = 1;
const int CAN_RUN = 1;
const int NOT_ADDED = 0;


//====================-MAIN-========================
int main(int argc, char *argv[]) {
    int arr[ARR_SIZE], curr_size = 0, prime_to_insert;

    int rc; // return code
    int main_socket, serving_socket;
    int fd;
    fd_set rfd, c_rfd;
    struct addrinfo con_kind,
            *addr_info_res;

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
    rc = listen(main_socket, NUM_OF_CLIENTS);
    if (rc) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&rfd);
    FD_SET(main_socket, &rfd);

    int all_running = 0;
    // check if all the processes are running
    while (all_running != 3) {
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
                rc = read(fd, &prime_to_insert, sizeof(int));
                if (rc == 0) {
                    close(fd);
                    FD_CLR(fd, &rfd);
                } else if (rc > 0) {
                    all_running += prime_to_insert;
                    prime_to_insert = 0;
                } else {
                    perror("read() failed");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    //send them the ok to run
    for (fd = main_socket + 1; fd < getdtablesize(); fd++) {
        write(fd, &CAN_RUN, sizeof(int));
    }


    // loop a 100 times
    for (int i = 0; i < ARR_SIZE; ++i) {
        c_rfd = rfd;
        rc = select(getdtablesize(), &c_rfd, NULL, NULL, NULL);
        if (FD_ISSET(main_socket, &c_rfd)) {
            serving_socket = accept(main_socket, NULL, NULL);
            if (serving_socket >= 0) {
                FD_SET(serving_socket, &rfd);
            }
        }

        // try insert into the array
        for (fd = main_socket + 1; fd < getdtablesize(); fd++) {
            if (FD_ISSET(fd, &c_rfd)) {
                rc = read(fd, &prime_to_insert, sizeof(int));
                if (rc == 0) {
                    close(fd);
                    FD_CLR(fd, &rfd);
                } else if (rc > 0) {
                    if (curr_size == 0 || arr[curr_size - 1] <= prime_to_insert) {
                        arr[curr_size++] = prime_to_insert;
                        write(fd, &ADDED, sizeof(int));
                    } else {
                        write(fd, &NOT_ADDED, sizeof(int));
                    }
                } else {
                    perror("read() failed");
                    exit(EXIT_FAILURE);
                }
            }
        }


        //send -1 to the processes to terminate, when it's the last cell
        if (i == ARR_SIZE - 1) {
            for (fd = main_socket + 1; fd < getdtablesize(); fd++) {
                write(fd, &END, sizeof(int));
            }
        }
    }

    printf("size: %d, min: %d, max: %d\n", curr_size, arr[0], arr[curr_size - 1]);

    return EXIT_SUCCESS;
}
