/********************************** (C) COPYRIGHT *******************************
 * File Name          : peripheral.C
 * Author             : zhangxiyi @WCH
 * Version            : v0.1
 * Date               : 2020/11/26
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "devinfoservice.h"
#include "peripheral.h"
#include "ble_uart_service.h"
#include "app_drv_fifo.h"
#include "app_uart.h"
#include "pch.h"
/*********************************************************************
 * MACROS
 */
extern volatile uint8_t app_sleep_lock;
/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD              1600

// How often to perform read rssi event
#define SBP_READ_RSSI_EVT_PERIOD             3200

// Parameter update delay
#define SBP_PARAM_UPDATE_DELAY               6400

// What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
#define DEFAULT_ADVERTISING_INTERVAL         6400

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE            GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 10=12.5ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    8

// Maximum connection interval (units of 1.25ms, 100=125ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    20

// Slave latency to use parameter update
#define DEFAULT_DESIRED_SLAVE_LATENCY        0

// Supervision timeout value (units of 10ms, 100=1s)
#define DEFAULT_DESIRED_CONN_TIMEOUT         100

// Company Identifier: WCH
#define WCH_COMPANY_ID                       0x07D7

#define SBP_MTU_UPDATE_EVT      0x0020
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
 
static void Peripheral_RequestMTU(uint16 connHandle);
uint8_t Peripheral_TaskID = INVALID_TASK_ID; // Task ID for internal task/event processing

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

//for send to ble
typedef enum
{
    SEND_TO_BLE_TO_SEND = 1,
    SEND_TO_BLE_ALLOC_FAILED,
    SEND_TO_BLE_SEND_FAILED,
} send_to_ble_state_t;
send_to_ble_state_t send_to_ble_state = SEND_TO_BLE_TO_SEND;

blePaControlConfig_t pa_lna_ctl;

connected_status status_led = NO_CONNECTED;
//static uint8 Peripheral_TaskID = INVALID_TASK_ID;   // Task ID for internal task/event processing

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8_t scanRspData[] = {
    // complete name
    10, // §ª§³§±§²§¡§£§­§¦§¯§°: §¥§Ý§Ú§ß§Ñ §Ò§Ý§à§Ü§Ñ (1 §Ò§Ñ§Û§ä §ä§Ú§á§Ñ + 9 §Ò§Ñ§Û§ä §Ò§å§Ü§Ó "VEGA SI11")
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'V', 'E', 'G', 'A', ' ', 'S', 'I', '1', '1',
    
    // connection interval range
    0x05, // §¥§Ý§Ú§ß§Ñ §Ò§Ý§à§Ü§Ñ (1 §Ò§Ñ§Û§ä §ä§Ú§á§Ñ + 4 §Ò§Ñ§Û§ä§Ñ §Ù§ß§Ñ§é§Ö§ß§Ú§Û)
    GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
    LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
    HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
    LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
    HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

    // TX power level
    0x02, // §¥§Ý§Ú§ß§Ñ §Ò§Ý§à§Ü§Ñ (1 §Ò§Ñ§Û§ä §ä§Ú§á§Ñ + 1 §Ò§Ñ§Û§ä §Ù§ß§Ñ§é§Ö§ß§Ú§ñ)
    GAP_ADTYPE_POWER_LEVEL,
    0 // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] = {
    // Flags; this sets the device to use limited discoverable
    // mode (advertises for 30 seconds at a time) instead of general
    // discoverable mode (advertises indefinitely)
    0x02, // length of this data
    GAP_ADTYPE_FLAGS,
    DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // service UUID, to notify central devices what services are included
    // in this peripheral
    0x03,                  // length of this data
    GAP_ADTYPE_16BIT_MORE, // some of the UUID's, but not all
    LO_UINT16(SIMPLEPROFILE_SERV_UUID),
    HI_UINT16(SIMPLEPROFILE_SERV_UUID)};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "VEGA SI11";

// Connection item list
static peripheralConnItem_t peripheralConnList;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);

static void peripheralParamUpdateCB(uint16 connHandle, uint16 connInterval,
                                    uint16 connSlaveLatency, uint16 connTimeout);
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList);
static void peripheralRssiCB(uint16 connHandle, int8 rssi);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t Peripheral_PeripheralCBs = {
    peripheralStateNotificationCB, // Profile State Change Callbacks
    peripheralRssiCB,              // When a valid RSSI is read from controller (not used by application)
    peripheralParamUpdateCB};

// Broadcast Callbacks
static gapRolesBroadcasterCBs_t Broadcaster_BroadcasterCBs = {
    NULL, // Not used in peripheral role
    NULL  // Receive scan request callback
};

// GAP Bond Manager Callbacks
static gapBondCBs_t Peripheral_BondMgrCBs = {
    NULL, // Passcode callback (not used by application)
    NULL  // Pairing / Bonding state Callback (not used by application)
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Peripheral_Init
 *
 * @brief   Initialization function for the Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */



void Peripheral_Init()
{
    Peripheral_TaskID = TMOS_ProcessEventRegister(Peripheral_ProcessEvent);

    // Setup the GAP Peripheral Role Profile
    {
        uint8  initial_advertising_enable = TRUE;
        uint16 desired_min_interval = 6;
        uint16 desired_max_interval = 1000;
        GATT_InitClient();
        // Set the GAP Role Parameters
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
        GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16), &desired_min_interval);
        GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16), &desired_max_interval);
    }

    // Set advertising interval
    {
        uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

        GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, advInt);
    }

    // Setup the GAP Bond Manager
    {
        uint32 passkey = 123456; 
        uint8  pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
        //uint8  pairMode = GAPBOND_PAIRING_MODE_INITIATE;
        uint8  mitm = TRUE;
        uint8  bonding = TRUE;
        uint8  ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;

        GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32), &passkey);
        GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8), &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8), &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8), &bonding);
    }

    // Initialize GATT attributes
    GGS_AddService(GATT_ALL_SERVICES);         // GAP
    GATTServApp_AddService(GATT_ALL_SERVICES); // GATT attributes
    DevInfo_AddService();                      // Device Information Service
    ble_uart_add_service(on_bleuartServiceEvt);

    // Set the GAP Characteristics
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, sizeof(attDeviceName), attDeviceName);

    // Init Connection Item
    peripheralInitConnItem(&peripheralConnList);

    // Register receive scan request callback
    GAPRole_BroadcasterSetCB(&Broadcaster_BroadcasterCBs);

    // Setup a delayed profile startup
    tmos_set_event(Peripheral_TaskID, SBP_START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      peripheralInitConnItem
 *
 * @brief   Init Connection Item
 *
 * @param   peripheralConnList -
 *
 * @return  NULL
 */
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList)
{
    peripheralConnList->connHandle = GAP_CONNHANDLE_INIT;
    peripheralConnList->connInterval = 0;
    peripheralConnList->connSlaveLatency = 0;
    peripheralConnList->connTimeout = 0;
}

uint32_t get_fattime(void)
{
    return 0;
}

/*********************************************************************
 * @fn      Peripheral_ProcessEvent
 *
 * @brief   Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 Peripheral_ProcessEvent(uint8 task_id, uint16 events)
{
    static attHandleValueNoti_t noti;
    //  VOID task_id; // TMOS required parameter that isn't used in this function

    if(events & SYS_EVENT_MSG)
    {
        uint8 *pMsg;

        if((pMsg = tmos_msg_receive(Peripheral_TaskID)) != NULL)
        {
            Peripheral_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

  if(events & SBP_MTU_UPDATE_EVT)
  {
    Peripheral_RequestMTU(peripheralConnList.connHandle);
    return (events ^ SBP_MTU_UPDATE_EVT);
  }

    if(events & SBP_START_DEVICE_EVT)
    {
        // Start the Device
        GAPRole_PeripheralStartDevice(Peripheral_TaskID, &Peripheral_BondMgrCBs, &Peripheral_PeripheralCBs);
        return (events ^ SBP_START_DEVICE_EVT);
    }
    if(events & SBP_PARAM_UPDATE_EVT)
    {
        // Send connect param update request
        GAPRole_PeripheralConnParamUpdateReq(peripheralConnList.connHandle,
                                             DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                                             DEFAULT_DESIRED_MAX_CONN_INTERVAL,
                                             DEFAULT_DESIRED_SLAVE_LATENCY,
                                             DEFAULT_DESIRED_CONN_TIMEOUT,
                                             Peripheral_TaskID);

        //        GAPRole_PeripheralConnParamUpdateReq( peripheralConnList.connHandle,
        //                                              10,
        //                                              20,
        //                                              0,
        //                                              400,
        //                                              Peripheral_TaskID);

        return (events ^ SBP_PARAM_UPDATE_EVT);
    }

    if(events & UART_TO_BLE_SEND_EVT)
    {
        static uint16_t read_length = 0;
        ;
        uint8_t result = 0xff;
        switch(send_to_ble_state)
        {
            case SEND_TO_BLE_TO_SEND:
                 //PRINT("TEST SEND 11111");
                //notify is not enabled
                if(!ble_uart_notify_is_ready(peripheralConnList.connHandle))
                {
                    if(peripheralConnList.connHandle == GAP_CONNHANDLE_INIT)
                    {
                        //connection lost, flush rx fifo here
                        app_drv_fifo_flush(&app_uart_rx_fifo);
                    }
                 //   PRINT("TEST SEND 2312312");
                    break;
                }
                
                // 1. §µ§Ù§ß§Ñ§Ö§Þ, §ã§Ü§à§Ý§î§Ü§à §Ò§Ñ§Û§ä §²§¦§¡§­§¾§¯§° §ß§Ñ§Ü§à§á§Ú§Ý§à§ã§î §Ó §Ò§å§æ§Ö§â§Ö §ß§Ñ §Õ§Ñ§ß§ß§í§Û §Þ§à§Þ§Ö§ß§ä
                uint16_t fifo_len = app_drv_fifo_length(&app_uart_rx_fifo);
                
                // 2. §¦§ã§Ý§Ú §Ó §Ò§å§æ§Ö§â§Ö §á§å§ã§ä§à, §ß§Ñ§Þ §ß§Ö§é§Ö§Ô§à §à§ä§á§â§Ñ§Ó§Ý§ñ§ä§î ¡ª §Ó§í§ç§à§Õ§Ú§Þ
                if (fifo_len == 0)
                {
                    //PRINT("TEST ");
                    break;
                }

                // 3. §©§Ñ§Õ§Ñ§Ö§Þ §Ò§Ñ§Ù§à§Ó§å§ð §Õ§Ý§Ú§ß§å §à§ä§á§â§Ñ§Ó§Ü§Ú §á§à MTU (§Ú§Ý§Ú §Ø§Ö§ã§ä§Ü§à 100)
                read_length = ATT_GetMTU(peripheralConnList.connHandle) - 3;
                //PRINT("TEST SEND %d", read_length);
                if (read_length == 0 || read_length > 100)
                {
                    read_length = 250; // §±§â§Ö§Õ§à§ç§â§Ñ§ß§Ú§ä§Ö§Ý§î
                }

                // 4. §¬§²§ª§´§ª§¹§¦§³§¬§ª§« §º§¡§¤: §¦§ã§Ý§Ú §ß§Ñ§Ü§à§á§Ú§Ó§ê§Ú§ç§ã§ñ §Õ§Ñ§ß§ß§í§ç §Þ§Ö§ß§î§ê§Ö, §é§Ö§Þ §Ý§Ú§Þ§Ú§ä MTU (§ß§Ñ§á§â§Ú§Þ§Ö§â, §Ó§ã§Ö§Ô§à 57 §Ò§Ñ§Û§ä),
                // §Þ§í §á§â§Ú§ß§å§Õ§Ú§ä§Ö§Ý§î§ß§à §å§Þ§Ö§ß§î§ê§Ñ§Ö§Þ read_length §Õ§à 57, §é§ä§à§Ò§í §Ù§Ñ§Ò§â§Ñ§ä§î §Ú§ç §£§³§¦ §Ù§Ñ §à§Õ§Ú§ß §â§Ñ§Ù
                if (fifo_len < read_length)
                {
                    read_length = fifo_len;
                }

                // 5. §¹§Ú§ä§Ñ§Ö§Þ §Õ§Ñ§ß§ß§í§Ö §Ú§Ù §Ò§å§æ§Ö§â§Ñ (§ä§Ö§á§Ö§â§î §å§ã§Ý§à§Ó§Ú§Ö §Ô§Ñ§â§Ñ§ß§ä§Ú§â§à§Ó§Ñ§ß§ß§à §Ó§í§á§à§Ý§ß§Ú§ä§ã§ñ)
                if(app_drv_fifo_length(&app_uart_rx_fifo) >= read_length)
                {
                    result = app_drv_fifo_read(&app_uart_rx_fifo, to_test_buffer, &read_length);
                    uart_to_ble_send_evt_cnt = 0;
                    
                    // §¥§Ñ§Ý§î§ê§Ö §ß§Ú§Ø§Ö §á§à §Ó§Ñ§ê§Ö§Þ§å §Ü§à§Õ§å §á§à§Û§Õ§Ö§ä §à§ä§á§â§Ñ§Ó§Ü§Ñ GATT_Notification(..., to_test_buffer, read_length);
                }
                else
                {
                    if(uart_to_ble_send_evt_cnt > 10)
                    {
                        result = app_drv_fifo_read(&app_uart_rx_fifo, to_test_buffer, &read_length);
                        uart_to_ble_send_evt_cnt = 0;
                    }
                    else
                    {
                        tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 4);
                        uart_to_ble_send_evt_cnt++;
                    }
                }


                if(APP_DRV_FIFO_RESULT_SUCCESS == result)
                {
                    noti.len = read_length;
                    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
                    if(noti.pValue != NULL)
                    {
                        tmos_memcpy(noti.pValue, to_test_buffer, noti.len);
                        result = ble_uart_notify(peripheralConnList.connHandle, &noti, 0);
                        if(result != SUCCESS)
                        {
                           // PRINT("R1:%02x\r\n", result);
                            send_to_ble_state = SEND_TO_BLE_SEND_FAILED;
                            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
                            tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
                        }
                        else
                        {
                            send_to_ble_state = SEND_TO_BLE_TO_SEND;
                            //app_fifo_write(&app_uart_tx_fifo,to_test_buffer,&read_length);
                            //app_drv_fifo_write(&app_uart_tx_fifo,to_test_buffer,&read_length);
                            read_length = 0;
                            tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
                        }
                    }
                    else
                    {
                        send_to_ble_state = SEND_TO_BLE_ALLOC_FAILED;
                        tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
                    }
                }
                else
                {
                    //send_to_ble_state = SEND_TO_BLE_FIFO_EMPTY;
                }
                break;
            case SEND_TO_BLE_ALLOC_FAILED:
            case SEND_TO_BLE_SEND_FAILED:

                noti.len = read_length;
                noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
                if(noti.pValue != NULL)
                {
                    tmos_memcpy(noti.pValue, to_test_buffer, noti.len);
                    result = ble_uart_notify(peripheralConnList.connHandle, &noti, 0);
                    if(result != SUCCESS)
                    {
                        //PRINT("R2:%02x\r\n", result);
                        send_to_ble_state = SEND_TO_BLE_SEND_FAILED;
                        GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
                        tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
                    }
                    else
                    {
                        send_to_ble_state = SEND_TO_BLE_TO_SEND;
                        //app_drv_fifo_write(&app_uart_tx_fifo,to_test_buffer,&read_length);
                        read_length = 0;
                        tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
                    }
                }
                else
                {
                    send_to_ble_state = SEND_TO_BLE_ALLOC_FAILED;
                    tmos_start_task(Peripheral_TaskID, UART_TO_BLE_SEND_EVT, 2);
                }
                break;
            default:
                break;
        }
        return (events ^ UART_TO_BLE_SEND_EVT);
    }
    // Discard unknown events
    return 0;
}

/*********************************************************************
 * @fn      Peripheral_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        default:
            break;
    }
}

/*********************************************************************
 * @fn      Peripheral_LinkEstablished
 *
 * @brief   Process link established.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkEstablished(gapRoleEvent_t *pEvent)
{
    gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

    // See if already connected
    if(peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
    {
        GAPRole_TerminateLink(pEvent->linkCmpl.connectionHandle);
        PRINT("Connection max...\n");
    }
    else
    {
        peripheralConnList.connHandle = event->connectionHandle;
        peripheralConnList.connInterval = event->connInterval;
        peripheralConnList.connSlaveLatency = event->connLatency;
        peripheralConnList.connTimeout = event->connTimeout;

        // Set timer for param update event
        tmos_start_task(Peripheral_TaskID, SBP_MTU_UPDATE_EVT, 20); // req to change mtu     
        tmos_start_task(Peripheral_TaskID, SBP_PARAM_UPDATE_EVT, SBP_PARAM_UPDATE_DELAY);

       // PRINT("Conn %x - Int %x \n", event->connectionHandle, event->connInterval);
    }
}

/*********************************************************************
 * @fn      Peripheral_LinkTerminated
 *
 * @brief   Process link terminated.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkTerminated(gapRoleEvent_t *pEvent)
{
    gapTerminateLinkEvent_t *event = (gapTerminateLinkEvent_t *)pEvent;

    if(event->connectionHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connHandle = GAP_CONNHANDLE_INIT;
        peripheralConnList.connInterval = 0;
        peripheralConnList.connSlaveLatency = 0;
        peripheralConnList.connTimeout = 0;

        // Restart advertising
        {
            uint8 advertising_enable = TRUE;
            GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &advertising_enable);
        }
    }
    else
    {
        PRINT("ERR..\n");
    }
}

/*********************************************************************
 * @fn      peripheralRssiCB
 *
 * @brief   RSSI callback.
 *
 * @param   connHandle - connection handle
 * @param   rssi - RSSI
 *
 * @return  none
 */
static void peripheralRssiCB(uint16 connHandle, int8 rssi)
{
    PRINT("RSSI -%d dB Conn  %x \n", -rssi, connHandle);
}

/*********************************************************************
 * @fn      peripheralParamUpdateCB
 *
 * @brief   Parameter update complete callback
 *
 * @param   connHandle - connect handle
 *          connInterval - connect interval
 *          connSlaveLatency - connect slave latency
 *          connTimeout - connect timeout
 *
 * @return  none
 */
static void peripheralParamUpdateCB(uint16 connHandle, uint16 connInterval,
                                    uint16 connSlaveLatency, uint16 connTimeout)
{
    if(connHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connInterval = connInterval;
        peripheralConnList.connSlaveLatency = connSlaveLatency;
        peripheralConnList.connTimeout = connTimeout;

        //PRINT("Update %x - Int %x \n", connHandle, connInterval);
    }
    else
    {
        PRINT("peripheralParamUpdateCB err..\n");
    }
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    switch(newState & GAPROLE_STATE_ADV_MASK)
    {
        case GAPROLE_STARTED:
            PRINT("Initialized..\n");
            break;

        case GAPROLE_ADVERTISING:
            if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
            }
            //PRINT("\nAdvertising..\n");
            break;

        case GAPROLE_CONNECTED:
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                Peripheral_LinkEstablished(pEvent);
                //PRINT("\nConnected..\n");
                status_led = CONNECTED;
            }
            break;

        case GAPROLE_CONNECTED_ADV:
            PRINT("Connected Advertising..\n");
            
            break;

        case GAPROLE_WAITING:
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Waiting for advertising..\n");
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
                //PRINT("\nDisconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
                app_sleep_lock = 0;
                status_led = NO_CONNECTED;
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                if(pEvent->gap.hdr.status != SUCCESS)
                {
                    PRINT("Waiting for advertising..\n");
                }
                else
                {
                    PRINT("Error..\n");
                }
            }
            else
            {
                PRINT("Error..%x\n", pEvent->gap.opcode);
            }
            break;

        case GAPROLE_ERROR:
            PRINT("Error..\n");
            break;

        default:
            break;
    }
}

static void Peripheral_RequestMTU(uint16_t connHandle)
{
    // §±§â§Ö§Õ§à§ç§â§Ñ§ß§Ú§ä§Ö§Ý§î: §Ö§ã§Ý§Ú §ã§à§Ö§Õ§Ú§ß§Ö§ß§Ú§Ö §Ö§ë§Ö §ß§Ö §Ù§Ñ§æ§Ú§Ü§ã§Ú§â§à§Ó§Ñ§ß§à, §Ó§í§ç§à§Õ§Ú§Þ
    if (connHandle == 0xFFFF || connHandle == 0x0000) 
    {
        return;
    }

    // §¬§Ý§Ñ§ã§ã§Ú§é§Ö§ã§Ü§à§Ö §ñ§Ó§ß§à§Ö §à§Ò§ì§ñ§Ó§Ý§Ö§ß§Ú§Ö §ã§ä§â§å§Ü§ä§å§â§í §Ù§Ñ§á§â§à§ã§Ñ
    attExchangeMTUReq_t req;
    req.clientRxMTU = 247; 

    // §£§í§Ù§í§Ó§Ñ§Ö§Þ §ã§Ú§ã§ä§Ö§Þ§ß§å§ð §æ§å§ß§Ü§è§Ú§ð §à§Ò§Þ§Ö§ß§Ñ MTU
    uint8_t status = GATT_ExchangeMTU(connHandle, &req, Peripheral_TaskID);
    
    // §¿§ä§à§ä §á§â§Ú§ß§ä §Ó§Þ§Ö§ã§ä§à "TEST MTU:2" §Õ§à§Ý§Ø§Ö§ß §ß§Ñ§Ü§à§ß§Ö§è-§ä§à §á§à§Ü§Ñ§Ù§Ñ§ä§î "TEST MTU:0"
  //  PRINT("TEST MTU:%d\r\n", status); 
}

/*********************************************************************
*********************************************************************/
