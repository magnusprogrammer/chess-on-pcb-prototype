# EdvinsCodeChess - Inductive Chess Piece Identifier

An embedded system for the ATmega328P (Arduino Uno) that identifies chess pieces using inductive sensing. By measuring changes in resonance frequency and signal damping (oscillation edges), the system distinguishes between different pieces based on the size and material of their metallic inserts.

## How It Works (Polling Method)

Instead of using sensitive hardware interrupts, this system utilizes a highly stable, software-polled hybrid approach:

1. Energy Burst: The MCU "kicks" the LC tank with 5 fast pulses (~240 kHz) via Pin 11 (PB3).
2. Free Oscillation: Pin 11 is immediately set to high-impedance (Hi-Z), allowing the LC tank to resonate naturally.
3. Software Polling: The code loops and polls the ACO (Analog Comparator Output) bit against the internal 1.1V reference to count active signal edges.
4. Debounce & Filtering: The system averages 10 samples and requires the same piece profile to be detected 3 times consecutively before validating the identity.

---

## LC Tank & PCB Coil Design

The baseline frequency (empty square) is calibrated to 241.5 kHz based on Thomson's resonance formula.

* Capacitor (C): 10 nF (0.01 uF). Use a high-quality film capacitor (e.g., Polypropylene) or a ceramic C0G/NP0 type to minimize temperature drift.
* Coil (L): ~43.5 uH (Microhenry).

### PCB Trace Coil
To replicate this project exactly, you will need to design a custom PCB trace coil configuration using the specific board layout and dimensions to match the target 43.5 uH inductance required for the chess square matrix.

---

## Output & Monitoring

To view the live identification data and calibration details, connect the board to your computer and open PuTTY (or any serial monitor) with the following settings:

* Baud Rate: 9600 (using double speed U2X0 mode for high accuracy)
* Data bits: 8
* Stop bits: 1
* Parity: None

## License
This project is open-source and free to use for chess and electronics hobbyists.
