# nrf52840-bare-metal
WIP, a library with several helpful features to do low level stuff with the NRF58240 MCU.

V1.0 Includes functionality to do basic Digital IO and includes an Arduino Style Millisecond timer using the in built RTC of the NRF58240.

**Digital Out**
- Constructor takes a port, pin, and optional starting value (saves a line of code)
- void .write(int val) -> set the value on that pin
- bool .read() -> read the current state of that pin
- Can also call write by doing: object = n;

**Digital In**
- Constructor takes a port, pin, and optional resistor mode
- .read() returns the current digital value on that pin
- .mode(PULLUP or PULLDOWN) changes the resistor mode

**Millis Counter**
- Technically can be used as any second counter, but is used in this application as a millisecond counter
- Optional constructor to change the frequency of the counter (by default this is 1000)
- .changeFrequency(float frequency) changes the frequency of the counter
- .begin() starts the counter
- .end() stops the counter
- .reset() clears the counter to 0

Todo: Serial Communication via the NRF's TWIM and also UART or SPI.
