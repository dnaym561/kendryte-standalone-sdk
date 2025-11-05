/* (c) 2023 cepstrum.co.jp */

#include <stdint.h>
#include <stdio.h>
#include "fpioa.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "uart.h"
#include "gpiohs.h"
#include "sysctl.h"

#define UART_NUM    UART_DEVICE_3

//-----------------------------------------------------------------
// M-sequence generater, return value = 0/1

unsigned mseq16bit(unsigned seed) {
  static uint16_t reg16bit=0x5a5a;
  uint16_t        msb;
  unsigned char   b0, b2, b3, b5;

  if (seed!=0) reg16bit=seed&0xffff;     // change seed value

  b0=reg16bit&0x1;
  b2=(reg16bit>>2)&0x1;
  b3=(reg16bit>>3)&0x1;
  b5=(reg16bit>>5)&0x1;
  msb=(b0^b2^b3^b5)&0x1;
  reg16bit=(reg16bit>>1)&0x7fff;
  reg16bit=reg16bit|(msb<<15);
  return msb;
}

//-----------------------------------------------------------------
// generate 32bit seed (register initial value) for 32bit xorshift

uint32_t gen32bit_seed(unsigned mseq_seed) {
  uint32_t seed;
  unsigned i;

  mseq16bit(mseq_seed);    // set M-sequence seed
  seed=0;
  for (i=0; i<16; i=i+1) mseq16bit(0);
  for (i=0; i<16; i=i+1) seed=(seed<<1)|mseq16bit(0);
  for (i=0; i<16; i=i+1) mseq16bit(0);
  for (i=0; i<16; i=i+1) seed=(seed<<1)|mseq16bit(0);
  return seed;
}

//-----------------------------------------------------------------
// generate 64bit seed (register initial value) for 64bit xorshift

uint64_t gen64bit_seed(unsigned mseq_seed) {
  uint64_t seed;
  unsigned i;

  mseq16bit(mseq_seed);    // set M-sequence seed
  seed=0;
  for (i=0; i<16; i=i+1) mseq16bit(0);
  for (i=0; i<16; i=i+1) seed=(seed<<1)|mseq16bit(0);
  for (i=0; i<16; i=i+1) mseq16bit(0);
  for (i=0; i<16; i=i+1) seed=(seed<<1)|mseq16bit(0);
  for (i=0; i<16; i=i+1) mseq16bit(0);
  for (i=0; i<16; i=i+1) seed=(seed<<1)|mseq16bit(0);
  for (i=0; i<16; i=i+1) mseq16bit(0);
  for (i=0; i<16; i=i+1) seed=(seed<<1)|mseq16bit(0);
  return seed;
}

//-----------------------------------------------------------------
// test gen32bit_seed() function

void io_mux_init(void)
{
    fpioa_set_function(4, FUNC_UART1_RX + UART_NUM * 2);
    fpioa_set_function(5, FUNC_UART1_TX + UART_NUM * 2);
    fpioa_set_function(24, FUNC_GPIOHS3);
}

int main() 
{
    io_mux_init();
    plic_init();
    sysctl_enable_irq();

    gpiohs_set_drive_mode(3, GPIO_DM_OUTPUT);
    gpio_pin_value_t value = GPIO_PV_HIGH;
    gpiohs_set_pin(3, value);

    uart_init(UART_NUM);
    uart_configure(UART_NUM, 115200, 8, UART_STOP_1, UART_PARITY_NONE);

    char *hel = {"ge_seed!\n"};
    uart_send_data(UART_NUM, hel, strlen(hel));

    int i;
    int iteration=20;
    char buffer[128];

    for (i=0; i<iteration; i=i+1) {
        sprintf( buffer, "A %3i %#010x\n", i, gen32bit_seed(0));
        uart_send_data(UART_NUM, buffer, strlen(buffer));
    }

    for (i=0; i<iteration; i=i+1) {
        sprintf( buffer, "B %3i %#010x\n", i, gen32bit_seed(i+1));
        uart_send_data(UART_NUM, buffer, strlen(buffer));
    }

    gen32bit_seed(123456);  // set new M-sequence seed
    for (i=0; i<iteration; i=i+1) {
       sprintf( buffer,  "C %3i %#010x\n", i, gen32bit_seed(0));
        uart_send_data(UART_NUM, buffer, strlen(buffer));
    }

    gen32bit_seed(time(NULL)&0xffff);  // set new M-sequence seed
    for (i=0; i<iteration; i=i+1) {
        sprintf( buffer,  "D %3i %#010x\n", i, gen32bit_seed(0));
        uart_send_data(UART_NUM, buffer, strlen(buffer));
    }


    /*
    for (i=0; i<iteration; i=i+1) printf("a %3i %10u\n", i, gen32bit_seed(0));

    for (i=0; i<iteration; i=i+1) printf("b %3i %10u\n", i, gen32bit_seed(i+1));

    gen32bit_seed(123456);  // set new M-sequence seed
    for (i=0; i<iteration; i=i+1) printf("c %3i %10u\n", i, gen32bit_seed(0));

    gen32bit_seed(time(NULL)&0xffff);  // set new M-sequence seed
    for (i=0; i<iteration; i=i+1) printf("d %3i %10u\n", i, gen32bit_seed(0));
    */
}


