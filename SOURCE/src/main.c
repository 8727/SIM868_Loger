#include "main.h"
char buffer[1000]={'\0'};

void vApplicationIdleHook( void ){
  vTaskList((char *)buffer);
}


void Settings(void *pvParameters){
  
  while(true){
    
    vTaskDelay(10);
  }
}

void Modules(void *pvParameters){
  
  while(true){
    
    vTaskDelay(10);
  }
}

void Messages(void *pvParameters){
  
  while(true){
    
    vTaskDelay(10);
  }
}

void History(void *pvParameters){
  
  while(true){
    
    vTaskDelay(100);
  }
}

int main(void){
//  Sysinit();
  
  xTaskCreate(Settings,"Settings", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(Modules,"Modules", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(Messages,"Messages", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(History,"History", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  
  vTaskStartScheduler();
  
  while(true){
    
  }
}
