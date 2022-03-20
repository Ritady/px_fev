
#include "tuya_zigbee_sdk.h"
#include "zigbee_dev_template.h"
#include "remote_control_command.h"

#define TY_CMD_SET_COLOR_MODE_CMD_ID            0xF0    //change mode cmd: white/color/scene/music
#define TY_CMD_SET_COLOR_SCENE_CMD_ID           0xF1    //scene data cmd
#define TY_CMD_SET_COLOR_REALTIME_DATA_CMD_ID   0xF2    //real time ctrl cmd: mode(1) + H(2) + S(2) + V(2) + B(2) + T(2) = 11 bytes

#define  ZCL_ID_ADD_GROUP          100
#define  ZCL_ID_LIGHT_MODE         101
#define  ZCL_ID_GROUP_LIGHT_MODE   102
#define  ZCL_ID_GROUP_ON           103
#define  ZCL_ID_GROUP_OFF          104
#define  ZCL_ID_GROUP_LEVEL        105
#define  ZCL_ID_GROUP_TEMPER       106
#define  ZCL_ID_ADD_SCENE          107
#define  ZCL_ID_RECALL_SCENE       108
#define  ZCL_ID_GROUP_RECALL_SCENE 109



uint8_t  pre_config_bright_level = 101;
uint16_t pre_config_group = 0x4003;
uint8_t colorExtention[4][13] = {{0x01,0x90,0x75,0x80,0x4a,0x6a,0x00,0x00,0xfe,0x00,0x00,0x00,0x00},
                                 {0x01,0xfe,0xeb,0x71,0x1d,0x68,0x00,0x00,0xfe,0x00,0x00,0x00,0x00},
                                 {0x01,0xfe,0x76,0x5e,0x34,0x5f,0x00,0x00,0xfe,0x00,0x00,0x00,0x00},
                                 {0x01,0xfe,0x76,0x50,0x81,0x54,0x00,0x00,0xfe,0x00,0x00,0x00,0x00}};

void remote_control_add_group(uint16_t dst_addr, uint8_t dst_ep, uint16_t group_id)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_ADD_GROUP;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_ADD_GROUP_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_DEV;
    send_data.addr.type.dev.dest_addr = dst_addr;
    send_data.addr.type.dev.src_ep = 1;
    send_data.addr.type.dev.dest_ep = dst_ep;
    send_data.addr.type.dev.cluster_id = CLUSTER_GROUPS_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = (group_id  & 0xFF);
    send_data.data.private.data[i++] = (group_id >> 8) & 0xFF;
    send_data.data.private.data[i++] = 0;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_group_light_mode_set(uint16_t dst_addr, uint8_t dst_ep, uint8_t light_mode)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_LIGHT_MODE;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = TY_CMD_SET_COLOR_MODE_CMD_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_DEV;
    send_data.addr.type.dev.dest_addr = dst_addr;
    send_data.addr.type.dev.src_ep = 1;
    send_data.addr.type.dev.dest_ep = dst_ep;
    send_data.addr.type.dev.cluster_id = CLUSTER_COLOR_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = light_mode;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_group_light_mode_group_set(uint16_t group_id, uint8_t dst_ep, uint8_t light_mode)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_GROUP_LIGHT_MODE;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = TY_CMD_SET_COLOR_MODE_CMD_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GROUP;
    send_data.addr.type.group.group_id = group_id;
    send_data.addr.type.group.src_ep = 1;
    send_data.addr.type.group.dest_ep = dst_ep;
    send_data.addr.type.group.cluster_id = CLUSTER_COLOR_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = light_mode;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_group_on(uint16_t group_id,uint8_t dst_ep)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_VIP_1;//QOS_VIP_1
    send_data.zcl_id = ZCL_ID_GROUP_ON;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_ON_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GROUP;
    send_data.addr.type.group.group_id = group_id;
    send_data.addr.type.group.src_ep = 1;
    send_data.addr.type.group.dest_ep = dst_ep;
    send_data.addr.type.group.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_binding_on(void)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_GROUP_ON;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_ON_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_BINDING;
    send_data.addr.type.bind.src_ep = 1;
    send_data.addr.type.bind.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_group_off(uint16_t group_id, uint8_t dst_ep)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_VIP_1;//QOS_VIP_1
    send_data.zcl_id = ZCL_ID_GROUP_OFF;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_OFF_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GROUP;
    send_data.addr.type.group.group_id = group_id;
    send_data.addr.type.group.src_ep = 1;
    send_data.addr.type.group.dest_ep = dst_ep;
    send_data.addr.type.group.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_binding_off(void)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_GROUP_OFF;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_OFF_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_BINDING;
    send_data.addr.type.bind.src_ep = 1;
    send_data.addr.type.bind.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_group_move_to_level(uint16_t group_id, uint8_t dst_ep, uint8_t level, uint16_t tran_time)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_VIP_1;//
    send_data.zcl_id = ZCL_ID_GROUP_LEVEL;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_MOVE_TO_LEVEL_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GROUP;
    send_data.addr.type.group.group_id = group_id;
    send_data.addr.type.group.src_ep = 1;
    send_data.addr.type.group.dest_ep = dst_ep;
    send_data.addr.type.group.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = level;
    send_data.data.private.data[i++] = (tran_time  & 0xFF);
    send_data.data.private.data[i++] = (tran_time >> 8) & 0xFF;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_group_level_step(uint16_t group_id, uint8_t dst_ep, uint8_t mode, uint8_t size, uint16_t tran_time)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_VIP_1;//
    send_data.zcl_id = ZCL_ID_GROUP_LEVEL;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_STEP_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GROUP;
    send_data.addr.type.group.group_id = group_id;
    send_data.addr.type.group.src_ep = 1;
    send_data.addr.type.group.dest_ep = dst_ep;
    send_data.addr.type.group.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = mode;
     send_data.data.private.data[i++] = size;
    send_data.data.private.data[i++] = (tran_time  & 0xFF);
    send_data.data.private.data[i++] = (tran_time >> 8) & 0xFF;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_binding_move_to_level(uint8_t level, uint16_t tran_time)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));
    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_GROUP_LEVEL;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_MOVE_TO_LEVEL_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_BINDING;
    send_data.addr.type.bind.src_ep = 1;
    send_data.addr.type.bind.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = level;
    send_data.data.private.data[i++] = (tran_time  & 0xFF);
    send_data.data.private.data[i++] = (tran_time >> 8) & 0xFF;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_group_move_level(uint16_t group_id, uint8_t dst_ep, uint8_t mode, uint8_t rate)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    //send_data.zcl_id = ZCL_ID_GROUP_LEVEL;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_MOVE_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GROUP;
    send_data.addr.type.group.group_id = group_id;
    send_data.addr.type.group.src_ep = 1;
    send_data.addr.type.group.dest_ep = dst_ep;
    send_data.addr.type.group.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = mode;
    send_data.data.private.data[i++] = rate;
    send_data.data.private.len = i;
    //dev_zigbee_clear_send_data(3, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_binding_move_level(uint8_t mode, uint8_t rate)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    //send_data.zcl_id = ZCL_ID_GROUP_LEVEL;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_MOVE_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_BINDING;
    send_data.addr.type.bind.src_ep = 1;
    send_data.addr.type.bind.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = mode;
    send_data.data.private.data[i++] = rate;
    send_data.data.private.len = i;
    //dev_zigbee_clear_send_data(3, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_group_stop_level(uint16_t group_id, uint8_t dst_ep)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    //send_data.zcl_id = ZCL_ID_GROUP_LEVEL;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_STOP_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GROUP;
    send_data.addr.type.group.group_id = group_id;
    send_data.addr.type.group.src_ep = 1;
    send_data.addr.type.group.dest_ep = dst_ep;
    send_data.addr.type.group.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.len = i;
    //dev_zigbee_clear_send_data(3, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_binding_stop_level(void)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    //send_data.zcl_id = ZCL_ID_GROUP_LEVEL;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_STOP_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_BINDING;
    send_data.addr.type.bind.src_ep = 1;
    send_data.addr.type.bind.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.len = i;
    //dev_zigbee_clear_send_data(3, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void  remote_control_group_move_color_temper(uint16_t group_id, uint8_t dst_ep, uint16_t temper, uint16_t tran_time)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_GROUP_TEMPER;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_MOVE_TO_COLOR_TEMPERATURE_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GROUP;
    send_data.addr.type.group.group_id = group_id;
    send_data.addr.type.group.src_ep = 1;
    send_data.addr.type.group.dest_ep = dst_ep;
    send_data.addr.type.group.cluster_id = CLUSTER_COLOR_CONTROL_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = (temper  & 0xFF);
    send_data.data.private.data[i++] = (temper >> 8) & 0xFF;
    send_data.data.private.data[i++] = (tran_time  & 0xFF);
    send_data.data.private.data[i++] =  (tran_time >> 8) & 0xFF;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void enhanced_add_scene(uint16_t dstAddr, uint16_t groupId, uint8_t sceneId)
{
    dev_send_data_t send_data;
    uint8_t i = 0,j = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_ADD_SCENE;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_ENHANCED_ADD_SCENE_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_DEV;
    send_data.addr.type.dev.dest_addr = dstAddr;
    send_data.addr.type.dev.src_ep = 1;
    send_data.addr.type.dev.dest_ep = 0xff;
    send_data.addr.type.dev.cluster_id = CLUSTER_SCENES_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = (groupId  & 0xFF);
    send_data.data.private.data[i++] = (groupId >> 8) & 0xFF;
    send_data.data.private.data[i++] = sceneId;
    send_data.data.private.data[i++] = 4;//trans time
    send_data.data.private.data[i++] = 0;//scent name
    send_data.data.private.data[i++] = 0;//scent name
    send_data.data.private.data[i++] = 0x06;//on-off extention
    send_data.data.private.data[i++] = 0x00;
    send_data.data.private.data[i++] = 1;
    send_data.data.private.data[i++] = colorExtention[sceneId][0];

    send_data.data.private.data[i++] = 0x08;//level extention
    send_data.data.private.data[i++] = 0x00;
    send_data.data.private.data[i++] = 1;
    send_data.data.private.data[i++] = colorExtention[sceneId][1];

    send_data.data.private.data[i++] = 0x00;//color extention
    send_data.data.private.data[i++] = 0x03;
    send_data.data.private.data[i++] = 11;//length
    for(j = 0;j < 11;j++){
        send_data.data.private.data[i++] = colorExtention[sceneId][j+2];
    }
    send_data.data.private.len = i;
    //dev_zigbee_clear_send_data(3, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void private_recall_scene(uint16_t dstAddr, uint8_t sceneId)
{
    dev_send_data_t send_data;
    uint8_t i = 0;
    memset(&send_data, 0, sizeof(dev_send_data_t));
    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_RECALL_SCENE;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = 0xFD;
    send_data.addr.type.dev.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    if(0 != dstAddr){
      send_data.addr.mode = SEND_MODE_DEV;
      send_data.addr.type.dev.src_ep = 1;
      send_data.addr.type.dev.dest_ep = 0xff;
      send_data.addr.type.dev.dest_addr = dstAddr;
    }
    else{
      send_data.addr.mode = SEND_MODE_GW;
      send_data.addr.type.gw.src_ep = 1;
    }
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = sceneId;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

void group_recall_scene(uint16_t groupId, uint8_t sceneId)
{
    dev_send_data_t send_data;
    uint8_t i = 0;

    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = QOS_1;
    send_data.zcl_id = ZCL_ID_GROUP_RECALL_SCENE;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_RECALL_SCENE_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GROUP;
    send_data.addr.type.group.group_id = groupId;
    send_data.addr.type.group.src_ep = 1;
    send_data.addr.type.group.dest_ep = 0xff;
    send_data.addr.type.group.cluster_id = CLUSTER_SCENES_CLUSTER_ID;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = (groupId  & 0xFF);
    send_data.data.private.data[i++] = (groupId >> 8) & 0xFF;
    send_data.data.private.data[i++] = sceneId;
    send_data.data.private.len = i;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 3000);
}

uint8_t level_control_level_set_up(uint8_t mode, uint8_t step)
{
    static bool_t brightLevelDefaultRun = FALSE;
    if(brightLevelDefaultRun == FALSE)
    {
        brightLevelDefaultRun = TRUE;
         return pre_config_bright_level;
    }
    if(mode == 0)//up
    {
        if(pre_config_bright_level > (254 - step))
        {
            pre_config_bright_level = 254;
        } else {
            pre_config_bright_level += step;
        }
    }
    else if(mode == 1)//down
    {
        if((pre_config_bright_level - step) <= 55)
        {
            pre_config_bright_level = 50;
        } else {
            pre_config_bright_level -= step;
        }
    }
    return pre_config_bright_level;
}

