 /*!
  *     COPYRIGHT NOTICE
  *     Copyright (c) 2013,山外科技
  *     All rights reserved.
  *     技术讨论：山外论坛 http://www.vcan123.com
  *
  *     除注明出处外，以下所有内容版权均属山外科技所有，未经允许，不得用于商业用途，
  *     修改内容时必须保留山外科技的版权声明。
  *
  * @file       main.c
  * @brief      山外K60 平台主程序
  * @author     山外科技
  * @version    v5.0
  * @date       2013-08-28
  */

#include "common.h"
#include "include.h"

uint8  nrf_rx_buff[CAMERA_SIZE + 2*COM_LEN + DATA_PACKET];         //预多
//uint8  nrf_tx_buff[CAMERA_SIZE + 2*COM_LEN + DATA_PACKET];         //预多
uint8 *imgbuff = (uint8 *)(((uint8 *)&nrf_rx_buff) + COM_LEN);     //图像地址


//函数声明
void PIT0_IRQHandler();
void PORTE_IRQHandler();


uint8  var1 , var2;
uint16 var3, var4;
uint32 var5, var6;

int16 var_wire[6]={0};

//处理NRF接受到的数据
void recive_nrf()
{
  var_wire[0]=var3;               ///  uint16
  var_wire[1]=var5;               //uint32
//  var_wire[2]=var3;
//  var_wire[3]=var4;
//  var_wire[4]=var5;
//  var_wire[5]=var6;
  if(var1==1) var_wire[3]=-var4;
  else var_wire[3]=var4;
  if(var2==1) var_wire[5]=-var6;
  else var_wire[5]=var6;
}

/*!
 *  @brief      main函数
 *  @since      v5.0
 *  @note       山外 无线调试 测试实验
 */
void  main(void)
{
    Site_t site     = {0, 0};                           //显示图像左上角位置
    Size_t imgsize  = {CAMERA_W, CAMERA_H};             //图像大小
    Size_t size     = {80, 60}; //{LCD_W, LCD_H};       //显示区域图像大小
    uint32 i;
    com_e     com;
    nrf_result_e nrf_result;

    /************************ 配置 K60 的优先级  ***********************/
    //K60 的默认优先级 都为 0
    //参考帖子：急求中断嵌套的例程 - 智能车讨论区 - 山外论坛
    //          http://www.vcan123.com/forum.php?mod=viewthread&tid=499&page=1#pid3270
    NVIC_SetPriorityGrouping(4);            //设置优先级分组,4bit 抢占优先级,没有亚优先级

    NVIC_SetPriority(PORTE_IRQn,0);         //配置优先级
    NVIC_SetPriority(PIT0_IRQn,1);          //配置优先级


    /************************ LCD 液晶屏 初始化  ***********************/
    LCD_init();

    /************************ 无线模块 NRF 初始化  ***********************/
    while(!nrf_init());
    //配置中断服务函数
    set_vector_handler(PORTE_VECTORn ,PORTE_IRQHandler);    			//设置 PORTE 的中断服务函数为 PORTE_VECTORn
    enable_irq(PORTE_IRQn);
    nrf_msg_init();                                                     //无线模块消息初始化


    /*********************** 按键消息 初始化  ***********************/
    key_event_init();                   //按键消息初始化
    pit_init_ms(PIT0,10);               //pit 定时中断(用于按键定时扫描)
    set_vector_handler(PIT0_VECTORn ,PIT0_IRQHandler);    //设置 PIT0 的中断服务函数为 PIT0_IRQHandler
    enable_irq(PIT0_IRQn);

    while(1)
    {
//      printf("666");
        /************************ 无线发送和接收数据  ***********************/
        do
        {
            nrf_result = nrf_msg_rx(&com, nrf_rx_buff);

        }while(nrf_result != NRF_RESULT_RX_NO);
        //--------处理接收数据-----------
        recive_nrf();
        vcan_sendware(var_wire, sizeof(var_wire));

        /*********************** 按键消息 处理  ***********************/
        deal_key_event();

    }
}

/*

知识点 提醒;

1.鹰眼 摄像头 采集回来的图像是 压缩图像，需要进行解压，参考如下 文章:
        鹰眼图像解压，转为 二维数组 - 智能车资料区 - 山外论坛
        http://www.vcan123.com/forum.php?mod=viewthread&tid=17


2.图像 地址 转换为 二维数组。
        如果是 鹰眼，参考 第1个知识点说明

        如果是其他 灰度图像摄像头，修改如下代码:
        uint8 *imgbuff = (uint8 *)(((uint8 *)&nrf_buff) + COM_LEN);     //图像地址
        改为
        uint8 (*imgbuff)[CAMERA_H][CAMERA_W] =   (uint8 (*)[CAMERA_H][CAMERA_W])(((uint8 *)&nrf_buff) + COM_LEN);     //图像地址
        实际上 就是强制 类型转换。

3.摄像头的采集地址 是在 摄像头初始化是传递进去的，此后的图像都采集到该地址上。
        camera_init(imgbuff);                                   //摄像头初始化，把图像采集到 imgbuff 地址

4.无线模块，检测数据接收后，才进行 发送，这样可以尽量避免 无线模块同时出于 发送状态。

5.嵌套中断的问题，参考如下帖子:
        急求中断嵌套的例程 - 智能车讨论区 - 山外论坛
        http://www.vcan123.com/forum.php?mod=viewthread&tid=499&page=1#pid3270

6.按键扫描，是采用定时扫描方式进行扫描的


*/

/*!
 *  @brief      PORTE中断服务函数
 *  @since      v5.0
 */
void PORTE_IRQHandler()
{
    uint8  n;    //引脚号
    uint32 flag;

    flag = PORTE_ISFR;
    PORTE_ISFR  = ~0;                                   //清中断标志位

    n = 27;
    if(flag & (1 << n))                                 //PTE27触发中断
    {
        nrf_handler();
    }
}


/*!
 *  @brief      PIT0中断服务函数
 *  @since      v5.0
 */
void PIT0_IRQHandler()
{
    key_IRQHandler();

    PIT_Flag_Clear(PIT0);
}

