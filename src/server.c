#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
// kvuli iotctl
#include <sys/ioctl.h>
#include "../include/server.h"
#include "../include/game_mngr.h"
#include "../include/packet_handler.h"

int send_packet(struct player* p, char* buf, long* len) {
    long total = 0; // how many bytes we've sent
    long bytesleft = *len; // how many we have left to send
    long n;
    while (total < *len) {
        n = send(p->fd, buf + total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    *len = total; // return number actually sent here
    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int start_server(unsigned port) {
    int server_socket, client_socket, fd, rval, a2read;
    char buf[BUFSIZ];
    socklen_t len_addr;
    struct sockaddr_in my_addr, peer_addr;
    fd_set client_socks, tests; // mnozina file deskriptoru (mj. i napr. socketu)

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    int param = 1;
    rval = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &param, sizeof(int));

    if (rval == -1) {
        printf("setsockopt ERR\n");
    }

    memset(&my_addr, 0, sizeof(struct sockaddr_in));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    rval = bind(server_socket, (struct sockaddr*) &my_addr, sizeof(struct sockaddr_in));

    char ip[64];
    inet_ntop(AF_INET, &my_addr.sin_addr, ip, sizeof(ip));
    if (rval == 0)
        printf("Successfully bound the socket to %s:%d\n", ip, port);
    else {
        printf("Could not bind the socket to %s:%d\n", ip, port);
        return rval;
    }

    rval = listen(server_socket, 5);
    if (rval == 0) {
        printf("Started listening to incoming connections...\n");
    } else {
        printf("Failed to start listening\n");
        return rval;
    }

    register_packets();

    // vyprazdnime sadu deskriptoru a vlozime server socket
    FD_ZERO(&client_socks);
    FD_SET(server_socket, &client_socks);

    for (;;) {
        // zkopirujeme si fd_set do noveho, stary by byl znicen (select ho modifikuje)
        tests = client_socks;

        // sada deskriptoru je po kazdem volani select prepsana sadou deskriptoru kde se neco delo
        rval = select(FD_SETSIZE, &tests, (fd_set*) NULL, (fd_set*) NULL, (struct timeval*) 0);

        if (rval < 0) {
            printf("Select ERR\n");
            return -1;
        }

        // vynechavame stdin, stdout, stderr
        for (fd = 3; fd < FD_SETSIZE; fd++) {
            // je dany socket v sade fd ze kterych lze cist ?
            if (FD_ISSET(fd, &tests)) {
                // je to server socket? prijmeme nove spojeni
                if (fd == server_socket) {
                    client_socket = accept(server_socket, (struct sockaddr*) &peer_addr, &len_addr);
                    FD_SET(client_socket, &client_socks);
                    printf("Pripojen novy klient a pridan do sady socketu\n");
                } else // je to klientsky socket? prijmem data
                {
                    // pocet bajtu co je pripraveno ke cteni
                    ioctl(fd, FIONREAD, &a2read);
                    // mame co cist
                    if (a2read > 0) {
                        memset(buf, 0, BUFSIZ);
                        recv(fd, &buf, BUFSIZ, 0);
                        printf("Prijato %s\n", buf);

                        // offset by 10 cuz of the magic, id, and len
                        struct packet* p = create_packet(0x0, 4, buf + 11);
                        handle_packet(fd, p);
                    } else // na socketu se stalo neco spatneho
                    {
                        close(fd);
                        FD_CLR(fd, &client_socks);
                        printf("Klient se odpojil a byl odebran ze sady socketu\n");
                    }
                }
            }
        }

    }
}