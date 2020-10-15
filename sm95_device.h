#ifndef _SM95_DEVICE_H__ 
#define _SM95_DEVICE_H__ 

#define  POINT_NUM  3    /*accuracy of pressure data*/

#include <rtthread.h>
#include <rtdevice.h>
#include <rtdef.h>

#include "sensor.h"

struct sm95_device
{
    rt_sensor_t sensor_p; 
    uint8_t update;      /*the time for update (extern device)*/
};
typedef struct sm95_device* sm95_dev_t;



/* sm device hardware info */
#define SM9541_ADDR             0x28 	         /* i2c slave address */

/* sm9541 register value */
#define SM9541_MAXCOUNT         14745
#define SM9541_MINCOUNT         1638
#define SM9541_I2C_BUS          "i2c2"           /* i2c linked */            
#define SM9541_DEVICE_NAME      "sm9541_sensor"  /* register sensor device name*/     
    
/* sm9541 dev info for RT-Thread sensor device   */
#define SM9541_PRESSURE_MAX     980.7  /*980.7*/
#define SM9541_PRESSURE_MIN     -49L
#define SM9541_PRESSURE_PERIOD  100    /*10ms */  /* read ten times in 1 second */
    


rt_err_t sm9541_device_init(const char* name, struct rt_sensor_config *cfg);


#endif
