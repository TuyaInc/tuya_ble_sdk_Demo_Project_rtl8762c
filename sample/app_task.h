/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_task.h
   * @brief     Routines to create App task and handle events & messages
   * @author    jane
   * @date      2017-06-02
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
#ifndef _APP_TASK_H_
#define _APP_TASK_H_

#include <stdbool.h>
#include "tuya_ble_type.h"

extern void driver_init(void);


bool tuya_event_queue_send(tuya_ble_evt_param_t *evt, uint32_t wait_ms);

/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init(void);

#endif

