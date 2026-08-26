#include "nvs_pairing.h"
#include <Preferences.h>

Preferences preferences;

#define NVS_NAMESPACE "pairing"
#define NVS_KEY       "peer_mac"


bool savePeerMAC(const uint8_t *mac)
{
    preferences.begin(NVS_NAMESPACE, false);

    char macString[18];

    snprintf(macString,
             sizeof(macString),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0],
             mac[1],
             mac[2],
             mac[3],
             mac[4],
             mac[5]);

    preferences.putString(NVS_KEY, macString);

    preferences.end();

    Serial.print("Saved peer MAC to NVS: ");
    Serial.println(macString);

    return true;
}


bool loadPeerMAC(uint8_t *mac)
{
    preferences.begin(NVS_NAMESPACE, true);

    String macString = preferences.getString(NVS_KEY, "");

    preferences.end();

    if (macString.length() == 0)
    {
        return false;
    }

    int values[6];

    if (sscanf(macString.c_str(),
               "%02X:%02X:%02X:%02X:%02X:%02X",
               &values[0],
               &values[1],
               &values[2],
               &values[3],
               &values[4],
               &values[5]) != 6)
    {
        return false;
    }

    for (int i = 0; i < 6; i++)
    {
        mac[i] = (uint8_t)values[i];
    }

    return true;
}


bool isPeerMACStored()
{
    preferences.begin(NVS_NAMESPACE, true);

    bool exists = preferences.isKey(NVS_KEY);

    preferences.end();

    return exists;
}


void clearPeerMAC()
{
    preferences.begin(NVS_NAMESPACE, false);

    preferences.remove(NVS_KEY);

    preferences.end();

    Serial.println("Peer MAC removed from NVS");
}


