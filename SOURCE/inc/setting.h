#ifndef _SETTINGS_H
#define _SETTINGS_H

//--------------------------------------------------------------------------------------------------------------------//
#include "stm32l4xx.h"
#include "sysInit.h"
//#include "eeprom.h"

//#include "rtc.h"

//--------------------------------------------------------------------------------------------------------------------//
#define INFO                              true
#define SIM868_DEBUG                      true       //  use printf debug
#define SIM868_CALL                       true       //  enable call
#define SIM868_MSG                        true       //  enable message
#define SIM868_GPRS                       true       //  enable gprs
#define SIM868_BLUETOOTH                  false      //  enable bluetooth , coming soon

#define ATC_CMSIS_OS                      true
#define ATC_TXDMA                         true
#define ATC_MAX_ELEMENTS                  1
#define ATC_MAX_ALWAYS_SEARCHING_STRING   10
#define ATC_MAX_ANSWER_SEARCHING_STRING   5

//--------------------------------------------------------------------------------------------------------------------//
typedef struct{
  uint32_t magicKEY;
  uint32_t dateBuild;
  uint32_t hwBuild;
  uint32_t swBuild;
}configFlash;

extern configFlash conf;

//--------------------------------------------------------------------------------------------------------------------//
void Settings(void *pvParameters);

#endif /* _SETTINGS_H */
