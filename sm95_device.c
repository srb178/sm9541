#include "sm95_device.h"

#define DBG_TAG  "sm95.dev"
#define DBG_LVL   DBG_LOG
#include <rtdbg.h>


static double pressure9541;             /* actual pressure */  
static double temperature9541;          /* actual temperature */


/*******************************************************************************
Fuc: sm95 sensor read register
*******************************************************************************/ 
static rt_err_t sm95_read_regs(rt_sensor_t psensor, rt_uint8_t *data, rt_uint8_t data_size)
{
    struct rt_i2c_msg msg[1];
    struct rt_i2c_bus_device  *i2c_bus = RT_NULL;
    rt_uint32_t slave_addr = 0;

    /* get i2c slave address */
    slave_addr = (rt_uint32_t)psensor->config.intf.user_data; 
    /* get i2c bus device */   
    i2c_bus = (struct rt_i2c_bus_device *)psensor->parent.user_data; /*void *user_data; device private data */
    
    
    msg[0].addr =(rt_uint8_t)slave_addr;
    msg[0].flags = RT_I2C_RD;
    msg[0].buf = data;    /*pointer for read the data buffer*/  
    msg[0].len = data_size;
    
    /*return for the number of message if succeeded*/
    if(rt_i2c_transfer(i2c_bus, msg, 1) == 1)  
    {
        return RT_EOK;
    }
    else
    {
        LOG_E("i2c2 bus read failed!\r\n");
        return -RT_ERROR;
    } 
}

/*******************************************************************************
Fuc: sm95 sensor read original data
*******************************************************************************/  
static rt_err_t sm95_read_adc(rt_sensor_t psensor, double* p9541, double* t9541)
{
	rt_uint8_t  buf95_read[4] = {0};
    rt_uint8_t  status;
    rt_err_t ans;
    float dat = 0.0f;
    rt_int32_t pressure_adc95;       /* pressure adc */
    rt_int32_t temperature_adc95;    /* temperature adc */
    
    
    ans = sm95_read_regs(psensor, buf95_read, 4);     
    if(ans == RT_EOK)
    {
        status = buf95_read[0]>>6;
        if(status == 0)
        {
            pressure_adc95 = ((buf95_read[0]&0x3f)<<8) | buf95_read[1];
            temperature_adc95 = (buf95_read[2]<<3) | (buf95_read[3]>>5);
              
            dat = ( psensor->info.range_max  - psensor->info.range_min ) /10;
            *p9541 = (pressure_adc95 - SM9541_MINCOUNT)*dat / (SM9541_MAXCOUNT - SM9541_MINCOUNT) + psensor->info.range_min/10;
            *t9541 = (float)temperature_adc95*200/2047 - 50;
            return RT_EOK;
        }           
    }
    return RT_ERROR;
}


/*******************************************************************************
Fuc: sm95 read for actual P/T data
*******************************************************************************/ 
static rt_size_t sm95_fetch_data(struct rt_sensor_device *psensor, void *buf, rt_size_t len )
{
    RT_ASSERT(buf);
    RT_ASSERT(psensor); 
    struct rt_sensor_data *sensor_data = buf;     
     
    if(len != 0)
    {
      /* One shot only read a data */
        if(psensor->config.mode == RT_SENSOR_MODE_POLLING) 
        {
            if(psensor->info.type == RT_SENSOR_CLASS_BARO)
            {
                /* actual pressure */            
                if(sm95_read_adc(psensor, &pressure9541, &temperature9541) == RT_EOK)
                { 
                    sensor_data->type = RT_SENSOR_CLASS_BARO;
                    if(len == 1)
                    {
                       
                        if(POINT_NUM == 4)
                        {
                            sensor_data->data.baro = pressure9541*10000;
                        }
                        else if(POINT_NUM == 3)
                        {
                            sensor_data->data.baro = pressure9541*1000;
                        }
                        else if(POINT_NUM == 2)
                        {
                            sensor_data->data.baro = pressure9541*100;
                        }
                        sensor_data->timestamp = rt_sensor_get_ts();
                        //LOG_I("sm9541 fetch data finished! \r\n");
                    }
                    else if(len == 2)
                    {
                        if(POINT_NUM == 4)
                        {
                            sensor_data->data.baro = pressure9541*10000;
                        }
                        else if(POINT_NUM == 3)
                        {
                            sensor_data->data.baro = pressure9541*1000;
                        }
                        else if(POINT_NUM == 2)
                        {
                            sensor_data->data.baro = pressure9541*100;
                        }
                        sensor_data->timestamp = rt_sensor_get_ts();
                        
                        sensor_data++;
                        sensor_data->type = RT_SENSOR_CLASS_TEMP;                
                        sensor_data->data.temp = (temperature9541*100); 
                        sensor_data->timestamp = rt_sensor_get_ts();
                        //LOG_I("sm9541 fetch data finished! \r\n");
                    }                         
                    else
                    {
                        LOG_E("input para 'len' illegal! \r\n");
                    }
                }
                else
                {
                    LOG_W("sm9541 fetch adc data wrong!\r\n");
                    return 0;
                }             
            }       
        }  
    }
    else
    {
        LOG_W(" input 'len' cannot be empty! ");
    }
    return 1;
}


/*******************************************************************************
Fuc: sm95 sensor control
*******************************************************************************/ 
static rt_err_t sm95_control(struct rt_sensor_device *psensor, int cmd, void *args)
{
    rt_err_t ret = RT_EOK;

    return ret;      
}
 
static struct rt_sensor_ops  sm95_sensor_ops =
{
    sm95_fetch_data,
    sm95_control,
};


/*******************************************************************************
Fuc: initialize sm9541 sensor
*******************************************************************************/
rt_err_t  sm9541_device_init(const char* name, struct rt_sensor_config *cfg )
{
    rt_err_t ret = RT_EOK;
    rt_sensor_t  sensor_temp = RT_NULL;  /*apply for the memory of sensor class*/ 
    struct rt_i2c_bus_device *sm95_i2c_bus = RT_NULL;
    
    sm95_i2c_bus = rt_i2c_bus_device_find(cfg->intf.dev_name);
    if(sm95_i2c_bus == RT_NULL)
    {
        LOG_E("i2c2 bus device %s not found! \r\n", cfg->intf.dev_name);
        ret = -RT_ERROR;
        goto _exit;        
    }
   
    /*sm9541 sensor register*/
    {
        sensor_temp = rt_calloc(1, sizeof(struct rt_sensor_device));
        if(sensor_temp == RT_NULL)
        {
            goto _exit;
        }
        rt_memset(sensor_temp, 0x0, sizeof(struct rt_sensor_device));
        
        sensor_temp->info.type = RT_SENSOR_CLASS_BARO;
        sensor_temp->info.vendor = RT_SENSOR_VENDOR_UNKNOWN;
        sensor_temp->info.model = "sm9541";    /* model name of sensor */
        sensor_temp->info.unit = RT_SENSOR_UNIT_PA;
        sensor_temp->info.intf_type = RT_SENSOR_INTF_I2C;
        sensor_temp->info.range_max = SM9541_PRESSURE_MAX*10;
        sensor_temp->info.range_min = SM9541_PRESSURE_MIN*10;
        
        
        /* read [1000/SM9541_PRESSURE_PERIOD] times in 1 second */
        sensor_temp->info.period_min = SM9541_PRESSURE_PERIOD;
     
        rt_memcpy( &sensor_temp->config, cfg, sizeof(struct rt_sensor_config) );
        sensor_temp->ops = &sm95_sensor_ops;
        
        /* underlying register */
        ret = rt_hw_sensor_register(sensor_temp, name, RT_DEVICE_FLAG_RDWR, (void *)sm95_i2c_bus/* private data*/);
        if(ret != RT_EOK)
        {
            LOG_E( "sm9541 device register err code: %d", ret );
            goto _exit;
        }
        else
        {
            LOG_I("sm9541 register succeed! \r\n");
        }
    }
    
    return RT_EOK;
    
    _exit:
        if(sensor_temp)
        {
            rt_free(sensor_temp);
        }                    
        return ret;
}

#ifdef   PKG_USING_SM95_SENSOR
static int rt_hw_sm9541_port(void)
{
    struct rt_sensor_config cfg;
        
    cfg.intf.dev_name = "i2c2";            /* i2c bus */
    cfg.intf.user_data = (void *)0x28;     /* i2c slave addr */
    sm9541_device_init(SM9541_DEVICE_NAME, &cfg);  /* sm9541 */

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_sm9541_port);  
#endif   /*PKG_USING_SM95_SENSOR*/
