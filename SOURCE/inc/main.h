#ifndef __MAIN_H
#define __MAIN_H

//--------------------------------------------------------------------------------------------------------------------//
#include "stm32l4xx.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

//#include "sysInit.h"
//#include "setting.h"
//#include "modules.h"
//#include "messages.h"
//#include "history.h"

//--------------------------------------------------------------------------------------------------------------------//
#define BUILD_YEAR (__DATE__[7] == '?' ? 1900 : (((__DATE__[7] - '0') * 1000 ) + (__DATE__[8] - '0') * 100 + (__DATE__[9] - '0') * 10 + __DATE__[10] - '0'))
#define BUILD_MONTH (__DATE__ [2] == '?' ? 1 : __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) : __DATE__ [2] == 'b' ? 2 : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
        : __DATE__ [2] == 'y' ? 5 : __DATE__ [2] == 'l' ? 7 : __DATE__ [2] == 'g' ? 8 : __DATE__ [2] == 'p' ? 9 : __DATE__ [2] == 't' ? 10 : __DATE__ [2] == 'v' ? 11 : 12)
#define BUILD_DAY (__DATE__[4] == '?' ? 1 : ((__DATE__[4] == ' ' ? 0 : ((__DATE__[4] - '0') * 10)) + __DATE__[5] - '0'))
#define BUILD_TIME_H (__TIME__[0] == '?' ? 1 : ((__TIME__[0] == ' ' ? 0 : ((__TIME__[0] - '0') * 10)) + __TIME__[1] - '0'))
#define BUILD_TIME_M (__TIME__[3] == '?' ? 1 : ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0'))
#define BUILD_TIME_S (__TIME__[6] == '?' ? 1 : ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0'))

//--------------------------------------------------------------------------------------------------------------------//
#define true                            1
#define false                           0
  
//--------------------------------------------------------------------------------------------------------------------//
#define GPIO_AF_0                       (0x0000000U) /*!< Select alternate function 0 */
#define GPIO_AF_1                       (0x0000001U) /*!< Select alternate function 1 */
#define GPIO_AF_2                       (0x0000002U) /*!< Select alternate function 2 */
#define GPIO_AF_3                       (0x0000003U) /*!< Select alternate function 3 */
#define GPIO_AF_4                       (0x0000004U) /*!< Select alternate function 4 */
#define GPIO_AF_5                       (0x0000005U) /*!< Select alternate function 5 */
#define GPIO_AF_6                       (0x0000006U) /*!< Select alternate function 6 */
#define GPIO_AF_7                       (0x0000007U) /*!< Select alternate function 7 */
#define GPIO_AF_8                       (0x0000008U) /*!< Select alternate function 8 */
#define GPIO_AF_9                       (0x0000009U) /*!< Select alternate function 9 */
#define GPIO_AF_10                      (0x000000AU) /*!< Select alternate function 10 */
#define GPIO_AF_11                      (0x000000BU) /*!< Select alternate function 11 */
#define GPIO_AF_12                      (0x000000CU) /*!< Select alternate function 12 */
#define GPIO_AF_13                      (0x000000DU) /*!< Select alternate function 13 */
#define GPIO_AF_14                      (0x000000EU) /*!< Select alternate function 14 */
#define GPIO_AF_15                      (0x000000FU) /*!< Select alternate function 15 */

#endif /* __MAIN_H */
