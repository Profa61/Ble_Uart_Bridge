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
#include "ble_uart_processing.h"
//#include "app_drv_fifo.h"


#include "app_drv_fifo.h"

// §¥§°§¢§¡§£§ª§´§¾ §¿§´§ª §³§´§²§°§¬§ª §£§£§¦§²§· §¶§¡§«§­§¡:
// extern uint8_t Peripheral_TaskID;  // §ª§Þ§á§à§â§ä§Ú§â§å§Ö§Þ ID §Ù§Ñ§Õ§Ñ§é§Ú §Ú§Ù peripheral.c

// extern app_drv_fifo_t app_uart_tx_fifo;
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
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        
        // §¹§ä§Ö§ß§Ú§Ö §Õ§Ö§ã§Ü§â§Ú§á§ä§à§â§Ñ §á§à§Õ§á§Ú§ã§Ü§Ú (CCCD)
        if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            *pLen = 2;
            tmos_memcpy(pValue, pAttr->pValue, 2);
            return (status);
        }

        // §¹§Ú§ã§ä§à§Ö §Õ§Ú§ß§Ñ§Þ§Ú§é§Ö§ã§Ü§à§Ö §é§ä§Ö§ß§Ú§Ö §Ú§Ù UART §Õ§Ý§ñ §ç§Ñ§â§Ñ§Ü§ä§Ö§â§Ú§ã§ä§Ú§Ü§Ú 0xEA03
        if(uuid == 0xEA03)
        {
            uint16_t fifo_len = app_drv_fifo_length(&app_uart_rx_fifo); 
            uint16_t copy_len = (fifo_len > 250) ? 250 : fifo_len;
            
            if (copy_len > 0)
            {
                app_drv_fifo_read(&app_uart_rx_fifo, pValue, &copy_len);
            }
            
            *pLen = copy_len; // §°§ä§Õ§Ñ§Ö§Þ §Ü§à§ß§æ§Ú§Ô§å§â§Ñ§ä§à§â§å §é§Ú§ã§ä§í§Û §à§ä§Ó§Ö§ä §Ò§Ö§Ù §Þ§Ñ§â§Ü§Ö§â§à§Ó
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

        // 2. §°§¢§²§¡§¢§°§´§¬§¡ §ª §´§¦§¤§ª§²§°§£§¡§¯§ª§¦ §±§¡§¬§¦§´§°§£ §¥§­§Á UART (0xEA03 §Ú 0xEA05)
        if((uuid == 0xEA03) || (uuid == 0xEA05)) 
        {
            uint16_t marker_len = 1;
            uint16_t write_len = len;


            if (offset == 0) 
            {
                //answering_machine(pValue, len); // §Ñ§Ó§ä§à§à§ä§Ó§Ö§ä§é§Ú§Ü

                
                ble_uart_ForwardUartToBle(pValue, len, pAttr); // §â§Ö§Ñ§Ý§Ú§Ù§Ñ§è§Ú§ñ ble-uart §Þ§à§ã§ä§Ñ §Õ§Ý§ñ §á§à§Õ§Ü§Ý§ð§é§Ö§ß§Ú§ñ §Ü §å§ã§ä§â§à§Û§ã§ä§Ó§å
 
            }
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
