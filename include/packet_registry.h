#ifndef SEMESTRALKA_PACKET_REGISTRY_H
#define SEMESTRALKA_PACKET_REGISTRY_H

#include "game_mngr.h"
#include "player_mngr.h"

// ++ PACKET RETURN CODES ++
#define PACKET_RESP_OK 0x00
#define PACKET_RESP_OK_INVALID_DATA 0xA0
#define PACKET_RESP_ERR_INVALID_DATA 0xA1
#define PACKET_RESP_ERR_NOT_RECVD 0xA2
#define PACKET_ERR_INVALID_ID 0xA3
#define PACKET_ERR_STATE_OUT_OF_BOUNDS 0xA4
// -- PACKET RETURN CODES --

// ++ PACKET OFFSETS ++
#define PACKET_OUT_OFFSET 0x80
#define PACKET_OUT(packet_in) packet_in + PACKET_OUT_OFFSET
#define PACKET_IN(offset, id) offset + id
// IN: 0-31 0x(00-1F); OUT: 128-159 0x(80-9F)
#define JUST_CONNECTED_P_OFFSET 0x00
// IN: 32-63 0x(20-3F) OUT: 160-191 0x(A0-BF)
#define LOGGED_IN_P_OFFSET 0x20
// IN: 64-96 0x(40-5F) OUT: 192-223 0x(C0-DF)
#define QUEUE_P_OFFSET 0x40
// IN: 96-127 0x(60-7F) OUT: 224-255 0x(E0-FF)
#define PLAY_P_OFFSET 0x60
// -- PACKET OFFSETS --

// ++ PACKET RESPONSES ++
#define RESPONSE_VALID "OK"
#define RESPONSE_INVALID "REJECTED"
// -- PACKET RESPONSES --

/**
 * The packet
 */
struct packet {
    unsigned int id; // The packet ID
    unsigned int len; // The packet data length/size
    char* data; // The data/payload of the packet

};

/**
 * Valid packets in the JUST_CONNECTED state
 */
enum just_connected_p {
    // A player has just joined the server
    // and sends their name
    HELLO_IN = PACKET_IN(JUST_CONNECTED_P_OFFSET, 0),

    // Sent to a player as a confirmation,
    // because a player with that name can
    // already exist
    HELLO_OUT = PACKET_OUT(HELLO_IN),
};

/**
 * Valid packets in the LOGGED_IN state
 */
enum logged_in_p {
    // A player wants to join/leave the queue
    QUEUE_IN = PACKET_IN(LOGGED_IN_P_OFFSET, 0),

    // Sent to a player to inform him
    // that he was/was not put to the queue
    QUEUE_OUT = PACKET_OUT(QUEUE_IN),

    RECONNECT_OUT = PACKET_OUT(PACKET_IN(LOGGED_IN_P_OFFSET, 1))
};

/**
 * Valid packets in the QUEUE state
 */
enum queue_p {

    LEAVE_QUEUE_IN = PACKET_IN(QUEUE_P_OFFSET, 0),
    LEAVE_QUEUE_OUT = PACKET_OUT(LEAVE_QUEUE_IN),

    // Sent to a player to inform him
    // that an opponent was found and
    // is put to a game
    GAME_START_OUT = PACKET_OUT(PACKET_IN(QUEUE_P_OFFSET, 1))
};

/**
 * Valid packets in the PLAY state
 */
enum play_p {
    // A player moved a piece
    MOVE_IN = PACKET_IN(PLAY_P_OFFSET, 0),

    // Sent to the sender as "MOVE_INVALID"
    // or sent "OK" to both players
    // to update their game
    MOVE_OUT = PACKET_OUT(MOVE_IN),

    // A player wants to offer a draw
    DRAW_OFFER_IN = PACKET_IN(PLAY_P_OFFSET, 1),

    // Sent to a player that the opponent offered a draw
    // or sent as a response to the draw offer
    // to the player who offered it
    DRAW_OFFER_OUT = PACKET_OUT(DRAW_OFFER_IN),

    // A player wants to resign
    RESIGN_IN = PACKET_IN(PLAY_P_OFFSET, 2),

    // A game is finished, sent to both players
    GAME_FINISH_OUT = PACKET_OUT(RESIGN_IN),

    // A message sent by a player
    MESSAGE_IN = PACKET_IN(PLAY_P_OFFSET, 3),

    // A message sent to a player
    MESSAGE_OUT = PACKET_OUT(MESSAGE_IN),

    // The opponent's name is not included in the game start packet,
    // so we need to inform the client of the opponent's username
    OPPONENT_NAME_OUT = PACKET_OUT(PACKET_IN(PLAY_P_OFFSET, 4)),

    MOVE_RESPONSE = PACKET_OUT(PACKET_IN(PLAY_P_OFFSET, 5)),

    KEEP_ALIVE_IN = PACKET_IN(PLAY_P_OFFSET, 0x1F),
    // A keep alive packet to check whether the player
    // is still connected
    KEEP_ALIVE_OUT = PACKET_OUT(PACKET_IN(PLAY_P_OFFSET, 0x1F))

};

/**
 * The packet handling function
 */
typedef int packet_handle(struct player* fd, char* data);

/**
 * Sends the packet to the player. To create a packet use
 * create_packet(unsigned, unsigned, char*). Packet IDs are
 * in enums "just_connected_p", "logged_in_p", "queue_p", "play_p".
 * @param pl the player
 * @param pc the packet
 * @return 0 if everything went alright
 */
int send_packet(struct player* pl, unsigned int id, char* data);

/**
 * Frees the packet from the memory
 * @param pc the packet
 */
void free_packet(struct packet** pc);

/**
 * Creates a packet
 * @param id the ID
 * @param len the packet data length/size
 * @param data the data
 * @return
 */
struct packet* create_packet(unsigned int id, const char* data);

/**
 * Gets the packet handler based on the packet ID
 * @param id the packet ID
 * @return the handle or NULL
 */
packet_handle* get_handler(unsigned int id, enum player_state pstate, int* erc);

/**
 * Registers the packets in the memory
 */
void init_preg();

#endif //SEMESTRALKA_PACKET_REGISTRY_H
