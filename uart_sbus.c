/*
  Copyright (c) Kouhei Ito
  MIT Licence
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"


/// \tag::uart_advanced[]

#define UART_ID uart0
#define BAUD_RATE 100000//115200
#define DATA_BITS 8
#define STOP_BITS 2//1
#define PARITY    UART_PARITY_EVEN

//０番と1番ピンに接続
#define UART_TX_PIN 0
#define UART_RX_PIN 1

static int chars_rxed = 0;
static int data_num=0;
uint8_t sbus_data[25];

uint8_t ch;

// RX interrupt handler
void on_uart_rx() {
    short data;
    while (uart_is_readable(UART_ID)) {
        ch = uart_getc(UART_ID);
        if(ch==0x0f&&chars_rxed==00){
            sbus_data[chars_rxed]=ch;
            //printf("%02X ",ch);
            chars_rxed++;
        }
        else if(chars_rxed>0){
            sbus_data[chars_rxed]=ch;
            //printf("%02X ",ch);
            chars_rxed++;            
        }


        // Can we send it back?
        //if (uart_is_writable(UART_ID)) {
        //    // Change it slightly first!
            //ch++;
        //    uart_putc(UART_ID, ch);
        //}
        
        
        switch(chars_rxed){
            case 3:
                data=(sbus_data[1]|(sbus_data[2]<<8)&0x07ff);
                printf("%04d ",data);
                break;
            case 4:
                data=(sbus_data[3]<<5|sbus_data[2]>>3)&0x07ff;
                printf("%04d ",data);
                break;
            case 6:
                data=(sbus_data[3]>>6|sbus_data[4]<<2|sbus_data[5]<<10)&0x07ff;
                printf("%04d ",data);
                break;
            case 7:
                data=(sbus_data[6]<<7|sbus_data[5]>>1)&0x07ff;
                printf("%04d ",data);
                break;
            case 8:
                data=(sbus_data[7]<<4|sbus_data[6]>>4)&0x07ff;
                printf("%04d ",data);
                break;
            case 10:
                data=(sbus_data[7]>>7|sbus_data[8]<<1|sbus_data[9]<<9)&0x07ff;
                printf("%04d ",data);
                break;
                
        }

        if(chars_rxed==25){
            printf("\n");
            chars_rxed=0;
        }
        
    }
}

int main() {
    stdio_init_all();
    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 2400);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
    uart_puts(UART_ID, "\nHello, uart interrupts\n");
    printf("Hello!\n");

    while (1){
        tight_loop_contents();
        //printf("%x\n",ch);
    }
}

/// \end:uart_advanced[]
