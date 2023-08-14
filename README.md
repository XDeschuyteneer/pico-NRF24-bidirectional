# pico-NRF24-bidirectional
Bidirection NRF24 communication on rpi pico (2 SPI, 2 NRF24, 1 single rpi pico)

## Requirement

* Raspberry pi pico
* PICO SDK installed
* CMake
* toolchain (gcc/etc.)
* 2 x NRF24L01+ module
* breadboard and cables

## Wiring

We will be using 2 SPI interfaces on the pico. One for each NRF24 module.
Change defines in main.cpp if you want to use other pins.

### First NRF24 module on spi0:

| rpi | NRF  |
| --- | ---- |
| 4   | MISO |
| 3   | MOSI |
| 2   | SCK  |
| 5   | CSN  |
| 6   | CE   |

Don't forget to connect VCC and GND.

### First NRF24 module on spi0:

| rpi | NRF  |
| --- | ---- |
| 12   | MISO |
| 11   | MOSI |
| 10   | SCK  |
| 13   | CSN  |
| 14   | CE   |

Don't forget to connect VCC and GND.

### Power supply

Connect 3.3V to VCC on both NRF24 modules.
This can be done on the rpi pico, but the power supply is not very stable, so at high rates, the NRF24 modules will not work properly. Prefer external power supply (lab power supply for example)

This example should work by using 3.3V/GND from pico, but only with minimum power RF24_PA_MIN (as defined in main.cpp)

### Address configuration

This test only uses 2 addresses set (a single Rx and a single Tx pipe).
Each rpi pico should use a set (inverted) to be able on one side to Tx and the other side Rx on the same address.
So Tx/Rx are aligned on both units.

To do so, I use the GPIO 1 on the rpi (short GND and GPIO1) on one of the rpi pico (not the other).

## Compile/Flash

you can simply plug both rpi on your computer and run `./start.sh` to compile and flash the 2 rpi pico.

But you can also compile manually:
```
mkdir build
cd build
cmake ..
make
```

Then flash the binary found at `build/Main.uf2`

## Monitor

Both Rpi will print Rx/Tx messages on the serial port (USB).
I put in the Tx payload their unique ID (based on memory ID).

So open 2 terminals and run one minicom in each terminal:
```
minicom -b 115200 -D /dev/TTY_PICO1
minicom -b 115200 -D /dev/TTY_PICO2
```

You should see on pico1 something like that:
```
Rx (me:E6605481DB9C8634): pipe=1, payload=msg from E6614104036E912B cnt=118384
Tx (me:E6605481DB9C8634): payload=msg from E6605481DB9C8634 cnt=118376
```

And on the other one:
```
Rx (me:E6614104036E912B): pipe=1, payload=msg from E6605481DB9C8634 cnt=118372
Tx (me:E6614104036E912B): payload=msg from E6614104036E912B cnt=118381
```