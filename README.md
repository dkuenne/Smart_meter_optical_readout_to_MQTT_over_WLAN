# Smart meter optical readout to MQTT over WLAN

# !!! WORK IN PROGRESS !!!

## Description

With this project you can get your energy smart meters data over the IR interface and send it to your MQTT broker over WLAN.

---

## Hardware

- ESP32-C3 Super Mini
- [ELV Smart Meter ES-IEC read head](https://de.elv.com/p/elv-bausatz-energiesensor-fuer-smart-meter-es-iec-stromzaehlerausleser-P142148/)
  - used on EFR smart meter
  - see full compatibility list on the manufacturers page

---

## Data

- OBIS Bezug Total 1.8.0
- OBIS Momentanverbrauch 16.7.0
- OBIS Geliefert Total 2.8.0
- Spannung L1
- Spannung L2
- Spannung L3
- Strom L1
- Strom L2
- Strom L3
- Winkel 1
- Winkel 2
- Phasenwinkel L1
- Phasenwinkel L2
- Phasenwinkel L3
- Frequenz

---

# License

The Smart meter optical readout to MQTT over WLAN project by Dominik Künne is licensed under CC BY-NC 4.0. To view a copy of this license, visit  https://creativecommons.org/licenses/by-nc/4.0

Please see the LICENSE file for details.