/*
 * ======================================================================
File: ex6b3.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
    Mahmood Jazmawy, ID = 211345277

 * this is the front end side of our program.
 * it prompts the user to enter a command 'q' or 'p'.
 * 'p': it waits for the user to enter a series of numbers that end with a 0.
 * it puts the numbers in a string and sends it to the prime server using it's socket
 * after it receives an answer from the server it prints it.
 *
 * 'q': it waits for the user to enter a series of number and a 0 at the end
 * of it. it puts the numbers in a string and sends it to the palindrome checking server.
 * the server sends back if the series of number is a palindrome or not.
 *
 * 'e': it quits the program and frees the socket.
 */

 //====================-INCLUDES-=================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for read/write/close
#include <sys/socket.h> /* socket interface functions */
#include <netdb.h> /* host to IP resolution */


//==============-DEFINE-=======================
#define MAX_LEN 100

//=======================-PROTOTYPES-=========
void handle_palindrome(int palindrome_socket);

void handle_prime(int prime_socket);

//=====================-MAIN-====================================
int main(int argc, char* argv[]) {

    int rc; /* system calls return value storage */
    int my_prime_socket, my_palindrome_socket;
    struct addrinfo prime_con_kind, * prime_addr_info;
    struct addrinfo palindrome_con_kind, * palindrome_addr_info;


    if (argc != 4) {
        fprintf(stderr, "Invalid arguments number");
        exit(EXIT_FAILURE);
    }


    //----------connect-to-prime-socket------------
    memset(&prime_con_kind, 0, sizeof(prime_con_kind));
    prime_con_kind.ai_family = AF_UNSPEC; // AF_INET, AF_INET6
    prime_con_kind.ai_socktype = SOCK_STREAM;

    if ((rc = getaddrinfo(argv[1], argv[2],
        &prime_con_kind,
        &prime_addr_info) != 0)) {
        fprintf(stderr, "(getaddrinfo() failed %s\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    my_prime_socket = socket(prime_addr_info->ai_family, // PF_INET
        prime_addr_info->ai_socktype, // SOCK_STREAM
        prime_addr_info->ai_protocol);

    if (my_prime_socket < 0) {
        perror("socket: allocation failed");
        exit(EXIT_FAILURE);
    }

    rc = connect(my_prime_socket,
        prime_addr_info->ai_addr, // addr of server: IP+PORT
        prime_addr_info->ai_addrlen);

    if (rc) {
        perror("connect failed:");
        exit(EXIT_FAILURE);
    }

    //----------connect-to-prime-socket------------
    memset(&palindrome_con_kind, 0, sizeof(palindrome_con_kind));
    palindrome_con_kind.ai_family = AF_UNSPEC; // AF_INET, AF_INET6
    palindrome_con_kind.ai_socktype = SOCK_STREAM;

    if ((rc = getaddrinfo(argv[3], argv[4],
        &palindrome_con_kind,
        &palindrome_addr_info) != 0)) {
        fprintf(stderr, "(getaddrinfo() failed %s\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    my_palindrome_socket = socket(palindrome_addr_info->ai_family, // PF_INET
        palindrome_addr_info->ai_socktype, // SOCK_STREAM
        palindrome_addr_info->ai_protocol);

    if (my_palindrome_socket < 0) {
        perror("socket: allocation failed");
        exit(EXIT_FAILURE);
    }

    rc = connect(my_palindrome_socket,
        palindrome_addr_info->ai_addr, // addr of server: IP+PORT
        palindrome_addr_info->ai_addrlen);

    if (rc) {
        perror("connect failed:");
        exit(EXIT_FAILURE);
    }


    //-----------main-loop----------------
    char command;
    while (1) {
        puts("enter type and command\n");
        scanf("%c", &command);

        if (command == 'q') { // if the user entered q
            handle_palindrome(my_palindrome_socket);
        }
        else if (command == 'p') { // if the user entered p
            handle_prime(my_prime_socket);
        }
        else if (command == 'e') { // if the user entered e
            break;

        }
        else { // if an invalid command was entered
            puts("invalid command!");
        }
    }
}


/**
 * this function is called when the user entered p as a command
 * it receives the numbers from the user and sends them to the prime checker server.
 */
void handle_prime(int prime_socket) {
    int rc;
    char usr_inpt[MAX_LEN] = { '\0' };
    char tmp[MAX_LEN] = { '\0' };
    char answer[MAX_LEN] = { '\0' };

    // get numbers until 0 is met
    while (1) {
        scanf("%s", tmp);
        if (strcmp(tmp, "0") == 0) {
            getchar(); // get rid of whitespace
            break;
        }
        // concatenates all the numbers into one string
        strcat(usr_inpt, tmp);
        if (strlen(usr_inpt) < MAX_LEN) {
            usr_inpt[strlen(usr_inpt)] = ' ';
        }
    }

    //send number to server
    write(prime_socket, usr_inpt, strlen(usr_inpt) + 1);
    // receive answer from server
    rc = read(prime_socket, answer, MAX_LEN);

    if (rc > 0) {
        puts(answer);
    }
    else {
        perror("read() failed here");
        exit(EXIT_FAILURE);
    }
}


/**
 * this function is called when the user entered q as a command
 * it receives a string from the user and sends it to the server
 * that checks for palindromes
 */
void handle_palindrome(int palindrome_socket) {
    int rc;
    char usr_inpt[MAX_LEN] = { '\0' };
    char tmp[MAX_LEN] = { '\0' };
    int answer;

    // get numbers until 0 is met
    while (1) {
        scanf("%s", tmp);
        if (strcmp(tmp, "0") == 0) {
            getchar(); // get rid of whitespace
            break;
        }
        // concatenates all the numbers into one string
        strcat(usr_inpt, tmp);
        if (strlen(usr_inpt) < MAX_LEN) {
            usr_inpt[strlen(usr_inpt)] = ' ';
        }
    }
    puts(usr_inpt);
    //send number to server
    write(palindrome_socket, usr_inpt, strlen(usr_inpt) + 1);
    // receive answer from server
    rc = read(palindrome_socket, &answer, sizeof(int));

    if (rc > 0) {

        if (answer == 1) {
            puts("is palindrome!");
        }
        else {
            puts("is NOT palindrome!");
        }
    }
    else {
        perror("read() failed here");
        exit(EXIT_FAILURE);
    }
}
