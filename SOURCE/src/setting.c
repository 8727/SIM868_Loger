#include "setting.h"

configFlash conf;

//uint32_t FlashConfigWrite(uint32_t addr, uint8_t cmd, uint32_t date){
//  
//  
//}

//uint32_t FlashConfigRead(void){
//  addr = EeWriteByte(addr);
    
    
    
    
    
    
    
    
    
 // }
    
void FlashConfig(void){
//  uint32_t addr = 0x00;
//  uint8_t cmd;
//  if(0xFF == EeReadByte(0x0000) && 0xFF == EeReadByte(0x1000)){
//    addr = EeWriteByte(addr, DEVICE_TYPE_M, (uint32_t *) DEVICE_TYPE);
    
    
    
    
    
                              
    
    
    
 // }
//  addr = 0x00;
//  addr = FlashConfigRead(addr, DEVICE_TYPE_M, (uint32_t *) DEVICE_TYPE);
}
    
  
  
  
//  if(buff[addr] == 0x00){ addr ++; x = buff[addr], addr ++;} // 8bit
//  if(buff[addr] == 0x40){ addr ++; x = buff[addr], x <<= 0x08; addr ++; x |= buff[addr], addr ++;} // 16bit
//  if(buff[addr] == 0x80){ addr ++; x = buff[addr], x <<= 0x08; addr ++; x |= buff[addr], x <<= 0x08; addr ++; x |= buff[addr], x <<= 0x08; addr ++; x |= buff[addr], addr ++;} // 32bit
  
  
  
//  RtcTypeDef dateBuild;
//  dateBuild.year  = BUILD_YEAR;
//  dateBuild.month = BUILD_MONTH;
//  dateBuild.day   = BUILD_DAY;
//  dateBuild.hour  = BUILD_TIME_H;
//  dateBuild.min   = BUILD_TIME_M;
//  dateBuild.sec   = BUILD_TIME_S;
  
  
  


void Settings(void *pvParameters){
  EeInit();
  FlashConfig();
  RtcInit();
//  I2cInit();
//  Nrf24Init();
//  CanInit();
//  Rs485Init();
  
  
  while(true){
    LED_OFF;
    vTaskDelay(3000);
    LED_ON;
    vTaskDelay(500);
  }
}
