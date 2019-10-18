# How to use

## Prerequisites

- Installed Linux: Also works in a VM

## Flashing

### 1. Connect the two micro USB cables to your board and PC/Notebook

### 2. [Install ARM gcc toolchain](https://github.com/iotaledger/documentation/blob/develop/iot/0.1/how-to-guides/install-arm-gcc-toolchain.md)

### 3. Install python 3 and pip

```bash
sudo apt-get install -y python3-all
sudo apt-get install -y python-pip3
```

### 4. If your Linux distribution has Python 2 pre-installed, make sure that it uses Python 3 by default

```bash
echo "alias python=python3" >> ~/.bashrc
```

### 5. [Install the J-Link or J-Link OB toolchain](https://gnu-mcu-eclipse.github.io/debug/jlink/install/)

### 6. Flash the application to the microcontroller

Check the port before. Use the port your device is connected to. Example:
$PORT = /dev/ttyACM3

```bash
cd examples/nimble_door_lock
make BOARD=nrf52840dk PORT=$PORT term flash
```

### 7. Run the server application

You are now in the RIOT OS terminal. The server application is a registered command.
You ran run it with `server start`. If you want to see all available commands. Execute `help`.


## Bluetooth characteristics

The characteristic UUIDs are defined in config.h
Three characteristics are available: access status, did and error_message

### Access status (read-only)

The access status characteristic can be read in order to get the current status of the lock.
The different status codes are defined in the AccessStatus.proto file.

### Did (read and write)

The Did characteristic is accessible for read and write. 
The write accepts a protobuf encoded message. The message for the Did characteristic is defined in DidRequest.proto.

### Error (read-only)

The error characteristic responds with the last error. The response is encoded as protobuf message. 
The definitions are available in ErrorResponse.proto


