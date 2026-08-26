#ifndef ESPNOW_PAIRING_H
#define ESPNOW_PAIRING_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// ============================================================
// PACKET TYPES
// ============================================================

#define PAIR_REQUEST   1
#define PAIR_RESPONSE  2
#define PAIR_CONFIRM   3

// ============================================================
// PAIRING PACKET
// ============================================================

typedef struct
{
    uint8_t type;

} PairPacket;

// ============================================================
// GLOBAL VARIABLES
// ============================================================

extern bool paired;

extern bool responseSent;

extern uint8_t peerMAC[6];

extern unsigned long lastPairRequest;

// ============================================================
// INITIALIZATION
// ============================================================

bool initESPNowPairing();

// ============================================================
// PAIRING FUNCTIONS
// ============================================================

void sendPairRequest();

void sendPairResponse(const uint8_t *mac);

void sendPairConfirm();

void loadSavedPair();

// ============================================================
// PEER MANAGEMENT
// ============================================================

bool addPeer(const uint8_t *mac);

// ============================================================
// UTILITY
// ============================================================

void printMAC(const uint8_t *mac);

// ============================================================
// ESP-NOW CALLBACKS
// ============================================================

void OnDataSent(
    const uint8_t *mac_addr,
    esp_now_send_status_t status
);

void OnDataRecv(
    const uint8_t *mac,
    const uint8_t *data,
    int len
);

#endif