#include "valve.h"
#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "periph_ctrl.h"
#include "pcnt.h" //Pulse counter
#include "esp_attr.h"
#include "esp_log.h"
#include "board/board.h"
#include "driver/drv883x.h"

#define OPEN 1
#define CLOSE 0

//Keep maxcounts and current valve position in rtc ram, this memory will not be cleared by deep-sleep
static RTC_DATA_ATTR uint8_t _valvePosition;
static RTC_DATA_ATTR uint32_t _maxCounts;

static const pcnt_unit_t pcnt_unit = PCNT_UNIT_0;
static DRV883X motor( MOTOR_ENABLE, MOTOR_PHASE, MOTOR_nSLEEP );
static SemaphoreHandle_t valveLock = NULL;

//StallGuard can obtain motor movement
class StallGuard {

    private:
        pcnt_unit_t _pcnt_unit;
        int16_t _lastCount;

    public:
        //Parameter: pcnt_unit whichs counter should be checked
        StallGuard( pcnt_unit_t unit ) : _pcnt_unit( unit ), _lastCount( 0 ) {}

        //Reset stored counter value to 0
        void reset() { _lastCount = 0; }

        //Get stored last counter value
        int16_t getLastCount() { return _lastCount; }

        //Check for count stalling
        bool isStalled() { 
            int16_t newCount;
            //Get counter value from pulse count peripheral register
            pcnt_get_counter_value ( _pcnt_unit, &newCount );
            //Comapre last count and new count
            if( newCount == _lastCount) {
                return true;
            } else {
                //Save new count as last count
                _lastCount = newCount;
                return false;
            }
        }

};

static void IRAM_ATTR pcnt_valve_intr_handler(void* arg) {

    //Stop motor move
    motor.stop();

    // Disable reflex source
    board_setReflexEn( false );

    //Stop counter
    pcnt_counter_pause ( pcnt_unit );

    // Disable event because we reached the target position
    pcnt_event_disable( pcnt_unit, PCNT_EVT_THRES_1 );

    // Release lock
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    //Clear interrtupt
    PCNT.int_clr.val = PCNT.int_st.val;

    xSemaphoreGiveFromISR( valveLock, &xHigherPriorityTaskWoken );

    if( xHigherPriorityTaskWoken == pdTRUE )
        portYIELD_FROM_ISR( );
}

void valve_init() {

    //Create lock for valve regulation
    valveLock = xSemaphoreCreateBinary();
    
    if( valveLock == NULL ) {
        ESP_LOGE( "VALVE", "Semaphore creation failed" );
    } else {
        //Give semaphore after creation to unlock it
        xSemaphoreGive( valveLock );
    }

    //Configuration for positive incremnt on active low reflex signal
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = REFLEX_SIG, //PCNT_PIN_NOT_USED
        .ctrl_gpio_num = PCNT_PIN_NOT_USED, //0x38 = //Const high level input (see esp32 technical reference manual 4.2.2)
        .lctrl_mode = PCNT_MODE_KEEP,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_INC,
        .counter_h_lim = INT16_MAX,
        .counter_l_lim = INT16_MIN,
        .unit = pcnt_unit,
        .channel = PCNT_CHANNEL_0
    };

    pcnt_unit_config( &pcnt_config );

    //Optional configure and enable input filter
    pcnt_set_filter_value( pcnt_unit, 10 );
    pcnt_filter_enable( pcnt_unit );

    //Register callback event
    pcnt_isr_register( pcnt_valve_intr_handler, NULL, ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM, NULL ); 
    //Enable ISR
    pcnt_intr_enable( pcnt_unit );

    //Print stored vlaues for valve position and maxi counts from RTC Ram
    ESP_LOGD( "VALVE", "RTC RAM max_conuts: %u", _maxCounts );
    ESP_LOGD( "VALVE", "RTC RAM _valvePosition: %u", _valvePosition );
    
}

//Set valve poisition 0-100% (0 = closed, 100 = Open), return true ond success, otherwise false
bool valve_set(uint8_t percent) {

    //if valve was not initalized the maxCounts is not set. No regualtion possible so return false
    if( _maxCounts == 0 )
        return false;

    //Set target postion to 100% if parameter ist >100%
    if( percent > 100)
        percent = 100;

    //Valveposition is already correct
    if( _valvePosition == percent )
        return true;

    //Get lock but wait max. 0.1 seconds
    if( xSemaphoreTake( valveLock, DELAY_MS(100) ) != pdTRUE )
        return false;

    //Calculate realtive counts for target position
    int16_t relativeCounts = (_maxCounts/100) * (_valvePosition - percent);

    //Set direction: opening for negative counts, closing for positive counts
    uint8_t direction = relativeCounts < 0 ? OPEN : CLOSE;

    //set relative count to absoulte value
    relativeCounts = abs( relativeCounts );

    //Create stallguard
    StallGuard stallGuard( pcnt_unit );

    /* Start regulation */

    //Enable reflex source
    board_setReflexEn( true );  

    //Set target value
    pcnt_set_event_value( pcnt_unit, PCNT_EVT_THRES_1, relativeCounts );
    //Enable event
    pcnt_event_enable( pcnt_unit, PCNT_EVT_THRES_1 );
    
    //Clear pulse counter
    pcnt_counter_pause ( pcnt_unit );
    pcnt_counter_clear( pcnt_unit ); 

    //Resume counter
    pcnt_counter_resume ( pcnt_unit );

    //Start motor movement
    motor.start( direction );

    //Wait for target position is reached but check for stalling each 0.5 Seconds
    while( xSemaphoreTake( valveLock, DELAY_MS(500) ) != pdTRUE && stallGuard.isStalled() != true )
    {
        ESP_LOGD( "VALVE", "Regulation counts: %u/%u", stallGuard.getLastCount(), relativeCounts );
    }

    //Get real counter value to calculate position and failure
    int16_t measuredCounts = 0;
    pcnt_get_counter_value( pcnt_unit, &measuredCounts );
    
    //Counter value is not equal to desired one -> regulation faied. Current valve position will be recalculated
    if( measuredCounts != relativeCounts ) {

        ESP_LOGE( "VALVE", "Regulation failed: Conuter %u/%u", measuredCounts, relativeCounts );

        /* Regulation failed -> Abort action and cleanup*/
        //Stop motor
        motor.stop();
        
        //Disable reflex source
        board_setReflexEn( false );
        
        //pause and reset counter
        pcnt_counter_pause ( pcnt_unit );
        pcnt_counter_clear( pcnt_unit );

        //Disable event
        pcnt_event_disable( pcnt_unit, PCNT_EVT_THRES_1 );

        //Calculate relative valve position
        int8_t relPosition = measuredCounts / (_maxCounts/100);
        //Add relPosition on valven opening, Substract relPosition on valve closing
        _valvePosition += direction == OPEN ? relPosition : -relPosition; 

        //release lock
        xSemaphoreGive( valveLock );

        return false;

    } else {

        ESP_LOGD( "VALVE", "Regulation done" );

        // Save actual postion
        _valvePosition = percent;

        //release lock
        xSemaphoreGive( valveLock );

        return true;
        
    }
}

//Init valve by extract and retract the valve till max
bool valve_calibration() {

    //Skip calibration if maxCount is already set
    if( _maxCounts != 0)
        return true;

    //Create stallguard
    StallGuard stallGuard( pcnt_unit );

    //Enable reflex light source
    board_setReflexEn( true );

    //clear pcnt counter
    pcnt_counter_pause ( pcnt_unit );
    pcnt_counter_clear( pcnt_unit );
    pcnt_counter_resume ( pcnt_unit );

    //Drive motor to full open
    motor.start( OPEN );
    //Wait motor spin
    vTaskDelay( DELAY_MS(500) );

    //Move until no pulses where detected -> We reached 100%
    do {
        vTaskDelay( DELAY_MS(25) );
    } while( !stallGuard.isStalled() );

    motor.stop();

    //Wait 1 second for motor and all gears stopped
    vTaskDelay( DELAY_MS(1000) );

    //Now that we reached 100%, we must find 0% position

    //clear pcnt counter, so we start at 100%
    pcnt_counter_pause ( pcnt_unit );
    pcnt_counter_clear( pcnt_unit );
    pcnt_counter_resume ( pcnt_unit );

    //Reset stallguard for new movement
    stallGuard.reset();

    //Drive motor to full closed
    motor.start( CLOSE );
    //Wait motor spin
    vTaskDelay( DELAY_MS(500) );

    //Move until no pulses where detected
    do {
        vTaskDelay( DELAY_MS(25) );
    } while( !stallGuard.isStalled() );

    motor.stop();

    board_setReflexEn( false );

    //If stallGuard counter is still 0, something went wrong (e.g. no motor movement, sensor failed)
    if( stallGuard.getLastCount() == 0 )
        return false;

    //Store counts for a full valve move
    _maxCounts = stallGuard.getLastCount();
    //Set actual valve position to fully closed
    _valvePosition = 0;

    ESP_LOGD( "VALVE", "Max CNT: %d", _maxCounts );

    return true;
}

//Get the actual valve position from 0 to 100%
uint8_t valve_get() {
    return _valvePosition;
}

//Get max counts
uint32_t valve_getMaxCounts() {
    return _maxCounts;
}