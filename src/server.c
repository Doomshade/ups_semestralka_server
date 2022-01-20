#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/tcp.h>

// kvuli iotctl
#include <sys/ioctl.h>
#include "../include/server.h"
#include "../include/packet_handler.h"
#include "../include/packet_validator.h"
#include "../include/queue_mngr.h"
#include "../include/chesspiece.h"
#include <sys/time.h>
#include <signal.h>

#define check(eval) if(!(eval)) printf("setsockopt error\n");

/**
 * Starts listening
 * @param server_socket the server socket fd
 * @param args the arguments passed to the main function
 * @return
 */
static int start_listening(int server_socket, struct arguments* args);

/**
 * Disconnects the player from the server
 * @param p the player
 * @param client_socks the fd set of clients
 */
static void disconnect(struct player* p, fd_set* client_socks, const char* reason);

/**
 * Initializes the server
 */
static void init_server();

/**
 * Handles a new client
 * @param server_socket the server socket fd
 * @param peer_addr the peer address
 * @param len_addr the ptr to the length of the address
 * @param client_socks the fd set of clients
 */
static void handle_new_client(int server_socket,
                              struct sockaddr_in* peer_addr,
                              unsigned int* len_addr,
                              fd_set* client_socks);

void enable_keepalive(int sock) {
    int yes = 1;
    check(setsockopt(
            sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) != -1);

    int idle = 1;
    check(setsockopt(
            sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int)) != -1);

    int interval = 1;
    check(setsockopt(
            sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int)) != -1);

    int maxpkt = 10;
    check(setsockopt(
            sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int)) != -1);
}

int start_server(struct arguments* args) {
    int server_socket, rval;
    struct sockaddr_in my_addr;
    int param = 1;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    //rval = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &param, sizeof(int));

    check(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &param, sizeof(int)) != -1);

    memset(&my_addr, 0, sizeof(struct sockaddr_in));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(args->port);
    inet_pton(AF_INET, args->ip, &(my_addr.sin_addr));

    printf("Starting server on %s:%d\n", args->ip, args->port);
    rval = bind(server_socket, (struct sockaddr*) &my_addr, sizeof(struct sockaddr_in));
    if (!rval) {
        printf("Successfully bound the socket\n");
    } else {
        printf("Could not bind the socket\n");
        return rval;
    }

    rval = listen(server_socket, 5);
    if (rval) {
        printf("Failed to start listening\n");
        return rval;
    }

    // ignore SIGPIPE
    sigaction(SIGPIPE, &(struct sigaction) {SIG_IGN}, NULL);
    return start_listening(server_socket, args);
}

int start_listening(int server_socket, struct arguments* args) {
    struct sockaddr_in peer_addr;
    int fd, rval, a2read;
    char* p_ptr = NULL;
    char* p_ptr_copy = NULL; // to later free the p_ptr
    unsigned int len_addr;
    fd_set client_socks, tests; // fd sets (for socks too)
    int erc = 0;
    struct packet* pckt;
    struct player* player = NULL;

    char buf[BUFSIZ];

    memset(&peer_addr, 0, sizeof(struct sockaddr_in));
    // initialize things
    init_server();

    // clear the descriptors and add the server socket
    FD_ZERO(&client_socks);
    FD_SET(server_socket, &client_socks);

    printf("Started listening to incoming connections...\n");
    for (;;) {
        // copy the old fd_set to the new one (select destroys them)
        tests = client_socks;

        rval = select(FD_SETSIZE, &tests, (fd_set*) NULL, (fd_set*) NULL, (struct timeval*) 0);
        if (rval < 0) {
            printf("Select err: %d\n", rval);
            return -1;
        }

        for (fd = 3; fd < FD_SETSIZE; fd++) {
            if (!FD_ISSET(fd, &tests)) {
                continue;
            }

            // is it a new connection?
            if (fd == server_socket) {
                handle_new_client(server_socket, &peer_addr, &len_addr, &client_socks);
                //tst(server_socket, &peer_addr, &len_addr, &client_socks);
                continue;
            }

            lookup_player_by_fd(fd, &player);
            // this should NOT return NULL as we handled the new connection
            if (!player) {
                printf("An error occurred during the creation of client (FD=%d)...\n", fd);
                continue;
            }
            printf("--------------------%s--------------------\n", player->name);


            // nope, a client sent some data
            ioctl(fd, FIONREAD, &a2read);

            // something happened on the client side
            if (a2read <= 0) {
                disconnect(player, &client_socks, NULL);
                continue;
            }

            p_ptr = malloc(sizeof(char) * (a2read + 1));
            memset(p_ptr, 0, sizeof(char) * (a2read + 1));

            // store the copy of the ptr to free because it gets
            // shifted in the handle_packet func
            // read the packet
            p_ptr_copy = p_ptr;
            recv(fd, p_ptr, a2read, 0);
            p_ptr[strcspn(p_ptr, "\n")] = 0;

#ifdef SERVER_DEBUG_MODE
            printf("Received %s (%lu) from %d\n", p_ptr, strlen(p_ptr), fd);
#endif

            HANDLE_PACKET:
#ifdef SERVER_DEBUG_MODE
            printf("Parsing %s...\n", p_ptr);
#endif
            pckt = parse_packet(&p_ptr, &erc, player);
            // the packet has not yet been fully buffered,
            // we still need to wait for the rest of the packet
            if (erc == PVAL_PACKET_NOT_YET_FULLY_BUFFERED) {
                goto FREE;
            }

            // the packet has been handled -> check if handled correctly
            // any error leads to a NULL pointer
            if (!pckt) {
                printf("Failed to parse packet: %d\n", erc);
                disconnect(player, &client_socks, "Invalid packet format!");
                goto FREE;
            }

            // the packet format is valid, handle it
            rval = handle_packet(player, pckt->id, pckt->data);
            free_packet(&pckt);

            // check the return value of the handled packet
            switch (rval) {
                case PACKET_RESP_OK:
                case PACKET_RESP_OK_INVALID_DATA:
                    break;
                case PACKET_RESP_ERR_NOT_RECVD:
                    printf("The player %s did not receive the packet (%d, %s)\n", player->name, pckt->id, pckt->data);
                    break;
                case PACKET_RESP_ERR_INVALID_DATA:
                    printf("The player %s sent invalid data\n", player->name);
#ifndef SERVER_DEBUG_MODE
                    player->invalid_sends++;
                    if (player->invalid_sends >= args->max_inval_pc) {
                        printf("The player %s sent too many invalid packets!\n", player->name);
                        goto _DC;
                    }
                    send_packet(player, INVALID_DATA_OUT, p_ptr_copy);
#endif
                    break;
                default:
                    printf("An error occurred when handling the packet (%d)\n", rval);
                    goto _DC;
            }

            // there could still be some leftover data in the buffer,
            // handle the rest of the data
            if (strlen(p_ptr) != 0) {
                goto HANDLE_PACKET;
            }

            if (0) {
                _DC:
                disconnect(player, &client_socks, "Invalid packet data!");
            }

            // no more data, free the malloced packet data
            FREE:
            free(p_ptr_copy);
            printf("--------------------%s--------------------\n\n", player->name);
        } // END FOR (FD)
    } // END WHILE(1)
    return 0;
}


void handle_new_client(int server_socket,
                       struct sockaddr_in* peer_addr,
                       unsigned int* len_addr,
                       fd_set* client_socks) {
    int client_socket;
    int rval;

    client_socket = accept(server_socket, (struct sockaddr*) peer_addr, len_addr);
    FD_SET(client_socket, client_socks);
    //enable_keepalive(client_socket);
    printf("A client has connected and was assigned FD #%d\n", client_socket);
    // this adds the client to the player array
    rval = handle_new_player(client_socket);
    if (rval == 2) {
        printf("An FD with ID %d is already registered, this should not happen!\n", client_socket);
        exit(0);
    }
}

void init_server() {
    init_pval(); // packet validator
    init_preg(); // packet registry
    init_qman(); // queue manager
    init_gman(); // game manager
    init_cpce(); // chess pieces
}

int handle_dc(struct player* p) {
    int fd;
    if (!p) {
        return 1;
    }

    fd = p->fd;
    free_buffers(fd); // cleanup in the packet validator (due to potential buffered header/data)
    pman_handle_dc(p); // cleanup in player manager and store the state
    qman_handle_dc(p); // removing from queue changes the state of the player ONLY if the packet is successfully sent
    // in our case the packet cannot be sent as the player has disconnected -> the player's state will remain the same
    gman_handle_dc(p); // inform the opponent that the player has DC'd

    close(fd);
    printf("%s has disconnected\n", p->name);
    printf("--------------------%s--------------------\n\n", p->name);
    return 0;
}

void disconnect(struct player* p, fd_set* client_socks, const char* reason) {
    char buf[512];
    struct game* g;
    if (!p || !client_socks) {
        return;
    }
    if (reason) {
        send_packet(p, DISCONNECT_OUT, reason);
        g = lookup_game(p);
        if (g) {
            sprintf(buf, "%d", (g->white == p ? BLACK : WHITE) | WIN_BY_RESIGNATION);
            send_packet(OPPONENT(g, p), GAME_FINISH_OUT, buf);
        }
    }
    FD_CLR(p->fd, client_socks);
    handle_dc(p);
}
