## SM41数字压力传感器

#### 1软件包简介

sm41软件包是基于RT-Thread sensor 框架实现的一个驱动包。

基于该软件包，用户应用程序可使用标准的sensor接口访问sm9541/sm3041,  获取传感器数据。



#### 2目录结构

| 名称          | 说明           |
| :------------ | -------------- |
| sm95_device.c | sm9541头文件   |
| sm95_device.h | sm9541源文件   |
| sm30_device.c | sm3041头文件   |
| sm30_device.h | sm3041源文件   |
| sys_test.c    | 测试应用程序   |
| README.md     | 软件包使用说明 |



#### 3传感器介绍

SM3041系列

| 功能 | 量程       | 精度  |
| :--- | :--------- | ----- |
| 气压 | 98—1373 Pa | 0.001 |
| 温度 | -20—+85 ℃  | /     |

SM9541系列

| 功能 | 量程              | 精度  |
| ---- | ----------------- | ----- |
| 气压 | -49.0 — +980.7 Pa | 0.001 |
| 温度 | -5 — +65 ℃        | 0.1   |





#### 4支持情况

| 包含设备     | 气压计 | 温度计 |
| ------------ | ------ | ------ |
| 通信接口     |        |        |
| IIC          | √      | √      |
| SPI          |        |        |
| **工作模式** |        |        |
| 轮询         | √      | √      |
| 中断         |        |        |
| FIFO         |        |        |





#### 5使用说明

##### 5.1依赖

- RT-Thread 4.0.0+

- sensor 框架组件

- I2C驱动，sm9541/sm3041 设备使用I2C进行数据通讯，需要I2C驱动框架支持



##### 5.2获取传感器软件包

使用 sm41 package 需要在 RT-Thread 的包管理器中选择，具体路径如下。

之后使用 `pkgs --update` 命令更新包到 BSP 中。

```python
RT-Thread online packages --->
    peripheral libraries and drivers --->
        sensors drivers --->
            [*] sm9541: sm9541 sensor driver package, support: barometric,temperature.
                    Version (latest)  --->
            [*] sm3041: sm3041 sensor driver package, support: barometric,temperature.
                    Version (latest)  --->
```

**Version**：软件包版本选择，默认选择最新版本。





##### 5.3参考样例（以sm9541为例）

sm9541 软件包初始化函数如下所示：

```c
rt_err_t  sm9541_device_init(const char* name, struct rt_sensor_config *cfg )
```

该函数需由用户调用，主要完成的功能:

- 根据配置信息配置i2c名称、i2c地址等（可增加其他配置信息），然后初始化设备
- 注册相应的传感器设备，完成 sm95 设备的注册

```c
#include "sm95_device.h"

static int rt_hw_sm9541_port(void)
{
    struct rt_sensor_config cfg;
        
    cfg.intf.dev_name = "i2c2";         /* i2c bus */
    cfg.intf.user_data = (void *)0x28;  /* i2c slave addr */
    sm9541_device_init("sm9541", &cfg);      /* sm9541 */

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_sm9541_port);
```







##### 5.4读取压力/温度数据

该软件包基于sensor框架，而sensor框架继承于RT-Thread标准设备框架，

故可以使用RT-Thread标准设备接口" find/open/read "读取传感器数据。

参考伪代码：

```c
/* 查找已初始化（注册）设备 */
sm95_dev = rt_device_find("baro_sm9541_sensor"); 
/* 打开设备 */
rt_device_open(sm95_dev, RT_DEVICE_FLAG_RDWR);
/* 读取所需数据 */
rt_device_read(sm95_dev, 0, &sm95_data, 2);
rt_device_read(sm95_dev, 0, &sm95_data, 1);
```







##### 5.5参考样例运行结果

输出格式： 数据 ,  timestamp



![](C:\Users\ruibi\Desktop\sample.png)



#### 6注意事项

暂无