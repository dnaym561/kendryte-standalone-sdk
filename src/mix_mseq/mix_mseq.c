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

unsigned char mseq13(unsigned seed) {
  static uint16_t reg13bit=0x0f5a;
  uint16_t        msb;
  unsigned char   b0, b1, b3, b4;

  if (seed!=0) reg13bit=seed&0x1fff;     // change seed value

  b0=reg13bit&0x1;
  b1=(reg13bit>>1)&0x1;
  b3=(reg13bit>>3)&0x1;
  b4=(reg13bit>>4)&0x1;
  msb=(b0^b1^b3^b4)&0x1;
  reg13bit=(reg13bit>>1)&0x0fff;
  reg13bit=reg13bit|(msb<<12);
  return msb;
}

//-----------------------------------------------------------------
// M-sequence generater, return value = 0/1

unsigned char mseq17(unsigned seed) {
  static uint32_t reg17bit=0x5a5a;
  uint32_t        msb;
  unsigned char   b0, b1, b2, b3;

  if (seed!=0) reg17bit=seed&0x0001ffff;     // change seed value

  b0=reg17bit&0x1;
  b1=(reg17bit>>1)&0x1;
  b2=(reg17bit>>2)&0x1;
  b3=(reg17bit>>3)&0x1;
  msb=(b0^b1^b2^b3)&0x1;
  reg17bit=(reg17bit>>1)&0x0000ffff;
  reg17bit=reg17bit|(msb<<16);
  return msb;
}

//-----------------------------------------------------------------
// M-sequence generater, return value = 0/1

unsigned char mseq19(unsigned seed) {
  static uint32_t reg19bit=0x000f5a5a;
  uint32_t        msb;
  unsigned char   b0, b1, b2, b3;

  if (seed!=0) reg19bit=seed&0x0007ffff;     // change seed value

  b0=reg19bit&0x1;
  b1=(reg19bit>>1)&0x1;
  b2=(reg19bit>>2)&0x1;
  b3=(reg19bit>>3)&0x1;
  msb=(b0^b1^b2^b3)&0x1;
  reg19bit=(reg19bit>>1)&0x0003ffff;
  reg19bit=reg19bit|(msb<<18);
  return msb;
}

//-----------------------------------------------------------------
// M-sequence generater, return value = 0/1

unsigned char mseq31(unsigned seed) {
  static uint32_t reg31bit=0x5fa05a0f;
  uint32_t        msb;
  unsigned char   b0, b1, b2, b3;

  if (seed!=0) reg31bit=seed&0x7fffffff;     // change seed value

  b0=reg31bit&0x1;
  b1=(reg31bit>>1)&0x1;
  b2=(reg31bit>>2)&0x1;
  b3=(reg31bit>>3)&0x1;
  msb=(b0^b1^b2^b3)&0x1;
  reg31bit=(reg31bit>>1)&0x3fffffff;
  reg31bit=reg31bit|(msb<<30);
  return msb;
}

//-----------------------------------------------------------------
// M-sequence generater, return value = 0/1

unsigned char mseq61(unsigned seed) {
  static uint64_t reg61bit=0x0afa5f5a0a505a5a;
  uint64_t        msb;
  unsigned char   b0, b1, b15, b16;

  if (seed!=0) reg61bit=seed&0x1fffffffffffffff;     // change seed value

  b0 =reg61bit&0x1;
  b1 =(reg61bit>>1)&0x1;
  b15=(reg61bit>>15)&0x1;
  b16=(reg61bit>>16)&0x1;
  msb=(b0^b1^b15^b16)&0x1;
  reg61bit=(reg61bit>>1)&0x0fffffffffffffff;
  reg61bit=reg61bit|(msb<<60);
  return msb;
}

//-----------------------------------------------------------------
// xorshift, return value = 0/1

unsigned char xorshift(uint32_t seed) {
  static uint32_t y=0xa05f55aa;

  if (seed!=0) y=seed;     // change seed value

  y=y^(y<<3);
  y=y^(y>>13);
  y=y^(y<<7);
  return y&0x1;
}

//-----------------------------------------------------------------
// 32bit xorshift

uint32_t xorshift32bit(uint32_t seed) {
  static uint32_t y=0x05a5f5a5a;

  if (seed!=0) y=seed;     // change seed value

  y=y^(y<<9);
  y=y^(y>>11);
  y=y^(y<<19);
  return y;
}

//-----------------------------------------------------------------
// 64bit xorshift

uint64_t xorshift64bit(uint64_t seed) {
  static uint64_t y=0xf0a50fa55aa0ff0;

  if (seed!=0) y=seed;     // change seed value

  y=y^(y<<3);
  y=y^(y>>35);
  y=y^(y<<14);
  return y;
}

//-----------------------------------------------------------------
// long period random number generater, return value = 0/1

unsigned char mix_rand(void) {
  return mseq13(0)^mseq17(0)^mseq19(0)^mseq31(0)^mseq61(0)^xorshift(0);
}

//-----------------------------------------------------------------
// 32bit long period random number generater

uint32_t mix_rand32bit(void) {
  uint32_t rnd;
  unsigned i;

  rnd=0;
  for (i=0; i<32; i=i+1) {
    rnd=(rnd<<1)|(mseq13(0)^mseq17(0)^mseq19(0)^mseq31(0)^mseq61(0));
  }
  rnd=rnd^xorshift32bit(0);
  return rnd;
}

//-----------------------------------------------------------------
// 64bit long period random number generater

uint64_t mix_rand64bit(void) {
  uint64_t rnd;
  unsigned i;

  rnd=0;
  for (i=0; i<64; i=i+1) {
    rnd=(rnd<<1)|(mseq13(0)^mseq17(0)^mseq19(0)^mseq31(0)^mseq61(0));
  }
  rnd=rnd^xorshift64bit(0);
  return rnd;
}

//-----------------------------------------------------------------

void io_mux_init(void)
{
    fpioa_set_function(4, FUNC_UART1_RX + UART_NUM * 2);
    fpioa_set_function(5, FUNC_UART1_TX + UART_NUM * 2);
    fpioa_set_function(24, FUNC_GPIOHS3);
}

int main(void) {
  unsigned long i;

    io_mux_init();
    plic_init();
    sysctl_enable_irq();

    gpiohs_set_drive_mode(3, GPIO_DM_OUTPUT);
    gpio_pin_value_t value = GPIO_PV_HIGH;
    gpiohs_set_pin(3, value);

    uart_init(UART_NUM);
    uart_configure(UART_NUM, 115200, 8, UART_STOP_1, UART_PARITY_NONE);

    char *hel = {"mix_mseq!\n"};
    uart_send_data(UART_NUM, hel, strlen(hel));

    char buffer[128];
    //for (i=0; i<100000; i=i+1) printf("%u\n", mix_rand());
    for (i=0; i<100000; i=i+1) {
        sprintf ( buffer, "%u\n", mix_rand32bit()>>9);
        uart_send_data(UART_NUM, buffer, strlen(buffer));
    }
    //for (i=0; i<100000; i=i+1) printf("%u\n", (unsigned)(mix_rand64bit()>>41));
}


