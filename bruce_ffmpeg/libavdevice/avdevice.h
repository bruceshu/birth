/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-22
 * Description:
 
*********************************/


#ifndef AVDEVICE_H
#define AVDEVICE_H

typedef struct AVDeviceInfo {
    char *device_name;                   /**< device name, format depends on device */
    char *device_description;            /**< human friendly name */
} AVDeviceInfo;

typedef struct AVDeviceInfoList {
    AVDeviceInfo **devices;              /**< list of autodetected devices */
    int nb_devices;                      /**< number of autodetected devices */
    int default_device;                  /**< index of default device or -1 if no default */
} AVDeviceInfoList;

#endif
