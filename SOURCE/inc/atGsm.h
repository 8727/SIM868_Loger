#ifndef _ATGSM_H
#define _ATGSM_H

//--------------------------------------------------------------------------------------------------------------------//
#include "stm32l4xx.h"
#include "sysInit.h"

//--------------------------------------------------------------------------------------------------------------------//
#define SIM868_USART                      USART2
#define SIM868_KEY_LOW                    GPIOA->BSRR = GPIO_BSRR_BR8
#define SIM868_KEY_HIGHT                  GPIOA->BSRR = GPIO_BSRR_BS8

//--------------------------------------------------------------------------------------------------------------------//
#if (SIM868_DEBUG == true)
  #define gsm_printf(...)     printf(__VA_ARGS__)
#else
  #define gsm_printf(...)     {};
#endif

#if ATC_CMSIS_OS == false
  #define atc_delay(x)  HAL_Delay(x)
#else
//  #include "cmsis_os.h"
  #define atc_delay(x)  osDelay(x)
#endif

#define         gsm_delay(x)            atc_delay(x)
#define         gsm_command(...)        atc_command(&gsm.atc, __VA_ARGS__)
#define         gsm_transmit(data,len)  atc_transmit(&gsm.atc,data,len)
#define         gsm_rxCallback()        atc_rxCallback(&gsm.atc)

typedef struct{
  USART_TypeDef*  usart;
  char            name[8];
  uint32_t        rxTime;
  uint8_t*        rxBuffer;
  uint8_t         rxTmp;
  uint16_t        rxIndex;
  uint16_t        rxSize;  
  uint16_t        rxTimeout; 
  uint8_t         txBusy; 
  int8_t          foundAnswer;  
  uint16_t        foundAnswerSize; 
  char*           foundAnswerString;
  char*           answerSearchingString[ATC_MAX_ANSWER_SEARCHING_STRING];
  char*           alwaysSearchingString[ATC_MAX_ALWAYS_SEARCHING_STRING]; 
  #if (ATC_TXDMA == true)
    DMA_TypeDef*    txDMA;
    uint32_t        txDMA_Stream;
    uint8_t         txDMAdone;
  #endif  
}ATC_t;

typedef enum{
  ATC_Element_Error = -1,
  ATC_Element_0,
  ATC_Element_1,
  ATC_Element_2,
  ATC_Element_3,
  ATC_Element_4,
}ATC_Element_t;

//--------------------------------------------------------------------------------------------------------------------//
typedef enum{
  gsm_tone_dialTone = 1,
  gsm_tone_calledSubscriberBusy = 2,
  gsm_tone_congestion = 3,
  gsm_tone_radioPathAcknowledge = 4,
  gsm_tone_rdioPathNotAvailableOrCallDropped = 5,
  gsm_tone_errorOrSpecialinformation = 6,
  gsm_tone_callWaitingTone = 7,
  gsm_tone_ringingTone = 8,
  gsm_tone_generalBeep = 16,
  gsm_tone_positiveAcknowledgementTone = 17,
  gsm_tone_negativeAcknowledgementOrErrorTone = 18,
  gsm_tone_indianDialTone = 19,
  gsm_tone_americanDialTone = 20
}gsm_tone_t;

typedef enum{
  gsm_msg_chSet_error = 0,
  gsm_msg_chSet_gsm,
  gsm_msg_chSet_ucs2,
  gsm_msg_chSet_ira,
  gsm_msg_chSet_hex,
  gsm_msg_chSet_pccp,
  gsm_msg_chSet_pcdn,
  gsm_msg_chSet_8859_1
}gsm_msg_chset_t;

typedef enum{
  gsm_msg_store_error = 0,
  gsm_msg_store_simcard,
  gsm_msg_store_module,
  gsm_msg_store_simcard_preferred,
  gsm_msg_store_module_preferred,
  gsm_msg_store_simcard_or_module,
}gsm_msg_store_t;

typedef enum{
  gsm_ftp_error_none = 0,
  gsm_ftp_error_error = 1,
  gsm_ftp_error_netError = 61,
  gsm_ftp_error_dnsError = 62,
  gsm_ftp_error_connectError = 63,
  gsm_ftp_error_timeout = 64,
  gsm_ftp_error_serverError = 65,
  gsm_ftp_error_operationNotAllow = 66,
  gsm_ftp_error_replayError = 70,
  gsm_ftp_error_userError = 71,
  gsm_ftp_error_passwordError = 72,
  gsm_ftp_error_typeError = 73,
  gsm_ftp_error_restError = 74,
  gsm_ftp_error_passiveError = 75,
  gsm_ftp_error_activeError = 76,
  gsm_ftp_error_operateError = 77,
  gsm_ftp_error_uploadError = 78,
  gsm_ftp_error_downloadError = 79,
  gsm_ftp_error_manualQuit = 86,
  gsm_ftp_error_notExist = 100,
}gsm_ftp_error_t;

typedef struct{
  uint8_t           year;
  uint8_t           month;
  uint8_t           day;
  uint8_t           hour;
  uint8_t           minute;
  uint8_t           second;
}gsm_time_t;

#if (SIM868_CALL == true)
  typedef struct{
    uint8_t           newCall;
    uint8_t           endCall;
    char              number[16];
    uint8_t           dtmfCount;
    uint8_t           dtmfUpdate;
    char              dtmfBuffer[16];
  }gsm_call_t;
#endif

#if (SIM868_MSG == true)
  typedef struct{
    uint8_t           textMode;
    gsm_msg_chset_t   characterSet;
    gsm_msg_store_t   storage;
    uint16_t          storageTotal;
    uint16_t          storageUsed;
    gsm_time_t        time;
    int16_t           newMsg;
    char              status[16];
    char              number[16];
  }gsm_msg_t;
#endif

#if (SIM868_GPRS == true)
  typedef struct{
    _Bool              connected;
    _Bool              connectedLast;
    char              ip[16];
    uint32_t          dataLen;
    uint32_t          dataCurrent;
    int16_t           code;
    uint8_t           tcpConnection;
    uint8_t           gotData;
    uint32_t          ftpExtOffset;
    char              mqttTopic[64];
    char              mqttMessage[64];
    uint8_t           mqttData;
    uint8_t           mqttConnected;
    uint8_t           mqttConnectedLast;
  }gsm_gprs_t;
#endif

typedef struct{
  uint8_t           power:1;
  uint8_t           registerd:1;
  uint8_t           netReg:1;
  uint8_t           netChange:1;
  uint8_t           simcardChecked:1;
  uint8_t           turnOff:1;
  uint8_t           turnOn:1;
}gsm_status_t;

typedef struct{
  uint8_t           inited;
  uint8_t           lock;
  uint8_t           error;  
  uint8_t           signal;
  gsm_status_t      status;
  atc_t             atc;
  uint8_t           buffer[_ATC_RXSIZE - 16];

  #if (SIM868_CALL == true)
    gsm_call_t        call;
  #endif
  #if (SIM868_MSG == true)
    gsm_msg_t         msg;
  #endif
  #if (SIM868_GPRS == true)
    gsm_gprs_t       gprs;
  #endif

}gsm_t;

extern  gsm_t   gsm;
//--------------------------------------------------------------------------------------------------------------------//
void             atc_process(void);  // put in main loop. 
void             atc_callback(ATC_Element_t ATC_Element);  //  put in usart callback
void             atc_callback_txDMA(ATC_Element_t ATC_Element);  //  put in usart txDMA callback
  #if (ATC_TXDMA == true)
    _Bool        atc_init(ATC_Element_t ATC_Element, USART_TypeDef *usart, uint16_t rxSize, uint16_t rxTimeout, char* name, DMA_TypeDef *DMA, uint32_t LL_DMA_STREAM_);
  #else
    _Bool        atc_init(ATC_Element_t ATC_Element, USART_TypeDef *usart, uint16_t rxSize, uint16_t rxTimeout, char* name);
  #endif
uint8_t          atc_sendAtCommand(ATC_Element_t ATC_Element, char *atCommand, uint32_t wait_ms, char *answerString, uint16_t answerSize, uint8_t searchingItems,...);
_Bool            atc_addAutoSearchString(ATC_Element_t ATC_Element, char *str);
uint8_t          atc_sendString(ATC_Element_t ATC_Element, char *str, uint32_t timeout);
uint8_t          atc_sendData(ATC_Element_t ATC_Element, uint8_t *data, uint16_t size, uint32_t timeout);
ATC_Element_t    atc_getElementByName(char *name);
uint16_t         atc_getSize(ATC_Element_t ATC_Element);
void             atc_userFoundString(ATC_Element_t ATC_Element, const char* foundStr, char* startPtr); 
//--------------------------------------------------------------------------------------------------------------------//
_Bool            gsm_init(void);
void             gsm_loop(void);
_Bool            gsm_power(_Bool on_off);
_Bool            gsm_lock(uint32_t timeout_ms);
void             gsm_unlock(void);
_Bool            gsm_setDefault(void);
_Bool            gsm_registered(void);
_Bool            gsm_setDefault(void);
_Bool            gsm_saveProfile(void);
_Bool            gsm_enterPinPuk(const char* string);
_Bool            gsm_getIMEI(char* string, uint8_t sizeOfString);
_Bool            gsm_getVersion(char* string, uint8_t sizeOfString);
_Bool            gsm_getModel(char* string, uint8_t sizeOfString);
_Bool            gsm_getServiceProviderName(char* string, uint8_t sizeOfString);
uint8_t          gsm_getSignalQuality_0_to_100(void);
_Bool            gsm_waitForRegister(uint8_t waitSecond);
_Bool            gsm_tonePlay(gsm_tone_t gsm_tone_, uint32_t durationMiliSecond, uint8_t level_0_100);
_Bool            gsm_toneStop(void);
_Bool            gsm_dtmf(char *string, uint32_t durationMiliSecond);
_Bool            gsm_ussd(char *command, char *answer, uint16_t sizeOfAnswer, uint8_t waitSecond);
//--------------------------------------------------------------------------------------------------------------------//
_Bool            gsm_call_answer(void);
_Bool            gsm_call_dial(const char *number, uint8_t waitSecond);
_Bool            gsm_call_end(void);
//--------------------------------------------------------------------------------------------------------------------//
_Bool            gsm_msg_textMode(_Bool on_off, _Bool integer);
_Bool            gsm_msg_isTextMode(void);
_Bool            gsm_msg_selectStorage(gsm_msg_store_t gsm_msg_store_);
_Bool            gsm_msg_selectCharacterSet(gsm_msg_chset_t gsm_msg_chset_);
_Bool            gsm_msg_deleteAll(void);
_Bool            gsm_msg_delete(uint16_t index);
_Bool            gsm_msg_send(const char *number,const char *msg);
_Bool            gsm_msg_read(uint16_t index);
_Bool            gsm_msg_updateStorage(void);
uint16_t         gsm_msg_getStorageUsed(void);
uint16_t         gsm_msg_getStorageTotal(void);
uint16_t         gsm_msg_getStorageFree(void);
//--------------------------------------------------------------------------------------------------------------------//
_Bool            gsm_gprs_setApName(const char *apName);
_Bool            gsm_gprs_connect(void);
_Bool            gsm_gprs_disconnect(void);
_Bool            gsm_gprs_httpInit(void);
_Bool            gsm_gprs_httpSetContent(const char *content);
_Bool            gsm_gprs_httpSetUserData(const char *data);
_Bool            gsm_gprs_httpSendData(const char *data, uint16_t timeout_ms);
int16_t          gsm_gprs_httpGet(const char *url, _Bool ssl, uint16_t timeout_ms);
int16_t          gsm_gprs_httpPost(const char *url, _Bool ssl, uint16_t timeout_ms);
uint32_t         gsm_gprs_httpDataLen(void);
uint16_t         gsm_gprs_httpRead(uint8_t *data, uint16_t len);
_Bool            gsm_gprs_httpTerminate(void);
gsm_ftp_error_t  gsm_gprs_ftpLogin(const char *ftpAddress, const char *ftpUserName, const char *ftpPassword, uint16_t port);
gsm_ftp_error_t  gsm_gprs_ftpUploadBegin(_Bool asciiFile, _Bool append, const char *path, const char *fileName, const uint8_t *data, uint16_t len);
gsm_ftp_error_t  gsm_gprs_ftpUpload(const uint8_t *data, uint16_t len);
gsm_ftp_error_t  gsm_gprs_ftpUploadEnd(void);
gsm_ftp_error_t  gsm_gprs_ftpExtUploadBegin(_Bool asciiFile, _Bool append, const char *path, const char *fileName);
gsm_ftp_error_t  gsm_gprs_ftpExtUpload(uint8_t *data, uint16_t len);
gsm_ftp_error_t  gsm_gprs_ftpExtUploadEnd(void);
gsm_ftp_error_t  gsm_gprs_ftpCreateDir(const char *path);
gsm_ftp_error_t  gsm_gprs_ftpRemoveDir(const char *path);
uint32_t         gsm_gprs_ftpGetSize(const char *path, const char *name);
gsm_ftp_error_t  gsm_gprs_ftpRemove(const char *path, const char *name);
gsm_ftp_error_t  gsm_gprs_ftpIsExistFolder(const char *path);
_Bool            gsm_gprs_ftpIsBusy(void);
gsm_ftp_error_t  gsm_gprs_ftpQuit(void);
_Bool            gsm_gprs_ntpServer(char *server, int8_t time_zone_in_quarter);
_Bool            gsm_gprs_ntpSyncTime(void);
_Bool            gsm_gprs_ntpGetTime(char *string);
_Bool            gsm_gprs_mqttConnect(const char *url, uint16_t port, _Bool cleanFlag, const char *clientID, uint16_t keepAliveSec, const char *user, const char *pass, uint16_t timeoutSec);
_Bool            gsm_gprs_mqttDisConnect(void);
_Bool            gsm_gprs_mqttSubscribe(const char *topic, _Bool qos);
_Bool            gsm_gprs_mqttUnSubscribe(const char *topic);
_Bool            gsm_gprs_mqttPublish(const char *topic, _Bool qos, _Bool retain, const char *message);
//--------------------------------------------------------------------------------------------------------------------//
void             gsm_callback_simcardReady(void);
void             gsm_callback_simcardPinRequest(void);
void             gsm_callback_simcardPukRequest(void);
void             gsm_callback_simcardNotInserted(void);
void             gsm_callback_networkRegister(void);
void             gsm_callback_networkUnregister(void);
void             gsm_callback_newCall(const char *number);
void             gsm_callback_endCall(void);
void             gsm_callback_dtmf(char *string, uint8_t len);
void             gsm_callback_newMsg(char *number, gsm_time_t time, char *msg);
void             gsm_callback_gprsConnected(void);
void             gsm_callback_gprsDisconnected(void);
void             gsm_callback_mqttMessage(char *topic, char *message);
void             gsm_callback_mqttDisconnect(void);
//--------------------------------------------------------------------------------------------------------------------//

#endif /* _ATGSM_H */
