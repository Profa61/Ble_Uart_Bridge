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
    //PRINT("ReadAttrCB\n");

    // Make sure it's not a blob operation (no attributes in the profile are long)
    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            *pLen = 2;
            tmos_memcpy(pValue, pAttr->pValue, 2);
        }
    }
    //    else
    //    {
    //        if(!tmos_memcmp(pAttr->type.uuid, ble_uart_TxCharUUID, 16))
    //        {
    //            *pLen = 1;
    //            pValue[0] = '1';
    //        }
    //        else if(!tmos_memcmp(pAttr->type.uuid, ble_uart_RxCharUUID, 16))
    //        {
    //            PRINT("read tx char\n");
    //        }
    //    }

    return (status);
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
    //uint8 notifyApp = 0xFF;
    // If attribute permissions require authorization to write, return error
    if(gattPermitAuthorWrite(pAttr->permissions))
    {
        // Insufficient authorization
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                    offset, GATT_CLIENT_CFG_NOTIFY);
            if(status == SUCCESS && ble_uart_AppCBs)
            {
                uint16         charCfg = BUILD_UINT16(pValue[0], pValue[1]);
                ble_uart_evt_t evt;

                //PRINT("CCCD set: [%d]\n", charCfg);
                evt.type = (charCfg == GATT_CFG_NO_OPERATION) ? BLE_UART_EVT_TX_NOTI_DISABLED : BLE_UART_EVT_TX_NOTI_ENABLED;
                ble_uart_AppCBs(connHandle, &evt);
            }
        }

        // §±§â§à§Ó§Ö§â§Ü§Ñ UUID §ç§Ñ§â§Ñ§Ü§ä§Ö§â§Ú§ã§ä§Ú§Ü (0xEA03 §Ú 0xEA05)
        if((uuid == 0xEA03) || (uuid == 0xEA05))
        {
            // --- §±§²§°§£§¦§²§¬§¡ §¯§¡ §¢§½§³§´§²§½§« §©§¡§±§²§°§³ §¬§°§¯§¶§ª§¤§µ§²§¡§´§°§²§¡ ---
            // §±§â§à§Ó§Ö§â§ñ§Ö§Þ §Õ§Ý§Ú§ß§å (7 §Ò§Ñ§Û§ä) §Ú §á§Ö§â§Ó§í§Ö §Ò§Ñ§Û§ä§í §Ù§Ñ§á§â§à§ã§Ñ: 04 02 03 EF ...
            // if (len == 7 && pValue[0] == 0x04 && pValue[1] == 0x02 && pValue[2] == 0x03 && pValue[3] == 0xEF)
            // {
            //     // §¶§à§â§Þ§Ú§â§å§Ö§Þ §Ø§Ö§ã§ä§Ü§à §Ù§Ñ§Õ§Ñ§ß§ß§í§Û §à§ä§Ó§Ö§ä: 04 03 03 EF 4E F3
            //     static uint8_t fast_response[] = {0x04, 0x03, 0x03, 0xEF,0x01, 0x4A, 0x2B};
                
            //     attHandleValueNoti_t noti;
            //     noti.len = sizeof(fast_response);
            //     noti.handle = ble_uart_ProfileAttrTbl[2].handle;
            //         // §º§Ý§Ö§Þ §ã§ä§â§à§Ô§à §Ó §Ü§Ñ§ß§Ñ§Ý §à§ä§Ó§Ö§ä§à§Ó (0xEA03)
                
            //     // §£§í§Õ§Ö§Ý§ñ§Ö§Þ §á§Ñ§Þ§ñ§ä§î §Ú§Ù §Ü§å§é§Ú BLE-§ã§ä§Ö§Ü§Ñ
            //     noti.pValue = GATT_bm_alloc(connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
            //     if (noti.pValue != NULL)
            //     {
            //         tmos_memcpy(noti.pValue, fast_response, noti.len);
                    
            //         // §°§ä§á§â§Ñ§Ó§Ý§ñ§Ö§Þ Notification §à§Ò§â§Ñ§ä§ß§à §Ü§à§ß§æ§Ú§Ô§å§â§Ñ§ä§à§â§å §Þ§Ô§ß§à§Ó§Ö§ß§ß§à
            //         if (GATT_Notification(connHandle, &noti, FALSE) != SUCCESS)
            //         {
            //             GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
            //         }
            //     }
                
            //     // §±§â§Ö§â§í§Ó§Ñ§Ö§Þ §Õ§Ñ§Ý§î§ß§Ö§Û§ê§å§ð §à§Ò§â§Ñ§Ò§à§ä§Ü§å, §é§ä§à§Ò§í §ï§ä§à§ä §á§Ñ§Ü§Ö§ä §¯§¦ §å§ê§Ö§Ý §Ó UART
            //     return (status); 
            // }

            // --- §°§¢§½§¹§¯§¡§Á §­§°§¤§ª§¬§¡ §¥§­§Á §°§³§´§¡§­§¾§¯§½§· §±§¡§¬§¦§´§°§£ (§³ §®§¡§²§¬§¦§²§¡§®§ª) ---
            if(ble_uart_AppCBs)
            {
                static uint8_t temp_ble_buffer[256]; 
                
                if(uuid == 0xEA03) {
                    temp_ble_buffer[0] = 0x03;
                } else {
                    temp_ble_buffer[0] = 0x05;
                }
                
                uint16_t copy_len = (len > 250) ? 250 : len;
                tmos_memcpy(&temp_ble_buffer[1], pValue, copy_len);
                
                ble_uart_evt_t evt;
                evt.type = BLE_UART_EVT_BLE_DATA_RECIEVED;
                evt.data.length = copy_len + 1;
                evt.data.p_data = temp_ble_buffer;
                
                ble_uart_AppCBs(connHandle, &evt);
            }
        }



        // // §±§â§à§Ó§Ö§â§Ü§Ñ UUID §ç§Ñ§â§Ñ§Ü§ä§Ö§â§Ú§ã§ä§Ú§Ü
        // if((uuid == 0xEA03) || (uuid == 0xEA05))
        // {
        //     if(ble_uart_AppCBs)
        //     {
        //         // §³§à§Ù§Õ§Ñ§Ö§Þ §Ó§â§Ö§Þ§Ö§ß§ß§í§Û §Ò§å§æ§Ö§â §ß§Ñ §ã§ä§Ö§Ü§Ö. 
        //         // §²§Ñ§Ù§Þ§Ö§â 251 §Ó§Ù§ñ§ä §ã §Ù§Ñ§á§Ñ§ã§à§Þ §á§à§Õ §Þ§Ñ§Ü§ã§Ú§Þ§Ñ§Ý§î§ß§í§Û BLE MTU (247 §Ò§Ñ§Û§ä + 1 §Ò§Ñ§Û§ä §Þ§Ñ§â§Ü§Ö§â§Ñ)
        //         uint8_t temp_buffer[251]; 
                
        //         // 1. §±§Ö§â§Ó§í§Þ §Ò§Ñ§Û§ä§à§Þ §Ù§Ñ§á§Ú§ã§í§Ó§Ñ§Ö§Þ §Þ§Ñ§â§Ü§Ö§â (0x03 §Ú§Ý§Ú 0x05 §Ó §Ù§Ñ§Ó§Ú§ã§Ú§Þ§à§ã§ä§Ú §à§ä §ä§à§Ô§à, §Ü§Ñ§Ü§à§Û UUID §á§â§Ú§ê§Ö§Ý)
        //         if(uuid == 0xEA03) {
        //             temp_buffer[0] = 0x03;
        //         } else {
        //             temp_buffer[0] = 0x05;
        //         }
                
        //         // 2. §¬§à§á§Ú§â§å§Ö§Þ §ã§Ñ§Þ BLE-§á§Ñ§Ü§Ö§ä §Ó §Ò§å§æ§Ö§â §ã§â§Ñ§Ù§å §á§à§ã§Ý§Ö §Þ§Ñ§â§Ü§Ö§â§ß§à§Ô§à §Ò§Ñ§Û§ä§Ñ
        //         // §°§Ô§â§Ñ§ß§Ú§é§Ú§Ó§Ñ§Ö§Þ §Õ§Ý§Ú§ß§å, §é§ä§à§Ò§í §ã§Ý§å§é§Ñ§Û§ß§à §ß§Ö §Ó§í§Û§ä§Ú §Ù§Ñ §Ô§â§Ñ§ß§Ú§è§í temp_buffer
        //         uint16_t copy_len = (len > 250) ? 250 : len;
        //         tmos_memcpy(&temp_buffer[1], pValue, copy_len);
                
        //         // 3. §©§Ñ§á§à§Ý§ß§ñ§Ö§Þ §ã§ä§â§å§Ü§ä§å§â§å §ã§à§Ò§í§ä§Ú§ñ §ß§à§Ó§í§Þ§Ú §Õ§Ñ§ß§ß§í§Þ§Ú
        //         ble_uart_evt_t evt;
        //         evt.type = BLE_UART_EVT_BLE_DATA_RECIEVED;
        //         evt.data.length = copy_len + 1; // §°§Ò§ë§Ñ§ñ §Õ§Ý§Ú§ß§Ñ §Ó§í§â§à§ã§Ý§Ñ §ß§Ñ 1 §Ò§Ñ§Û§ä §Þ§Ñ§â§Ü§Ö§â§Ñ
        //         evt.data.p_data = temp_buffer;  // §±§Ö§â§Ö§Õ§Ñ§Ö§Þ §å§Ü§Ñ§Ù§Ñ§ä§Ö§Ý§î §ß§Ñ §ß§Ñ§ê §Þ§à§Õ§Ú§æ§Ú§è§Ú§â§à§Ó§Ñ§ß§ß§í§Û §Ò§å§æ§Ö§â
                
        //         // 4. §°§ä§á§â§Ñ§Ó§Ý§ñ§Ö§Þ §ã§à§Ò§í§ä§Ú§Ö §Ó §á§â§Ú§Ý§à§Ø§Ö§ß§Ú§Ö (§Ó peripheral.c)
        //         ble_uart_AppCBs(connHandle, &evt);
        //     }
        // }



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
