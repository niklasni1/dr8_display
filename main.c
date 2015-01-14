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
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}


static void
cs_on(void)
{
  GPIO_ResetBits(GPIOD, GPIO_Pin_15);
} 
static void
cs_off(void)
{
  GPIO_SetBits(GPIOD, GPIO_Pin_15);
}
static void
led_on(void)
{
  GPIO_SetBits(GPIOD, GPIO_Pin_13);
} 
static void
led_off(void)
{
  GPIO_ResetBits(GPIOD, GPIO_Pin_13);
}
static void
reset_high(void)
{
  GPIO_SetBits(GPIOD, GPIO_Pin_14);
} 
static void
reset_low(void)
{
  GPIO_ResetBits(GPIOD, GPIO_Pin_14);
}

void
setup_usart(void)
{ 
  GPIO_InitTypeDef gpio_init; 
  USART_InitTypeDef usart_init;
  USART_ClockInitTypeDef clock_init;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  //pins
  //USART2
  //PA2 TX
  //PA4 CK
  gpio_init.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4;
  gpio_init.GPIO_Mode  = GPIO_Mode_AF;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gpio_init);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_USART2);

  //usart periph 
  clock_init.USART_Clock = USART_Clock_Enable;
  clock_init.USART_CPOL = USART_CPOL_High;
  clock_init.USART_CPHA = USART_CPHA_2Edge;
  clock_init.USART_LastBit = USART_LastBit_Enable; 
  USART_ClockInit(USART2, &clock_init);
  
  usart_init.USART_BaudRate = 115200;
  usart_init.USART_WordLength = USART_WordLength_8b;
  usart_init.USART_StopBits = USART_StopBits_1_5;
  usart_init.USART_Parity = USART_Parity_No;
  usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart_init.USART_Mode = USART_Mode_Tx; 
  USART_Init(USART2, &usart_init); 

  USART_Cmd(USART2, ENABLE);
}

void 
USART_putc(USART_TypeDef* USARTx, volatile char c){ 
    // wait until data register is empty
    while( !(USARTx->SR & 0x00000040) );
    USART_SendData(USARTx, c);
} 

int 
main(void)
{
  setup_gpios();
  setup_usart();
  delay(2000); 
  reset_low();
  delay(2000); 
  reset_high();
  delay(1000);

  while(1) {
    cs_on();
    delay(1000);
    USART_putc(USART2, 0b11110011); // all digits on
    cs_off();
    delay(1000);
    cs_on();
    delay(1000);
    USART_putc(USART2, 0b11110000); // all digits off
    cs_off();
    delay(1000);
  }
}
