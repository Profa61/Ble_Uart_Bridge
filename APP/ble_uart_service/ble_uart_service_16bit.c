/********************************** (C) COPYRIGHT *******************************
 * File Name          : ble_uart_service_16bit_char.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/01/19
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
#include "stdint.h"
#include "ble_uart_service.h"
#include "app_drv_fifo.h"


#include "app_drv_fifo.h"

// §¥§°§¢§¡§£§ª§´§¾ §¿§´§ª §³§´§²§°§¬§ª §£§£§¦§²§· §¶§¡§«§­§¡:
extern uint8_t Peripheral_TaskID;  // §ª§Þ§á§à§â§ä§Ú§â§å§Ö§Þ ID §Ù§Ñ§Õ§Ñ§é§Ú §Ú§Ù peripheral.c
#define APP_UART_TX_EVT   0x0002   // §©§Ñ§Õ§Ñ§Ö§Þ §ß§à§Þ§Ö§â §ã§à§Ò§í§ä§Ú§ñ (§ã§Ó§Ö§â§î§ä§Ö §ã§à §ã§Ó§à§Ú§Þ §Ó peripheral.h)
extern app_drv_fifo_t app_uart_tx_fifo;
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED    7

#define RAWPASS_TX_VALUE_HANDLE       2
#define RAWPASS_RX_VALUE_HANDLE       5
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// §³§Ö§â§Ó§Ú§ã (0xEA02)
const uint8_t ble_uart_ServiceUUID[ATT_BT_UUID_SIZE] = { 0x02, 0xEA };

// §·§Ñ§â§Ñ§Ü§ä§Ö§â§Ú§ã§ä§Ú§Ü§Ñ 0xEA03 (Little-Endian: 0x03, 0xEA)
const uint8_t ble_uart_TxCharUUID[ATT_BT_UUID_SIZE]  = { 0x03, 0xEA };

// §·§Ñ§â§Ñ§Ü§ä§Ö§â§Ú§ã§ä§Ú§Ü§Ñ 0xEA05 (Little-Endian: 0x05, 0xEA)
const uint8_t ble_uart_RxCharUUID[ATT_BT_UUID_SIZE]  = { 0x05, 0xEA };


/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern app_drv_fifo_t app_uart_rx_fifo; 
extern app_drv_fifo_t app_uart_tx_fifo;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static ble_uart_ProfileChangeCB_t ble_uart_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Profile Service attribute
static const gattAttrType_t ble_uart_Service = {ATT_BT_UUID_SIZE, ble_uart_ServiceUUID};

// Profile Characteristic 1 Properties
//static uint8 ble_uart_RxCharProps = GATT_PROP_WRITE_NO_RSP| GATT_PROP_WRITE;
static uint8 ble_uart_RxCharProps = GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic 1 Value
static uint8 ble_uart_RxCharValue[BLE_UART_RX_BUFF_SIZE];
//static uint8 ble_uart_RxCharValue[1];

// Profile Characteristic 2 Properties
//static uint8 ble_uart_TxCharProps = GATT_PROP_NOTIFY| GATT_PROP_INDICATE;
static uint8 ble_uart_TxCharProps = GATT_PROP_READ | GATT_PROP_NOTIFY | GATT_PROP_WRITE;

// Characteristic 2 Value
static uint8 ble_uart_TxCharValue = 0;

// Simple Profile Characteristic 2 User Description
static gattCharCfg_t ble_uart_TxCCCD[4];
// static gattCharCfg_t ble_uart_TxCCCD[4] = {
//     { .connHandle = INVALID_CONNHANDLE, .value = GATT_CLIENT_CFG_NOTIFY },
//     { .connHandle = INVALID_CONNHANDLE, .value = GATT_CLIENT_CFG_NOTIFY },
//     { .connHandle = INVALID_CONNHANDLE, .value = GATT_CLIENT_CFG_NOTIFY },
//     { .connHandle = INVALID_CONNHANDLE, .value = GATT_CLIENT_CFG_NOTIFY }
//};
/*********************************************************************
 * Profile Attributes - Table
 */


static gattAttribute_t ble_uart_ProfileAttrTbl[] = {
    // 1. §ª§ß§Ú§è§Ú§Ñ§Ý§Ú§Ù§Ñ§è§Ú§ñ §ã§Ñ§Þ§à§Ô§à §ã§Ö§â§Ó§Ú§ã§Ñ 0xEA02
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, 
        GATT_PERMIT_READ,                       
        0,                                      
        (uint8 *)&ble_uart_Service // §±§Ö§â§Ö§Õ§Ñ§Ö§Þ §ß§Ñ§á§â§ñ§Þ§å§ð §Ú§Þ§ñ §Þ§Ñ§ã§ã§Ú§Ó§Ñ §Ò§Ö§Ù §Ù§ß§Ñ§Ü§Ñ &            
    },

    // 2. §¥§Ö§Ü§Ý§Ñ§â§Ñ§è§Ú§ñ §·§Ñ§â§Ñ§Ü§ä§Ö§â§Ú§ã§ä§Ú§Ü§Ú 0xEA03 (§°§ä§Ó§Ö§ä§í/§¹§ä§Ö§ß§Ú§Ö/§µ§Ó§Ö§Õ§à§Þ§Ý§Ö§ß§Ú§ñ)
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        // §³§Ó§à§Û§ã§ä§Ó§Ñ: §â§Ñ§Ù§â§Ö§ê§Ñ§Ö§Þ §¹§ä§Ö§ß§Ú§Ö, §©§Ñ§á§Ú§ã§î (§ã §à§ä§Ó§Ö§ä§à§Þ §Ú §Ò§Ö§Ù) §Ú §µ§Ó§Ö§Õ§à§Þ§Ý§Ö§ß§Ú§ñ
        &(uint8_t){ GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP | GATT_PROP_NOTIFY }
    },

    // 3. §©§ß§Ñ§é§Ö§ß§Ú§Ö §·§Ñ§â§Ñ§Ü§ä§Ö§â§Ú§ã§ä§Ú§Ü§Ú 0xEA03
    {
        {ATT_BT_UUID_SIZE, ble_uart_TxCharUUID}, 
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,    // §ª§³§±§²§¡§£§­§¦§¯§°: §²§Ñ§Ù§â§Ö§ê§Ñ§Ö§Þ §Ú §é§Ú§ä§Ñ§ä§î, §Ú §á§Ú§ã§Ñ§ä§î §Ó §ß§Ö§×
        0,
        (uint8 *)&ble_uart_TxCharValue
    },

    // 4. §¥§Ö§ã§Ü§â§Ú§á§ä§à§â §Ü§à§ß§æ§Ú§Ô§å§â§Ñ§è§Ú§Ú §Ü§Ý§Ú§Ö§ß§ä§Ñ (CCCD) §Õ§Ý§ñ 0xEA03
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,   
        0,
        (uint8 *)&ble_uart_TxCCCD               
    },

    // 5. §¥§Ö§Ü§Ý§Ñ§â§Ñ§è§Ú§ñ §·§Ñ§â§Ñ§Ü§ä§Ö§â§Ú§ã§ä§Ú§Ü§Ú 0xEA05 (§©§Ñ§á§Ú§ã§î §á§Ñ§â§Ñ§Þ§Ö§ä§â§à§Ó)
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &(uint8_t){ GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP }
    },

    // 6. §©§ß§Ñ§é§Ö§ß§Ú§Ö §·§Ñ§â§Ñ§Ü§ä§Ö§â§Ú§ã§ä§Ú§Ü§Ú 0xEA05
    {
        {ATT_BT_UUID_SIZE, ble_uart_RxCharUUID}, 
        GATT_PERMIT_WRITE,                       // §²§Ñ§Ù§â§Ö§ê§Ñ§Ö§Þ §Ü§à§ß§æ§Ú§Ô§å§â§Ñ§ä§à§â§å §á§Ú§ã§Ñ§ä§î §ã§ð§Õ§Ñ
        0,
        &ble_uart_RxCharValue[0]
    }
};






/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t ble_uart_ReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 *pLen, uint16 offset, uint16 maxLen, uint8 method);
static bStatus_t ble_uart_WriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                      uint8 *pValue, uint16 len, uint16 offset, uint8 method);

static void ble_uart_HandleConnStatusCB(uint16 connHandle, uint8 changeType);

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
gattServiceCBs_t ble_uart_ProfileCBs = {
    ble_uart_ReadAttrCB,  // Read callback function pointer
    ble_uart_WriteAttrCB, // Write callback function pointer
    NULL                  // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      ble_uart_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t ble_uart_add_service(ble_uart_ProfileChangeCB_t cb)
{
    uint8 status = SUCCESS;

    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, ble_uart_TxCCCD);
    // Register with Link DB to receive link status change callback
    linkDB_Register(ble_uart_HandleConnStatusCB);

    //    ble_uart_TxCCCD.connHandle = INVALID_CONNHANDLE;
    //    ble_uart_TxCCCD.value = 0;
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService(ble_uart_ProfileAttrTbl,
                                         GATT_NUM_ATTRS(ble_uart_ProfileAttrTbl),
                                         GATT_MAX_ENCRYPT_KEY_SIZE,
                                         &ble_uart_ProfileCBs);
    if(status != SUCCESS)
        PRINT("Add ble uart service failed!\n");
    ble_uart_AppCBs = cb;

    return (status);
}

/*********************************************************************
 * @fn          ble_uart_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */


static bStatus_t ble_uart_ReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                     uint8 *pValue, uint16 *pLen, uint16 offset, uint16 maxLen, uint8 method)
{
    bStatus_t status = SUCCESS;

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // §³§à§Ò§Ú§â§Ñ§Ö§Þ 16-§Ò§Ú§ä§ß§í§Û UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        
        // 1. §¹§ä§Ö§ß§Ú§Ö §Õ§Ö§ã§Ü§â§Ú§á§ä§à§â§Ñ §á§à§Õ§á§Ú§ã§Ü§Ú (CCCD)
        if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            *pLen = 2;
            tmos_memcpy(pValue, pAttr->pValue, 2);
            return (status);
        }

        // 2. §¥§ª§¯§¡§®§ª§¹§¦§³§¬§°§¦ §¹§´§¦§¯§ª§¦ §ª§© UART §¥§­§Á §·§¡§²§¡§¬§´§¦§²§ª§³§´§ª§¬§ª 0xEA03
        // 2. §¹§ª§³§´§°§¦ §¥§ª§¯§¡§®§ª§¹§¦§³§¬§°§¦ §¹§´§¦§¯§ª§¦ §ª§© UART §¥§­§Á §·§¡§²§¡§¬§´§¦§²§ª§³§´§ª§¬§ª 0xEA03 (§¢§¦§© §®§¡§²§¬§¦§²§¡)
        if(uuid == 0xEA03)
        {
            // §µ§Ù§ß§Ñ§Ö§Þ, §ã§Ü§à§Ý§î§Ü§à §Ò§Ñ§Û§ä §á§â§Ú§Ý§Ö§ä§Ö§Ý§à §á§à §á§â§à§Ó§à§Õ§å §Ú §ã§Ö§Û§é§Ñ§ã §Ý§Ö§Ø§Ú§ä §Ó §Ò§å§æ§Ö§â§Ö UART
            uint16_t fifo_len = app_drv_fifo_length(&app_uart_rx_fifo); 
            
            // §°§Ô§â§Ñ§ß§Ú§é§Ú§Ó§Ñ§Ö§Þ §Õ§Ý§Ú§ß§å §á§Ñ§Ü§Ö§ä§Ñ §á§à§Õ MTU (§Þ§Ñ§Ü§ã§Ú§Þ§å§Þ 250 §Ò§Ñ§Û§ä)
            uint16_t copy_len = (fifo_len > 250) ? 250 : fifo_len;
            
            if (copy_len > 0)
            {
                // §£§í§é§Ú§ä§í§Ó§Ñ§Ö§Þ §Õ§Ñ§ß§ß§í§Ö §Ú§Ù UART FIFO §¯§¡§±§²§Á§®§µ§À §Ó §Ò§å§æ§Ö§â BLE pValue
                // §¢§à§Ý§î§ê§Ö §ß§Ö§ä §ß§Ú§Ü§Ñ§Ü§Ú§ç §á§â§à§Þ§Ö§Ø§å§ä§à§é§ß§í§ç §Ò§å§æ§Ö§â§à§Ó §Ú §Ù§Ñ§ä§Ú§â§Ñ§ß§Ú§ñ §ç§Ó§à§ã§ä§Ñ §á§Ñ§Ü§Ö§ä§Ñ!
                app_drv_fifo_read(&app_uart_rx_fifo, pValue, &copy_len);
            }
            
            // §©§Ñ§Õ§Ñ§Ö§Þ §é§Ö§ã§ä§ß§å§ð §Õ§Ý§Ú§ß§å §à§ä§Ó§Ö§ä§Ñ: §â§à§Ó§ß§à §ã§ä§à§Ý§î§Ü§à §Ò§Ñ§Û§ä, §ã§Ü§à§Ý§î§Ü§à §å§ê§Ý§à §Ó pValue
            *pLen = copy_len;
            
            return (status);
        }

    }

    return (ATT_ERR_ATTR_NOT_FOUND);
}

/*********************************************************************
 * @fn      simpleProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 *
 * @return  Success or Failure
 */

static bStatus_t ble_uart_WriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                      uint8 *pValue, uint16 len, uint16 offset, uint8 method)
{
    bStatus_t status = SUCCESS;

    if(gattPermitAuthorWrite(pAttr->permissions))
    {
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        
        // 1. §°§Ò§â§Ñ§Ò§à§ä§Ü§Ñ §á§à§Õ§á§Ú§ã§Ü§Ú §ß§Ñ §å§Ó§Ö§Õ§à§Þ§Ý§Ö§ß§Ú§ñ (CCCD)
        if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                    offset, GATT_CLIENT_CFG_NOTIFY);
            if(status == SUCCESS && ble_uart_AppCBs)
            {
                uint16         charCfg = BUILD_UINT16(pValue[0], pValue[1]);
                ble_uart_evt_t evt;
                evt.type = (charCfg == GATT_CFG_NO_OPERATION) ? BLE_UART_EVT_TX_NOTI_DISABLED : BLE_UART_EVT_TX_NOTI_ENABLED;
                ble_uart_AppCBs(connHandle, &evt);
            }
            return (status);
        }

        // 2. §ª§³§±§²§¡§£§­§¦§¯§°: §ª§ß§ä§Ö§Ý§Ý§Ö§Ü§ä§å§Ñ§Ý§î§ß§à§Ö §ä§Ö§Ô§Ú§â§à§Ó§Ñ§ß§Ú§Ö §Ü§Ñ§ß§Ñ§Ý§à§Ó §ß§Ñ §à§ã§ß§à§Ó§Ö §ã§Þ§Ö§ë§Ö§ß§Ú§ñ offset
        if((uuid == 0xEA03) || (uuid == 0xEA05)) 
        {
            // §±§Ö§â§Ö§Þ§Ö§ß§ß§í§Ö §Õ§Ý§ñ §á§Ö§â§Ö§Õ§Ñ§é§Ú §Õ§Ý§Ú§ß §á§à §å§Ü§Ñ§Ù§Ñ§ä§Ö§Ý§ð §Ó §Ò§Ú§Ò§Ý§Ú§à§ä§Ö§Ü§å FIFO
            uint16_t marker_len = 1;
            uint16_t write_len = len;

            // §¬§²§ª§´§ª§¹§¦§³§¬§ª§« §º§¡§¤: §¥§à§Ò§Ñ§Ó§Ý§ñ§Ö§Þ §Þ§Ñ§â§Ü§Ö§â §´§°§­§¾§¬§° §Ö§ã§Ý§Ú §ï§ä§à §ß§Ñ§é§Ñ§Ý§à §á§Ñ§Ü§Ö§ä§Ñ (offset == 0)
            if (offset == 0)
            {
                uint8_t channel_marker = (uuid == 0xEA03) ? 0x03 : 0x05;
                app_drv_fifo_write(&app_uart_tx_fifo, &channel_marker, &marker_len);
            }
            
            // §£§ã§Ö§Ô§Õ§Ñ §Õ§à§á§Ú§ã§í§Ó§Ñ§Ö§Þ §á§à§Ý§Ö§Ù§ß§å§ð §ß§Ñ§Ô§â§å§Ù§Ü§å (§Ú §Õ§Ý§ñ §á§Ö§â§Ó§à§Û §é§Ñ§ã§ä§Ú, §Ú §Õ§Ý§ñ §Ó§ã§Ö§ç §á§à§ã§Ý§Ö§Õ§å§ð§ë§Ú§ç "§ç§Ó§à§ã§ä§à§Ó")
            if (write_len > 0)
            {
                app_drv_fifo_write(&app_uart_tx_fifo, pValue, &write_len);
            }
            
            // §£§Ù§Ó§à§Õ§Ú§Þ §ä§Ñ§ã§Ü §æ§Ú§Ù§Ú§é§Ö§ã§Ü§à§Ô§à §Ó§í§Ó§à§Õ§Ñ §ß§Ñ§Ü§à§á§Ý§Ö§ß§ß§à§Ô§à §Ü§Ñ§Õ§â§Ñ §Ó §á§â§à§Ó§à§Õ UART1
            tmos_start_task(Peripheral_TaskID, APP_UART_TX_EVT, 2);
        }
    }

    return (status);
}

/*********************************************************************
 * @fn          simpleProfile_HandleConnStatusCB
 *
 * @brief       Simple Profile link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 */
static void ble_uart_HandleConnStatusCB(uint16 connHandle, uint8 changeType)
{
    // Make sure this is not loopback connection
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // Reset Client Char Config if connection has dropped
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            //ble_uart_TxCCCD[0].value = 0;
            GATTServApp_InitCharCfg(connHandle, ble_uart_TxCCCD);
            //PRINT("clear client configuration\n");
        }
    }
}

uint8 ble_uart_notify_is_ready(uint16 connHandle)
{
    return (GATT_CLIENT_CFG_NOTIFY == GATTServApp_ReadCharCfg(connHandle, ble_uart_TxCCCD));
}
/*********************************************************************
 * @fn          BloodPressure_IMeasNotify
 *
 * @brief       Send a notification containing a bloodPressure
 *              measurement.
 *
 * @param       connHandle - connection handle
 * @param       pNoti - pointer to notification structure
 *
 * @return      Success or Failure
 */
bStatus_t ble_uart_notify(uint16 connHandle, attHandleValueNoti_t *pNoti, uint8 taskId)
{
    //uint16 value = ble_uart_TxCCCD[0].value;
    uint16 value = GATTServApp_ReadCharCfg(connHandle, ble_uart_TxCCCD);
    // If notifications enabled
    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        // Set the handle
        pNoti->handle = ble_uart_ProfileAttrTbl[RAWPASS_TX_VALUE_HANDLE].handle;

        // Send the Indication
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

/*********************************************************************
*********************************************************************/
