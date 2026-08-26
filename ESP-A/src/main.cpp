#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// =====================================================
// CHANGE ONLY THIS MAC ON EACH ESP32
// =====================================================

// On ESP-A -> put ESP-B MAC here
// On ESP-B -> put ESP-A MAC here
uint8_t peerMAC[] = {
    0xBB, 0xBB, 0xBB,
    0xBB, 0xBB, 0xBB
};


// =====================================================
// Receive callback
// =====================================================

void OnDataRecv(
    const uint8_t *mac,
    const uint8_t *data,
    int len)
{
    Serial.println();
    Serial.println("==============================");
    Serial.println("DATA RECEIVED");

    Serial.print("From MAC: ");

    for (int i = 0; i < 6; i++)
    {
        if (mac[i] < 0x10)
            Serial.print("0");

        Serial.print(mac[i], HEX);

        if (i < 5)
            Serial.print(":");
    }

    Serial.println();

    Serial.print("Length: ");
    Serial.println(len);

    Serial.print("Data: ");

    for (int i = 0; i < len; i++)
    {
        Serial.print((char)data[i]);
    }

    Serial.println();

    Serial.println("==============================");
}


// =====================================================
// Send callback
// =====================================================

void OnDataSent(
    const uint8_t *mac_addr,
    esp_now_send_status_t status)
{
    Serial.print("Send status: ");

    if (status == ESP_NOW_SEND_SUCCESS)
    {
        Serial.println("SUCCESS");
    }
    else
    {
        Serial.println("FAIL");
    }
}


// =====================================================
// Setup
// =====================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("ESP-NOW BIDIRECTIONAL TEST");
    Serial.println("==============================");

    // -------------------------------------------------
    // WiFi
    // -------------------------------------------------

    WiFi.mode(WIFI_STA);

    Serial.print("My MAC: ");
    Serial.println(WiFi.macAddress());


    // -------------------------------------------------
    // ESP-NOW initialization
    // -------------------------------------------------

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ERROR: ESP-NOW initialization failed");
        return;
    }

    Serial.println("ESP-NOW initialized");


    // -------------------------------------------------
    // Register callbacks
    // -------------------------------------------------

    esp_now_register_recv_cb(OnDataRecv);

    esp_now_register_send_cb(OnDataSent);


    // -------------------------------------------------
    // Add peer
    // -------------------------------------------------

    esp_now_peer_info_t peerInfo = {};

    memcpy(peerInfo.peer_addr, peerMAC, 6);

    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("ERROR: Failed to add peer");
        return;
    }

    Serial.println("Peer added");

    Serial.println("Ready!");
}


// =====================================================
// Loop
// =====================================================

void loop()
{
    static unsigned long lastSend = 0;

    if (millis() - lastSend >= 2000)
    {
        lastSend = millis();

        char message[50];

        snprintf(
            message,
            sizeof(message),
            "Hello from %s",
            WiFi.macAddress().c_str()
        );

        Serial.print("Sending: ");
        Serial.println(message);

        esp_err_t result = esp_now_send(
            peerMAC,
            (uint8_t *)message,
            strlen(message)
        );

        if (result != ESP_OK)
        {
            Serial.print("Send error: ");
            Serial.println(result);
        }
    }
}