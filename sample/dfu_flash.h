#ifndef flash_ota_h
#define flash_ota_h

#include <stdint.h>
#include <stdbool.h>

/*whether support ota image transfer during normal working mode.*/
#define OTA_NORMAL_MODE 1

/*#include "ota_service.h"*/
/*#define Signature_KERNEL 0x4B4E*/
/*#define Signature_USER   0x5552*/

#define DFU_HEADER_SIZE  12 /*currently, first 12 byte only will be treated as image header*/

#define IMG_HEADER_SIZE  1024

#define SHA256_BUFFER_SIZE 256
#define SHA256_LENGTH 32

bool isbufferchecksupport(void);
bool isAesEnable(void);
uint32_t dfu_report_target_fw_info(uint16_t signature, uint16_t *p_origin_fw_version,
                                   uint32_t *p_offset);
uint32_t dfu_report_target_ic_type(uint16_t signature, uint8_t *p_ic_type);
uint32_t silence_ota_update(uint16_t signature, uint32_t offset, uint32_t length, void *p_void);
uint32_t dfu_flash_erase(uint16_t signature, uint32_t offset);
uint32_t dfu_checkbufcrc(uint8_t *buf, uint32_t length, uint16_t mCrcVal);
bool dfu_reset(uint16_t signature);
void dfu_fw_active_reset(void);
bool dfu_check_checksum(uint16_t signature);
uint32_t dfu_get_bank_size(uint16_t signature);
//void dfu_init(void);
void dfu_switch_to_ota_mode(void);
bool dfu_get_fw_version(uint16_t signature, uint16_t *p_origin_fw_version);
void dfu_prepare_image(void);
bool dfu_erase_img_flash_area(uint32_t start_addr, uint32_t size, bool with_semaphore);
void dfu_check_ota_tmp_flash(uint16_t signature);
#endif
