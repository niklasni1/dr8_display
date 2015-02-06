/*
 * This program turns on the 4 leds of the stm32f4 discovery board
 * one after another.
 * It defines shortcut definitions for the led pins and
 * stores the order of the leds in an array which is being
 * iterated in a loop.
 *
 * This program is free human culture like poetry, mathematics
 * and science. You may use it as such.
 */

#include <math.h>
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"
#include "commands.h"

#define SRAM_SIZE ((uint32_t)(2*1024*1024))

/* This is apparently needed for libc/libm (eg. powf()). */
int __errno;


static void delay(__IO uint32_t nCount)
{
    while(nCount--)
        __asm("nop"); // do nothing
}

static void
setup_gpios(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}

static void
led_on(void)
{
  GPIO_SetBits(GPIOD, GPIO_Pin_15);
} 
static void
led_off(void)
{
  GPIO_ResetBits(GPIOD, GPIO_Pin_15);
}

static void
reset_low(void)
{
  GPIO_ResetBits(GPIOD, GPIO_Pin_14);
} 
static void
reset_high(void)
{
  GPIO_SetBits(GPIOD, GPIO_Pin_14);
}

void
setup_spi(void)
{ 
  GPIO_InitTypeDef gpio_init; 
  SPI_InitTypeDef spi_init;

  //clockage
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE); 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 

  //pins for SPI2
  //PB12 CS\
  //PB13 CLK
  //(PB14 MISO)
  //PB15 MOSI
  gpio_init.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
  gpio_init.GPIO_Mode  = GPIO_Mode_AF;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &gpio_init);

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

  //spi periph 
  SPI_StructInit(&spi_init);
  spi_init.SPI_Direction = SPI_Direction_1Line_Tx;
  spi_init.SPI_Mode = SPI_Mode_Master;
  spi_init.SPI_DataSize = SPI_DataSize_8b;
  spi_init.SPI_CPOL = SPI_CPOL_High;
  spi_init.SPI_CPHA = SPI_CPHA_2Edge;
  spi_init.SPI_NSS = SPI_NSS_Soft;
  spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
  spi_init.SPI_CRCPolynomial = 0;
  spi_init.SPI_FirstBit = SPI_FirstBit_MSB;

  SPI_Init(SPI2, &spi_init);

  SPI_Cmd(SPI2, ENABLE);
}

void
send_byte(uint8_t byte) {
  led_off();
  SPI_I2S_SendData(SPI2,byte);
  delay(400);
  led_on();
}

void
write_char_to_ram(uint8_t slot, uint8_t data[5]) {
  send_byte(WRITE_TO_RAM);
  send_byte(slot); // 0-indexed!

  for(int i = 0; i < 5; i++) {
    send_byte(data[i]);
  };
}

void
generate_single_segment(uint8_t segment, uint8_t data[]) { 
  uint8_t row = 0b01<<(7-(segment/5));
  uint8_t column = (segment%5);
  data[0] = (column==0) ? row : 0;
  data[1] = (column==1) ? row : 0;
  data[2] = (column==2) ? row : 0;
  data[3] = (column==3) ? row : 0;
  data[4] = (column==4) ? row : 0;
}

int 
main(void)
{
  setup_gpios();
  setup_spi();
  delay(2000); 
  reset_low();
  delay(2000); 
  reset_high();
  delay(1000);

  send_byte(ALL_OFF);
  delay(100);
  send_byte(FULL_LENGTH);
  delay(100);
  send_byte(NORMAL);
  delay(100);

  uint8_t char1[15] = {
    0x01, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x00
  };

  uint8_t char2[5] = {
    0x00, 0x00, 0x00, 0x00, 0x00
  };

  send_byte(AUTO_INCR_ON);
  delay(100);

  for (int i = 0; i<16; i++) {
    send_byte(0x90 + i); 
    delay(100);
  }

  send_byte(AUTO_INCR_OFF);
  delay(100);
  send_byte(0b00001111);
  delay(100);
  
  while(1) {
    /*
    for (int i = 0; i<15; i+=5) {
      write_char_to_ram(0, char1 + (i%15));
      delay(100);
      write_char_to_ram(1, char1 + ((i+5)%15));
      delay(100);
      write_char_to_ram(2, char1 + ((i+10)%15));
      delay(2000000); 
    }
    */
    for (int j = 0; j<16; j++) { 
      for (int i = 0; i<36; i++) {
        generate_single_segment(i, char2);
        delay(100);
        write_char_to_ram(j, char2);
        delay(500000);
      }
    }
    send_byte(ALL_ON);
    delay(2000000);
    send_byte(NORMAL);
  }
}
