# PPM2CRSF
A converter for PPM signals to CRSF (Crossfire) serial protocol for the ESP32 Arduino platform.
The main purpose for this is recycling old/vintage transmitters to make them connectable to
an ExpressLRS JR transmitter module or an ExpressLRS "RX as TX".

This code has been tested with an ESP32-C3 Pro Mini, a Radiomaster Ranger Micro 2.4 GHz ELRS module
and a vintage Multiplex MC 1010 Transmitter.

```mermaid
graph LR;
    Transmitter-->|PPM|ESP32;
    ESP32-->|CRSF|ExpressLRS-Module;
```
The signal for input to a JR module is inverted. To use the code for an "RX as TX" receiver, you
probably have to change the `INVERT_SIGNAL` definition to `false` (not tested yet).

>[!Note]
>Original code can be found here: https://github.com/tnioa/PPM2CRFS.
>Unfortunately, this code did not work for me, so
>I did some bugfixing and (hopefully) enhanced readability.
