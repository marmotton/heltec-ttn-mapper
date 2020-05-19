#include <Arduino.h>
#include "config.h"

// LoRa
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// GPS
#include <TinyGPS++.h>
TinyGPSPlus gps;


// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }


const lmic_pinmap lmic_pins = {
    .nss = LMIC_NSS_PIN,
    .rxtx = LMIC_RXTX_PIN,
    .rst = LMIC_RST_PIN,
    .dio = {LMIC_DIO_PIN1, LMIC_DIO_PIN2, LMIC_DIO_PIN3},
};


// LoRa events
void onEvent(ev_t ev) {
    switch (ev) {
        case EV_TXCOMPLETE:
            // EV_TXCOMPLETE includes waiting for RX windows. Process the received message here.
            if (LMIC.dataLen) {
                Serial.print("Received LoRa message (");
                Serial.print(LMIC.dataLen);
                Serial.println(" bytes).");
            }
            break;

        default:
            break;
    }
}

void sendLoRa() {
    uint8_t message[11];

    // Scale latitude and longitude
    uint32_t latitude  = ( gps.location.lat() + 90 ) * ( 0xFFFFFF / 180.0 ); // range -90 to +90, mapped to 0 to 16'777'215 (3 bytes)
    uint32_t longitude = ( gps.location.lng() + 180 ) * ( 0xFFFFFF / 360.0 ); // range -180 to +180, mapped to 0 to 16'777'215 (3 bytes)
    uint16_t altitude  = ( gps.altitude.meters() + 200 ) * ( 0xFFFF / 5200.0 ); // range -200 to +5000, mapped to 0 to 65'535 (2 bytes)
    uint8_t sats = gps.satellites.value();
    uint8_t hdop = round(gps.hdop.value() / 10);  // hdop is given in 100ths
    uint8_t kmh = round(gps.speed.kmph());

    memcpy(message, &latitude, 3);
    memcpy(message + 3, &longitude, 3);
    memcpy(message + 6, &altitude, 2);
    message[8] = sats;
    message[9] = hdop;
    message[10] = kmh;

    Serial.println("Sending LoRa message...");
    LMIC_setTxData2(1, message, sizeof(message), 0);
}


void setup() {
    // USB serial port
    Serial.begin(115200);

    // GPS serial port
    Serial1.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // ABP authentication
    uint8_t appskey[] = _APPSKEY;
    uint8_t nwkskey[] = _NWKSKEY;
    u4_t DEVADDR = _DEVADDR;
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);

    // Setup european TTN channels (868MHz)
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI); 
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);

    // Disable link check validation (not supported by TTN yet)
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set spreading factor and power
    LMIC_setDrTxpow(LORA_SF, 14);
}


unsigned long last_lora_update = 0;

void loop() {
    while (Serial1.available() > 0 ){
        gps.encode(Serial1.read());
    }

    if (gps.location.isUpdated()) {
        Serial.println("\nGPS location updated:");
        Serial.print("Lat: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Lon: ");
        Serial.println(gps.location.lng(), 6);
        Serial.print("Alt: ");
        Serial.println(gps.altitude.meters(), 1);
        Serial.print("Sats: ");
        Serial.println(gps.satellites.value());
        Serial.print("HDOP: ");
        Serial.println(gps.hdop.value() / 100.0, 1);
        Serial.print("km/h: ");
        Serial.println(gps.speed.kmph(), 1);

        if (millis() - last_lora_update > LORA_MSG_INTERVAL_S * 1000) {
            sendLoRa();
            last_lora_update = millis();
        }
    }

    // Execute the LMIC scheduler
    os_runloop_once();
}
