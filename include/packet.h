#ifndef SEMESTRALKA_PACKET_H
#define SEMESTRALKA_PACKET_H

#define PACKET_MAGIC_HEADER "CHESS"
#define PACKET_FORMAT "%s%02d%03d"

struct packet {
    unsigned int id;
    unsigned int len;
    char* data;
};

struct packet* create_packet(unsigned int id, unsigned int len, char* data);

#endif //SEMESTRALKA_PACKET_H
