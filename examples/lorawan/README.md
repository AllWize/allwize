# AllWize to LoRaWAN - Proof of Concept

These two example codes are a PoC to send messages from a Wize node to The Things Network (or any other LoRaWAN server). 

## Implementation

The PoC is based on two protocol implementations. 

* LoRaWAN frame support for nodes
* Semtech legacy packet forwarder running on an ESP8266 acting as gateway

First the Wize payload in the node (sender) is encoded as a LoRaWAN compatible frame. This means that the total payload size is bigger than with just LoRaWAN, since its wrapped with Wize headers and checksum. The encription is done in the sender, just like with a regular LoRaWan node, using both the network and the application session keys. Only Activation-by-Personalisation (ABP) is supported.

The Wize message is received by a Wize single-channel gateway. The gateway does not decrypt the message and acts as a dumb bridge building an UDP frame compatible with the Semtech legacy packet forwarder protocol. It presents itself as an FSK gateway, providing useful information in the channel, datarate and RSSI fields.

Current limitations due to implementation details are:

* Only uplink support
* Only ABP support
* Payload size is big (i.e. more time on air)

## Usage

### Node (Sender)

To encode the message as a LoRaWAN compatible frame you just have to use the AllWize_LoRaWAN class instead of the AllWize class and provide proper activation keys. The AllWize_LoRaWAN class wraps up the AllWize class providing LoRaWAN frame encoding and AES encryption. The it relies on the underneath AllWize class to send the LoRaWAN message inside a Wize frame.

Here you have a fragment of the sample code:

```
#include "AllWize_LoRaWAN.h"
AllWize_LoRaWAN allwize(RX_PIN, TX_PIN, RESET_PIN);

void wizeSetup() {

    // Init communication to the module
    allwize.begin();
    if (!allwize.waitForReady()) {
        Serial.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    // Wize settings
    allwize.slave();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);

    // LoRaWan settings
    allwize.joinABP(DEVADDR, APPSKEY, NWKSKEY);

}

```

### Gateway

The gateway code is meant to be run on an ESP8266 board with Wize module. This is the standard setup for our AllWize G1 gateways (a Wemos D1 with an AllWize K1 shield).

The gateway code is split into different "modules" (components) so it's easier to manage and understand. These modules are responsible for different actions:

* **main**: main entry point, initializes the different components and manages the polling
* **debug**: serial debug definitions
* **wifi**: handles WiFi connectivity
* **ntp**: keeps local RTC in time using NTP
* **wize**: handles incomming Wize messages
* **lorawan**: keeps gateway "alive" by pinging the server every 30s, encodes and sends messages inside UDP frames to the server

To configure the different modules, first copy the `configuration.sample.h` file to `configuration.h` and edit it providing proper information. At least you should provide the WiFi connection credentials and the Gateway configuration (location, type, description and admin email).

You will need two thrid party libraries (both available in the Arduino Library Manager):

* **EspSoftwareSerial** by @plerup (https://github.com/plerup/espsoftwareserial)
* **NtpClientLib** by @gmag11 (https://github.com/gmag11/NtpClient)

## License

Copyright (C) 2018-2020 by AllWize ([http://allwize.io](http://allwize.io))

AllWize library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

AllWize library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with AllWize library.  If not, see <http://www.gnu.org/licenses/>.
