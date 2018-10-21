# Milestone 1- Stranger Things LED
Designed for the TI MSP430G2553 Microcontroller

## Design
A UART serial connection sends a packett to the microcontroller, then the microcontroller uses hardware Pulse Width Modulation to send current through a circuit controlling an RGB LED. This entire unit is a node. There are transmit and recieve wires which can be connected to other nodes so that a single packet can send control information to multiple nodes.

## Functionality
The UART packet is sent to the first node in bytes, this can be sent from the serial source or from the prior node. The first byte tells the node how long the packet measured in bytes. The node sends the length in bytes minus the three it will use to control its LED on to the next node. The following three bytes describe the intensity of each color in the RGB LED. The number is rated between 0 and 255 for a 0% to 100% intensity. Each bit following these is sent on to the next node.
