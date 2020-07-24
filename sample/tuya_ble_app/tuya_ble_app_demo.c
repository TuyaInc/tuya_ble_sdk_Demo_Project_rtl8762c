#include <os_msg.h>
#include <os_task.h>
#include "os_timer.h"
#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_heap.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_event.h"
#include "tuya_ble_app_demo.h"
#include "tuya_ble_log.h"
#include "tuya_ble_app_ota.h"
#include "tuya_ble_demo_version.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define APP_CUSTOM_TASK_PRIORITY             1         //!< Task priorities
#define APP_CUSTOM_TASK_STACK_SIZE           256 * 6   //!<  Task stack size
#define MAX_NUMBER_OF_TUYA_CUSTOM_MESSAGE    0x20      //!<  GAP message queue size
/*============================================================================*
 *                              Variables
 *============================================================================*/
void *app_custom_task_handle;
void *tuya_custom_queue_handle;


tuya_ble_device_param_t device_param = {0};

static const char auth_key_test[] = "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
static const char device_id_test[] = "zzzzzzzzzzzzzzzz";
static const uint8_t mac_test[6] = {0x11,0x22,0x33,0x44,0x55,0x66}; //The actual MAC address is : 66:55:44:33:22:11


#define APP_CUSTOM_EVENT_1  1
#define APP_CUSTOM_EVENT_2  2
#define APP_CUSTOM_EVENT_3  3
#define APP_CUSTOM_EVENT_4  4
#define APP_CUSTOM_EVENT_5  5

static uint8_t dp_data_array[255+3];
static uint16_t dp_data_len = 0;

typedef struct {
    uint8_t data[50];
} custom_data_type_t;

void custom_data_process(int32_t evt_id,void *data)
{
    custom_data_type_t *event_1_data;
    TUYA_APP_LOG_DEBUG("custom event id = %d",evt_id);
    switch (evt_id)
    {
    case APP_CUSTOM_EVENT_1:
        event_1_data = (custom_data_type_t *)data;
        TUYA_APP_LOG_HEXDUMP_DEBUG("received APP_CUSTOM_EVENT_1 data:",event_1_data->data,50);
        break;
    case APP_CUSTOM_EVENT_2:
        break;
    case APP_CUSTOM_EVENT_3:
        break;
    case APP_CUSTOM_EVENT_4:
        break;
    case APP_CUSTOM_EVENT_5:
        break;
    default:
        break;

    }
}

custom_data_type_t custom_data;

void custom_evt_1_send_test(uint8_t data)
{
    tuya_ble_custom_evt_t event;

    for(uint8_t i=0; i<50; i++)
    {
        custom_data.data[i] = data;
    }
    event.evt_id = APP_CUSTOM_EVENT_1;
    event.custom_event_handler = (void *)custom_data_process;
    event.data = &custom_data;
    tuya_ble_custom_event_send(event);
}


static uint16_t sn = 0;
static uint32_t time_stamp = 1587795793;

void app_custom_task(void *p_param)
{
    tuya_ble_cb_evt_param_t event;

    while (true)
    {
        if (os_msg_recv(tuya_custom_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            int16_t result = 0;
            switch (event.evt)
            {
            case TUYA_BLE_CB_EVT_CONNECTE_STATUS:
                TUYA_APP_LOG_INFO("received tuya ble conncet status update event,current connect status = %d",event.connect_status);
                break;
            case TUYA_BLE_CB_EVT_DP_WRITE:
                dp_data_len = event.dp_write_data.data_len;
                memset(dp_data_array,0,sizeof(dp_data_array));
                memcpy(dp_data_array,event.dp_write_data.p_data,dp_data_len);
                TUYA_APP_LOG_HEXDUMP_DEBUG("received dp write data :",dp_data_array,dp_data_len);
                tuya_ble_dp_data_report(dp_data_array,dp_data_len);//1
                break;
            case TUYA_BLE_CB_EVT_DP_DATA_REPORT_RESPONSE:
                TUYA_APP_LOG_INFO("received dp data report response result code =%d",event.dp_response_data.status);
                break;
            case TUYA_BLE_CB_EVT_DP_DATA_WTTH_TIME_REPORT_RESPONSE:
                TUYA_APP_LOG_INFO("received dp data report response result code =%d",event.dp_response_data.status);
                break;
            case TUYA_BLE_CB_EVT_DP_DATA_WITH_FLAG_REPORT_RESPONSE:
                TUYA_APP_LOG_INFO("received dp data with flag report response sn = %d , flag = %d , result code =%d",event.dp_with_flag_response_data.sn,event.dp_with_flag_response_data.mode
                                  ,event.dp_with_flag_response_data.status);
                break;
            case TUYA_BLE_CB_EVT_DP_DATA_WITH_FLAG_AND_TIME_REPORT_RESPONSE:
                TUYA_APP_LOG_INFO("received dp data with flag and time report response sn = %d , flag = %d , result code =%d",event.dp_with_flag_and_time_response_data.sn,
                                  event.dp_with_flag_and_time_response_data.mode,event.dp_with_flag_and_time_response_data.status);

                break;
            case TUYA_BLE_CB_EVT_UNBOUND:

                TUYA_APP_LOG_INFO("received unbound req");

                break;
            case TUYA_BLE_CB_EVT_ANOMALY_UNBOUND:

                TUYA_APP_LOG_INFO("received anomaly unbound req");

                break;
            case TUYA_BLE_CB_EVT_DEVICE_RESET:

                TUYA_APP_LOG_INFO("received device reset req");

                break;
            case TUYA_BLE_CB_EVT_DP_QUERY:
                TUYA_APP_LOG_INFO("received TUYA_BLE_CB_EVT_DP_QUERY event");
                if(dp_data_len>0)
                {
                    tuya_ble_dp_data_report(dp_data_array,dp_data_len);
                }
                break;
            case TUYA_BLE_CB_EVT_OTA_DATA:
                tuya_ota_proc(event.ota_data.type,event.ota_data.p_data,event.ota_data.data_len);
                break;
            case TUYA_BLE_CB_EVT_NETWORK_INFO:
                TUYA_APP_LOG_INFO("received net info : %s",event.network_data.p_data);
                tuya_ble_net_config_response(result);
                break;
            case TUYA_BLE_CB_EVT_WIFI_SSID:

                break;
            case TUYA_BLE_CB_EVT_TIME_STAMP:
                TUYA_APP_LOG_INFO("received unix timestamp : %s ,time_zone : %d",event.timestamp_data.timestamp_string,event.timestamp_data.time_zone);
                break;
            case TUYA_BLE_CB_EVT_TIME_NORMAL:

                break;
            case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH:
                TUYA_APP_LOG_HEXDUMP_DEBUG("received ble passthrough data :",event.ble_passthrough_data.p_data,event.ble_passthrough_data.data_len);
                tuya_ble_data_passthrough(event.ble_passthrough_data.p_data,event.ble_passthrough_data.data_len);
                break;
            default:
                TUYA_APP_LOG_WARNING("app_tuya_cb_queue msg: unknown event type 0x%04x",event.evt);
                break;
            }

            tuya_ble_event_response(&event);
        }
    }

}

void tuya_ble_app_init(void)
{
    os_task_create(&app_custom_task_handle, "app_custom", app_custom_task, 0, APP_CUSTOM_TASK_STACK_SIZE,APP_CUSTOM_TASK_PRIORITY);
    os_msg_queue_create(&tuya_custom_queue_handle, MAX_NUMBER_OF_TUYA_CUSTOM_MESSAGE, sizeof(tuya_ble_cb_evt_param_t));

    device_param.device_id_len = 16;    //If use the license stored by the SDK,initialized to 0, Otherwise 16 or 20.
    
    if(device_param.device_id_len==16)
    {
        memcpy(device_param.auth_key,(void *)auth_key_test,AUTH_KEY_LEN);
        memcpy(device_param.device_id,(void *)device_id_test,DEVICE_ID_LEN);
        memcpy(device_param.mac_addr.addr,mac_test,6);
        device_param.mac_addr.addr_type = TUYA_BLE_ADDRESS_TYPE_RANDOM;
    }
    
    device_param.p_type = TUYA_BLE_PRODUCT_ID_TYPE_PID;
    device_param.product_id_len = 8;
    memcpy(device_param.product_id,APP_PRODUCT_ID,8);
    device_param.firmware_version = TY_APP_VER_NUM;
    device_param.hardware_version = TY_HARD_VER_NUM;

    tuya_ble_sdk_init(&device_param);

    tuya_ble_callback_queue_register(tuya_custom_queue_handle);
    tuya_ble_device_delay_ms(200);
    tuya_ota_init();
    
    TUYA_APP_LOG_INFO("tuya ble demo version : "TUYA_BLE_DEMO_VERSION_STR);
    TUYA_APP_LOG_INFO("app version : "TY_APP_VER_STR);
 
}


