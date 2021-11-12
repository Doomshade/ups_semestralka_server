#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// kvuli iotctl
#include <sys/ioctl.h>
#include "../include/server.h"
#include "../include/packet_handler.h"
#include "../include/packet_validator.h"
#include "../include/player_mngr.h"

void disconnect(int fd, fd_set* client_socks);

int start_server(unsigned port) {
    int server_socket, client_socket, fd, rval, a2read; // .., .., a client's FD, return value, amount to read from recv
    // char p_buf[BUFSIZ]; // packet buffer
    char* p_ptr = NULL;
    char* p_ptr_copy = NULL; // to later free the p_ptr
    char ip[64]; // buffer to output IP:port
    unsigned int len_addr;
    struct sockaddr_in my_addr, peer_addr;
    fd_set client_socks, tests; // fd sets (for socks too)
    int param = 1, erc = 0;
    struct packet* pckt;
    struct player* player = NULL;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    rval = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &param, sizeof(int));

    if (rval == -1) {
        printf("setsockopt ERR\n");
    }

    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    memset(&peer_addr, 0, sizeof(struct sockaddr_in));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    rval = bind(server_socket, (struct sockaddr*) &my_addr, sizeof(struct sockaddr_in));

    inet_ntop(AF_INET, &my_addr.sin_addr, ip, sizeof(ip));
    if (!rval) {
        printf("Successfully bound the socket to %s:%d\n", ip, port);
    } else {
        printf("Could not bind the socket to %s:%d\n", ip, port);
        return rval;
    }

    rval = listen(server_socket, 5);
    if (!rval) {
        printf("Started listening to incoming connections...\n");
    } else {
        printf("Failed to start listening\n");
        return rval;
    }

    // initialize packet validator and register all packets
    init_pvalidator();
    register_packets();

    // clear the descriptors and add the server socket
    FD_ZERO(&client_socks);
    FD_SET(server_socket, &client_socks);

    for (;;) {
        // copy the old fd_set to the new one (select destroys them)
        tests = client_socks;

        rval = select(FD_SETSIZE, &tests, (fd_set*) NULL, (fd_set*) NULL, (struct timeval*) 0);

        if (rval < 0) {
            printf("Select err: %d\n", rval);
            return -1;
        }

        for (fd = 3; fd < FD_SETSIZE; fd++) {
            if (FD_ISSET(fd, &tests)) {
                if (fd == server_socket) {
                    client_socket = accept(server_socket, (struct sockaddr*) &peer_addr, &len_addr);
                    FD_SET(client_socket, &client_socks);
                    printf("A client has connected and was assigned fd %d\n", client_socket);

                    // this adds the client to the player array
                    handle_new_connection(client_socket);
                    continue;
                }
                // client sent some data
                // amount to read
                ioctl(fd, FIONREAD, &a2read);

                // something happened on the client side
                if (a2read <= 0) {
                    disconnect(fd, &client_socks);
                    continue;
                }

                // the player could have reconnected, look him up
                // this will not do anything the first time the player connects,
                // only once the HELLO packet is sent the reconnection will attempt
                // to look up the client by his name and update the player's FD
                // to this new one
                // handle_possible_reconnection(fd, &player);

                p_ptr = malloc(sizeof(char) * (a2read + 1));
                memset(p_ptr, 0, sizeof(char) * (a2read + 1));
                p_ptr_copy = p_ptr; // store the copy of the ptr to free because it gets
                // shifted in the handle_packet func
                // read the packet
                recv(fd, p_ptr, a2read, 0);
                printf("Received %s (%lu) from %d\n", p_ptr, strlen(p_ptr), fd);

                lookup_player_by_fd(fd, &player); // look for the connected player
                // this should NOT return NULL as we handled the new connection

                if (!player) {
                    printf("An error occurred during the creation of client (FD=%d)...\n", fd);
                    continue;
                }

                HANDLE_PACKET:
                printf("Parsing %s...\n", p_ptr);
                pckt = parse_packet(&p_ptr, &erc, player);

                // the packet has not yet been fully buffered,
                // we still need to wait for the rest of the packet
                if (erc == PACKET_NOT_YET_FULLY_BUFFERED) {
                    printf("Not yet fully buffered...\n");
                    continue;
                }

                // the packet has been handled -> check if handled correctly
                // any error leads to a NULL pointer
                if (!pckt) {
                    printf("Failed to parse packet: %d\n", erc);
                    disconnect(fd, &client_socks);
                    goto FREE;
                }
                rval = handle_packet(player, pckt); // the packet is valid, handle it

                // the packet was sent in an invalid state
                if (rval == CLIENT_SENT_PACKET_IN_INVALID_STATE) {
                    printf("A client has sent a packet in an invalid state\n");
                    disconnect(fd, &client_socks);
                    goto FREE;
                }

                // there could still be some leftover data in the buffer,
                // handle the rest of the data
                if (strlen(p_ptr) != 0) {
                    goto HANDLE_PACKET;
                }

                // no more data, free the malloced packet data
                FREE:
                free(pckt);
                free(p_ptr_copy);

            }
        }

    }
}

void disconnect(int fd, fd_set* client_socks) {
    free_buffers(fd); // cleanup in the packet_validator (due to potential buffered header/data)
    handle_disconnection(fd); // cleanup in player_mngr and store the state
    close(fd);
    FD_CLR(fd, client_socks);
    printf("A client has disconnected\n");
}
