/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: remote_control_command.h
 * @Description: common function include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.10.09 14:28:01
 * @LastEditTime: 2021.10.09 17:28:01
 */

#ifndef _LIGHT_CONTROL_COMMAND_H_
#define _LIGHT_CONTROL_COMMAND_H_

#include "tuya_zigbee_sdk.h"
#include "zigbee_dev_template.h"


void  remote_control_add_group(uint16_t dst_addr,uint8_t dst_ep,uint16_t group_id);

void  remote_control_group_on(uint16_t group_id,uint8_t dst_ep);

void  remote_control_binding_on(void);

void  remote_control_group_off(uint16_t group_id,uint8_t dst_ep);

void  remote_control_binding_off(void);

void  remote_control_group_move_to_level(uint16_t group_id,uint8_t dst_ep,uint8_t level,uint16_t tran_time);

void  remote_control_group_level_step(uint16_t group_id,uint8_t dst_ep,uint8_t mode,uint8_t size,uint16_t tran_time);

void  remote_control_binding_move_to_level(uint8_t level,uint16_t tran_time);

void  remote_control_group_move_level(uint16_t group_id,uint8_t dst_ep,uint8_t mode,uint8_t rate);

void  remote_control_binding_move_level(uint8_t mode,uint8_t rate);

void  remote_control_group_stop_level(uint16_t group_id,uint8_t dst_ep);

void  remote_control_binding_stop_level(void);

void  remote_control_group_light_mode_set(uint16_t dst_addr,uint8_t dst_ep,uint8_t light_mode);

void  remote_control_group_light_mode_group_set(uint16_t group_id,uint8_t dst_ep,uint8_t light_mode);

void  remote_control_group_move_color_temper(uint16_t group_id,uint8_t dst_ep,uint16_t temper,uint16_t tran_time);

uint8_t level_control_level_set_up(uint8_t mode,uint8_t step);

void enhanced_add_scene(uint16_t dstAddr,uint16_t groupId,uint8_t sceneId);

void private_recall_scene(uint16_t dstAddr,uint8_t sceneId);

void group_recall_scene(uint16_t groupId,uint8_t sceneId);

#endif
