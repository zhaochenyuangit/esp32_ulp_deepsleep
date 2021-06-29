# ESP32 edge interrupt ULP example
this code uses ULP coprocessor to detect edge interrupts on two rtc pins.
## Usage
1. enable ULP coprocessor under idf.py menuconfig->ESP32-Specific and assign 2K memory to it before compilation.
2. connect GPIO34, 35 to the signal source, and pull them down with a pull-down resistor (40k Ohm).
3. if choose other pins as input, barrier_main.S file must be adapt to the corresonding rtc gpio number of the new pins. GPIO 34 and 35 correspond to rtc number 4 and 5 respectively.
4. the ulp program will initialize once when the board is powered on and run forever. If an edge interrupt occurs on GPIO34 or 35, a struct will be sent to gpio_evt_queue. The struct has three members, a timestamp with 10us precision, a rtc gpio number, and the level of the pin showing when, on which pin did the interrupt happend and whether it was an rising or falling edge interrupt.
## Code Overview
The ULP will poll the state of the rtc pins every 20 ms. If the state of one pin has changed, ULP will store the rtc gpio number of the pin, the current state of the pin and the current rtc timestamp into a ring buffer, and then wake up the main processor. The main processor will read all new data in the ring buffer and send them to the queue for further processing.
