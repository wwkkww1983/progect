#ifndef __display_zq_H__
#define __display_zq_H__

///////////////////  函数函数外部 /////////////////////////////
//extern
extern void LCD_show();
extern void lcd_show_1();
extern void lcd_show_3();
extern void lcd_show_2();
extern void lcd_show_4();      //主要用于采集图像显示赛道信息
extern void lcd_show_0();
extern void lcd_show_5();           //主要用于显示发车图像

extern void  bmp_init();
extern void bmp_display();

extern void color_change();

///////////////////  变量外部声明   ///////////////////////////
extern char color[10];
extern int8 key_flag_clear;



#endif
