# PPM2CRSF
A converter for PPM signals to CRSF (Crossfire) serial protocol for the ESP32 Arduino platform.
The main purpose for this is recycling old/vintage transmitters to make them connectable to
an ExpressLRS JR transmitter module or an ExpressLRS "RX as TX".

This code has been tested with an ESP32-C3 Pro Mini, a Radiomaster Ranger Micro 2.4 GHz ELRS module,
a BetaFPV Nano receiver (programmed as "RX as TX") and a vintage Multiplex MC 1010 Transmitter.

```mermaid
graph LR;
    TX[RC Transmitter]-->|PPM Signal|ESP32;
    ESP32-->|CRSF|E[ExpressLRS-Module or\nRX as TX];
```
The signal for input to a JR module is inverted. To use the code for an "RX as TX" receiver, you
have to change the `INVERT_SIGNAL` definition to `false`.

| Board        | Status                | Remarks                                                              |
| ------------ | --------------------- | -------------------------------------------------------------------- |
| ESP32        | ✅ Recommended        | 420000 Baud, internal UART Hardware inverter                         |
| Arduino Nano | ✅ works              | 115200 Baud safe, 420000 Baud, needs external inverter for JR Module |
| RP2040       | Probably working      | Not yet tested                                                       |

## Schematic diagrams

PDF: [SBUS2CRSF.pdf](https://github.com/user-attachments/files/30149794/SBUS2CRSF.pdf)

| <img width="998" height="399" alt="scheme1" src="https://github.com/user-attachments/assets/9ef70b11-fa41-42f1-82c1-c897da21aceb" /> |
| :--: |
| Using a BetaFPV Receiver Rx as Tx |

| <img width="998" height="414" alt="scheme2" src="https://github.com/user-attachments/assets/2485108b-2174-480f-9eb8-cb95364c3208" /> |
| :--: |
| Using a JR Module |
