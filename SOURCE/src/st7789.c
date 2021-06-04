#include "st7789.h"

//--------------------------------------------------------------------------------------------------------------------//
void ST7789_Write(uint8_t byte){
  while(!(ST7789_SPI->SR & SPI_SR_TXE));
  ST7789_SPI->DR = byte;
}

//--------------------------------------------------------------------------------------------------------------------//
void ST7789_SendCmd(uint8_t cmd){
  ST7789_DC_COM;
  while(!(ST7789_SPI->SR & SPI_SR_TXE));
  ST7789_SPI->DR = cmd;
}

//--------------------------------------------------------------------------------------------------------------------//
void ST7789_SendData(uint8_t data){
  ST7789_DC_DATA;
  while(!(ST7789_SPI->SR & SPI_SR_TXE));
  ST7789_SPI->DR = data;
}

//--------------------------------------------------------------------------------------------------------------------//
void ST7789_SleepModeEnter( void ){
  ST7789_SendCmd(ST7789_Cmd_SLPIN);
  DelayMs(500);
}

//--------------------------------------------------------------------------------------------------------------------//
void ST7789_MemAccessModeSet(uint8_t Rotation, uint8_t VertMirror, uint8_t HorizMirror, uint8_t IsBGR){
  uint8_t Value;
  Rotation &= 7; 
  ST7789_SendCmd(ST7789_Cmd_MADCTL);
  switch(Rotation){
    case 0:
      Value = 0;
      break;
    case 1:
      Value = ST7789_MADCTL_MX;
      break;
    case 2:
      Value = ST7789_MADCTL_MY;
      break;
    case 3:
      Value = ST7789_MADCTL_MX | ST7789_MADCTL_MY;
      break;
    case 4:
      Value = ST7789_MADCTL_MV;
      break;
    case 5:
      Value = ST7789_MADCTL_MV | ST7789_MADCTL_MX;
      break;
    case 6:
      Value = ST7789_MADCTL_MV | ST7789_MADCTL_MY;
      break;
    case 7:
      Value = ST7789_MADCTL_MV | ST7789_MADCTL_MX | ST7789_MADCTL_MY;
      break;
  }
  
  if (VertMirror)
    Value = ST7789_MADCTL_ML;
  if (HorizMirror)
    Value = ST7789_MADCTL_MH;
  
  if (IsBGR)
    Value |= ST7789_MADCTL_BGR;
  
  ST7789_SendData(Value);
}

//--------------------------------------------------------------------------------------------------------------------//
void ST7789_InversionMode(uint8_t Mode){
  if (Mode)
    ST7789_SendCmd(ST7789_Cmd_INVON);
  else
    ST7789_SendCmd(ST7789_Cmd_INVOFF);
}

//--------------------------------------------------------------------------------------------------------------------//
void ST7789_FillScreen(uint16_t color){
  ST7789_FillRect(0, 0,  ST7789_Width, ST7789_Height, color);
}

void ST7789_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
  if ((x >= ST7789_Width) || (y >= ST7789_Height)) return;
  if ((x + w) > ST7789_Width) w = ST7789_Width - x;
  if ((y + h) > ST7789_Height) h = ST7789_Height - y;
  ST7789_SetWindow(x, y, x + w - 1, y + h - 1);
  for (uint32_t i = 0; i < (h * w); i++) ST7789_RamWrite(&color, 1);
}

void ST7789_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1){
  ST7789_ColumnSet(x0, x1);
  ST7789_RowSet(y0, y1);
  ST7789_SendCmd(ST7789_Cmd_RAMWR);
}

void ST7789_RamWrite(uint16_t *pBuff, uint16_t Len){
  while (Len--){
    ST7789_SendData(*pBuff >> 8);  
    ST7789_SendData(*pBuff & 0xFF);
  }
}

static void ST7789_ColumnSet(uint16_t ColumnStart, uint16_t ColumnEnd){
  if(ColumnStart > ColumnEnd)
    return;
  if(ColumnEnd > ST7789_Width)
    return;
  
  ColumnStart += ST7789_X_Start;
  ColumnEnd += ST7789_X_Start;
  
  ST7789_SendCmd(ST7789_Cmd_CASET);
  ST7789_SendData(ColumnStart >> 8);  
  ST7789_SendData(ColumnStart & 0xFF);  
  ST7789_SendData(ColumnEnd >> 8);  
  ST7789_SendData(ColumnEnd & 0xFF);  
}

static void ST7789_RowSet(uint16_t RowStart, uint16_t RowEnd){
  if(RowStart > RowEnd)
    return;
  if(RowEnd > ST7789_Height)
    return;
  
  RowStart += ST7789_Y_Start;
  RowEnd += ST7789_Y_Start;
  
  ST7789_SendCmd(ST7789_Cmd_RASET);
  ST7789_SendData(RowStart >> 8);  
  ST7789_SendData(RowStart & 0xFF);  
  ST7789_SendData(RowEnd >> 8);  
  ST7789_SendData(RowEnd & 0xFF);  
}

void ST7789_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){
  ST7789_DrawLine(x1, y1, x1, y2, color);
  ST7789_DrawLine(x2, y1, x2, y2, color);
  ST7789_DrawLine(x1, y1, x2, y1, color);
  ST7789_DrawLine(x1, y2, x2, y2, color);
}

void ST7789_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t fillcolor){
  if(x1 > x2)
    SwapInt16Values(&x1, &x2);
  if(y1 > y2)
    SwapInt16Values(&y1, &y2);
  ST7789_FillRect(x1, y1, x2 - x1, y2 - y1, fillcolor);
}

void ST7789_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){
  // Вертикальная линия
  if(x1 == x2){
    // Отрисовываем линию быстрым методом
    if(y1 > y2)
      ST7789_FillRect(x1, y2, 1, y1 - y2 + 1, color);
    else
      ST7789_FillRect(x1, y1, 1, y2 - y1 + 1, color);
    return;
  }
  // Горизонтальная линия
  if(y1 == y2){
    // Отрисовываем линию быстрым методом
    if(x1 > x2)
      ST7789_FillRect(x2, y1, x1 - x2 + 1, 1, color);
    else
      ST7789_FillRect(x1, y1, x2 - x1 + 1, 1, color);
    return;
  }
  ST7789_DrawLine_Slow(x1, y1, x2, y2, color);
}

static void SwapInt16Values(int16_t *pValue1, int16_t *pValue2){
  int16_t TempValue = *pValue1;
  *pValue1 = *pValue2;
  *pValue2 = TempValue;
}

void ST7789_DrawPixel(int16_t x, int16_t y, uint16_t color){
  if ((x < 0) ||(x >= ST7789_Width) || (y < 0) || (y >= ST7789_Height))
    return;

  ST7789_SetWindow(x, y, x, y);
  ST7789_RamWrite(&color, 1);
}

void ST7789_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor){
  int x = 0;
  int y = radius;
  int delta = 1 - 2 * radius;
  int error = 0;

  while(y >= 0){
    ST7789_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, fillcolor);
    ST7789_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, fillcolor);
    error = 2 * (delta + y) - 1;
    if(delta < 0 && error <= 0){
      ++x;
      delta += 2 * x + 1;
      continue;
    }
    error = 2 * (delta - x) - 1;
    if(delta > 0 && error > 0){
      --y;
      delta += 1 - 2 * y;
      continue;
    }
    ++x;
    delta += 2 * (x - y);
    --y;
  }
}

void ST7789_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color){
  int x = 0;
  int y = radius;
  int delta = 1 - 2 * radius;
  int error = 0;

  while (y >= 0){
    ST7789_DrawPixel(x0 + x, y0 + y, color);
    ST7789_DrawPixel(x0 + x, y0 - y, color);
    ST7789_DrawPixel(x0 - x, y0 + y, color);
    ST7789_DrawPixel(x0 - x, y0 - y, color);
    error = 2 * (delta + y) - 1;
    if(delta < 0 && error <= 0){
      ++x;
      delta += 2 * x + 1;
      continue;
    }
    error = 2 * (delta - x) - 1;
    if(delta > 0 && error > 0){
      --y;
      delta += 1 - 2 * y;
      continue;
    }
    ++x;
    delta += 2 * (x - y);
    --y;
  }
}

void ST7789_DrawChar_5x8(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, unsigned char c) {
  if((x >= 240) || (y >= 240) || ((x + 4) < 0) || ((y + 7) < 0)) return;
  if(c<128)            c = c-32;
  if(c>=144 && c<=175) c = c-48;
  if(c>=128 && c<=143) c = c+16;
  if(c>=176 && c<=191) c = c-48;
  if(c>191)  return;
  for(uint8_t i=0; i<6; i++ ){
    uint8_t line;
    if (i == 5) line = 0x00;
    else line = font[(c*5)+i];
    for (uint8_t j = 0; j<8; j++){
      if (line & 0x01) ST7789_DrawPixel(x + i, y + j, TextColor);
      else if (!TransparentBg) ST7789_DrawPixel(x + i, y + j, BgColor);
      line >>= 1;
    }
  }
}

void ST7789_DrawChar_7x11(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, unsigned char c){
  uint8_t i,j;
  uint8_t buffer[11];
  if((x >= 240) || (y >= 240) || ((x + 4) < 0) || ((y + 7) < 0)) return;
  // Copy selected simbol to buffer
  memcpy(buffer,&font7x11[(c-32)*11],11);
  for(j=0;j<11;j++){
    for(i=0;i<7;i++){
      if((buffer[j] & (1<<i)) == 0) {
        if(!TransparentBg) ST7789_DrawPixel(x + i, y + j, BgColor);
      }else ST7789_DrawPixel(x + i, y + j, TextColor);
    }
  }
}

void ST7789_print_5x8(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, char *str){
  unsigned char type = *str;
  if (type>=128) x = x - 3;
  while (*str){
    ST7789_DrawChar_5x8(x, y, TextColor, BgColor, TransparentBg, *str++); 
    unsigned char type = *str;
    if (type>=128) x=x+3;
    else x=x+6;
  }
}

void ST7789_print_7x11(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, char *str){
  unsigned char type = *str;
  if (type>=128) x = x - 3;
  while (*str){
    ST7789_DrawChar_7x11(x, y, TextColor, BgColor, TransparentBg, *str++); 
    unsigned char type = *str;
    if (type>=128) x=x+8;
    else x=x+8;
  }
}

void ST7789_Init(void){
  GPIOB->MODER &= ~(GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER14 | GPIO_MODER_MODER15);
  GPIOC->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER7);
  GPIOB->MODER |= (GPIO_MODER_MODE12_0 | GPIO_MODER_MODE13_1 | GPIO_MODER_MODE14_0 | GPIO_MODER_MODE15_1);
  GPIOC->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE7_0);
  GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR12 | GPIO_OSPEEDER_OSPEEDR13 | GPIO_OSPEEDER_OSPEEDR14 | GPIO_OSPEEDER_OSPEEDR15);
  GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR7);
  GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL12 |  GPIO_AFRH_AFSEL13 | GPIO_AFRH_AFSEL14 | GPIO_AFRH_AFSEL15);
  GPIOB->AFR[1] |= (GPIO_AFRH_AFSEL13_0 | GPIO_AFRH_AFSEL13_2 | GPIO_AFRH_AFSEL15_0 | GPIO_AFRH_AFSEL15_2);
  GPIOC->AFR[0] &= ~(GPIO_AFRL_AFSEL0 |  GPIO_AFRL_AFSEL7);
  
  ST7789_PW_ON;
  ST7789_CS_HIGHT;
  
  RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;
  ST7789_SPI->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR);
  ST7789_SPI->CR2 &= ~(SPI_CR2_DS);
  ST7789_SPI->CR2 |= (SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2);
  ST7789_SPI->CR2 |= SPI_CR2_NSSP;
  ST7789_SPI->CR1 |= SPI_CR1_SPE;
  
  ST7789_RESET_LOW;
  DelayMs(10);
  ST7789_RESET_HIGHT;
  DelayMs(150);
  
  ST7789_CS_LOW;
  ST7789_SendCmd(ST7789_Cmd_SWRESET);
  DelayMs(130);
  
  ST7789_SendCmd(ST7789_Cmd_SLPOUT);
  DelayMs(500);
  
  ST7789_SendCmd(ST7789_Cmd_COLMOD);
  ST7789_SendData((ST7789_ColorMode_65K | ST7789_ColorMode_16bit) & 0x77);
  DelayMs(10);
  ST7789_MemAccessModeSet(4, 1, 1, 0);
  DelayMs(10);
  ST7789_SendCmd(ST7789_Cmd_INVON);
  DelayMs(10);
  ST7789_FillScreen(BLACK);
  ST7789_SendCmd(ST7789_Cmd_DISPON);;
  DelayMs(100);
  ST7789_CS_HIGHT;
}

