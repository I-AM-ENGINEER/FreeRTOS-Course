#ifndef SYSTEM_H__
#define SYSTEM_H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "queue.h"
#include "usart.h"


/******************************** Config ************************************ */

#define GAME_MAX_LEVEL                    (3)
#define GAME_DELAY_BETWEEN_FLASHES_MS     (500)    
#define GAME_LIGHT_TIME_DIFFICULTY1_MS    (1000)
#define GAME_LIGHT_TIME_DIFFICULTY2_MS    (500)
#define GAME_MAX_ENTER_TIME_MS            (3000)
#define GAME_PRELAUNCH_DELAY_MS           (1000)
#define GAME_CORRECT_INPUT_LEDON_TIME_MS  (300)
#define GAME_DELAY_BETWEEN_ROUNDS         (1000)
#define GAME_WIN_FLASH_TIME_MS            (1000)
#define GAME_BUTTONS_COUNT                (3)

#define TASK1_PERIOD_MS                   (1000)
#define TASK2_PERIOD_MIN_MS               (1000)
#define TASK2_PERIOD_MAX_MS               (3000)
#define KEYBOARD_QUEUE_LENGTH             (16) 
#define KEYBOARD_POOL_PERIOD_MS           (10)
#define GAME_MENU_START_TEXT              ("Select dificulty (1 or 2):\r\n")
#define GAME_GAMEOVER_INCORRECT_TEXT      ("Incorrect input, game over\r\n")
#define GAME_GAMEOVER_TIMEOUT_TEXT        ("Input timeout, game over\r\n")
#define GAME_WIN_TEXT                     ("Game won\r\n")


typedef enum{
    KEYBOARD_EVENT_PRESS
} keyboard_event_t;

typedef struct{
    keyboard_event_t event_type;
    uint8_t button_num;
} keyboard_message_t;


#ifdef __cplusplus

class UART_STM32{
public:
    UART_STM32( UART_HandleTypeDef* huart, size_t receive_queue_length ) :
          _huart(huart)
    {
        _send_mutex = xSemaphoreCreateMutex();
        _receive_mutex = xSemaphoreCreateMutex();
        _receive_buffer = xQueueCreate(receive_queue_length, sizeof(uint8_t));
    }
    
    void Init( void ){
    }

    size_t Send( const uint8_t* buff, size_t size, uint32_t timeout ){
        size_t cnt = 0;
        HAL_StatusTypeDef res;
        TickType_t timestamp1 = xTaskGetTickCount();
        xSemaphoreTake(_send_mutex, pdMS_TO_TICKS(timeout));
        uint32_t semaphore_take_time = pdTICKS_TO_MS(xTaskGetTickCount() - timestamp1);
        if(semaphore_take_time > timeout){
            goto timeout_event;
        }
        timeout -= semaphore_take_time;
        res = HAL_UART_Transmit(_huart, buff, static_cast<uint16_t>(size), timeout);
        if(res != HAL_OK){
            goto timeout_event;
        }
        cnt = size;
timeout_event:
        xSemaphoreGive(_send_mutex);
        return cnt;
    }

    size_t Recv( char* buff, size_t size, uint32_t timeout ) {
        size_t cnt = 0;
        TickType_t timestamp1 = xTaskGetTickCount();
        uint32_t byte_max_receive_time;
        uint32_t current_receive_time;
        xSemaphoreTake(_receive_mutex, pdMS_TO_TICKS(timeout));
        current_receive_time = pdTICKS_TO_MS(xTaskGetTickCount() - timestamp1);
        if(current_receive_time > timeout){
            goto timeout_event;
        }

        byte_max_receive_time = timeout - current_receive_time;
        for(size_t i = 0; i < size; i++){
            uint8_t rcv_chr;
            xQueueReceive(_receive_buffer, &rcv_chr, byte_max_receive_time);
            current_receive_time = pdTICKS_TO_MS(xTaskGetTickCount() - timestamp1);
            if(current_receive_time > timeout){
                goto timeout_event;
            }
            byte_max_receive_time = timeout - current_receive_time;
            buff[cnt++] = rcv_chr;
        }
timeout_event:
        xSemaphoreGive(_receive_mutex);
        return cnt;
    }
    
    void ISR_ReceiveByte( uint8_t byte ){
        BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
        xQueueSendToBackFromISR(_receive_buffer, &byte, &pxHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    }
private:
    SemaphoreHandle_t _receive_mutex;
    SemaphoreHandle_t _send_mutex;
    QueueHandle_t _receive_buffer;
    UART_HandleTypeDef* _huart;
};


class Device{
public:    
    struct TasksFunctions{
        static void UART_Sender1( void* args );
        static void UART_Sender2( void* args );
    };
    static TimerHandle_t keyboard_checker_timer;
    static uint8_t uart_rx_byte_buffer;
    static UART_STM32 uart1;
    static QueueHandle_t keyboard_events;

    struct HW{
        static void SetLeds( uint8_t mask );
        static uint8_t ReadButtons( void );
    };
};

class Game{
public:
    static void MainTask( void* args );
    static void Start( void );
    static void SetDifficulty( uint8_t newDifficulty );
    static uint32_t GetLightOnTimeMs( void );
    static void DisplayPatterns( uint16_t current_level );
    static bool CheckInput( uint16_t current_level );
private:
    static uint8_t _difficulty;
    static uint8_t _patterns[GAME_MAX_LEVEL];
};

extern "C" {
#endif

void SystemRun( void );

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H__
