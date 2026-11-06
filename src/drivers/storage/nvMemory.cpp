#include "nvMemory.h"

#ifdef NVMEM_SPIFFS

#include <SPIFFS.h>
#include <FS.h>
#include <ArduinoJson.h>

#include "../devices/device.h"
#include "storage.h"

nvMemory::nvMemory() : Initialized_(false){};

nvMemory::~nvMemory()
{
    if (Initialized_)
        SPIFFS.end();
};

/// @brief Save settings to config file on SPIFFS
/// @param TSettings* Settings to be saved.
/// @return true on success
bool nvMemory::saveConfig(TSettings* Settings)
{
    if (init())
    {
        // Save Config in JSON format
        Serial.println(F("SPIFS: Saving configuration."));

        // Create a JSON document
        StaticJsonDocument<512> json;
        json[JSON_SPIFFS_KEY_POOLURL] = Settings->PoolAddress;
        json[JSON_SPIFFS_KEY_POOLPORT] = Settings->PoolPort;
        json[JSON_SPIFFS_KEY_POOLPASS] = Settings->PoolPassword;
        json[JSON_SPIFFS_KEY_WALLETID] = Settings->BtcWallet;
        json[JSON_SPIFFS_KEY_TIMEZONE] = Settings->Timezone;
        json[JSON_SPIFFS_KEY_STATS2NV] = Settings->saveStats;
        json[JSON_SPIFFS_KEY_INVCOLOR] = Settings->invertColors;
        json[JSON_SPIFFS_KEY_BRIGHTNESS] = Settings->Brightness;

        if (json.overflowed())
        {
            Serial.println(F("SPIFS: Config too large, not saved"));
            return false;
        }

        // Atomar schreiben: erst in Temp-Datei, dann umbenennen. So bleibt bei
        // Stromausfall waehrend des Schreibens die alte Konfiguration erhalten.
        // (Der JSON-Inhalt wird bewusst nicht ins Serial-Log geschrieben: enthaelt Passwort.)
        const char* tmpFile = "/config.tmp";
        File configFile = SPIFFS.open(tmpFile, "w");
        if (!configFile)
        {
            // Error, file did not open
            Serial.println("SPIFS: Failed to open config file for writing");
            return false;
        }

        if (serializeJson(json, configFile) == 0)
        {
            // Error writing file
            Serial.println(F("SPIFS: Failed to write to file"));
            configFile.close();
            SPIFFS.remove(tmpFile);
            return false;
        }
        // Close file
        configFile.close();
        SPIFFS.remove(JSON_CONFIG_FILE);
        if (!SPIFFS.rename(tmpFile, JSON_CONFIG_FILE))
        {
            Serial.println(F("SPIFS: Failed to rename config file"));
            return false;
        }
        return true;
    };
    return false;
}

/// @brief Load settings from config file located in SPIFFS.
/// @param TSettings* Struct to update with new settings.
/// @return true on success
bool nvMemory::loadConfig(TSettings* Settings)
{
    // Uncomment if we need to format filesystem
    // SPIFFS.format();

    // Load existing configuration file
    // Read configuration from FS json

    if (init())
    {
        if (SPIFFS.exists(JSON_CONFIG_FILE))
        {
            // The file exists, reading and loading
            File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
            if (configFile)
            {
                Serial.println("SPIFS: Loading config file");
                StaticJsonDocument<512> json;
                DeserializationError error = deserializeJson(json, configFile);
                configFile.close();
                // (JSON-Inhalt nicht ins Serial-Log: enthaelt Passwort)
                if (!error)
                {
                    Settings->PoolAddress = json[JSON_SPIFFS_KEY_POOLURL] | Settings->PoolAddress;
                    // strlcpy mit Laengenbegrenzung; nur kopieren, wenn Schluessel vorhanden
                    // (sonst waere Quelle == Ziel, ueberlappendes strcpy).
                    if (json.containsKey(JSON_SPIFFS_KEY_POOLPASS))
                        strlcpy(Settings->PoolPassword, json[JSON_SPIFFS_KEY_POOLPASS] | "", sizeof(Settings->PoolPassword));
                    if (json.containsKey(JSON_SPIFFS_KEY_WALLETID))
                        strlcpy(Settings->BtcWallet, json[JSON_SPIFFS_KEY_WALLETID] | "", sizeof(Settings->BtcWallet));
                    if (json.containsKey(JSON_SPIFFS_KEY_POOLPORT))
                    {
                        int port = json[JSON_SPIFFS_KEY_POOLPORT].as<int>();
                        if (port > 0 && port <= 65535) Settings->PoolPort = port;
                    }
                    if (json.containsKey(JSON_SPIFFS_KEY_TIMEZONE))
                    {
                        int tz = json[JSON_SPIFFS_KEY_TIMEZONE].as<int>();
                        if (tz >= -12 && tz <= 14) Settings->Timezone = tz;
                    }
                    if (json.containsKey(JSON_SPIFFS_KEY_STATS2NV))
                        Settings->saveStats = json[JSON_SPIFFS_KEY_STATS2NV].as<bool>();
                    if (json.containsKey(JSON_SPIFFS_KEY_INVCOLOR)) {
                        Settings->invertColors = json[JSON_SPIFFS_KEY_INVCOLOR].as<bool>();
                    } else {
                        Settings->invertColors = false;
                    }
                    if (json.containsKey(JSON_SPIFFS_KEY_BRIGHTNESS)) {
                        Settings->Brightness = json[JSON_SPIFFS_KEY_BRIGHTNESS].as<int>();
                    } else {
                        Settings->Brightness = 250;
                    }
                    return true;
                }
                else
                {
                    // Error loading JSON data
                    Serial.println("SPIFS: Error parsing config file!");
                }
            }
            else
            {
                Serial.println("SPIFS: Error opening config file!");
            }
        }
        else
        {
            Serial.println("SPIFS: No config file available!");
        }
    }
    return false;
}

/// @brief Delete config file from SPIFFS
/// @return true on successs
bool nvMemory::deleteConfig()
{
    Serial.println("SPIFS: Erasing config file..");
    return SPIFFS.remove(JSON_CONFIG_FILE); //Borramos fichero
}

/// @brief Prepare and mount SPIFFS
/// @return true on success
bool nvMemory::init()
{
    if (!Initialized_)
    {
        Serial.println("SPIFS: Mounting File System...");
        // May need to make it begin(true) first time you are using SPIFFS
        Initialized_ = SPIFFS.begin(false) || SPIFFS.begin(true);
        Initialized_ ? Serial.println("SPIFS: Mounted") : Serial.println("SPIFS: Mounting failed.");
    }
    else
    {
        Serial.println("SPIFS: Already Mounted");
    }
    return Initialized_;
};

#else

nvMemory::nvMemory() {}
nvMemory::~nvMemory() {}
bool nvMemory::saveConfig(TSettings* Settings) { return false; }
bool nvMemory::loadConfig(TSettings* Settings) { return false; }
bool nvMemory::deleteConfig() { return false; }
bool nvMemory::init() { return false; }


#endif //NVMEM_TYPE