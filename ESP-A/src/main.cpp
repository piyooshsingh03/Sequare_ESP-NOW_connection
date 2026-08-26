#include <Arduino.h>

#include "espnow_pairing.h"

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
    // Initialize ESP-NOW
    // --------------------------------------------------------

    if (!initESPNowPairing())
    {
        Serial.println(
            "ESP-NOW setup failed"
        );

        return;
    }

    Serial.println();
    Serial.println(
        "Waiting for another ESP32..."
    );

    // --------------------------------------------------------
    // Check saved pairing
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "Checking saved pairing..."
    );

    loadSavedPair();
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