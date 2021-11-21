#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
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

#define check(eval) if(!(eval)) printf("setsockopt error\n");

/**
 * Starts listening to
 * @param server_socket
 * @param peer_addr
 * @return
 */
int start_listening(int server_socket);

/**
 * Disconnects the player from the server
 * @param p the player
 * @param client_socks
 */
void disconnect(struct player* p, fd_set* client_socks);

void init_server();

void handle_new_client(int server_socket,
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

int start_server(unsigned port) {
    int server_socket, rval;
    char ip[64]; // buffer to output IP:port
    struct sockaddr_in my_addr;
    int param = 1;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    //rval = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &param, sizeof(int));

    check(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &param, sizeof(int)) != -1);

    memset(&my_addr, 0, sizeof(struct sockaddr_in));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    inet_ntop(AF_INET, &my_addr.sin_addr, ip, sizeof(ip));
    printf("Starting server on %s:%d\n", ip, port);
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
    return start_listening(server_socket);
}

int start_listening(int server_socket) {
    struct sockaddr_in peer_addr;
    int fd, rval, a2read;
    char* p_ptr = NULL;
    char* p_ptr_copy = NULL; // to later free the p_ptr
    unsigned int len_addr;
    fd_set client_socks, tests; // fd sets (for socks too)
    int erc = 0;
    struct packet* pckt;
    struct player* player = NULL;

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
            lookup_player_by_fd(fd, &player); // look for the connected player

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
                disconnect(player, &client_socks);
                continue;
            }

            p_ptr = malloc(sizeof(char) * (a2read + 1));
            memset(p_ptr, 0, sizeof(char) * (a2read + 1));

            // store the copy of the ptr to free because it gets
            // shifted in the handle_packet func
            // read the packet
            p_ptr_copy = p_ptr;
            recv(fd, p_ptr, a2read, 0);
#ifdef __DEBUG_MODE
            printf("Received %s (%lu) from %d\n", p_ptr, strlen(p_ptr), fd);
#endif

            HANDLE_PACKET:
#ifdef __DEBUG_MODE
            printf("Parsing %s...\n", p_ptr);
#endif
            pckt = parse_packet(&p_ptr, &erc, player);
            // the packet has not yet been fully buffered,
            // we still need to wait for the rest of the packet
            if (erc == PACKET_NOT_YET_FULLY_BUFFERED) {
#ifdef __DEBUG_MODE
                printf("Not yet fully buffered...\n");
#endif
                goto FREE;
            }

            // the packet has been handled -> check if handled correctly
            // any error leads to a NULL pointer
            if (!pckt) {
                printf("Failed to parse packet: %d\n", erc);

// don't disconnect the player if the packet format is incorrect
#ifdef __DEBUG_MODE
                free_buffers(fd);
#else
                disconnect(player, &client_socks);
#endif
                goto FREE;
            }

            rval = handle_packet(player, pckt); // the packet format is valid, handle it
            switch (rval) {
                case 0:
                    break;
                case PACKET_ERR_INVALID_ID:
                    printf("A client sent a packet with invalid ID\n");
// don't disconnect if the packet data is invalid either
#ifdef __DEBUG_MODE
                    free_buffers(fd);
#else
                    disconnect(player, &client_socks);
#endif
                    goto FREE;
                case PACKET_ERR_STATE_OUT_OF_BOUNDS:
                    printf("A client sent a packet in a state that was out of bounds\n");
                    printf("This should not happen! Contact the authors for fix\n");
#ifdef __DEBUG_MODE
                    free_buffers(fd);
#else
                    disconnect(player, &client_socks);
#endif
                    goto FREE;
                case PACKER_ERR_INVALID_CLIENT_STATE:
                    printf("A client sent a packet in an invalid state\n");
#ifdef __DEBUG_MODE
                    free_buffers(fd);
#else
                    disconnect(player, &client_socks);
#endif
                    goto FREE;
                default:
                    printf("An error occurred when handling the packet (%d)\n", rval);
                    free_buffers(fd);
                    goto FREE;
            }

            // there could still be some leftover data in the buffer,
            // handle the rest of the data
            if (strlen(p_ptr) != 0) {
                goto HANDLE_PACKET;
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
    enable_keepalive(client_socket);
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

void disconnect(struct player* p, fd_set* client_socks) {
    int fd;
    if (!p || !client_socks) {
        return;
    }

    fd = p->fd;
    free_buffers(fd); // cleanup in the packet_validator (due to potential buffered header/data)
    remove_from_queue(p); // remove the player from the queue
    handle_disconnection(p); // cleanup in player_mngr and store the state

    close(fd);
    FD_CLR(fd, client_socks);
    printf("%s has disconnected\n", p->name);
}
