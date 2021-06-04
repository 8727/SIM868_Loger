
#ifndef _GSMCONFIG_H_
#define _GSMCONFIG_H_

#define SIM868_USART                      USART2
#define SIM868_KEY_GPIO                   GPIOD
#define SIM868_KEY_PIN                    GPIO_PIN_7

#define SIM868_DEBUG                      true       //  use printf debug
#define SIM868_CALL                       true       //  enable call
#define SIM868_MSG                        true       //  enable message
#define SIM868_GPRS                       true       //  enable gprs
#define SIM868_BLUETOOTH                  false      //  enable bluetooth , coming soon

#endif /*_GSMCONFIG_H_ */
