#include "atGsm.h"

gsm_t       gsm;
ATC_t       atc_elements[ATC_MAX_ELEMENTS];
uint8_t     atc_cnt;

//--------------------------------------------------------------------------------------------------------------------//
__weak void  atc_userFoundString(ATC_Element_t ATC_Element, const char* foundStr, char* startPtr){
     
}

//--------------------------------------------------------------------------------------------------------------------//
void atc_callback(ATC_Element_t ATC_Element){
  if(LL_USART_IsActiveFlag_RXNE(atc_elements[ATC_Element].usart) && LL_USART_IsEnabledIT_RXNE(atc_elements[ATC_Element].usart)){
    if(atc_elements[ATC_Element].rxIndex < atc_elements[ATC_Element].rxSize - 1){
      atc_elements[ATC_Element].rxBuffer[atc_elements[ATC_Element].rxIndex] = LL_USART_ReceiveData8(atc_elements[ATC_Element].usart);      
      atc_elements[ATC_Element].rxIndex++;
    }else
      LL_USART_ReceiveData8(atc_elements[ATC_Element].usart);      
  }
  atc_elements[ATC_Element].rxTime = HAL_GetTick();
}

//--------------------------------------------------------------------------------------------------------------------//
void  atc_callback_txDMA(ATC_Element_t ATC_Element){
  #if (ATC_TXDMA == true)
  switch(atc_elements[ATC_Element].txDMA_Stream){
    case LL_DMA_STREAM_0:
      if(LL_DMA_IsActiveFlag_TC0(atc_elements[ATC_Element].txDMA)){
        LL_DMA_ClearFlag_TC0(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;
      }
    break;
    case LL_DMA_STREAM_1:
      if(LL_DMA_IsActiveFlag_TC1(atc_elements[ATC_Element].txDMA)){
        LL_DMA_ClearFlag_TC1(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }
    break;
    case LL_DMA_STREAM_2:
      if(LL_DMA_IsActiveFlag_TC2(atc_elements[ATC_Element].txDMA)){
        LL_DMA_ClearFlag_TC2(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }
    break;
    case LL_DMA_STREAM_3:
      if(LL_DMA_IsActiveFlag_TC3(atc_elements[ATC_Element].txDMA)){
        LL_DMA_ClearFlag_TC3(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }
    break;
    case LL_DMA_STREAM_4:
      if(LL_DMA_IsActiveFlag_TC4(atc_elements[ATC_Element].txDMA)){
        LL_DMA_ClearFlag_TC4(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }
    break;
    case LL_DMA_STREAM_5:
      if(LL_DMA_IsActiveFlag_TC5(atc_elements[ATC_Element].txDMA)){
        LL_DMA_ClearFlag_TC5(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }
    break;
    case LL_DMA_STREAM_6:
      if(LL_DMA_IsActiveFlag_TC6(atc_elements[ATC_Element].txDMA)){
        LL_DMA_ClearFlag_TC6(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }
    break;
    case LL_DMA_STREAM_7:
      if(LL_DMA_IsActiveFlag_TC7(atc_elements[ATC_Element].txDMA)){
        LL_DMA_ClearFlag_TC7(atc_elements[ATC_Element].txDMA);
        atc_elements[ATC_Element].txDMAdone = 1;    
      }
    break;
  }
  #endif
}

//--------------------------------------------------------------------------------------------------------------------//
void atc_process(void){
  for(uint8_t el = 0; el < atc_cnt ; el++){
    if((atc_elements[el].rxIndex > 0) && (HAL_GetTick() - atc_elements[el].rxTime > atc_elements[el].rxTimeout)){
      //  +++ answer AtCommand
      for(uint8_t ans = 0 ; ans < ATC_MAX_ANSWER_SEARCHING_STRING ; ans++){
        if(atc_elements[el].answerSearchingString[ans] == NULL)
          break;
        char *ansStr = strstr((char*)atc_elements[el].rxBuffer, atc_elements[el].answerSearchingString[ans]);         
        if(ansStr != NULL){
          if(atc_elements[el].foundAnswerString != NULL)
            strncpy(atc_elements[el].foundAnswerString, ansStr, atc_elements[el].foundAnswerSize);
          atc_elements[el].foundAnswer = ans; 
          break;
        }
      }
      //  --- answer AtCommand      
      //  +++ auto searching  
      for(uint8_t au = 0 ; au < ATC_MAX_ALWAYS_SEARCHING_STRING ; au++){
        if(atc_elements[el].alwaysSearchingString[au] == NULL)
          break;
        char *autoStr = strstr((char*)atc_elements[el].rxBuffer, atc_elements[el].alwaysSearchingString[au]);
        if(autoStr != NULL){
          atc_userFoundString((ATC_Element_t)el, atc_elements[el].alwaysSearchingString[au], autoStr);
        }
      }
      //  --- auto searching
      atc_elements[el].rxIndex = 0;
      memset(atc_elements[el].rxBuffer, 0, atc_elements[el].rxSize);
    }
  }
}

//--------------------------------------------------------------------------------------------------------------------//
uint8_t atc_sendString(ATC_Element_t ATC_Element, char *str, uint32_t timeout){
  if((ATC_Element < ATC_Element_0) || (ATC_Element >= atc_cnt))
    return 0;
  while(atc_elements[ATC_Element].txBusy == 1)
    atc_delay(1);
  atc_elements[ATC_Element].txBusy = 1;
  uint32_t startTime = HAL_GetTick();
  #if (ATC_TXDMA == false)
    for(uint16_t i = 0 ; i<strlen(str) ; i++){
      while (!LL_USART_IsActiveFlag_TXE(atc_elements[ATC_Element].usart)){
        if(HAL_GetTick() - startTime > timeout){
          atc_elements[ATC_Element].txBusy = 0;
          return 0;
        }   
      }
      LL_USART_TransmitData8(atc_elements[ATC_Element].usart, str[i]);   
    }
    while (!LL_USART_IsActiveFlag_TC(atc_elements[ATC_Element].usart)){
      if(HAL_GetTick() - startTime > timeout){
        atc_elements[ATC_Element].txBusy = 0;
        return 0;
      }   
    }
  #endif
  #if (ATC_TXDMA == true)
    atc_elements[ATC_Element].txDMAdone = 0;
    LL_DMA_ConfigAddresses(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream,\
      (uint32_t)str, LL_USART_DMA_GetRegAddr(atc_elements[ATC_Element].usart),\
      LL_DMA_GetDataTransferDirection(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream));
    LL_DMA_SetDataLength(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream, strlen(str));
    LL_DMA_EnableStream(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream);
    while(atc_elements[ATC_Element].txDMAdone == 0){
      if(HAL_GetTick() - startTime > timeout){
        LL_DMA_DisableStream(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream);
        atc_elements[ATC_Element].txBusy = 0;
        return 0;
      }
      atc_delay(1);
    }
  #endif
  atc_elements[ATC_Element].txBusy = 0;
  return 1;
}

//--------------------------------------------------------------------------------------------------------------------//
uint8_t atc_sendData(ATC_Element_t ATC_Element, uint8_t *data, uint16_t size, uint32_t timeout){
  if((ATC_Element < ATC_Element_0) || (ATC_Element >= atc_cnt))
    return 0;
  while(atc_elements[ATC_Element].txBusy == 1)
    atc_delay(1);
  atc_elements[ATC_Element].txBusy = 1;
  uint32_t startTime = HAL_GetTick();
  #if (ATC_TXDMA == false)
    for (uint16_t i = 0; i<size; i++){
      while (!LL_USART_IsActiveFlag_TXE(atc_elements[ATC_Element].usart)){
        if(HAL_GetTick() - startTime > timeout){
          atc_elements[ATC_Element].txBusy = 0;
          return 0;
        }   
      }
      LL_USART_TransmitData8(atc_elements[ATC_Element].usart, data[i]);
    }
    while (!LL_USART_IsActiveFlag_TC(atc_elements[ATC_Element].usart)){
      if(HAL_GetTick() - startTime > timeout){
        atc_elements[ATC_Element].txBusy = 0;
        return 0;
      }  
    }
  #endif
  #if (ATC_TXDMA == true)
    atc_elements[ATC_Element].txDMAdone = 0;
    LL_DMA_ConfigAddresses(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream,\
        (uint32_t)data, LL_USART_DMA_GetRegAddr(atc_elements[ATC_Element].usart),\
        LL_DMA_GetDataTransferDirection(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream));
    LL_DMA_SetDataLength(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream, size);
    while(atc_elements[ATC_Element].txDMAdone == 0){
      atc_delay(1);
      if(HAL_GetTick() - startTime > timeout){
        atc_elements[ATC_Element].txBusy = 0;
        return 0;
      }
    }
  #endif
  atc_elements[ATC_Element].txBusy = 0;
  return 1;
}

//--------------------------------------------------------------------------------------------------------------------//
uint8_t atc_sendAtCommand(ATC_Element_t ATC_Element, char *atCommand, uint32_t wait_ms, char *answerString, uint16_t answerSize, uint8_t searchingItems,...){
  if((ATC_Element < ATC_Element_0) || (ATC_Element >= atc_cnt))
    return 0;
  atc_process();
  uint32_t startTime;
  atc_elements[ATC_Element].foundAnswerSize = answerSize;
  atc_elements[ATC_Element].foundAnswerString = answerString;
  va_list tag;
  va_start (tag,searchingItems);
  for(uint8_t i=0; i<searchingItems ; i++){
    char *str = va_arg (tag, char*);
    #if (ATC_CMSIS_OS == false)
      atc_elements[ATC_Element].answerSearchingString[i] = malloc(strlen(str));
    #endif
    #if (ATC_CMSIS_OS == true)
      atc_elements[ATC_Element].answerSearchingString[i] = pvPortMalloc(strlen(str));
    #endif
    if(atc_elements[ATC_Element].answerSearchingString[i] != NULL)
      strcpy(atc_elements[ATC_Element].answerSearchingString[i], str);
  }
  va_end(tag);
  atc_elements[ATC_Element].foundAnswer = -1;
  uint8_t retValue = 0;
  if(atc_sendString(ATC_Element, atCommand, 1000) == 0)
    goto SEND_FAILD;  
  if(answerString != NULL)
    memset(answerString, 0 , answerSize);
  startTime = HAL_GetTick();  
  while(1){
    atc_delay(1);
    atc_process();  
    if(atc_elements[ATC_Element].foundAnswer >= 0){
      retValue = atc_elements[ATC_Element].foundAnswer + 1;
      break;
    }
    if(HAL_GetTick() - startTime > wait_ms){
      if(searchingItems == 0)
        retValue = 1;
      break;
    }
  }
  SEND_FAILD:
  #if (ATC_CMSIS_OS == false)
    for(uint8_t i = 0 ; i < ATC_MAX_ANSWER_SEARCHING_STRING ; i++){
      free(atc_elements[ATC_Element].answerSearchingString[i]);
      atc_elements[ATC_Element].answerSearchingString[i] = NULL;  
    }
    free(atc_elements[ATC_Element].foundAnswerString);
    atc_elements[ATC_Element].foundAnswerString = NULL;
  #endif
  #if (ATC_CMSIS_OS == true)
    for(uint8_t i = 0 ; i < ATC_MAX_ANSWER_SEARCHING_STRING ; i++){
      vPortFree(atc_elements[ATC_Element].answerSearchingString[i]);
      atc_elements[ATC_Element].answerSearchingString[i] = NULL;
  }
  #endif  
  return retValue;
}

//--------------------------------------------------------------------------------------------------------------------//
ATC_Element_t atc_getElementByName(char *name){
  for(uint8_t i = 0 ; i < ATC_MAX_ELEMENTS ; i++){
    if(strcmp(atc_elements[i].name, name) == 0)
      return (ATC_Element_t)i;            
  }
  return ATC_Element_Error;  
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool atc_addAutoSearchString(ATC_Element_t ATC_Element, char *str){
  if((ATC_Element < ATC_Element_0) || (ATC_Element >= atc_cnt))
    return false;
  for(uint8_t s = 0; s < ATC_MAX_ALWAYS_SEARCHING_STRING ; s++){
    if(atc_elements[ATC_Element].alwaysSearchingString[s] == NULL){
      #if (ATC_CMSIS_OS == false)
        atc_elements[ATC_Element].alwaysSearchingString[s] = malloc(strlen(str));            
      #endif
      #if (ATC_CMSIS_OS == true)
        atc_elements[ATC_Element].alwaysSearchingString[s] = pvPortMalloc(strlen(str));            
      #endif
      if(atc_elements[ATC_Element].alwaysSearchingString[s] != NULL){
        strcpy(atc_elements[ATC_Element].alwaysSearchingString[s], str);
        return true;
      }else
        break;
    }
  }
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
uint16_t atc_getSize(ATC_Element_t ATC_Element){
  return sizeof(atc_elements[ATC_Element]);
}

//--------------------------------------------------------------------------------------------------------------------//
#if (ATC_TXDMA == false)
_Bool atc_init(ATC_Element_t ATC_Element, USART_TypeDef *usart, uint16_t rxSize, uint16_t rxTimeout, char* name){
#endif  
#if (ATC_TXDMA == true)
_Bool atc_init(ATC_Element_t ATC_Element, USART_TypeDef *usart, uint16_t rxSize, uint16_t rxTimeout, char* name, DMA_TypeDef *DMA, uint32_t LL_DMA_STREAM_){
#endif

  if(ATC_Element != atc_cnt)
    return false;
  memset(&atc_elements[ATC_Element], 0, sizeof(ATC_t));
  atc_elements[ATC_Element].usart = usart;  
  atc_elements[ATC_Element].rxSize = rxSize;
  atc_elements[ATC_Element].rxTimeout = rxTimeout;
  strncpy(atc_elements[ATC_Element].name, name, 8);
  #if (ATC_TXDMA == true)
    atc_elements[ATC_Element].txDMA = DMA;
    atc_elements[ATC_Element].txDMA_Stream = LL_DMA_STREAM_;
    LL_DMA_EnableIT_TC(atc_elements[ATC_Element].txDMA, atc_elements[ATC_Element].txDMA_Stream);
    LL_USART_EnableDMAReq_TX(atc_elements[ATC_Element].usart);
  #endif
  #if (ATC_CMSIS_OS == false)
    atc_elements[ATC_Element].rxBuffer = malloc(rxSize);
  #endif
  #if (ATC_CMSIS_OS == true)
    atc_elements[ATC_Element].rxBuffer = pvPortMalloc(rxSize);
  #endif
  if(atc_elements[ATC_Element].rxBuffer != NULL){
    atc_cnt++;
    LL_USART_EnableIT_RXNE(atc_elements[ATC_Element].usart);
    return true;
  }
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
void gsm_found(char *found_str){
  char *str;
  str = strstr(found_str, "POWER DOWN\r\n");
  if(str != NULL){
    gsm.status.power = 0;
    return;
  }
  str = strstr(found_str, "\r\n+CREG: ");
  if(str != NULL){
    int16_t p1 = -1, p2 = -1;
    sscanf(str, "\r\n+CREG: %hd,%hd", &p1, &p2);
    if(p2 == 1){
      gsm.status.netReg = 1;
      gsm.status.netChange = 1;
      return;
    }else{
      gsm.status.netReg = 0;
      gsm.status.netChange = 1;
      return;
    }
  }
  #if (SIM868_CALL == true)
    str = strstr(found_str, "\r\n+CLIP:");
    if(str != NULL){
      if(sscanf(str, "\r\n+CLIP: \"%[^\"]\"", gsm.call.number) == 1)
        gsm.call.newCall = 1;
      return;
    }
    str = strstr(found_str, "\r\nNO CARRIER\r\n");
    if(str != NULL){
      gsm.call.endCall = 1;
      return;
    }
    str = strstr(found_str, "\r\n+DTMF:");
    if(str != NULL){
      char c = 0;
      if(sscanf(str, "\r\n+DTMF: %c\r\n", &c) == 1){
        if(gsm.call.dtmfCount < sizeof(gsm.call.dtmfBuffer) - 1){
          gsm.call.dtmfBuffer[gsm.call.dtmfCount] = c;
          gsm.call.dtmfCount++;
          gsm.call.dtmfUpdate = 1;
        }
      }
      return;
    }
  #endif
  #if (SIM868_MSG == true)
    str = strstr(found_str, "\r\n+CMTI:");
    if(str != NULL){
      str = strchr(str, ',');
      if(str != NULL){
        str++;
        gsm.msg.newMsg = atoi(str);
        return;
      }
    }
  #endif
  #if (SIM868_GPRS == true)
    str = strstr(found_str, "\r\nCLOSED\r\n");
    if(str != NULL){
      gsm.gprs.tcpConnection = 0;
      return;
    }
    str = strstr(found_str, "\r\n+CIPRXGET: 1\r\n");
    if(str != NULL){
      gsm.gprs.gotData = 1;
      return;
    }
    str = strstr(found_str, "\r\n+SMSTATE: 0\r\n");
    if(str != NULL){
      gsm.gprs.mqttConnected = 0;
      return;
    }
    str = strstr(found_str, "\r\n+SMPUBLISH: ");
    if(str != NULL){
      memset(gsm.gprs.mqttMessage, 0, sizeof(gsm.gprs.mqttMessage));
      memset(gsm.gprs.mqttTopic, 0, sizeof(gsm.gprs.mqttTopic));
      str = strtok(str, "\"");
      do{
        str = strtok(NULL, "\"");
        if(str == NULL)
          break;
        char *endStr;
        uint8_t len;
        endStr = strtok(NULL, "\"");
        if(endStr == NULL)
          break;
        len = endStr - str;
        if(len > sizeof(gsm.gprs.mqttTopic))
          len = sizeof(gsm.gprs.mqttTopic);
        if(len > 2)
          len --;
        strncpy(gsm.gprs.mqttTopic, str, len);
        str = strtok(NULL, "\"");
        if(str == NULL)
          break;
        endStr = strtok(NULL, "\"");
        if(endStr == NULL)
          break;
        len = endStr - str;
        if(len > sizeof(gsm.gprs.mqttMessage))
          len = sizeof(gsm.gprs.mqttMessage);
        if(len > 2)
          len --;
        strncpy(gsm.gprs.mqttMessage, str, len);
        gsm.gprs.mqttData = 1;      
        
      }while(0);
      return;
    }
  #endif
  #if (SIM868_BLUETOOTH == true)
    str = strstr(found_str, "");
  #endif
}

//--------------------------------------------------------------------------------------------------------------------//
void gsm_init_commands(void){
  gsm_command("AT&F0\r\n", 5000, NULL, 0, 1, "\r\nOK\r\n");
  gsm_command("ATE1\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n");
  gsm_command("AT+CREG=1\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n");
  gsm_command("AT+FSHEX=0\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n");
  #if (SIM868_CALL == true)
    gsm_command("AT+COLP=1\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n");
    gsm_command("AT+CLIP=1\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n");
    gsm_command("AT+DDET=1\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n");
  #endif
  #if (SIM868_MSG == true)
    gsm_msg_textMode(true, false);
    gsm_msg_selectStorage(gsm_msg_store_module);
    gsm_msg_selectCharacterSet(gsm_msg_chSet_ira);
  #endif
  #if (SIM868_GPRS == true)
    gsm_command("AT+CIPSHUT\r\n", 5000, NULL, 0, 2, "\r\nSHUT OK\r\n", "\r\nERROR\r\n");
    gsm_command("AT+CIPHEAD=0\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n");
    gsm_command("AT+CIPRXGET=1\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n");
  #endif
  #if (SIM868_BLUETOOTH == true)

  #endif
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_lock(uint32_t timeout_ms){
  uint32_t start = HAL_GetTick();
  while (HAL_GetTick() - start < timeout_ms){
    if(gsm.lock == 0){
      gsm.lock = 1;
      return true;
    }
    gsm_delay(1);
  }
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
void gsm_unlock(){
  gsm.lock = 0;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_init(void){
  if(gsm.inited == 1)
    return true;
  gsm_printf("[GSM] init begin\r\n");
  SIM868_KEY_HIGHT;
  gsm_delay(3000);
  memset(&gsm, 0, sizeof(gsm));
  atc_init(&gsm.atc, "GSM ATC", _GSM_USART, gsm_found);
  if(atc_addSearch(&gsm.atc, "POWER DOWN\r\n") == false)
    return false;
  if(atc_addSearch(&gsm.atc, "\r\n+CREG:") == false)
    return false;
  #if (SIM868_CALL == true)
    if(atc_addSearch(&gsm.atc, "\r\n+CLIP:") == false)
      return false;
    if(atc_addSearch(&gsm.atc, "\r\nNO CARRIER\r\n") == false)
      return false;
    if(atc_addSearch(&gsm.atc, "\r\n+DTMF:") == false)
      return false;
  #endif
  #if (SIM868_MSG == true)
    if(atc_addSearch(&gsm.atc, "\r\n+CMTI:") == false)
      return false;
    gsm.msg.newMsg = -1;
  #endif
  #if (SIM868_GPRS == true)
    if(atc_addSearch(&gsm.atc, "\r\nCLOSED\r\n") == false)
      return false;
    if(atc_addSearch(&gsm.atc, "\r\n+CIPRXGET: 1\r\n") == false)
      return false;
     if(atc_addSearch(&gsm.atc, "\r\n+SMSTATE: ") == false)
      return false;
     if(atc_addSearch(&gsm.atc, "\r\n+SMPUBLISH: ") == false)
      return false;
  #endif
  #if (SIM868_BLUETOOTH == true)

  #endif
  gsm_delay(1000);
  gsm_printf("[GSM] init done\r\n");
  gsm.inited = 1;
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
void gsm_loop(void){
  static uint32_t gsm_time_1s = 0;
  static uint32_t gsm_time_10s = 0;
  static uint32_t gsm_time_60s = 0;
  static uint8_t gsm_time_10s_check_power = 0;  
  atc_loop(&gsm.atc);
  char str1[64];
  char str2[16];
  //  +++ 1s timer  ######################
  if(HAL_GetTick() - gsm_time_1s > 1000){
    gsm_time_1s = HAL_GetTick();
    gsm_time_10s_check_power++;
    if((gsm.status.turnOn == 1) && (gsm.lock == 0) && (gsm_time_10s_check_power == 10)){
      gsm_time_10s_check_power = 0;
      if(gsm_command("AT\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
        gsm.error++;
      else
        gsm.error = 0;
      if(gsm.error >= 10){
        gsm.status.power = 0;
        if(gsm_power(true))
          gsm.error = 0;
      }
    }
    if((gsm.status.turnOff == 1) && (gsm_time_10s_check_power == 10)){
      gsm_time_10s_check_power = 0;
      gsm.error = 0;
      if(gsm_command("AT\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") == 1){
        gsm_power(false);
      }
    }
    
  #if (SIM868_CALL == true || SIM868_MSG == true || SIM868_GPRS == true)
      //  +++ simcard check
      if((gsm.status.power == 1) && (gsm.status.simcardChecked == 0)){
        if(gsm_command("AT+CPIN?\r\n", 1000, str1, sizeof(str1), 2, "\r\n+CPIN:", "\r\nERROR\r\n") == 1){
          if(sscanf(str1, "\r\n+CPIN: %[^\r\n]", str2) == 1){
            if(strcmp(str2, "READY") == 0){
              gsm_callback_simcardReady();
              gsm.status.simcardChecked = 1;
            }
            if(strcmp(str2, "SIM PIN") == 0){
              gsm_callback_simcardPinRequest();
              gsm.status.simcardChecked = 1;
            }
            if(strcmp(str2, "SIM PUK") == 0){
              gsm_callback_simcardPukRequest();
              gsm.status.simcardChecked = 1;
            }
          }
        }else{
          gsm_callback_simcardNotInserted();
        }
      }
      //  --- simcard check
      //  +++ network check
      if((gsm.status.power == 1) && (gsm.status.netChange == 1)){
        gsm.status.netChange = 0;
        if(gsm.status.netReg == 1){
          gsm.status.registerd = 1;
          gsm_callback_networkRegister();
        }else{
          gsm.status.registerd = 0;
          gsm_callback_networkUnregister();
        }
      }
  #endif
    //  --- network check
    //  +++ call check
  #if (SIM868_CALL == true)
      if(gsm.status.power == 1){
        if(gsm.call.newCall == 1){
          gsm.call.newCall = 0;
          gsm_callback_newCall(gsm.call.number);
        }
        if(gsm.call.endCall == 1){
          gsm.call.endCall = 0;
          gsm_callback_endCall();
        }
        if(gsm.call.dtmfUpdate == 1){
          gsm.call.dtmfUpdate = 0;
          gsm_callback_dtmf(gsm.call.dtmfBuffer, gsm.call.dtmfCount);
        }
      }
  #endif
    //  --- call check
    //  +++ msg check
  #if (SIM868_MSG == true)
      if(gsm.status.power == 1){
        if(gsm.msg.newMsg >= 0){
          if(gsm_msg_read(gsm.msg.newMsg)){
            gsm_msg_delete(gsm.msg.newMsg);
            gsm_callback_newMsg(gsm.msg.number, gsm.msg.time, (char*) gsm.buffer);
          }
          gsm.msg.newMsg = -1;
        }
      }
  #endif
    //  --- msg check
    //  +++ network check
  #if (SIM868_GPRS == true)
      if(gsm.status.power == 1){
        if(gsm.gprs.mqttData == 1){
          gsm.gprs.mqttData = 0;
          gsm_callback_mqttMessage(gsm.gprs.mqttTopic, gsm.gprs.mqttMessage);
        }
        if((gsm.gprs.mqttConnected == 1) && (gsm.gprs.mqttConnectedLast == 0)){
          gsm.gprs.mqttConnectedLast = 1;      
        }else 
          if((gsm.gprs.mqttConnected == 0) && (gsm.gprs.mqttConnectedLast == 1)){
          gsm.gprs.mqttConnectedLast = 0;
          gsm_callback_mqttDisconnect();
          }
      }
  #endif
    //  --- network check
  }
  //  --- 1s timer  ######################
  //  +++ 10s timer ######################
  if((HAL_GetTick() - gsm_time_10s > 10000) && (gsm.status.power == 1)){
    gsm_time_10s = HAL_GetTick();

  #if (SIM868_CALL == true || SIM868_MSG == true || SIM868_GPRS == true)
      //  +++ check network
      if((gsm.status.power == 1) && (gsm.lock == 0)){
        gsm_getSignalQuality_0_to_100();
        if(gsm.status.netReg == 0){
          gsm_command("AT+CREG?\r\n", 1000, NULL, 0, 0);
        }
      }
      //  --- check network
      //  +++ msg check
    #if (SIM868_MSG == true)
        if(gsm.lock == 0){
          if(gsm.msg.storageUsed > 0){
            for(uint16_t i = 0; i < 150; i++){
              if(gsm_msg_read(i)){
                gsm_msg_delete(i);
                gsm_callback_newMsg(gsm.msg.number, gsm.msg.time, (char*) gsm.buffer);
              }
            }
            gsm_msg_updateStorage();
          }
        }
    #endif
      //  --- msg check
      //  +++ gprs check
    #if (SIM868_GPRS == true)
        if((gsm.status.power == 1) && (gsm.lock == 0)){
          if(gsm.gprs.connected){
            if(gsm_command("AT+SAPBR=2,1\r\n", 1000, str1, sizeof(str1), 2, "\r\n+SAPBR: 1,", "\r\nERROR\r\n") == 1){
              if(sscanf(str1, "\r\n+SAPBR: 1,1,\"%[^\"\r\n]", gsm.gprs.ip) == 1){
                if(gsm.gprs.connectedLast == false){
                  gsm.gprs.connected = true;
                  gsm.gprs.connectedLast = true;
                  gsm_callback_gprsConnected();
                }
              }else{
                if(gsm.gprs.connectedLast == true){
                  gsm.gprs.connected = false;
                  gsm.gprs.connectedLast = false;
                  gsm_callback_gprsDisconnected();
                }
              }
            }else{
              if(gsm.gprs.connectedLast == true){
                gsm.gprs.connected = false;
                gsm.gprs.connectedLast = false;
                gsm_callback_gprsDisconnected();
              }
            }
          }
        }
    #endif
      //  --- gprs check

  #endif
  }
  //  --- 10s timer ######################
  //  +++ 60s timer ######################
  if((HAL_GetTick() - gsm_time_60s > 60000) && (gsm.status.power == 1)){
    gsm_time_60s = HAL_GetTick();

    //  +++ msg check
  #if (SIM868_MSG == true)
      if(gsm.lock == 0)
        gsm_msg_updateStorage();
  #endif
    //  --- msg check
  }
  //  --- 60s timer  ######################
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_power(_Bool on_off){
  gsm_printf("[GSM] power(%d) begin\r\n", on_off);
  uint8_t state = 0;
  if(on_off){
    gsm.status.turnOn = 1;
    gsm.status.turnOff = 0;  
  }else{
    gsm.status.turnOn = 0;
    gsm.status.turnOff = 1;    
  }
  if(gsm_command("AT\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n") == 1)
    state = 1;
  if((on_off == true) && (state == 1)){
    memset(&gsm.status, 0, sizeof(gsm.status));
    if(on_off)
      gsm.status.turnOn = 1;
    else
      gsm.status.turnOff = 1;    
    gsm.status.power = 1;
    gsm_init_commands();
    gsm_printf("[GSM] power(%d) done\r\n", on_off);
    return true;
  }
  if((on_off == true) && (state == 0)){
    memset(&gsm.status, 0, sizeof(gsm.status));
    if(on_off)
      gsm.status.turnOn = 1;
    else
      gsm.status.turnOff = 1; 
    SIM868_KEY_LOW;
    gsm_delay(1500);
    SIM868_KEY_HIGHT;
    gsm_delay(3000);
    for(uint8_t i = 0; i < 5; i++){
      if(gsm_command("AT\r\n", 1000, NULL, 0, 1, "\r\nOK\r\n") == 1){
        state = 1;
        break;
      }
    }
    if(state == 1){
      gsm_delay(5000);
      gsm_init_commands();
      gsm.status.power = 1;
      gsm_printf("[GSM] power(%d) done\r\n", on_off);
      return true;
    }else{
      gsm_printf("[GSM] power(%d) failed!\r\n", on_off);
      return false;
    }
  }
  if((on_off == false) && (state == 0)){
    gsm_printf("[GSM] power(%d) done\r\n", on_off);
    gsm.status.power = 0;
    return true;
  }
  if((on_off == false) && (state == 1)){
    SIM868_KEY_LOW;
    gsm_delay(1500);
    SIM868_KEY_HIGHT;
    gsm_delay(3000);
    gsm.status.power = 0;
    gsm_printf("[GSM] power(%d) done\r\n", on_off);
    return true;
  }
  gsm_printf("[GSM] power(%d) failed!\r\n", on_off);
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_registered(void){
  return gsm.status.registerd;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_setDefault(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] default() failed!\r\n");
    return false;
  }
  if(gsm_command("AT&F0\r\n", 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] default() done\r\n");
    gsm_unlock();
    return true;
  }
  gsm_printf("[GSM] default() failed!\r\n");
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_saveProfile(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] getIMEI() failed!\r\n");
    return false;
  }
  if(gsm_command("AT&W\r\n", 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] getIMEI() done\r\n");
    gsm_unlock();
    return true;
  }
  gsm_printf("[GSM] saveProfile() failed!\r\n");
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_enterPinPuk(const char *string){
  char str[32];
  if(string == NULL){
    gsm_printf("[GSM] enterPinPuk() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] enterPinPuk() failed!\r\n");
    return false;
  }
  sprintf(str, "AT+CPIN=%s\r\n", string);
  if(gsm_command(str, 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] enterPinPuk() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] enterPinPuk() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_getIMEI(char *string, uint8_t sizeOfString){
  if((string == NULL) || (sizeOfString < 15)){
    gsm_printf("[GSM] getIMEI() failed!\r\n");
    return false;
  }
  char str[32];
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] getIMEI() failed!\r\n");
    return false;
  }
  if(gsm_command("AT+GSN\r\n", 1000 , str, sizeof(str), 2, "AT+GSN", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] getIMEI() failed!\r\n");
    gsm_unlock();
    return false;
  }
  if(sscanf(str, "\r\nAT+GSN\r\n %[^\r\n]", string) != 1){
    gsm_printf("[GSM] getIMEI() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] getIMEI() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_getVersion(char *string, uint8_t sizeOfString){
  if(string == NULL){
    gsm_printf("[GSM] getVersion() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] getVersion() failed!\r\n");
    return false;
  }
  char str1[16 + sizeOfString];
  char str2[sizeOfString + 1];
  if(gsm_command("AT+CGMR\r\n", 1000 , str1, sizeof(str1), 2, "AT+GMM", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] getVersion() failed!\r\n");
    gsm_unlock();
    return false;
  }
  if(sscanf(str1, "\r\nAT+CGMR\r\n %[^\r\n]", str2) != 1){
    gsm_printf("[GSM] getVersion() failed!\r\n");
    gsm_unlock();
    return false;
  }
  strncpy(string, str2, sizeOfString);
  gsm_printf("[GSM] getVersion() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_getModel(char *string, uint8_t sizeOfString){
  if(string == NULL){
    gsm_printf("[GSM] getModel() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] getModel() failed!\r\n");
    return false;
  }
  char str1[16 + sizeOfString];
  char str2[sizeOfString + 1];
  if(gsm_command("AT+GMM\r\n", 1000 , str1, sizeof(str1), 2, "AT+GMM", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] getModel() failed!\r\n");
    gsm_unlock();
    return false;
  }
  if(sscanf(str1, "\r\nAT+GMM\r\n %[^\r\n]", str2) != 1){
    gsm_printf("[GSM] getModel() failed!\r\n");
    gsm_unlock();
    return false;
  }
  strncpy(string, str2, sizeOfString);
  gsm_printf("[GSM] getModel() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_getServiceProviderName(char *string, uint8_t sizeOfString){
  if(string == NULL){
    gsm_printf("[GSM] getServiceProviderName() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] getServiceProviderName() failed!\r\n");
    return false;
  }
  char str1[16 + sizeOfString];
  char str2[sizeOfString + 1];
  if(gsm_command("AT+CSPN?\r\n", 1000 , str1, sizeof(str1), 2, "\r\n+CSPN:", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] getServiceProviderName() failed!\r\n");
    gsm_unlock();
    return false;
  }
  if(sscanf(str1, "\r\n+CSPN: \"%[^\"]\"", str2) != 1){
    gsm_printf("[GSM] getServiceProviderName() failed!\r\n");
    gsm_unlock();
    return false;
  }
  strncpy(string, str2, sizeOfString);
  gsm_printf("[GSM] getServiceProviderName() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
uint8_t gsm_getSignalQuality_0_to_100(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] getSignalQuality_0_to_100() failed!\r\n");
    return false;
  }
  char str[32];
  int16_t p1, p2;
  if(gsm_command("AT+CSQ\r\n", 1000, str, sizeof(str), 2, "\r\n+CSQ:", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] getSignalQuality_0_to_100() failed!\r\n");
    gsm_unlock();
    return 0;
  }
  if(sscanf(str, "\r\n+CSQ: %hd,%hd\r\n", &p1, &p2) != 2){
    gsm_printf("[GSM] getSignalQuality_0_to_100() failed!\r\n");
    gsm_unlock();
    return 0;
  }
  if(p1 == 99)
    gsm.signal = 0;
  else
    gsm.signal = (p1 * 100) / 31;
  gsm_printf("[GSM] getSignalQuality_0_to_100() done\r\n");
  gsm_unlock();
  return gsm.signal;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_waitForRegister(uint8_t waitSecond){
  gsm_printf("[GSM] waitForRegister(%d second) begin\r\n", waitSecond);
  uint32_t startTime = HAL_GetTick();
  while (HAL_GetTick() - startTime < (waitSecond * 1000)){
    gsm_delay(100);
    gsm_loop();
    if(gsm.status.registerd == 1){
      for(uint8_t i = 0; i < 10; i++){
        gsm_loop();
        gsm_delay(500);
      }
      gsm_printf("[GSM] waitForRegister() done\r\n");
      return true;
    }
    if(gsm.inited == 0)
      continue;
  }
  gsm_printf("[GSM] waitForRegister() failed!\r\n");
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_tonePlay(gsm_tone_t gsm_tone_, uint32_t durationMiliSecond, uint8_t level_0_100){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] tonePlay() failed!\r\n");
    return false;
  }
  char str[32];
  sprintf(str, "AT+SNDLEVEL=0,%d\r\n", level_0_100);
  if(gsm_command(str, 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] tonePlay() failed!\r\n");
    gsm_unlock();
    return false;
  }
  sprintf(str, "AT+STTONE=1,%d,%d\r\n", gsm_tone_, (int) durationMiliSecond);
  if(gsm_command(str, 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] tonePlay() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] tonePlay() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_toneStop(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] toneStop() failed!\r\n");
    return false;
  }
  if(gsm_command("AT+STTONE=0\r\n", 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] toneStop() done\r\n");
    gsm_unlock();
    return true;
  }
  gsm_printf("[GSM] toneStop() failed!\r\n");
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_dtmf(char *string, uint32_t durationMiliSecond){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] dtmf() failed!\r\n");
    return false;
  }
  char str[32];
  sprintf(str, "AT+VTS=\"%s\",%d\r\n", string, (int) (durationMiliSecond / 100));
  if(gsm_command(str, 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] dtmf() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] dtmf() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_ussd(char *command, char *answer, uint16_t sizeOfAnswer, uint8_t waitSecond){
  gsm_printf("[GSM] ussd() begin\r\n");
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] ussd() failed!\r\n");
    return false;
  }
  if(command == NULL){
    if(gsm_command("AT+CUSD=2\r\n", 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
      gsm_printf("[GSM] ussd() failed!\r\n");
      gsm_unlock();
      return false;
    }
    gsm_printf("[GSM] ussd() done\r\n");
    gsm_unlock();
    return true;
  }else
    if(answer == NULL){
    char str[16 + strlen(command)];
    sprintf(str, "AT+CUSD=0,\"%s\"\r\n", command);
    if(gsm_command(str, waitSecond * 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
      gsm_printf("[GSM] ussd() failed!\r\n");
      gsm_unlock();
      return false;
    }
    gsm_printf("[GSM] ussd() done\r\n");
    gsm_unlock();
    return true;
    }else{
    memset(answer, 0, sizeOfAnswer);
    char str[16 + strlen(command)];
    sprintf(str, "AT+CUSD=1,\"%s\"\r\n", command);
    if(gsm_command(str, waitSecond * 1000, (char* )gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+CUSD:", "\r\nERROR\r\n") != 1){
      gsm_command("AT+CUSD=2\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
      gsm_printf("[GSM] ussd() failed!\r\n");
      gsm_unlock();
      return false;
    }
    gsm_command("AT+CUSD=2\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
    char *start = strstr((char*) gsm.buffer, "\"");
    if(start != NULL){
      char *end = strstr(start, "\", ");
      if(end != NULL){
        start++;
        strncpy(answer, start, end - start);
        gsm_printf("[GSM] ussd() done\r\n");
        gsm_unlock();
        return true;
      }else{
        gsm_printf("[GSM] ussd() failed!\r\n");
        gsm_unlock();
        return false;
      }
    }else{
      gsm_printf("[GSM] ussd() failed!\r\n");
      gsm_unlock();
      return false;
    }
    }
}

//--------------------------------------------------------------------------------------------------------------------//

#if (SIM868_GPRS == true)
//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_setApName(const char *apName){
  if(apName == NULL){
    gsm_printf("[GSM] gprs_setApName() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_setApName(%s) failed!\r\n", apName);
    return false;
  }
  if(gsm_command("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_setApName(%s) failed!\r\n", apName);
    gsm_unlock();
    return false;
  }
  sprintf((char*)gsm.buffer, "AT+SAPBR=3,1,\"APN\",\"%s\"\r\n", apName);
  if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") == 1){
    gsm_printf("[GSM] gprs_setApName(%s) done\r\n", apName);
    gsm_unlock();
    return true;
  }
  gsm_printf("[GSM] gprs_setApName(%s) failed!\r\n", apName);
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_connect(void){
  gsm_printf("[GSM] gprs_connect() begin\r\n");
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_connect() failed!\r\n");
    return false;
  }
  gsm_command("AT+SAPBR=0,1\r\n", 5000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
  gsm_delay(2000);
  if(gsm_command("AT+SAPBR=1,1\r\n", 90000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm.gprs.connected = false;
    gsm.gprs.connectedLast = false;
    gsm_printf("[GSM] gprs_connect() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_delay(2000);
  if(gsm_command("AT+SAPBR=2,1\r\n", 1000, (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+SAPBR: 1,1,", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_connect() failed!\r\n");
    gsm.gprs.connected = false;
    gsm.gprs.connectedLast = false;
    gsm_unlock();
    return false;
  }
  memset(gsm.gprs.ip, 0, sizeof(gsm.gprs.ip));
  sscanf((char*)gsm.buffer, "\r\n+SAPBR: 1,1,\"%[^\"\r\n]", gsm.gprs.ip);
  gsm.gprs.connected = true;
  gsm.gprs.connectedLast = true;
  gsm_printf("[GSM] gprs_connect() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_disconnect(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_disconnect() failed!\r\n");
    return false;
  }
  gsm.gprs.connected = false;
  gsm.gprs.connectedLast = false;
  gsm_command("AT+CIPSHUT\r\n", 5000, NULL, 0, 2, "\r\nSHUT OK\r\n", "\r\nERROR\r\n");
  if(gsm_command("AT+SAPBR=0,1\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") == 1){
    gsm_printf("[GSM] gprs_disconnect() done\r\n");
    gsm_unlock();
    return true;
  }
  gsm_printf("[GSM] gprs_disconnect() failed!\r\n");
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_httpInit(void){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_httpInit() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_httpInit() failed!\r\n");
    return false;
  }
  gsm_command("AT+HTTPTERM\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
  if(gsm_command("AT+HTTPINIT\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_httpInit() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm.gprs.dataCurrent = 0;
  gsm_printf("[GSM] gprs_httpInit() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_httpSetContent(const char *content){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_httpSetContent() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_httpSetContent() failed!\r\n");
    return false;
  }
  sprintf((char*)gsm.buffer, "AT+HTTPPARA=\"CONTENT\",\"%s\"\r\n", content);
  if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_httpSetContent() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] gprs_httpSetContent() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_httpSetUserData(const char *data){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_httpSetUserData() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_httpSetUserData() failed!\r\n");
    return false;
  }
  gsm_transmit((uint8_t* ) "AT+HTTPPARA=\"USERDATA\",\"", strlen("AT+HTTPPARA=\"USERDATA\",\""));
  gsm_transmit((uint8_t* ) data, strlen(data));
  if(gsm_command("\"\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_httpSetUserData() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] gprs_httpSetUserData() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_httpSendData(const char *data, uint16_t timeout_ms){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_httpSendData() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_httpSendData() failed!\r\n");
    return false;
  }
  sprintf((char*)gsm.buffer, "AT+HTTPDATA=%d,%d\r\n", strlen(data), timeout_ms);
  do{
    if(gsm_command((char*)gsm.buffer, timeout_ms, NULL, 0, 2, "\r\nDOWNLOAD\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command(data, timeout_ms, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    gsm_delay(timeout_ms);
    gsm_printf("[GSM] gprs_httpSendData() done\r\n");
    gsm_unlock();
    return true;
  }while(0);
  gsm_printf("[GSM] gprs_httpSendData() failed!\r\n");
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
int16_t gsm_gprs_httpGet(const char *url, _Bool ssl, uint16_t timeout_ms){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_httpGet(%s) failed!\r\n", url);
    return -1;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_httpGet(%s) failed!\r\n", url);
    return false;
  }
  gsm.gprs.code = -1;
  gsm.gprs.dataLen = 0;
  do{
    if(gsm_command("AT+HTTPPARA=\"CID\",1\r\n", 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+HTTPPARA=\"URL\",\"%s\"\r\n", url);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command("AT+HTTPPARA=\"REDIR\",1\r\n", 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(ssl){
      if(gsm_command("AT+HTTPSSL=1\r\n", 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
        break;
    }else{
      if(gsm_command("AT+HTTPSSL=0\r\n", 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
        break;
    }
    if(gsm_command("AT+HTTPACTION=0\r\n", timeout_ms , (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+HTTPACTION:", "\r\nERROR\r\n") != 1)
      break;
    sscanf((char*)gsm.buffer, "\r\n+HTTPACTION: 0,%hd,%d\r\n", &gsm.gprs.code, &gsm.gprs.dataLen);
  }while(0);
  gsm_printf("[GSM] gprs_httpGet(%s) done. answer: %d\r\n", url, gsm.gprs.code);
  gsm_unlock();
  return gsm.gprs.code;
}

//--------------------------------------------------------------------------------------------------------------------//
int16_t gsm_gprs_httpPost(const char *url, _Bool ssl, uint16_t timeout_ms){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_httpPost(%s) failed!\r\n", url);
    return -1;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_httpPost(%s) failed!\r\n", url);
    return false;
  }
  gsm.gprs.code = -1;
  gsm.gprs.dataLen = 0;
  do{
    if(gsm_command("AT+HTTPPARA=\"CID\",1\r\n", 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+HTTPPARA=\"URL\",\"%s\"\r\n", url);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command("AT+HTTPPARA=\"REDIR\",1\r\n", 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(ssl){
      if(gsm_command("AT+HTTPSSL=1\r\n", 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
        break;
    }else{
      if(gsm_command("AT+HTTPSSL=0\r\n", 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
        break;
    }
    if(gsm_command("AT+HTTPACTION=1\r\n", timeout_ms , (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+HTTPACTION:", "\r\nERROR\r\n") != 1)
      break;
    sscanf((char*)gsm.buffer, "\r\n+HTTPACTION: 1,%hd,%d\r\n", &gsm.gprs.code, &gsm.gprs.dataLen);
  }while(0);
  gsm_printf("[GSM] gprs_httpPost(%s) done. answer: %d\r\n", url, gsm.gprs.code);
  gsm_unlock();
  return gsm.gprs.code;
}

//--------------------------------------------------------------------------------------------------------------------//
uint32_t gsm_gprs_httpDataLen(void){
  return gsm.gprs.dataLen;
}

//--------------------------------------------------------------------------------------------------------------------//
uint16_t gsm_gprs_httpRead(uint8_t *data, uint16_t len){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_httpRead() failed!\r\n");
    return 0;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_httpRead() failed!\r\n");
    return false;
  }
  memset(gsm.buffer, 0, sizeof(gsm.buffer));
  if(len >= sizeof(gsm.buffer) - 32)
    len = sizeof(gsm.buffer) - 32;
  char buf[32];
  sprintf(buf, "AT+HTTPREAD=%d,%d\r\n", gsm.gprs.dataCurrent, len);
  if(gsm_command(buf, 1000 , (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+HTTPREAD: ", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_httpRead() failed!\r\n");
    gsm_unlock();
    return 0;
  }
  if(sscanf((char*)gsm.buffer, "\r\n+HTTPREAD: %hd\r\n", &len) != 1){
    gsm_printf("[GSM] gprs_httpRead() failed!\r\n");
    gsm_unlock();
    return 0;
  }
  gsm.gprs.dataCurrent += len;
  if(gsm.gprs.dataCurrent >= gsm.gprs.dataLen)
    gsm.gprs.dataCurrent = gsm.gprs.dataLen;
  
  uint8_t *s = (uint8_t*)strchr((char*)&gsm.buffer[10], '\n');  
  if(s == NULL){
    gsm_printf("[GSM] gprs_httpRead() failed!\r\n");
    gsm_unlock();
    return 0;
  }
  s++;
  for(uint16_t i = 0 ; i < len; i++)
    gsm.buffer[i] = *s++;
  if(data != NULL)
    memcpy(data, gsm.buffer, len);
  gsm_printf("[GSM] gprs_httpRead() done. length: %d\r\n", len);
  gsm_unlock();
  return len;      
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_httpTerminate(void){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_httpTerminate() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_httpTerminate() failed!\r\n");
    return false;
  }
  if(gsm_command("AT+HTTPTERM\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") == 1){
    gsm_printf("[GSM] gprs_httpTerminate() done\r\n");
    gsm_unlock();
    return true;
  }
  gsm_printf("[GSM] gprs_httpTerminate() failed!\r\n");
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpLogin(const char *ftpAddress, const char *ftpUserName, const char *ftpPassword, uint16_t port){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpLogin() failed!\r\n");
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpLogin() failed!\r\n");
    return gsm_ftp_error_error;
  }
  gsm_ftp_error_t ret = gsm_ftp_error_error;
  do{
    if(gsm_command("AT+FTPMODE=1\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command("AT+FTPCID=1\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+FTPSERV=\"%s\"\r\n", ftpAddress);
    if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+FTPPORT=%d\r\n", port);
    if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+FTPUN=\"%s\"\r\n", ftpUserName);
    if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+FTPPW=\"%s\"\r\n", ftpPassword);
    if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    ret = gsm_ftp_error_none;
  } while(0);
  gsm_printf("[GSM] gprs_ftpLogin() done\r\n");
  gsm_unlock();
  return ret;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpUploadBegin(_Bool asciiFile, _Bool append, const char *path, const char *fileName, const uint8_t *data, uint16_t len){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpUploadBegin(%s/%s) failed!\r\n", path, fileName);
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpUploadBegin(%s/%s) failed!\r\n", path, fileName);
    return gsm_ftp_error_error;
  }
  char *s;
  char answer[64];
  gsm_ftp_error_t error = gsm_ftp_error_error;
  do{
    gsm_command("AT+FTPEXTPUT=0\r\n", 5000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
    if(asciiFile)
      sprintf((char*)gsm.buffer, "AT+FTPTYPE=\"A\"\r\n");
    else
      sprintf((char*)gsm.buffer, "AT+FTPTYPE=\"I\"\r\n");
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(append)
      sprintf((char*)gsm.buffer, "AT+FTPPUTOPT=\"APPE\"\r\n");
    else
      sprintf((char*)gsm.buffer, "AT+FTPPUTOPT=\"STOR\"\r\n");
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+FTPPUTPATH=\"%s\"\r\n", path);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+FTPPUTNAME=\"%s\"\r\n", fileName);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command("AT+FTPPUT=1\r\n", 75000 , answer, sizeof(answer), 2, "\r\n+FTPPUT: 1,", "\r\nERROR\r\n") != 1)
      break;
    s = strchr(answer, ',');
    if(s == NULL)
      break;
    s++;
    if(atoi(s) != 1){
      error = (gsm_ftp_error_t) atoi(s);
      break;
    }
    sprintf((char*)gsm.buffer, "AT+FTPPUT=2,%d\r\n", len);
    if(gsm_command((char*)gsm.buffer, 5000 , answer, sizeof(answer), 2, "\r\n+FTPPUT: 2,", "\r\nERROR\r\n") != 1)
      break;
    s = strchr(answer, ',');
    if(s == NULL)
      break;
    s++;
    if(atoi(s) != len)
      break;
    gsm_transmit((uint8_t* )data, len);
    gsm_command("", 120 * 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
    error = gsm_ftp_error_none;
  }while(0);
  gsm_printf("[GSM] gprs_ftpUploadBegin(%s/%s) done\r\n", path, fileName);
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpUpload(const uint8_t *data, uint16_t len){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpUpload() failed!\r\n");
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpUpload() failed!\r\n");
    return gsm_ftp_error_error;
  }
  char *s;
  char answer[64];
  gsm_ftp_error_t error = gsm_ftp_error_error;
  do{
    sprintf((char*)gsm.buffer, "AT+FTPPUT=2,%d\r\n", len);
    if(gsm_command((char*)gsm.buffer, 5000 , answer, sizeof(answer), 2, "\r\n+FTPPUT: 2,", "\r\nERROR\r\n") != 1)
      break;
    s = strchr(answer, ',');
    if(s == NULL)
      return gsm_ftp_error_error;
    s++;
    if(atoi(s) != len)
      return gsm_ftp_error_error;
    gsm_transmit((uint8_t* )data, len);
    if(gsm_command("", 120 * 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    error = gsm_ftp_error_none;
  }while(0);
  gsm_printf("[GSM] gprs_ftpUpload() done\r\n");
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpUploadEnd(void){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpUploadEnd() failed!\r\n");
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpUploadEnd() failed!\r\n");
    return gsm_ftp_error_error;
  }
  if(gsm_command("AT+FTPPUT=2,0\r\n", 5000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_ftpUploadEnd() failed!\r\n");
    gsm_unlock();
    return gsm_ftp_error_error;
  }
  gsm_printf("[GSM] gprs_ftpUploadEnd() done\r\n");
  gsm_unlock();
  return gsm_ftp_error_none;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpExtUploadBegin(_Bool asciiFile, _Bool append, const char *path, const char *fileName){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpExtUploadBegin() failed!\r\n");
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpExtUploadBegin() failed!\r\n");
    return gsm_ftp_error_error;
  }
  gsm_ftp_error_t error = gsm_ftp_error_error;
  do{
    gsm_command("AT+FTPEXTPUT=0\r\n", 5000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
    gsm_delay(100);
    if(asciiFile)
      sprintf((char*)gsm.buffer, "AT+FTPTYPE=\"A\"\r\n");
    else
      sprintf((char*)gsm.buffer, "AT+FTPTYPE=\"I\"\r\n");
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    gsm_delay(100);
    if(append)
      sprintf((char*)gsm.buffer, "AT+FTPPUTOPT=\"APPE\"\r\n");
    else
      sprintf((char*)gsm.buffer, "AT+FTPPUTOPT=\"STOR\"\r\n");
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    gsm_delay(100);
    sprintf((char*)gsm.buffer, "AT+FTPPUTPATH=\"%s\"\r\n", path);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    gsm_delay(100);
    sprintf((char*)gsm.buffer, "AT+FTPPUTNAME=\"%s\"\r\n", fileName);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    gsm_delay(100);
    if(gsm_command("AT+FTPEXTPUT=1\r\n", 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    gsm.gprs.ftpExtOffset = 0;
    error = gsm_ftp_error_none;
  }while(0);
  gsm_printf("[GSM] gprs_ftpExtUploadBegin() done. answer: %d\r\n", error);
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpExtUpload(uint8_t *data, uint16_t len){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpExtUpload() failed!\r\n");
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpExtUpload() failed!\r\n");
    return gsm_ftp_error_error;
  }
  gsm_ftp_error_t error = gsm_ftp_error_error;
  char answer[64];
  do{
    sprintf((char*)gsm.buffer, "AT+FTPEXTPUT=2,%d,%d,5000\r\n", gsm.gprs.ftpExtOffset, len);
    if(gsm_command((char*)gsm.buffer, 5000, answer, sizeof(answer), 2, "\r\n+FTPEXTPUT: ", "\r\nERROR\r\n") != 1)
      break;
    char *s = strchr(answer, ',');
    if(s == NULL)
      break;
    s++;
    uint32_t d = atoi(s);
    if(d != len)
      break;
    gsm_delay(100);
    gsm_transmit(data, len);
    if(gsm_command("", 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    gsm.gprs.ftpExtOffset += len;
    error = gsm_ftp_error_none;
  }while(0);
  gsm_printf("[GSM] gprs_ftpExtUpload() done. answer: %d\r\n", error);
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpExtUploadEnd(void){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpExtUploadEnd() failed!\r\n");
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpExtUploadEnd() failed!\r\n");
    return gsm_ftp_error_error;
  }
  gsm_ftp_error_t error = gsm_ftp_error_error;
  do{
    if(gsm_command("AT+FTPPUT=1\r\n", 75000, (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+FTPPUT: 1,", "\r\nERROR\r\n") != 1)
      break;
    char *s = strchr((char*)gsm.buffer, ',');
    if(s == NULL)
      break;
    s++;
    error = (gsm_ftp_error_t) atoi(s);
    gsm_command("AT+FTPEXTPUT=0\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
  }while(0);
  gsm_printf("[GSM] gprs_ftpExtUploadEnd() done. answer: %d\r\n", error);
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpCreateDir(const char *path){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpCreateDir(%s) failed!\r\n", path);
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpCreateDir(%s) failed!\r\n", path);
    return gsm_ftp_error_error;
  }
  gsm_ftp_error_t error = gsm_ftp_error_error;
  do{
    sprintf((char*)gsm.buffer, "AT+FTPGETPATH=\"%s\"\r\n", path);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command("AT+FTPMKD\r\n", 75000 , (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+FTPMKD: 1,", "\r\nERROR\r\n") != 1)
      break;
    char *s = strchr((char*)gsm.buffer, ',');
    if(s == NULL)
      break;
    s++;
    error = (gsm_ftp_error_t) atoi(s);
  }while(0);
  gsm_printf("[GSM] gprs_ftpCreateDir(%s) done. answer: %d\r\n", path, error);
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpRemoveDir(const char *path){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpRemoveDir(%s) failed!\r\n", path);
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpRemoveDir(%s) failed!\r\n", path);
    return gsm_ftp_error_error;
  }
  gsm_ftp_error_t error = gsm_ftp_error_error;
  do{
    sprintf((char*)gsm.buffer, "AT+FTPGETPATH=\"%s\"\r\n", path);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command("AT+FTPRMD\r\n", 75000 , (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+FTPRMD: 1,", "\r\nERROR\r\n") != 1)
      break;
    char *s = strchr((char*)gsm.buffer, ',');
    if(s == NULL)
      break;
    s++;
    error = (gsm_ftp_error_t) atoi(s);
  }while(0);
  gsm_printf("[GSM] gprs_ftpRemoveDir(%s) done. answer: %d\r\n", path, error);
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
uint32_t gsm_gprs_ftpGetSize(const char *path, const char *name){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpGetSize(%s, %s) failed!\r\n", path, name);
    return 0;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpGetSize(%s, %s) failed!\r\n", path, name);
    return 0;
  }
  uint32_t error = 0;
  do{
    sprintf((char*)gsm.buffer, "AT+FTPGETPATH=\"%s\"\r\n", path);
    if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+FTPGETNAME=\"%s\"\r\n", name);
    if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command("AT+FTPSIZE\r\n", 75000, (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+FTPSIZE: 1,", "\r\nERROR\r\n") != 1)
      break;
    char *s = strchr((char*)gsm.buffer, ',');
    if(s == NULL)
      break;
    s++;
    if(atoi(s) == 0){
      s = strchr((char*)gsm.buffer, ',');
      if(s == NULL)
        break;
      s++;
      error = atoi(s);
    }
  }while(0);
  gsm_printf("[GSM] gprs_ftpGetSize(%s, %s) done. answer: %d\r\n", path, name, error);
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpRemove(const char *path, const char *name){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpRemove(%s, %s) failed!\r\n", path, name);
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpRemove(%s, %s) failed!\r\n", path, name);
    return gsm_ftp_error_error;
  }
  gsm_ftp_error_t error = gsm_ftp_error_error;
  do{
    sprintf((char*)gsm.buffer, "AT+FTPGETPATH=\"%s\"\r\n", path);
    if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    sprintf((char*)gsm.buffer, "AT+FTPGETNAME=\"%s\"\r\n", name);
    if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command("AT+FTPDELE\r\n", 75000, (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+FTPDELE: 1,", "\r\nERROR\r\n") != 1)
      break;
    char *s = strchr((char*)gsm.buffer, ',');
    if(s == NULL)
      break;
    s++;
    error = (gsm_ftp_error_t) atoi(s);
  }while(0);
  gsm_printf("[GSM] gprs_ftpRemove(%s, %s) done. answer: %d\r\n", path, name, error);
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpIsExistFolder(const char *path){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpIsExistFolder(%s) failed!\r\n", path);
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpIsExistFolder(%s) failed!\r\n", path);
    return gsm_ftp_error_error;
  }
  gsm_ftp_error_t error = gsm_ftp_error_error;
  char answer[32];
  do{
    sprintf((char*)gsm.buffer, "AT+FTPGETPATH=\"%s\"\r\n", path);
    if(gsm_command((char*)gsm.buffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1)
      break;
    if(gsm_command("AT+FTPLIST=1\r\n", 75000, answer, sizeof(answer), 2, "\r\n+FTPLIST: ", "\r\nERROR\r\n") != 1)
      break;
    uint8_t i1 = 0, i2 = 0;
    if(sscanf(answer, "\r\n+FTPLIST: %hhd,%hhd", &i1, &i2) != 2)
      break;
    if(i1 == 1 && i2 == 1){
      gsm_command("AT+FTPQUIT\r\n", 75000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
      error = gsm_ftp_error_none;
      break;
    }
    if(i1 == 1 && i2 == 77){
      error = gsm_ftp_error_notExist;
      break;
    }
  }while(0);
  gsm_printf("[GSM] gprs_ftpIsExistFolder(%s) done. answer: %d\r\n", path, error);
  gsm_unlock();
  return error;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_ftpIsBusy(void){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpIsBusy() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpIsBusy() failed!\r\n");
    return false;
  }
  if(gsm_command("AT+FTPSTATE\r\n", 75000, NULL, 0, 3, "\r\n+FTPSTATE: 0\r\n", "\r\n+FTPSTATE: 1\r\n", "\r\nERROR\r\n") == 1){
    gsm_printf("[GSM] gprs_ftpIsBusy() done. true\r\n");
    gsm_unlock();
    return true;
  }
  gsm_printf("[GSM] gprs_ftpIsBusy() done. false\r\n");
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
gsm_ftp_error_t gsm_gprs_ftpQuit(void){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ftpQuit() failed!\r\n");
    return gsm_ftp_error_error;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ftpQuit() failed!\r\n");
    return gsm_ftp_error_error;
  }
  if(gsm_command("AT+FTPQUIT\r\n", 5000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_ftpQuit() failed!\r\n");
    gsm_unlock();
    return gsm_ftp_error_error;
  }
  gsm_printf("[GSM] gprs_ftpQuit() done\r\n");
  gsm_unlock();
  return gsm_ftp_error_none;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_ntpServer(char *server, int8_t time_zone_in_quarter){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ntpServer(%s, %d) failed!\r\n", server, time_zone_in_quarter);
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ntpServer(%s, %d) failed!\r\n", server, time_zone_in_quarter);
    return false;
  }
  sprintf((char*)gsm.buffer, "AT+CNTP=\"%s\",%d\r\n", server, time_zone_in_quarter);
  if(gsm_command((char*)gsm.buffer, 10000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_ntpServer(%s, %d) failed!\r\n", server, time_zone_in_quarter);
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] gprs_ntpServer(%s, %d) done\r\n", server, time_zone_in_quarter);
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_ntpSyncTime(void){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_ntpSyncTime() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ntpSyncTime() failed!\r\n");
    return false;
  }
  if(gsm_command("AT+CNTP\r\n", 10000, NULL, 0, 2, "\r\n+CNTP: 1\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_ntpSyncTime() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] gprs_ntpSyncTime() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_ntpGetTime(char *string){
  if(string == NULL){
    gsm_printf("[GSM] gprs_ntpGetTime() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_ntpGetTime() failed!\r\n");
    return false;
  }
  if(gsm_command("AT+CCLK?\r\n", 10000, (char*)gsm.buffer, sizeof(gsm.buffer), 2, "\r\n+CCLK:", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_ntpGetTime() failed!\r\n");
    gsm_unlock();
    return false;
  }
  sscanf((char*)gsm.buffer, "\r\n+CCLK: \"%[^\"\r\n]", string);
  gsm_printf("[GSM] gprs_ntpGetTime() done. %s\r\n", string);
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_mqttConnect(const char *url, uint16_t port, _Bool cleanFlag, const char *clientID, uint16_t keepAliveSec, const char *user, const char *pass, uint16_t timeoutSec){
  if((gsm.gprs.connected == false) || (url == NULL)){
    gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
    return false;
  }
  //  set CID to 1
  sprintf((char*)gsm.buffer, "AT+SMCONF=\"CID\",1\r\n");  
  if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
    gsm_unlock();
    return false;
  }
  //  set keep alive time
  if(keepAliveSec < 60)
    keepAliveSec = 60;
  if(keepAliveSec > 3600)
    keepAliveSec = 3600;
  sprintf((char*)gsm.buffer, "AT+SMCONF=\"KEEPALIVE\",%d\r\n", keepAliveSec);
  if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
    gsm_unlock();
    return false;
  }  
  //  set timeout
  sprintf((char*)gsm.buffer, "AT+SMCONF=\"TIMEOUT\",%d\r\n", timeoutSec);
  if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
    gsm_unlock();
    return false;
  } 
  //  set client id
  if(clientID != NULL){
    sprintf((char*)gsm.buffer, "AT+SMCONF=\"CLIENTID\",\"%s\"\r\n", clientID);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
      gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
      gsm_unlock();
      return false;
    }  
  }
  //  set user name
  if(user != NULL){
    sprintf((char*)gsm.buffer, "AT+SMCONF=\"USERNAME\",\"%s\"\r\n", user);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
      gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
      gsm_unlock();
      return false;
    }  
  }
  //  set password
  if(pass != NULL){
    sprintf((char*)gsm.buffer, "AT+SMCONF=\"PASSWORD\",\"%s\"\r\n", pass);
    if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
      gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
      gsm_unlock();
      return false;
    }  
  }
  //  set URL and port
  sprintf((char*)gsm.buffer, "AT+SMCONF=\"URL\",\"%s:%d\"\r\n", url, port);
  if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
    gsm_unlock();
    return false;
  }
  //  clear flag or not
  sprintf((char*)gsm.buffer, "AT+SMCONF=\"CLEANSS\",%d\r\n", cleanFlag);
  if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
    gsm_unlock();
    return false;
  }
  //  connect to server
  if(gsm_command("AT+SMCONN\r\n", timeoutSec * 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttConnect() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm.gprs.mqttConnected = 1;
  gsm_printf("[GSM] gprs_mqttConnect() done\r\n");
  gsm_unlock();
  return true;   
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_mqttDisConnect(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_mqttDisConnect() failed!\r\n");
    return false;
  }
  if(gsm_command("AT+SMDISC\r\n", 10000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttDisConnect() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm.gprs.mqttConnected = 0;
  gsm_printf("[GSM] gprs_mqttDisConnect() done\r\n");
  gsm_unlock();
  return true;   
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_mqttSubscribe(const char *topic, _Bool qos){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_mqttSubscribe() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_mqttSubscribe() failed!\r\n");
    return false;
  }
  sprintf((char*)gsm.buffer, "AT+SMSUB=\"%s\",%d\r\n", topic, qos);
  if(gsm_command((char*)gsm.buffer, 65000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttSubscribe() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] gprs_mqttSubscribe() done\r\n");
  gsm_unlock();
  return true;   
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_mqttUnSubscribe(const char *topic){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_mqttUnSubscribe() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_mqttUnSubscribe() failed!\r\n");
    return false;
  }
  sprintf((char*)gsm.buffer, "AT+SMUNSUB=\"%s\"\r\n", topic);
  if(gsm_command((char*)gsm.buffer, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttUnSubscribe() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] gprs_mqttUnSubscribe() done\r\n");
  gsm_unlock();
  return true;   
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_gprs_mqttPublish(const char *topic, _Bool qos, _Bool retain, const char *message){
  if(gsm.gprs.connected == false){
    gsm_printf("[GSM] gprs_mqttPublish() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] gprs_mqttPublish() failed!\r\n");
    return false;
  }
  sprintf((char*)gsm.buffer, "AT+SMPUB=\"%s\",%d,%d,\"%s\"\r\n", topic, qos, retain, message);
  if(gsm_command((char*)gsm.buffer , 65000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] gprs_mqttPublish() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] gprs_mqttPublish() done\r\n");
  gsm_unlock();
  return true;   
}

//--------------------------------------------------------------------------------------------------------------------//
#endif

#if (SIM868_MSG == true)
//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_msg_updateStorage(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] msg_updateStorage() failed!\r\n");
    return false;
  }
  char str[64];
  char s[5];
  if(gsm_command("AT+CPMS?\r\n", 1000 , str, sizeof(str), 2, "\r\n+CPMS:", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] msg_updateStorage() failed!\r\n");
    gsm_unlock();
    return false;
  }
  if(sscanf(str, "\r\n+CPMS: \"%[^\"]\",%hd,%hd,", s, &gsm.msg.storageUsed, &gsm.msg.storageTotal) != 3){
    gsm_printf("[GSM] msg_updateStorage() failed!\r\n");
    gsm_unlock();
    return false;
  }
  if(strcmp(s, "SM") == 0)
    gsm.msg.storage = gsm_msg_store_simcard;
  else if(strcmp(s, "ME") == 0)
    gsm.msg.storage = gsm_msg_store_module;
  else if(strcmp(s, "SM_P") == 0)
    gsm.msg.storage = gsm_msg_store_simcard_preferred;
  else if(strcmp(s, "ME_P") == 0)
    gsm.msg.storage = gsm_msg_store_module_preferred;
  else if(strcmp(s, "MT") == 0)
    gsm.msg.storage = gsm_msg_store_simcard_or_module;
  else
    gsm.msg.storage = gsm_msg_store_error;
  gsm_printf("[GSM] msg_updateStorage() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
uint16_t gsm_msg_getStorageUsed(void){
  gsm_msg_updateStorage();
  return gsm.msg.storageUsed;
}

//--------------------------------------------------------------------------------------------------------------------//
uint16_t gsm_msg_getStorageTotal(void){
  gsm_msg_updateStorage();
  return gsm.msg.storageTotal;
}

//--------------------------------------------------------------------------------------------------------------------//
uint16_t gsm_msg_getStorageFree(void){
  gsm_msg_updateStorage();
  return gsm.msg.storageTotal - gsm.msg.storageUsed;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_msg_textMode(_Bool on_off, _Bool integer){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] msg_textMode() failed!\r\n");
    return false;
  }
  if(on_off){
    if(gsm_command("AT+CMGF=1\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") == 1){
      gsm.msg.textMode = 1;
      if(integer == false){
        gsm_command("AT+CSMP=17,167,0,0\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
        gsm_printf("[GSM] msg_textMode() done. text: true, integer: false \r\n");
      }else{
        gsm_command("AT+CSMP=17,167,0,8\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n");
        gsm_printf("[GSM] msg_textMode() done. text: true, integer: true \r\n");
      }
      gsm_unlock();
      return true;
    }
  }else{
    if(gsm_command("AT+CMGF=0\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") == 1){
      gsm.msg.textMode = 0;
      gsm_printf("[GSM] msg_textMode() done. text: false\r\n");
      gsm_unlock();
      return true;
    }
  }
  gsm_printf("[GSM] msg_textMode() failed!\r\n");
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_msg_isTextMode(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] msg_isTextMode() failed!\r\n");
    return false;
  }
  uint8_t ans;
  ans = gsm_command("AT+CMGF?\r\n", 1000, NULL, 0, 3, "\r\n+CMGF: 0\r\n", "\r\n+CMGF: 1\r\n", "\r\nERROR\r\n");
  if(ans == 1){
    gsm.msg.textMode = 0;
    gsm_printf("[GSM] msg_isTextMode() done. false\r\n");
    gsm_unlock();
    return false;
  }else 
      if(ans == 1){
      gsm.msg.textMode = 1;
      gsm_printf("[GSM] msg_isTextMode() done. true\r\n");
      gsm_unlock();
      return true;
    }else{
      gsm_printf("[GSM] msg_isTextMode() failed!\r\n");
      gsm_unlock();
      return false;
    }
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_msg_deleteAll(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] msg_deleteAll() failed!\r\n");
    return false;
  }
  if(gsm.msg.textMode){
    if(gsm_command("AT+CMGDA=\"DEL ALL\"\r\n", 25000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
      gsm_printf("[GSM] msg_deleteAll() failed!\r\n");
      gsm_unlock();
      return false;
    }
    gsm_printf("[GSM] msg_deleteAll() done\r\n");
    gsm_unlock();
    return true;
  }else{
    if(gsm_command("AT+CMGDA=6\r\n", 25000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
      gsm_printf("[GSM] msg_deleteAll() failed!\r\n");
      gsm_unlock();
      return false;
    }
    gsm_printf("[GSM] msg_deleteAll() done\r\n");
    gsm_unlock();
    return true;
  }
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_msg_delete(uint16_t index){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] msg_delete(%d) failed!\r\n", index);
    return false;
  }
  char str[32];
  sprintf(str, "AT+CMGD=%d\r\n", index);
  if(gsm_command(str, 5000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") == 1){
    gsm_printf("[GSM] msg_delete(%d) done\r\n", index);
    gsm_unlock();
    return true;
  }
  gsm_printf("[GSM] msg_delete(%d) failed!\r\n", index);
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_msg_send(const char *number, const char *msg){
  gsm_printf("[GSM] msg_send() begin\r\n");
  if((number == NULL) || (msg == NULL)){
    gsm_printf("[GSM] msg_send() failed!\r\n");
    return false;
  }
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] msg_send() failed!\r\n");
    return false;
  }
  if(gsm.msg.textMode){
    char str[32];
    sprintf(str, "AT+CMGS=\"%s\"\r\n", number);
    if(gsm_command(str, 5000 , NULL, 0, 2, "\r\r\n> ", "\r\nERROR\r\n") != 1){
      sprintf(str, "%c", 27);
      gsm_command(str, 1000, NULL, 0, 0);
      gsm_printf("[GSM] msg_send() failed!\r\n");
      gsm_unlock();
      return false;
    }
    sprintf((char*) gsm.buffer, "%s%c", msg, 26);
    if(gsm_command((char*)gsm.buffer, 80000 , NULL, 0, 2, "\r\n+CMGS:", "\r\nERROR\r\n") != 1){
      gsm_printf("[GSM] msg_send() failed!\r\n");
      gsm_unlock();
      return false;
    }
    gsm_printf("[GSM] msg_send() done\r\n");
    gsm_unlock();
    return true;
  }else{
    gsm_printf("[GSM] msg_send() failed!\r\n");
    gsm_unlock();
    return false;
  }
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_msg_selectStorage(gsm_msg_store_t gsm_msg_store_){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] msg_selectStorage() failed!\r\n");
    return false;
  }
  char str[64];
  switch(gsm_msg_store_){
  case gsm_msg_store_simcard:
    sprintf(str, "AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n");
    break;
  case gsm_msg_store_module:
    sprintf(str, "AT+CPMS=\"ME\",\"ME\",\"ME\"\r\n");
    break;
  case gsm_msg_store_simcard_preferred:
    sprintf(str, "AT+CPMS=\"SM_P\",\"SM_P\",\"SM_P\"\r\n");
    break;
  case gsm_msg_store_module_preferred:
    sprintf(str, "AT+CPMS=\"ME_P\",\"ME_P\",\"ME_P\"\r\n");
    break;
  case gsm_msg_store_simcard_or_module:
    sprintf(str, "AT+CPMS=\"MT\",\"MT\",\"MT\"\r\n");
    break;
  default:
    gsm_printf("[GSM] msg_selectStorage() failed!\r\n");
    gsm_unlock();
    return false;
  }
  if(gsm_command(str, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] msg_selectStorage() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm.msg.storage = gsm_msg_store_;
  gsm_printf("[GSM] msg_selectStorage() done\r\n");
  gsm_unlock();
  gsm_msg_updateStorage();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_msg_selectCharacterSet(gsm_msg_chset_t gsm_msg_chSet_){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] msg_selectCharacterSet() failed!\r\n");
    return false;
  }
  char str[64];
  switch (gsm_msg_chSet_){
  case gsm_msg_chSet_8859_1:
    sprintf(str, "AT+CSCS=\"8859-1\"\r\n");
    break;
  case gsm_msg_chSet_gsm:
    sprintf(str, "AT+CSCS=\"GSM\"\r\n");
    break;
  case gsm_msg_chSet_ira:
    sprintf(str, "AT+CSCS=\"IRA\"\r\n");
    break;
  case gsm_msg_chSet_pccp:
    sprintf(str, "AT+CSCS=\"PCCP\"\r\n");
    break;
  case gsm_msg_chSet_hex:
    sprintf(str, "AT+CSCS=\"HEX\"\r\n");
    break;
  case gsm_msg_chSet_ucs2:
    sprintf(str, "AT+CSCS=\"UCS2\"\r\n");
    break;
  case gsm_msg_chSet_pcdn:
    sprintf(str, "AT+CSCS=\"PCDN\"\r\n");
    break;
  default:
    gsm_printf("[GSM] msg_selectCharacterSet() failed!\r\n");
    gsm_unlock();
    return false;
  }
  if(gsm_command(str, 1000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] msg_selectCharacterSet() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm.msg.characterSet = gsm_msg_chSet_;
  gsm_printf("[GSM] msg_selectCharacterSet() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_msg_read(uint16_t index){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] msg_read(%d) failed!\r\n", index);
    return false;
  }
  //  +++ text mode
  if(gsm.msg.textMode == 1){
    char str[20];
    uint16_t d[6];
    sprintf(str, "AT+CMGR=%d\r\n", index);
    if(gsm_command(str, 5000, (char* )gsm.buffer, sizeof(gsm.buffer), 3, "\r\n+CMGR:", "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
      gsm_printf("[GSM] msg_read(%d) failed!\r\n", index);
      gsm_unlock();
      return false;
    }
    sscanf((char*) gsm.buffer, "\r\n+CMGR: \"%[^\"]\",\"%[^\"]\",\"\",\"%hd/%hd/%hd,%hd:%hd:%hd%*d\"", gsm.msg.status,
        gsm.msg.number, &d[0], &d[1], &d[2], &d[3], &d[4], &d[5]);
    gsm.msg.time.year = d[0];
    gsm.msg.time.month = d[1];
    gsm.msg.time.day = d[2];
    gsm.msg.time.hour = d[3];
    gsm.msg.time.minute = d[4];
    gsm.msg.time.second = d[5];
    uint8_t cnt = 0;
    char *s = strtok((char*) gsm.buffer, "\"");
    while(s != NULL){
      s = strtok(NULL, "\"");
      if(cnt == 6){
        s += 2;
        char *end = strstr(s, "\r\nOK\r\n");
        if(end != NULL){
          strncpy((char*) &gsm.buffer[0], s, end - s);
          memset(&gsm.buffer[end - s], 0, sizeof(gsm.buffer) - (end - s));
          gsm_printf("[GSM] msg_read(%d) done\r\n", index);
          gsm_unlock();
          return true;
        }else{
          gsm_printf("[GSM] msg_read(%d) failed!\r\n", index);
          gsm_unlock();
          return false;
        }
      }
      cnt++;
    }
  }
  //  --- text mode
  //  +++ pdu mode
  else{
    gsm_printf("[GSM] msg_read(%d) failed!\r\n", index);
    gsm_unlock();
    return false;
  }
  //  --- pdu mode
  gsm_printf("[GSM] msg_read(%d) failed!\r\n", index);
  gsm_unlock();
  return false;
}

//--------------------------------------------------------------------------------------------------------------------//
#endif

#if (SIM868_CALL == true)
//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_call_answer(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] call_answer() failed!\r\n");
    return false;
  }
  if(gsm_command("ATA\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] call_answer() failed!\r\n");
    gsm_unlock();
    return false;
  }
  memset(gsm.call.dtmfBuffer, 0, sizeof(gsm.call.dtmfBuffer));
  gsm.call.dtmfCount = 0;
  gsm.call.dtmfUpdate = 0;
  gsm_printf("[GSM] call_answer() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_call_dial(const char *number, uint8_t waitSecond){
  gsm_printf("[GSM] call_dial() begin\r\n");
  char str[32];
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] call_dial() failed!\r\n");
    return false;
  }
  memset(gsm.call.dtmfBuffer, 0, sizeof(gsm.call.dtmfBuffer));
  gsm.call.dtmfCount = 0;
  gsm.call.dtmfUpdate = 0;
  sprintf(str, "ATD%s;\r\n", number);
  uint8_t ans = gsm_command(str, waitSecond * 1000, NULL, 0, 5, "\r\nNO DIALTONE\r\n", "\r\nBUSY\r\n",
      "\r\nNO CARRIER\r\n", "\r\nNO ANSWER\r\n", "\r\nOK\r\n");
  if(ans == 5){
    gsm_printf("[GSM] call_dial() done\r\n");
    gsm_unlock();
    return true;
  }else{
    gsm_printf("[GSM] call_dial() failed!\r\n");
    gsm_unlock();
    gsm_call_end();
    return false;
  }
}

//--------------------------------------------------------------------------------------------------------------------//
_Bool gsm_call_end(void){
  if(gsm_lock(10000) == false){
    gsm_printf("[GSM] call_end() failed!\r\n");
    return false;
  }
  if(gsm_command("ATH\r\n", 20000 , NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") != 1){
    gsm_printf("[GSM] call_end() failed!\r\n");
    gsm_unlock();
    return false;
  }
  gsm_printf("[GSM] call_end() done\r\n");
  gsm_unlock();
  return true;
}

//--------------------------------------------------------------------------------------------------------------------//
#endif
