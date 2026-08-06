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

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * CONSTANTS
 */

#define SBP_PERIODIC_EVT_PERIOD              300 // How often to perform periodic event
#define SBP_READ_RSSI_EVT_PERIOD             3200 // How often to perform read rssi event
#define SBP_PARAM_UPDATE_DELAY               2000 // Parameter update delay
#define DEFAULT_ADVERTISING_INTERVAL         160 // What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
#define DEFAULT_DISCOVERABLE_MODE            GAP_ADTYPE_FLAGS_GENERAL // Limited discoverable mode advertises for 30.72s, and then stops
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    6 // Minimum connection interval (units of 1.25ms, 10=12.5ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    12 // Maximum connection interval (units of 1.25ms, 100=125ms)
#define DEFAULT_DESIRED_SLAVE_LATENCY        0 // Slave latency to use parameter update
#define DEFAULT_DESIRED_CONN_TIMEOUT         100 // Supervision timeout value (units of 10ms, 100=1s)
#define WCH_COMPANY_ID                       0x07D7 // Company Identifier: WCH

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint8_t Peripheral_TaskID = INVALID_TASK_ID; // Task ID for internal task/event processing
uint8_t Peripheral_bondedFlag = false;
/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

typedef enum
{
  SEND_TO_BLE_TO_SEND = 1,
  SEND_TO_BLE_ALLOC_FAILED,
  SEND_TO_BLE_SEND_FAILED,
} send_to_ble_state_t;

send_to_ble_state_t send_to_ble_state = SEND_TO_BLE_TO_SEND;

blePaControlConfig_t pa_lna_ctl;

uint8 scanRspData[] =   // GAP - SCAN RSP data (max size = 31 bytes)
{
  10, // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'V', 'E', 'G', 'A', ' ', 'S', 'I', '1', '1',
  0x05, // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), // 100ms
  HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
  LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL), // 1s
  HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
  0x02, // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0 // 0dBm
};

static uint8 advertData[] = { // GAP - Advertisement data (max size = 31 bytes, though this is best kept short to conserve power while advertisting)
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

uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "VEGA SI11"; // GAP GATT Attributes

peripheralConnItem_t peripheralConnList; // Connection item list

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);
static void peripheralParamUpdateCB(uint16 connHandle, uint16 connInterval, uint16 connSlaveLatency, uint16 connTimeout);
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList);
static void peripheralRssiCB(uint16 connHandle, int8 rssi);
static void peripheralPasscodeCB(uint8_t *deviceAddr, uint16_t connectionHandle, uint8_t uiInputs, uint8_t uiOutputs);
static void peripheralPairStateCB(uint16_t connHandle, uint8_t state, uint8_t status);

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
//        NULL,
//        NULL
    peripheralPasscodeCB, // Passcode callback (not used by application)
    peripheralPairStateCB  // Pairing / Bonding state Callback (not used by application)
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
 static uint32_t passkey_1 = 123456;
void Peripheral_Init()
{
  Peripheral_TaskID = TMOS_ProcessEventRegister(Peripheral_ProcessEvent);
  // Setup the GAP Peripheral Role Profile
  {
    uint8  initial_advertising_enable = TRUE;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
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
    GAP_SetParamValue(TGAP_ADV_SCAN_REQ_NOTIFY, ENABLE);
  }
  // Setup the GAP Bond Manager
  {
    uint32 passkey = passkey_1;
    uint8 pairMode = 0;
    uint8 mitm = 0;
    uint8 bonding = 0;
    pairMode = GAPBOND_PAIRING_MODE_INITIATE;
    mitm = false;
    bonding = true;
    GATT_InitClient();
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
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
static void peripheralPasscodeCB(uint8_t *deviceAddr, uint16_t connectionHandle, uint8_t uiInputs, uint8_t uiOutputs)
{
  GAPBondMgr_PasscodeRsp(connectionHandle, SUCCESS, passkey_1);
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
static void peripheralPairStateCB(uint16_t connHandle, uint8_t state, uint8_t status)
{
    if(state == GAPBOND_PAIRING_STATE_COMPLETE)
    {
      if(status == SUCCESS) Peripheral_bondedFlag = true;
    }
    else if(state == GAPBOND_PAIRING_STATE_BONDED)
    {
      if(status == SUCCESS) Peripheral_bondedFlag = true;
    }
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

/*********************************************************************
 * @fn      Peripheral_RequestMTU
 *
 * @brief   Request MTU update to 250 bytes
 *
 * @param   connHandle - connection handle
 *
 * @return  none
 */
static void Peripheral_RequestMTU(uint16 connHandle)
{
    // Отправляем запрос на изменение MTU
    uint8_t res = GATT_ExchangeMTU(connHandle, &(attExchangeMTUReq_t){.clientRxMTU = 250}, Peripheral_TaskID);
    PRINT("MTU UPDATED %d", res);
    // Можно добавить задержку перед запросом, если нужно
    // tmos_start_task(Peripheral_TaskID, SBP_MTU_UPDATE_EVT, 100);
}

/*********************************************************************
 * @fn
 *
 * @brief
 *
 * @param
 *
 * @return  NULL
 */
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
//******************************************************************************
  if(events & SYS_EVENT_MSG)
  {
    uint8 *pMsg;
    if((pMsg = tmos_msg_receive(Peripheral_TaskID)) != NULL)
    {
      Peripheral_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);
      tmos_msg_deallocate(pMsg); // Release the TMOS message
    }
    return (events ^ SYS_EVENT_MSG);  // return unprocessed events
  }
//******************************************************************************
  if(events & SBP_START_DEVICE_EVT)
  {
    // Start the Device
    GAPRole_PeripheralStartDevice(Peripheral_TaskID, &Peripheral_BondMgrCBs, &Peripheral_PeripheralCBs);
    return (events ^ SBP_START_DEVICE_EVT);
  }
//******************************************************************************
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
//******************************************************************************
  if(events & SBP_READ_RSSI_EVT)
  {
    GAPRole_ReadRssiCmd(peripheralConnList.connHandle);
    tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);
    return (events ^ SBP_READ_RSSI_EVT);
  }
//******************************************************************************
  if(events & SBP_MTU_UPDATE_EVT)
  {
    Peripheral_RequestMTU(peripheralConnList.connHandle);
    return (events ^ SBP_MTU_UPDATE_EVT);
  }
//******************************************************************************
  if(events & UART_TO_BLE_SEND_EVT)
      {
          static uint16_t read_length = 0;
          uint8_t result = 0xff;
          switch(send_to_ble_state)
          {
              case SEND_TO_BLE_TO_SEND:
                  PRINT("SEND DATA HERE");
                  if(!ble_uart_notify_is_ready(peripheralConnList.connHandle)) //notify is not enabled
                  {
                    if(peripheralConnList.connHandle == GAP_CONNHANDLE_INIT) app_drv_fifo_flush(&app_uart_tx_fifo);
                    PRINT("ERROR NOTI");
                    break;
                  }
                  read_length = ATT_GetMTU(peripheralConnList.connHandle) - 3;
                  if(read_length > sizeof(to_test_buffer)) read_length = sizeof(to_test_buffer);
                  result = app_drv_fifo_read(&app_uart_tx_fifo, to_test_buffer, &read_length);
                  if(APP_DRV_FIFO_RESULT_SUCCESS == result)
                  {
                    PRINT("START SEND %d", read_length);
                    noti.len = read_length;
                    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
                    if(noti.pValue != NULL)
                    {
                      tmos_memcpy(noti.pValue, to_test_buffer, noti.len);
                      result = ble_uart_notify(peripheralConnList.connHandle, &noti, 0);
                      if(result != SUCCESS)
                      {
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
                    PRINT("EWEWE");
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
//******************************************************************************
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
        case GATT_MSG_EVENT:
        {
            gattMsgEvent_t *pMsgEvent = (gattMsgEvent_t *)pMsg;

            // Обработка ответа на MTU Exchange
            if(pMsgEvent->method == ATT_EXCHANGE_MTU_RSP)
            {
                // MTU успешно установлен, можно использовать actualMTU
                // В вашем случае actualMTU должно быть 250
            }
            break;
        }
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
  }
  else
  {
    peripheralConnList.connHandle = event->connectionHandle;
    peripheralConnList.connInterval = event->connInterval;
    peripheralConnList.connSlaveLatency = event->connLatency;
    peripheralConnList.connTimeout = event->connTimeout;
    tmos_start_task(Peripheral_TaskID, SBP_MTU_UPDATE_EVT, 20); // req to change mtu
    // Set timer for param update event
    tmos_start_task(Peripheral_TaskID, SBP_PARAM_UPDATE_EVT, SBP_PARAM_UPDATE_DELAY);
    tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);
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
    tmos_stop_task(Peripheral_TaskID, SBP_READ_RSSI_EVT);
    Peripheral_bondedFlag = false;
    // Restart advertising
    {
      uint8 advertising_enable = TRUE;
      GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &advertising_enable);
    }
  }
  else
  {
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
static void peripheralParamUpdateCB(uint16 connHandle, uint16 connInterval, uint16 connSlaveLatency, uint16 connTimeout)
{
  if(connHandle == peripheralConnList.connHandle)
  {
    peripheralConnList.connInterval = connInterval;
    peripheralConnList.connSlaveLatency = connSlaveLatency;
    peripheralConnList.connTimeout = connTimeout;
  }
  else
  {
  }
}

/*********************************************************************
 * @fn
 *
 * @brief
 * @param
 * @param
 *
 * @return
 */
bool ble_uart_disconnect(void)
{
  if(peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
  {
    GAPRole_TerminateLink(peripheralConnList.connHandle);
    return true;
  }
  else return false;
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
            break;

        case GAPROLE_ADVERTISING:
            if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
            }
            break;

        case GAPROLE_CONNECTED:
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                Peripheral_LinkEstablished(pEvent);
            }
            break;

        case GAPROLE_CONNECTED_ADV:
            break;

        case GAPROLE_WAITING:
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                if(pEvent->gap.hdr.status != SUCCESS)
                {
                }
                else
                {
                }
            }
            else
            {
            }
            break;

        case GAPROLE_ERROR:
            break;

        default:
            break;
    }
}

/*********************************************************************
*********************************************************************/
