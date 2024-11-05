#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include <ctype.h>

#define RED_LED      (1U << 1)  // Port F Pin 1
#define BLUE_LED     (1U << 2)  // Port F Pin 2
#define GREEN_LED    (1U << 3)  // Port F Pin 3

void initUART0(void) {
    // Activate UART0 and Port A
    SYSCTL_RCGCUART_R |= (1U << 0);       // Enable UART0 clock
    SYSCTL_RCGCGPIO_R |= (1U << 0);       // Enable GPIO Port A clock

    // Set up PA0 and PA1 for UART communication
    GPIO_PORTA_AFSEL_R |= (1U << 0) | (1U << 1); // Enable alternate functions on PA0 and PA1
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) | 0x00000011; // Configure PA0 and PA1 for UART
    GPIO_PORTA_DEN_R |= (1U << 0) | (1U << 1); // Digital enable on PA0, PA1

    // Set UART0 for 9600 baud rate, 8-N-1 frame format
    UART0_CTL_R &= ~UART_CTL_UARTEN;     // Disable UART0 while configuring
    UART0_IBRD_R = 104;                  // Integer part of Baud Rate Divisor (BRD)
    UART0_FBRD_R = 11;                   // Fractional part of BRD
    UART0_LCRH_R = UART_LCRH_WLEN_8;     // 8 data bits, no parity, 1 stop bit
    UART0_CTL_R |= UART_CTL_UARTEN;      // Enable UART0
}

char readUART0(void) {
    // Wait for a character in the receive FIFO
    while ((UART0_FR_R & UART_FR_RXFE) != 0); // Check RXFE (Receive FIFO Empty)
    return (char)(UART0_DR_R & 0xFF);         // Read the received character
}

void writeUART0(char data) {
    // Wait for space in the transmit FIFO
    while ((UART0_FR_R & UART_FR_TXFF) != 0); // Check TXFF (Transmit FIFO Full)
    UART0_DR_R = data;                        // Write the character to transmit
}

void initLEDs(void) {
    // Activate Port F for LED control
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;     // Enable clock for Port F

    // Set PF1, PF2, and PF3 as outputs and enable digital function
    GPIO_PORTF_DIR_R |= RED_LED | BLUE_LED | GREEN_LED; // Set direction to output
    GPIO_PORTF_DEN_R |= RED_LED | BLUE_LED | GREEN_LED; // Enable digital function on LED pins
}

void controlLED(char colorCode) {
    GPIO_PORTF_DATA_R &= ~(RED_LED | BLUE_LED | GREEN_LED); // Turn off all LEDs

    // Turn on LED based on the received color code
    if (colorCode == 'R' || colorCode == 'r') {
        GPIO_PORTF_DATA_R |= RED_LED; // Red LED on
    }
    else if (colorCode == 'B' || colorCode == 'b') {
        GPIO_PORTF_DATA_R |= BLUE_LED; // Blue LED on
    }
    else if (colorCode == 'G' || colorCode == 'g') {
        GPIO_PORTF_DATA_R |= GREEN_LED; // Green LED on
    }
}

int main(void) {
    initUART0();  // Initialize UART0 settings
    initLEDs();   // Set up Port F LEDs

    while (1) {
        char receivedChar = readUART0(); // Receive a character via UART

        if (receivedChar) {                // Verify a character is received
            writeUART0(receivedChar);      // Echo the character back to sender
            controlLED(receivedChar);      // Set LED based on the character
        }
    }
}
