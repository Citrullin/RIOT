/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example demonstrating of a IOTA address generation in RIOT
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "thread.h"
#include "xtimer.h"
#include "timex.h"

#include "periph/gpio.h"
#include "periph/spi.h"

int SPI_BUS = 0;
int SPI_CS = GPIO_PIN(0, 4);

/*int ModeReg	= 0x11 << 1;	// defines general modes for transmitting and receiving
int TxModeReg = 0x12 << 1;	// defines transmission data rate and framing
int RxModeReg = 0x13 << 1; // defines reception data rate and framing
    */

typedef enum {
    RFID_STATUS_OK,
    RFID_STATUS_NO_ROOM,
    RFID_STATUS_ERROR,
    RFID_STATUS_TIMEOUT,
    RFID_STATUS_COLLISION,
    RFID_STATUS_CRC_WRONG,
    RFID_STATUS_MIFARE_NACK,
} rfid_status_code_t;

typedef enum {
    CommandReg = 0x01 << 1,
    DivIrqReg = 0x05 << 1,
    ControlReg = 0x0C << 1,
    TxModeReg = 0x12 << 1,
    RxModeReg = 0x13 << 1,
    ModWidthReg = 0x24 << 1,
    ComIrqReg = 0x04 << 1,
    ErrorReg = 0x06 << 1,
    FIFOLevelReg = 0x0A << 1,
    FIFODataReg = 0x09 << 1,
    BitFramingReg = 0x0D << 1,
    CollReg = 0x0E << 1,
    CRCResultRegH = 0x21 << 1,	// shows the MSB and LSB values of the CRC calculation
    CRCResultRegL = 0x22 << 1,
} rfid_device_register_t;

typedef enum {
    CMD_DEVICE_TRANSRECEIVE = 0x0C << 1,
    CMD_DEVICE_IDLE = 0x00 << 1,
    CMD_CALCCRC = 0x03 << 1,
} rfid_device_command_t;

typedef enum {
    CMD_CARD_REQA = 0x26 << 1,
} rfid_card_command_t;

void rfid_start_transaction(void){
    //spi_acquire (spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk)
    spi_acquire (SPI_BUS, SPI_CS, SPI_MODE_1, SPI_CLK_100KHZ);
}

void rfid_end_transaction(void){
    spi_release (SPI_BUS);
}

char * get_status_name(rfid_status_code_t status){
    switch(status){
        case RFID_STATUS_OK: return "RFID_STATUS_OK";
        case RFID_STATUS_NO_ROOM: return "RFID_STATUS_NO_ROOM";
        case RFID_STATUS_ERROR: return "RFID_STATUS_ERROR";
        case RFID_STATUS_TIMEOUT: return "RFID_STATUS_TIMEOUT";
        case RFID_STATUS_COLLISION: return "RFID_STATUS_COLLISION";
        case RFID_STATUS_CRC_WRONG: return "RFID_STATUS_CRC_WRONG";
        case RFID_STATUS_MIFARE_NACK: return "RFID_STATUS_MIFARE_NACK";
    }

    return 0;
}

uint8_t rfid_write_register_byte(rfid_device_register_t addr, uint8_t byte){
    printf("write byte: 0x%x \n", byte & 0xff);
    rfid_start_transaction();
    uint8_t res = spi_transfer_reg(SPI_BUS, SPI_CS, ( addr & 0x7E ), byte);
    rfid_end_transaction();
    return res;
}

void rfid_write_register_bytes(rfid_device_register_t addr, void *bytes, size_t len){
    rfid_start_transaction();
    spi_transfer_regs(SPI_BUS, SPI_CS,( addr & 0x7E ), bytes, '\0', len);
    rfid_end_transaction();
}

uint8_t rfid_read_register_byte(rfid_device_register_t addr){
    printf("read register byte: 0x%x \n", addr & 0xff);
    //spi_transfer_reg (spi_t bus, spi_cs_t cs, uint8_t reg, uint8_t out);
    rfid_start_transaction();
    uint8_t res = spi_transfer_reg(SPI_BUS, SPI_CS, ( 0x80 | addr ), '\0');
    rfid_end_transaction();

    return res;
}

void rfid_read_register_bytes(
        rfid_device_register_t reg,	///< The register to read from. One of the PCD_Register enums.
        uint8_t count,			///< The number of bytes to read
        uint8_t *values,		///< Byte array to store the values in.
        uint8_t rxAlign
){

    if (count == 0) {
        return;
    }
    //Serial.print(F("Reading ")); 	Serial.print(count); Serial.println(F(" bytes from register."));
    uint8_t address = 0x80 | reg << 1;				// MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
    uint8_t index = 0;							// Index in values array.
    rfid_start_transaction();

    count--;								// One read is performed outside of the loop
    rfid_read_register_byte(address);					// Tell MFRC522 which address we want to read
    if (rxAlign) {		// Only update bit positions rxAlign..7 in values[0]
        // Create bit mask for bit positions rxAlign..7
        uint8_t mask = (0xFF << rxAlign) & 0xFF;
        // Read value and tell that we want to read the same address again.
        uint8_t value = rfid_read_register_byte(address);
        // Apply mask to both current value of values[0] and the new data in value.
        values[0] = (values[0] & ~mask) | (value & mask);
        index++;
    }
    while (index < count) {
        values[index] = rfid_read_register_byte(address);	// Read value and tell that we want to read the same address again.
        index++;
    }
    values[index] = rfid_read_register_byte(address);
    rfid_write_register_byte(address, 0); // Read the final byte. Send 0 to stop reading.
    rfid_end_transaction();
}

void rfid_reset_rxModeReg(void){
    puts("Start Reset RX MODE REG...");
    puts("Write bytes.");
    uint8_t res = rfid_write_register_byte(RxModeReg, 0x00);
    printf("RX Mod reg result: %d \n", res);
}

void rfid_reset_txModeReg(void){
    puts("Start TX Mode Reg...");
    puts("Write bytes.");
    uint8_t res = rfid_write_register_byte(TxModeReg, 0x00);
    printf("TX Mod Reg result: %d\n", res);
}

void rfid_reset_modWidthReg(void){
    puts("Start Mod Width Reg...");
    puts("Write Bytes.");
    uint8_t res = rfid_write_register_byte(ModWidthReg, 0x26);
    printf("Mod width reg result: %d\n", res);
}

void rfid_set_register_bit_mask(rfid_device_register_t reg,	///< The register to update. One of the PCD_Register enums.
                                uint8_t mask			///< The bits to set.
) {
    uint8_t tmp;
    tmp = rfid_read_register_byte(reg);
    rfid_write_register_byte(reg, tmp | mask);			// set bit mask
}

void rfid_device_clear_register_bit_mask(rfid_device_register_t reg, uint8_t mask){
    uint8_t tmp;
    puts("Read Register bit mask...");
    tmp = rfid_read_register_byte(reg);
    puts("Write Register bit mask...");
    rfid_write_register_byte(reg, tmp & (~mask));
}

rfid_status_code_t rfid_calc_crc(	uint8_t *data,		///< In: Pointer to the data to transfer to the FIFO for CRC calculation.
                       uint8_t length,	///< In: The number of bytes to transfer.
                       uint8_t *result	///< Out: Pointer to result buffer. Result is written to result[0..1], low byte first.
) {
    rfid_write_register_byte(CommandReg, CMD_DEVICE_IDLE);		// Stop any active command.
    rfid_write_register_byte(DivIrqReg, 0x04);				// Clear the CRCIRq interrupt request bit
    rfid_write_register_byte(FIFOLevelReg, 0x80);			// FlushBuffer = 1, FIFO initialization
    rfid_write_register_bytes(FIFODataReg, data, length);	// Write data to the FIFO
    rfid_write_register_byte(CommandReg, CMD_CALCCRC);		// Start the calculation

    // Wait for the CRC calculation to complete. Each iteration of the while-loop takes 17.73μs.
    // TODO check/modify for other architectures than Arduino Uno 16bit

    // Wait for the CRC calculation to complete. Each iteration of the while-loop takes 17.73us.
    for (uint16_t i = 5000; i > 0; i--) {
        // DivIrqReg[7..0] bits are: Set2 reserved reserved MfinActIRq reserved CRCIRq reserved reserved
        uint8_t n = rfid_read_register_byte(DivIrqReg);
        if (n & 0x04) {									// CRCIRq bit set - calculation done
            rfid_write_register_byte(CommandReg, CMD_DEVICE_IDLE);	// Stop calculating CRC for new content in the FIFO.
            // Transfer the result from the registers to the result buffer
            result[0] = rfid_read_register_byte(CRCResultRegL);
            result[1] = rfid_read_register_byte(CRCResultRegH);
            return RFID_STATUS_OK;
        }
    }
    // 89ms passed and nothing happend. Communication with the MFRC522 might be down.
    return RFID_STATUS_TIMEOUT;
}

rfid_status_code_t rfid_device_communicate_with_card(	uint8_t command,		///< The command to execute. One of the PCD_Command enums.
                                                         uint8_t waitIRq,		///< The bits in the ComIrqReg register that signals successful completion of the command.
                                                         uint8_t *sendData,		///< Pointer to the data to transfer to the FIFO.
                                                         uint8_t sendLen,		///< Number of bytes to transfer to the FIFO.
                                                         uint8_t *backData,		///< nullptr or pointer to buffer if data should be read back after executing the command.
                                                         uint8_t *backLen,		///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
                                                         uint8_t *validBits,	///< In/Out: The number of valid bits in the last byte. 0 for 8 valid bits.
                                                         uint8_t rxAlign,		///< In: Defines the bit position in backData[0] for the first bit received. Default 0.
                                                         bool checkCRC		///< In: True => The last two bytes of the response is assumed to be a CRC_A that must be validated.
) {
    puts("Communicate with card...");
    // Prepare values for BitFramingReg
    uint8_t txLastBits = validBits ? *validBits : 0;
    uint8_t bitFraming = (rxAlign << 4) + txLastBits;		// RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]

    rfid_write_register_byte(CommandReg, CMD_DEVICE_IDLE);			// Stop any active command.
    rfid_write_register_byte(ComIrqReg, 0x7F);					// Clear all seven interrupt request bits
    rfid_write_register_byte(FIFOLevelReg, 0x80);				// FlushBuffer = 1, FIFO initialization

    rfid_write_register_bytes(FIFODataReg, (void *) sendData, (size_t) sendLen);	// Write sendData to the FIFO
    rfid_write_register_byte(BitFramingReg, bitFraming);		// Bit adjustments
    rfid_write_register_byte(CommandReg, command);				// Execute the command
    if (command == CMD_DEVICE_TRANSRECEIVE) {
        rfid_set_register_bit_mask(BitFramingReg, 0x80);	// StartSend=1, transmission of data starts
    }

    // Wait for the command to complete.
    // In PCD_Init() we set the TAuto flag in TModeReg. This means the timer automatically starts when the PCD stops transmitting.
    // Each iteration of the do-while-loop takes 17.86μs.
    // TODO check/modify for other architectures than Arduino Uno 16bit
    uint16_t i;
    for (i = 2000; i > 0; i--) {
        uint8_t n = rfid_read_register_byte(ComIrqReg);	// ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq HiAlertIRq LoAlertIRq ErrIRq TimerIRq
        if (n & waitIRq) {					// One of the interrupts that signal success has been set.
            break;
        }
        if (n & 0x01) {						// Timer interrupt - nothing received in 25ms
            return RFID_STATUS_TIMEOUT;
        }
    }
    // 35.7ms and nothing happend. Communication with the MFRC522 might be down.
    if (i == 0) {
        return RFID_STATUS_TIMEOUT;
    }

    // Stop now if any errors except collisions were detected.
    uint8_t errorRegValue = rfid_read_register_byte(ErrorReg); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
    printf("Error Register: %i\n", errorRegValue);
    if (errorRegValue & 0x13) {	 // BufferOvfl ParityErr ProtocolErr
        return RFID_STATUS_ERROR;
    }

    uint8_t _validBits = 0;

    // If the caller wants data back, get it from the MFRC522.
    if (backData && backLen) {
        uint8_t n = rfid_read_register_byte(FIFOLevelReg);	// Number of bytes in the FIFO
        printf("Read byte N: %i", n);
        printf("Back length: %i", *backLen);
        if (n > *backLen) {
            return RFID_STATUS_NO_ROOM;
        }
        *backLen = n;											// Number of bytes returned
        rfid_read_register_bytes(FIFODataReg, n, backData, rxAlign);	// Get received data from FIFO
        _validBits = rfid_read_register_byte(ControlReg) & 0x07;		// RxLastBits[2:0] indicates the number of valid bits in the last received byte. If this value is 000b, the whole byte is valid.
        if (validBits) {
            *validBits = _validBits;
        }
    }

    // Tell about collisions
    if (errorRegValue & 0x08) {		// CollErr
        return RFID_STATUS_COLLISION;
    }

    // Perform CRC_A validation if requested.
    if (backData && backLen && checkCRC) {
        // In this case a MIFARE Classic NAK is not OK.
        if (*backLen == 1 && _validBits == 4) {
            return RFID_STATUS_MIFARE_NACK;
        }
        // We need at least the CRC_A value and all 8 bits of the last byte must be received.
        if (*backLen < 2 || _validBits != 0) {
            return RFID_STATUS_CRC_WRONG;
        }
        // Verify CRC_A - do our own calculation and store the control in controlBuffer.
        uint8_t controlBuffer[2];
        rfid_status_code_t status = rfid_calc_crc(&backData[0], *backLen - 2, &controlBuffer[0]);
        if (status != RFID_STATUS_OK) {
            return status;
        }
        if ((backData[*backLen - 2] != controlBuffer[0]) || (backData[*backLen - 1] != controlBuffer[1])) {
            return RFID_STATUS_CRC_WRONG;
        }
    }

    return RFID_STATUS_OK;
}

rfid_status_code_t rfid_device_transceive_data(	uint8_t *sendData,		///< Pointer to the data to transfer to the FIFO.
                                                   uint8_t sendLen,		///< Number of bytes to transfer to the FIFO.
                                                   uint8_t *backData,		///< nullptr or pointer to buffer if data should be read back after executing the command.
                                                   uint8_t *backLen,		///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
                                                   uint8_t *validBits,	///< In/Out: The number of valid bits in the last byte. 0 for 8 valid bits. Default nullptr.
                                                   uint8_t rxAlign,		///< In: Defines the bit position in backData[0] for the first bit received. Default 0.
                                                   bool checkCRC		///< In: True => The last two bytes of the response is assumed to be a CRC_A that must be validated.
) {
    uint8_t waitIRq = 0x30;		// RxIRq and IdleIRq
    return rfid_device_communicate_with_card
            (CMD_DEVICE_TRANSRECEIVE, waitIRq, sendData, sendLen, backData, backLen, validBits, rxAlign, checkCRC);
}


rfid_status_code_t rfid_request_card_typeA(uint8_t *bufferATQA,	///< The buffer to store the ATQA (Answer to request) in
                                           uint8_t *bufferSize){
    uint8_t validBits;
    rfid_status_code_t status;

    printf("BufferATQA: %i \n", *bufferATQA);
    printf("Buffersize: %i \n", *bufferSize);
    if (*bufferATQA == '\0' || *bufferSize < 2) {	// The ATQA response is 2 bytes long.
        return RFID_STATUS_NO_ROOM;
    }
    puts("Clear register bit mask...");
    rfid_device_clear_register_bit_mask(CollReg, 0x80);		// ValuesAfterColl=1 => Bits received after collision are cleared.
    validBits = 7;									// For REQA and WUPA we need the short frame format - transmit only 7 bits of the last (and only) byte. TxLastBits = BitFramingReg[2..0]

    uint8_t command = CMD_CARD_REQA;
    puts("Transreceive data...");
    status = rfid_device_transceive_data (&command, 1, bufferATQA, bufferSize, &validBits, '\0', false);
    if (status != RFID_STATUS_OK) {
        return status;
    }
    printf("Buffer Size: %i\n", *bufferSize);
    printf("Valid Bits: %i\n", validBits);
    if (*bufferSize != 2 || validBits != 0) {		// ATQA must be exactly 16 bits.
        return RFID_STATUS_ERROR;
    }
    return RFID_STATUS_OK;
}

bool rfid_is_new_card_present(void){
    puts("Is New Card present?");
    uint8_t bufferATQA[2];
    uint8_t bufferSize = sizeof(bufferATQA);

    rfid_reset_rxModeReg();
    rfid_reset_txModeReg();
    rfid_reset_modWidthReg();

    puts("Request Card Type A...");
    rfid_status_code_t result = rfid_request_card_typeA(bufferATQA, &bufferSize);
    printf("RFID status code: %s \n", get_status_name(result));
    return (result == RFID_STATUS_OK || result == RFID_STATUS_COLLISION);
}


#define INTERVAL (10U * US_PER_SEC)

int main(void)
{
    puts("SPI RFID");
    puts("=====================================");

    puts("SPI BUS INIT...");
    spi_init(SPI_BUS);

    puts("SPI CS INIT...");
    spi_init_cs(SPI_BUS, (spi_cs_t) GPIO_PIN(0, 4));
    puts("SPI CS initialized.");

    /*
    //spi_acquire (spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk)
    spi_acquire(SPI_BUS, SPI_CS, SPI_MODE_0, SPI_CLK_100KHZ);

    spi_transfer_byte (SPI_BUS, SPI_CS, true, 0x07);

    //spi_transfer_bytes (spi_t bus, spi_cs_t cs, bool cont, const void *out, void *in, size_t len)

    */

    xtimer_ticks32_t last_wakeup = xtimer_now();

    while(1) {
        xtimer_periodic_wakeup(&last_wakeup, INTERVAL);
        //printf("slept until %" PRIu32 "\n", xtimer_usec_from_ticks(xtimer_now()));

        bool is_new = rfid_is_new_card_present();
        printf("New card: %d\n", is_new);
    }

    return 0;
}