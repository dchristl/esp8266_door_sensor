# ESP82666 Door sensor
## Introduction

This simple application can recognize the open state of connected door sensors and then calls an url.

## Workflow
- Wifi and url configuration by serial if not already done 
- Check all pins, if any is set to high (door open)
- Calling the configured Url
- Going to deep sleep for 5 minutes, if all doors were closed or 1 minute if any door was open
- Repeat

## Possible Pins
- D2 (GPIO 4)
- D3 (GPIO 0)
- D5 (GPIO 14)
- D6 (GPIO 13)
- D7 (GPIO 12)

## Reset configuration
Configuration can be resetted by connecting Pin D1 (GPIO5) to ground.
Because there is deep sleep involved, you have to enable this on your ESP8266. Normally by connecting reset and D0.
