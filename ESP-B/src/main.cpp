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
// BROADCAST MAC
// ============================================================

uint8_t broadcastAddress[] =
{
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF
};

// ============================================================
// PAIRING PACKET
// ============================================================

typedef struct
{
    uint8_t type;

} PairPacket;

// ============================================================
// VARIABLES
// ============================================================

bool paired = false;

bool responseSent = false;

uint8_t peerMAC[6];

unsigned long lastPairRequest = 0;

// ============================================================
// PRINT MAC
// ============================================================

void printMAC(const uint8_t *mac)
{
    for (int i = 0; i < 6; i++)
    {
        if (i > 0)
            Serial.print(":");

        if (mac[i] < 0x10)
            Serial.print("0");

        Serial.print(mac[i], HEX);
    }
}

// ============================================================
// SEND CALLBACK
// ============================================================

void OnDataSent(
    const uint8_t *mac_addr,
    esp_now_send_status_t status)
{
    Serial.print("Send status: ");

    if (status == ESP_NOW_SEND_SUCCESS)
        Serial.println("SUCCESS");
    else
        Serial.println("FAIL");
}

// ============================================================
// ADD PEER
// ============================================================

bool addPeer(const uint8_t *mac)
{
    // Already exists?
    if (esp_now_is_peer_exist(mac))
    {
        return true;
    }

    esp_now_peer_info_t peerInfo = {};

    memcpy(
        peerInfo.peer_addr,
        mac,
        6
    );

    peerInfo.channel = 0;

    peerInfo.encrypt = false;

    esp_err_t result =
        esp_now_add_peer(&peerInfo);

    if (result == ESP_OK)
    {
        Serial.print("Peer added: ");
        printMAC(mac);
        Serial.println();

        return true;
    }

    Serial.print("Failed to add peer. Error: ");
    Serial.println(result);

    return false;
}

// ============================================================
// SEND PAIR RESPONSE
// ============================================================

void sendPairResponse(const uint8_t *mac)
{
    PairPacket packet;

    packet.type = PAIR_RESPONSE;

    Serial.println();
    Serial.println("Sending PAIR_RESPONSE...");

    esp_err_t result =
        esp_now_send(
            mac,
            (uint8_t *)&packet,
            sizeof(packet)
        );

    if (result == ESP_OK)
    {
        Serial.println("PAIR_RESPONSE queued");
    }
    else
    {
        Serial.println("PAIR_RESPONSE failed");
    }
}

// ============================================================
// SEND PAIR CONFIRM
// ============================================================

void sendPairConfirm()
{
    PairPacket packet;

    packet.type = PAIR_CONFIRM;

    Serial.println();
    Serial.println("Sending PAIR_CONFIRM...");

    esp_err_t result =
        esp_now_send(
            peerMAC,
            (uint8_t *)&packet,
            sizeof(packet)
        );

    if (result == ESP_OK)
    {
        Serial.println("PAIR_CONFIRM queued");
    }
    else
    {
        Serial.println("PAIR_CONFIRM failed");
    }
}

// ============================================================
// SEND PAIR REQUEST
// ============================================================

void sendPairRequest()
{
    PairPacket packet;

    packet.type = PAIR_REQUEST;

    Serial.println();
    Serial.println("==============================");
    Serial.println("Sending PAIR_REQUEST");
    Serial.println("==============================");

    esp_err_t result =
        esp_now_send(
            broadcastAddress,
            (uint8_t *)&packet,
            sizeof(packet)
        );

    if (result == ESP_OK)
    {
        Serial.println("PAIR_REQUEST sent");
    }
    else
    {
        Serial.println("PAIR_REQUEST failed");
    }
}

// ============================================================
// RECEIVE CALLBACK
// ============================================================

void OnDataRecv(
    const uint8_t *mac,
    const uint8_t *data,
    int len)
{
    // --------------------------------------------------------
    // Check packet size
    // --------------------------------------------------------

    if (len != sizeof(PairPacket))
    {
        Serial.println("Unknown packet received");
        return;
    }

    PairPacket packet;

    memcpy(
        &packet,
        data,
        sizeof(PairPacket)
    );

    // ========================================================
    // PAIR REQUEST
    // ========================================================

    if (packet.type == PAIR_REQUEST)
    {
        Serial.println();
        Serial.println("==============================");
        Serial.println("PAIR_REQUEST RECEIVED");
        Serial.println("==============================");

        Serial.print("Request from: ");

        printMAC(mac);

        Serial.println();

        // ----------------------------------------------------
        // If already paired, ignore request
        // ----------------------------------------------------

        if (paired)
        {
            Serial.println(
                "Already paired - ignoring request"
            );

            return;
        }

        // ----------------------------------------------------
        // Save sender MAC
        // ----------------------------------------------------

        memcpy(
            peerMAC,
            mac,
            6
        );

        Serial.print("Candidate peer: ");

        printMAC(peerMAC);

        Serial.println();

        // ----------------------------------------------------
        // Add candidate as ESP-NOW peer
        // ----------------------------------------------------

        if (!addPeer(peerMAC))
        {
            Serial.println(
                "Could not add candidate peer"
            );

            return;
        }

        // ----------------------------------------------------
        // Send response
        // ----------------------------------------------------

        sendPairResponse(peerMAC);

        responseSent = true;
    }

    // ========================================================
    // PAIR RESPONSE
    // ========================================================

    else if (packet.type == PAIR_RESPONSE)
    {
        Serial.println();
        Serial.println("==============================");
        Serial.println("PAIR_RESPONSE RECEIVED");
        Serial.println("==============================");

        Serial.print("Response from: ");

        printMAC(mac);

        Serial.println();

        // ----------------------------------------------------
        // Already paired?
        // ----------------------------------------------------

        if (paired)
        {
            Serial.println(
                "Already paired"
            );

            return;
        }

        // ----------------------------------------------------
        // Save peer MAC
        // ----------------------------------------------------

        memcpy(
            peerMAC,
            mac,
            6
        );

        Serial.print("Peer MAC: ");

        printMAC(peerMAC);

        Serial.println();

        // ----------------------------------------------------
        // Add peer
        // ----------------------------------------------------

        if (!addPeer(peerMAC))
        {
            Serial.println(
                "Failed to add peer"
            );

            return;
        }

        // ----------------------------------------------------
        // Send confirmation
        // ----------------------------------------------------

        sendPairConfirm();

        // ----------------------------------------------------
        // Mark paired
        // ----------------------------------------------------

        paired = true;

        Serial.println();
        Serial.println("==============================");
        Serial.println("PAIRING COMPLETE");
        Serial.println("==============================");
    }

    // ========================================================
    // PAIR CONFIRM
    // ========================================================

    else if (packet.type == PAIR_CONFIRM)
    {
        Serial.println();
        Serial.println("==============================");
        Serial.println("PAIR_CONFIRM RECEIVED");
        Serial.println("==============================");

        Serial.print("Confirmed by: ");

        printMAC(mac);

        Serial.println();

        // ----------------------------------------------------
        // Already paired?
        // ----------------------------------------------------

        if (paired)
        {
            Serial.println(
                "Already paired"
            );

            return;
        }

        // ----------------------------------------------------
        // Save peer MAC
        // ----------------------------------------------------

        memcpy(
            peerMAC,
            mac,
            6
        );

        // ----------------------------------------------------
        // Add peer
        // ----------------------------------------------------

        if (!addPeer(peerMAC))
        {
            Serial.println(
                "Failed to add peer"
            );

            return;
        }

        // ----------------------------------------------------
        // Mark paired
        // ----------------------------------------------------

        paired = true;

        Serial.println();
        Serial.println("==============================");
        Serial.println("PAIRING COMPLETE");
        Serial.println("==============================");

        Serial.print("Partner MAC: ");

        printMAC(peerMAC);

        Serial.println();
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("ESP-NOW PAIRING TEST");
    Serial.println("==============================");

    // --------------------------------------------------------
    // WiFi
    // --------------------------------------------------------

    WiFi.mode(WIFI_STA);

    delay(100);

    Serial.print("My MAC: ");

    Serial.println(
        WiFi.macAddress()
    );

    // --------------------------------------------------------
    // ESP-NOW
    // --------------------------------------------------------

    if (esp_now_init() != ESP_OK)
    {
        Serial.println(
            "ESP-NOW initialization FAILED"
        );

        return;
    }

    Serial.println(
        "ESP-NOW initialized"
    );

    // --------------------------------------------------------
    // Callbacks
    // --------------------------------------------------------

    esp_now_register_send_cb(
        OnDataSent
    );

    esp_now_register_recv_cb(
        OnDataRecv
    );

    // --------------------------------------------------------
    // Add broadcast peer
    // --------------------------------------------------------

    esp_now_peer_info_t broadcastPeer = {};

    memcpy(
        broadcastPeer.peer_addr,
        broadcastAddress,
        6
    );

    broadcastPeer.channel = 0;

    broadcastPeer.encrypt = false;

    if (esp_now_add_peer(&broadcastPeer) != ESP_OK)
    {
        Serial.println(
            "Failed to add broadcast peer"
        );

        return;
    }

    Serial.println(
        "Broadcast peer added"
    );

    Serial.println();
    Serial.println(
        "Waiting for another ESP32..."
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // ========================================================
    // NOT PAIRED
    // ========================================================

    if (!paired)
    {
        if (millis() - lastPairRequest >= 2000)
        {
            lastPairRequest = millis();

            sendPairRequest();
        }
    }

    // ========================================================
    // PAIRED
    // ========================================================

    else
    {
        static bool printed = false;

        if (!printed)
        {
            printed = true;

            Serial.println();
            Serial.println(
                "NORMAL MODE"
            );

            Serial.print(
                "Partner MAC: "
            );

            printMAC(peerMAC);

            Serial.println();

            Serial.println(
                "Pairing stopped."
            );
        }
    }

    delay(10);
}