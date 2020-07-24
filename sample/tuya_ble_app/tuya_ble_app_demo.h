#ifndef TUYA_BLE_APP_DEMO_H_
#define TUYA_BLE_APP_DEMO_H_


#ifdef __cplusplus
extern "C" {
#endif


#define APP_PRODUCT_ID      "xxxxxxxx"

#define APP_BUILD_FIRMNAME  "tuya_ble_sdk_Demo_Project_rtl8762"

//固件版本
#define TY_APP_VER_NUM       0x0100
#define TY_APP_VER_STR	     "1.0" 	

//硬件版本
#define TY_HARD_VER_NUM      0x0100
#define TY_HARD_VER_STR	     "1.0" 	



void tuya_ble_app_init(void);


#ifdef __cplusplus
}
#endif

#endif // 

