#include "lmic.h"

// TTN keys
#include "ttn_keys.h"

// Time between LoRa messages
#define LORA_MSG_INTERVAL_S 20

// LoRa spreading factor
#define LORA_SF DR_SF7

// Pins
#define LMIC_NSS_PIN 18
#define LMIC_RXTX_PIN LMIC_UNUSED_PIN
#define LMIC_RST_PIN 14
#define LMIC_DIO_PIN1 26
#define LMIC_DIO_PIN2 33
#define LMIC_DIO_PIN3 32

#define GPS_RX_PIN 23
#define GPS_TX_PIN 22
#define GPS_BAUDRATE 9600
