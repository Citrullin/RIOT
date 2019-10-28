#ifndef IOTA_DOOF_LOCK_CONFIG_H
#define IOTA_DOOF_LOCK_CONFIG_H

// DEBUG, INFO, WARN, ERROR, FATAL, OFF
#define LOG_LEVEL DEBUG

#define DEVICE_NAME "IOTA door lock"
#define DEVICE_INFO_MODEL "IOTA HID door lock with %s microcontroller."
#define DEVICE_INFO_MANUFACTURER "IOTA HID door lock (Version: %s)\n"

#define GATEWAY_UART_PORT UART_DEV(1)
#define GATEWAY_UART_BAUDRATE 115200

#define SPI_BUS SPI_DEV(0)
#define SPI_CS_PIN GPIO_PIN(1, 4)
#define SPI_MODE SPI_MODE_0
#define SPI_CLK SPI_CLK_100KHZ

#define GATEWAY_RESPONSE_BUFFER_SIZE 500

#define GATEWAY_WAIT_SLEEP_TIME 1000
#define GATEWAY_REPEAT_SLEEP_TIME 50

//Todo: remove, because Android App and NodeJS don't use Protobuf
#define DECODE_SCHEMA_SIZE 20
#define DECODE_METHOD_SIZE 20
#define DECODE_ID_SIZE 100

#define RESPONSE_BUFFER_SIZE 500
#define ENCODE_BUFFER_SIZE 500
#define DID_BUFFER_SIZE 5000
#define ERROR_BUFFER_SIZE 100

#define AES_KEY_SIZE 128
#define AES_BLOCK_SIZE 256

#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A28

//Todo: Use UUID string representation
#define GATT_SERVICE_UUID  0x35, 0xa3, 0xc7, 0x14, 0x3f, 0x03, 0x3e, 0xa1, \
                           0xfc, 0x48, 0x37, 0xd1, 0xb3, 0x38, 0xce, 0x1b

#define GATT_DID_CHARACTERISTIC_UUID   0x35, 0xa3, 0xc7, 0x14, 0x3e, 0x03, 0x3e, 0xa1, \
                                       0xfa, 0x48, 0x37, 0xd1, 0xb3, 0x38, 0xce, 0x1b

#define GATT_ACCESS_STATUS_CHARACTERISTIC_UUID  0x35, 0xa3, 0xc7, 0x14, 0x3e, 0x03, 0x3e, 0xa1, \
                                                0xfb,0x48, 0x37, 0xd1, 0xb3, 0x38, 0xce, 0x1b

#define GATT_ERROR_MESSAGE_CHARACTERISTIC_UUID   0x62, 0x17, 0x99, 0x7e, 0x50, 0x27, 0x38, 0xba, \
                                                                    0x3b, 0x4f, 0x70, 0x30, 0x86, 0x83, 0xf2, 0x35

#endif //IOTA_DOOF_LOCK_CONFIG_H
