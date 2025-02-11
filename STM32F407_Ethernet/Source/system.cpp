#include "system.h"
#include "main.h"
#include "rng.h"
#include "eth.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "dp83848.h"

/******************************** Config ************************************ */

#define TASK1_PERIOD_MS       (1000)
#define ETH_BUFF_SIZE         (4096)
/******************************** Tasks **************************************/

int32_t dp83848_Init (void);
int32_t dp83848_DeInit (void);
int32_t dp83848_ReadReg   (uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);
int32_t dp83848_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal);
int32_t dp83848_GetTick( void );

dp83848_Object_t DP83848;
dp83848_IOCtx_t DP83848_IOCtx = {
    dp83848_Init,
    dp83848_DeInit,
    dp83848_WriteReg,
    dp83848_ReadReg,
    dp83848_GetTick
};

int32_t dp83848_Init (void){
    HAL_ETH_SetMDIOClockRange(&heth);
    return 0;
}

int32_t dp83848_DeInit (void){
    return 0;
}

int32_t dp83848_ReadReg   (uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal){
    if(HAL_ETH_ReadPHYRegister(&heth, DevAddr, RegAddr, pRegVal) != HAL_OK){
        return -1;
    }
    return 0;
}

int32_t dp83848_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal){
    if(HAL_ETH_WritePHYRegister(&heth, DevAddr, RegAddr, RegVal) != HAL_OK){
        return -1;
    }
    return 0;
}

int32_t dp83848_GetTick( void ){
    return HAL_GetTick();
}

typedef struct {
    ETH_BufferTypeDef *AppBuff;
    uint8_t buffer[100]__ALIGNED(32);
} ETH_AppBuff;

void HAL_ETH_RxAllocateCallback(uint8_t** buff) {
    ETH_BufferTypeDef* p = (ETH_BufferTypeDef*)pvPortMalloc(100);
    if (p){
        *buff = (uint8_t*) p + offsetof(ETH_AppBuff, buffer);
        p->next = NULL;
        p->len = 100;
    } else {
        *buff = NULL;
    }
}

void HAL_ETH_RxLinkCallback(void** pStart, void** pEnd, uint8_t * buff, uint16_t Length){
  ETH_BufferTypeDef** ppStart = (ETH_BufferTypeDef**) pStart;
  ETH_BufferTypeDef** ppEnd = (ETH_BufferTypeDef**) pEnd;
  ETH_BufferTypeDef* p = NULL;
  p = (ETH_BufferTypeDef*)(buff - offsetof(ETH_AppBuff, buffer));
  p->next = NULL;
  p->len = 100;
  if (! * ppStart){
    *ppStart = p;
  }else{
    (*ppEnd)->next = p;
  }
  *ppEnd = p;
}


void System::TasksFunctions::ETH_State( void* args ){
    ETH_MACConfigTypeDef MACConf = {0};
    MX_ETH_Init();
    HAL_ETH_Start(&heth);
    DP83848_RegisterBusIO(&DP83848, &DP83848_IOCtx);
    DP83848_Init(&DP83848);
    xTaskCreate(System::TasksFunctions::ETH_Sender, 
                "ETH state",
                configMINIMAL_STACK_SIZE,
                NULL,
                1,
                &System::TasksHandlers::ETH_Sender
    );
    while(1) {
        static bool first_time = true;
        int32_t link_state = DP83848_GetLinkState(&DP83848);
        if(link_state == DP83848_STATUS_LINK_DOWN){
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
            //first_time = true;
        }else{
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
            if(first_time){
                first_time = false;
                HAL_ETH_GetMACConfig(&heth, &MACConf);
                MACConf.DuplexMode = ETH_FULLDUPLEX_MODE;
                MACConf.Speed = ETH_SPEED_100M;
                MACConf.DropTCPIPChecksumErrorPacket = DISABLE;
                MACConf.ForwardRxUndersizedGoodPacket = ENABLE;
                HAL_ETH_SetMACConfig(&heth, &MACConf);
                HAL_ETH_Start(&heth);  
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

uint8_t dest_mac[] = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x10}; // Destination MAC Address
uint8_t src_mac[] = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x00};  // Source MAC Address
uint8_t type[] = {0x08,0x00 }; // EtherType set to IPV4 packet

void System::TasksFunctions::ETH_Sender( void* args ){
    char* message = (char*)pvPortMalloc(100);//"Fuck The World!";
    while(1) {
        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
        vTaskDelay(pdMS_TO_TICKS(100));
        ETH_BufferTypeDef TxBuffer;
        strcpy(message, "Fuck The World!");
        size_t payload_len = strlen(message);
        TxBuffer.buffer = (uint8_t*)message;
        TxBuffer.len = payload_len + sizeof(dest_mac) + sizeof(src_mac) + sizeof(type) + payload_len;
        TxBuffer.next = NULL;
        ETH_TxPacketConfig TxConfig;
        memset(&TxConfig, 0, sizeof(TxConfig));
        TxConfig.TxBuffer = &TxBuffer;
        HAL_ETH_Transmit(&heth, &TxConfig, 1000);
        HAL_ETH_ReleaseTxPacket(&heth);
    }
}

/******************************** Init ***************************************/

void SystemRun( void ){
    MX_RNG_Init();
    srand(HAL_RNG_GetRandomNumber(&hrng));
    xTaskCreate(System::TasksFunctions::ETH_State, 
                "ETH state",
                configMINIMAL_STACK_SIZE,
                NULL,
                1,
                &System::TasksHandlers::ETH_State
    );
    vTaskStartScheduler();
}

/*
uint32_t HAL_GetTick(void){
  return xTaskGetTickCount();
}*/