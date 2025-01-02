#ifndef ENET_H
#define ENET_H

#include <stddef.h>
#include <stdint.h>
#include <cstdlib>  // For malloc, free, realloc
#include <cstring>  // For memcpy
#include <cassert>  // For assert
#include <cstdio>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

/* Safe sprintf macro */
//#define sprintf_sd(d, fmt, ...) char d[256]; snprintf(d, sizeof(d), fmt, __VA_ARGS__)
//#define sprintf_sd(d, fmt, ...) char d[256]; snprintf(d, sizeof(d), fmt, __VA_ARGS__)

// ENet constants and typedefs
typedef uint8_t enet_uint8;
typedef uint16_t enet_uint16;
typedef uint32_t enet_uint32;
typedef uint64_t enet_uint64;

// Constants
typedef enum {
    ENET_PEER_STATE_DISCONNECTED = 0,
    ENET_PEER_STATE_CONNECTING,
    ENET_PEER_STATE_ACKNOWLEDGING_CONNECT,
    ENET_PEER_STATE_CONNECTION_PENDING,
    ENET_PEER_STATE_CONNECTION_SUCCEEDED,
    ENET_PEER_STATE_CONNECTED,
    ENET_PEER_STATE_DISCONNECT_LATER,
    ENET_PEER_STATE_DISCONNECTING,
    ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT,
    ENET_PEER_STATE_ZOMBIE
} ENetPeerState;

#define ENET_VERSION 0
#define ENET_HOST_ANY 0
#define ENET_PORT_ANY 0
#define ENET_PACKET_FLAG_RELIABLE 1
#define ENET_PACKET_FLAG_UNSEQUENCED 2
#define ENET_PACKET_FLAG_NO_ALLOCATE 4
#define ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT 8
#define ENET_PACKET_FLAG_SENT 16

#define ENET_EVENT_TYPE_CONNECT 10
#define ENET_EVENT_TYPE_RECEIVE 11
#define ENET_EVENT_TYPE_DISCONNECT 12
#define ENET_PEER_PACKET_THROTTLE_SCALE 12

#define ENET_SOCKET_NULL 0
#define ENET_SOCKET_WAIT_RECEIVE 0
#define ENET_SOCKET_TYPE_DATAGRAM 0
#define ENET_SOCKET_TYPE_STREAM 0


/* Placeholder macros */
#define ENET_HOST_BANDWIDTH_LIMIT 0
#define ENET_HOST_TO_NET_16(value) (value)
#define ENET_HOST_TO_NET_32(value) (value)
#define ENET_NET_TO_HOST_16(value) (value)
#define ENET_NET_TO_HOST_32(value) (value)


// Structures

// Forward declaration for ENetPeer
typedef struct _ENetPeer ENetPeer;
typedef int ENetSocket;

// Structures
typedef struct _ENetHost {
    ENetPeer *peers; // Simulate an array of peers
    size_t peerCount;
} ENetHost;

typedef struct _ENetPacket {
    uint8_t *data;
    size_t dataLength;
    uint32_t flags;
    int referenceCount; // Added to fix the error
} ENetPacket;

typedef struct _ENetAddress {
    uint32_t host;  // IP address
    uint16_t port;  // Port number
} ENetAddress;

typedef struct _ENetPeer {
    int state;          // Connection state (as a placeholder for ENetPeerState)
    ENetAddress address; // Peer address
    void *data;          // Optional user data
} ENetPeer;

typedef struct _ENetEvent {
    int type;             // Event type
    ENetPeer *peer;       // Associated peer
    ENetPacket *packet;   // Associated packet
} ENetEvent;

typedef struct _ENetBuffer {
    size_t dataLength;
    void *data;
} ENetBuffer;

// Function stubs
static inline int enet_initialize(void) {
	printf("enet dummy initialize\n");
    return 0; // Return 0 to indicate success in this dummy implementation
}

static inline void enet_deinitialize(void) {
    // No operation for the dummy implementation
}

static inline int in6_equal(uint32_t a, uint32_t b) {
    return a == b;
}


static inline ENetHost* enet_host_create(const ENetAddress *address, size_t peerCount, size_t channelLimit, uint32_t incomingBandwidth, uint32_t outgoingBandwidth) {
    ENetHost *host = (ENetHost *)malloc(sizeof(ENetHost));
    host->peers = (ENetPeer *)malloc(sizeof(ENetPeer) * peerCount);
    host->peerCount = peerCount;

    for (size_t i = 0; i < peerCount; ++i) {
        host->peers[i].state = ENET_PEER_STATE_DISCONNECTED;
    }
    return host;
}

static inline void enet_host_destroy(ENetHost *host) {
    if (host) {
        free(host->peers);
        free(host);
    }
}

static inline void enet_host_bandwidth_limit(ENetHost *host, uint32_t incomingBandwidth, uint32_t outgoingBandwidth) {}

static inline void enet_host_flush(ENetHost *host) {}

static inline int enet_host_service(ENetHost *host, ENetEvent *event, uint32_t timeout) {
    return 0; // Dummy implementation
}

static inline ENetPeer* enet_host_connect(ENetHost *host, const ENetAddress *address, size_t channelCount, uint32_t data) {
    for (size_t i = 0; i < host->peerCount; ++i) {
        if (host->peers[i].state == ENET_PEER_STATE_DISCONNECTED) {
            host->peers[i].state = ENET_PEER_STATE_CONNECTED;
            host->peers[i].address = *address;
            return &host->peers[i];
        }
    }
    return NULL;
}

static inline void enet_peer_reset(ENetPeer *peer) {
    if (peer) {
        peer->state = ENET_PEER_STATE_DISCONNECTED;
    }
}

static inline void enet_peer_disconnect(ENetPeer *peer, uint32_t data) {
    if (peer) {
        peer->state = ENET_PEER_STATE_DISCONNECTED;
    }
}

static inline void enet_peer_throttle_configure(ENetPeer *peer, uint32_t interval, uint32_t acceleration, uint32_t deceleration) {}

static inline ENetPacket* enet_packet_create(const void *data, size_t dataLength, uint32_t flags) {
    ENetPacket* packet = (ENetPacket*)malloc(sizeof(ENetPacket));
    if (packet == NULL) {
        return NULL;
    }

    packet->data = (uint8_t*)malloc(dataLength);
    if (packet->data == NULL) {
        free(packet);
        return NULL; 
    }
    //memcpy(packet->data, data, dataLength);
    packet->dataLength = dataLength;
    packet->flags = flags;
    packet->referenceCount = 1;

    return packet;
}

static inline void enet_packet_destroy(ENetPacket *packet) {
    if (packet && --packet->referenceCount <= 0) {
        free(packet->data);
        free(packet);
    }
}

static inline void enet_packet_resize(ENetPacket *packet, size_t dataLength) {
    packet->data = (uint8_t*)realloc(packet->data, dataLength);
    packet->dataLength = dataLength;
}

static inline int enet_address_set_host(ENetAddress *address, const char *name) {
    address->host = 127 << 24 | 1; // Dummy IP (127.0.0.1)
    return 0;
}

static inline void enet_host_broadcast(ENetHost *host, uint8_t channelID, ENetPacket *packet) {}

static inline int enet_address_get_host(const ENetAddress *address, char *hostName, size_t nameLength) {
    snprintf(hostName, nameLength, "dummy-host");
    return 0;
}

// Keep only one implementation:
/*
static inline void enet_peer_throttle_configure(
    ENetPeer *peer, 
    uint32_t interval, 
    uint32_t acceleration, 
    uint32_t deceleration
) {
    // Dummy logic
    (void)peer;
    (void)interval;
    (void)acceleration;
    (void)deceleration;
}
*/

static inline int enet_peer_send(ENetPeer *peer, uint8_t channelID, ENetPacket *packet) {
    return 0; // Simulated success
}

/* Dummy socket-related functions */
static inline ENetSocket enet_socket_create(int type) {
    return ENET_SOCKET_NULL + 1; // Simulated socket creation
}

static inline int enet_socket_send(ENetSocket socket, const ENetAddress *address, const void *buffer, size_t bufferLength) {
    return (socket != ENET_SOCKET_NULL) ? bufferLength : -1; // Simulated send
}

static inline int enet_socket_receive(ENetSocket socket, ENetAddress *address, void *buffer, size_t bufferLength) {
    return (socket != ENET_SOCKET_NULL) ? bufferLength : -1; // Simulated receive
}

/*static inline int enet_socket_wait(ENetSocket socket, uint32_t *events, uint32_t timeout) {
    if (socket != ENET_SOCKET_NULL) {
        *events = ENET_SOCKET_WAIT_RECEIVE; // Simulate an event
        return 1; // Simulated success
    }
    return -1;
}*/

static inline void enet_socket_destroy(ENetSocket socket) {
    // Dummy implementation
}

static inline int enet_socket_connect(ENetSocket socket, const ENetAddress *address) {
    return 0; // Dummy implementation for successful connection
}

static inline int enet_socket_wait(ENetSocket socket, enet_uint32 *events, uint32_t timeout) {
    if (socket != ENET_SOCKET_NULL) {
        *events = ENET_SOCKET_WAIT_RECEIVE; // Simulate an event
        return 1; // Simulated success
    }
    return -1;
}


#endif // ENET_H
