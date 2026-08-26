#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// ============================================================
// ESP-NOW BROADCAST ADDRESS
// ============================================================

uint8_t broadcastAddress[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// ============================================================
// VARIABLES
// ============================================================

bool peerFound = false;

uint8_t peerMAC[6];

unsigned long lastDiscoveryTime = 0;

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

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("Send status: ");

    if (status == ESP_NOW_SEND_SUCCESS)
        Serial.println("SUCCESS");
    else
        Serial.println("FAIL");
}

// ============================================================
// RECEIVE CALLBACK
// ============================================================

void OnDataRecv(
    const uint8_t *mac,
    const uint8_t *incomingData,
    int len)
{
    Serial.println();
    Serial.println("==============================");
    Serial.println("DATA RECEIVED");

    Serial.print("From MAC: ");
    printMAC(mac);
    Serial.println();

    Serial.print("Length: ");
    Serial.println(len);

    Serial.print("Data: ");

    for (int i = 0; i < len; i++)
    {
        Serial.print((char)incomingData[i]);
    }

    Serial.println();

    // --------------------------------------------------------
    // CHECK FOR DISCOVERY REQUEST
    // --------------------------------------------------------

    if (len == 11 &&
        memcmp(incomingData, "PAIR_REQUEST", 11) == 0)
    {
        Serial.println("PAIR REQUEST RECEIVED");

        // Save sender MAC
        memcpy(peerMAC, mac, 6);

        peerFound = true;

        Serial.print("Discovered peer MAC: ");
        printMAC(peerMAC);
        Serial.println();

        // ----------------------------------------------------
        // SEND RESPONSE BACK
        // ----------------------------------------------------

        const char response[] = "PAIR_RESPONSE";

        esp_err_t result = esp_now_send(
            peerMAC,
            (const uint8_t *)response,
            sizeof(response) - 1
        );

        if (result == ESP_OK)
        {
            Serial.println("PAIR_RESPONSE sent");
        }
        else
        {
            Serial.println("PAIR_RESPONSE send failed");
        }
    }

    // --------------------------------------------------------
    // CHECK FOR DISCOVERY RESPONSE
    // --------------------------------------------------------

    if (len == 12 &&
        memcmp(incomingData, "PAIR_RESPONSE", 12) == 0)
    {
        Serial.println("PAIR RESPONSE RECEIVED");

        // Save sender MAC
        memcpy(peerMAC, mac, 6);

        peerFound = true;

        Serial.print("Discovered peer MAC: ");
        printMAC(peerMAC);
        Serial.println();
    }
}

// ============================================================
// SEND DISCOVERY REQUEST
// ============================================================

void sendDiscoveryRequest()
{
    const char request[] = "PAIR_REQUEST";

    Serial.println();
    Serial.println("==============================");
    Serial.println("Sending discovery request...");
    Serial.println("==============================");

    esp_err_t result = esp_now_send(
        broadcastAddress,
        (const uint8_t *)request,
        sizeof(request) - 1
    );

    if (result == ESP_OK)
    {
        Serial.println("Discovery request sent");
    }
    else
    {
        Serial.println("Discovery request failed");
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
    Serial.println("ESP-NOW AUTO DISCOVERY");
    Serial.println("==============================");

    // --------------------------------------------------------
    // WiFi initialization
    // --------------------------------------------------------

    WiFi.mode(WIFI_STA);

    delay(100);

    Serial.print("My MAC: ");
    Serial.println(WiFi.macAddress());

    // --------------------------------------------------------
    // ESP-NOW initialization
    // --------------------------------------------------------

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW initialization FAILED");
        return;
    }

    Serial.println("ESP-NOW initialized");

    // --------------------------------------------------------
    // Register callbacks
    // --------------------------------------------------------

    esp_now_register_send_cb(OnDataSent);

    esp_now_register_recv_cb(OnDataRecv);

    // --------------------------------------------------------
    // Add broadcast peer
    // --------------------------------------------------------

    esp_now_peer_info_t peerInfo = {};

    memcpy(
        peerInfo.peer_addr,
        broadcastAddress,
        6
    );

    peerInfo.channel = 0;

    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add broadcast peer");
        return;
    }

    Serial.println("Broadcast peer added");

    Serial.println();
    Serial.println("Waiting for another ESP32...");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // If peer has NOT been discovered
    // send discovery request every 2 seconds
    // --------------------------------------------------------

    if (!peerFound)
    {
        if (millis() - lastDiscoveryTime >= 2000)
        {
            lastDiscoveryTime = millis();

            sendDiscoveryRequest();
        }
    }
    else
    {
        // ----------------------------------------------------
        // Peer discovered
        // ----------------------------------------------------

        static bool printed = false;

        if (!printed)
        {
            printed = true;

            Serial.println();
            Serial.println("==============================");
            Serial.println("PEER DISCOVERED");
            Serial.println("==============================");

            Serial.print("Peer MAC: ");
            printMAC(peerMAC);
            Serial.println();

            Serial.println("Discovery complete.");
        }
    }

    delay(10);
}