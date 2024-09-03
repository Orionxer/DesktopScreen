#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ft6336.h"
#include "bsp_i2c.h"
#include "debug_log.h"
#include "bsp_gpio.h"
#include "esp_err.h"
#include "screen.h"


//触摸芯片最大5组触摸点，FT6335最大支持双触
const uint16_t FT6236_TPX_TBL[5]=
{
	FT_TP1_REG,
	FT_TP2_REG,
	FT_TP3_REG,
	FT_TP4_REG,
	FT_TP5_REG
};

TouchPoint_T gTPS;

//扫描触摸屏寄存器状态、数据
static void scan_ft6336()
{
    uint8_t i = 0;
    uint8_t sta = 0;
    uint8_t buf[4] = {0};
    uint8_t gestid = 0;
    // 读取触摸点的状态
    ESP_ERROR_CHECK(touch_i2c_write_read(0x02, &sta, 1));
    ESP_ERROR_CHECK(touch_i2c_write_read(0x01, &gestid, 1));
    gTPS.touch_count = sta;
    // 最大支持两点触摸
    // DBG_LOGI("Touch Point Number = %d", gTPS.touch_count);
    // 判断是否有触摸点按下，0x02寄存器的低4位表示有效触点个数
    if (sta & 0x0f)
    {
        //~(0xFF << (sta & 0x0F))将点的个数转换为触摸点按下有效标志
        gTPS.touch_sta = ~(0xFF << (sta & 0x0F));
        // 分别判断触摸点1-5是否被按下
        for (i = 0; i < 2; i++)                   
        {
            // 读取触摸点坐标
            if (gTPS.touch_sta & (1 << i))                        
            {                                                     
                // 读取被按下则读取对应触摸点XY坐标值
                ESP_ERROR_CHECK(touch_i2c_write_read(FT6236_TPX_TBL[i], buf, 4));
                gTPS.x[i] = ((uint16_t)(buf[0] & 0X0F) << 8) + buf[1];
                gTPS.y[i] = ((uint16_t)(buf[2] & 0X0F) << 8) + buf[3];
                // printf("%x %x %x %x x=%d y=%d\n",buf[0],buf[1],buf[2],buf[3],gTPS.x[i],gTPS.y[i]);
                // if((buf[0]&0XC0)!=0X80)
                // {
                // 	gTPS.x[i]=gTPS.y[i]=0;//必须是contact事件，才认为有效
                // 	gTPS.touch_sta &=0xe0;	//清除触摸点有效标记
                // 	return;
                // }
            }
        }
        gTPS.touch_sta |= TP_PRES_DOWN; // 触摸按下标记
    }
    else
    {
        if (gTPS.touch_sta & TP_PRES_DOWN) // 之前是被按下的
        {
            gTPS.touch_sta &= ~0x80;       // 触摸松开标记
        }
        else
        {
            gTPS.x[0] = 0;
            gTPS.y[0] = 0;
            gTPS.touch_sta &= 0xe0; // 清除触摸点有效标记
        }
    }
}

/**
 * @brief   转换为实际位置
 * @param   position 
 * @note    FT6336U最多支持两点触控
 */
static void count_position_ft6336(TP_POSITION_T *position){
	// printf("------count_position_ft6336 %d------\n",gTPS.touch_count);
    // 触摸点个数
    switch(gTPS.touch_count)
	{
		case 1:		
        	DBG_LOGD("x = %d, y = %d", gTPS.x[0], gTPS.y[0]);
            // ! 测试绘制像素点
            EPD_Dis_Part(gTPS.x[0], gTPS.y[0], NULL, 8, 8);
            //软件滤掉无效操作
			if((gTPS.x[0]<200) && (gTPS.y[0]<200))
			{
				position->status = 1;
				position->x = gTPS.x[0];
				position->y = gTPS.y[0];
				return;
			}
            else
            {
                DBG_LOGW("Invalid Touch, x = %d, y = %d", position->x, position->y);
            }
			break;
	    case 2:
			if((gTPS.x[0]!=0)&&(gTPS.y[0]!=0)
				&&(gTPS.x[1]!=0)&&(gTPS.y[1]!=0)
				&&(gTPS.x[0]<200)&&(gTPS.y[0]<200)
				&&(gTPS.x[1]<200)&&(gTPS.y[1]<200))//软件滤掉无效操作
			{
				//To 152x152
				gTPS.x[0]=gTPS.x[0]*152/200; 
				gTPS.y[0]=gTPS.y[0]*152/200;	
				gTPS.x[1]=gTPS.x[1]*152/200; 
				gTPS.y[1]=gTPS.y[1]*152/200;	
				/******调试使用****/
				// printf("触摸点个数：:%d\r\n",gTPS.touch_count);	//FT6336U最多支持两点触控
				// printf("x0:%d,y0:%d\r\n",gTPS.x[0],gTPS.y[0]);
				// printf("x1:%d,y1:%d\r\n",gTPS.x[1],gTPS.y[1]);
			}
			break;					
		default:
            DBG_LOGW("Invalid Touch point = %d", gTPS.touch_count);
			break;						
	}
    for(int i=0;i<2;i++)
	{
		gTPS.x[i]=0;
		gTPS.y[i]=0;
	}
	position->status = 0;
	position->x = gTPS.x[0];
	position->y = gTPS.y[0];
}

void get_ft6336_touch_sta(TP_POSITION_T *position)
{
    scan_ft6336();
    count_position_ft6336(position);
}

void touch_ft6336_init(void)
{
    DBG_LOGI("Initializing Touch FT6336");
    // 初始化触摸复位与中断引脚
    touch_gpio_init();
    // 复位触摸芯片
    set_tp_rst_level(0);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    set_tp_rst_level(1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    // [x] I2C 初始化
    touch_i2c_master_init();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    uint8_t i2c_write_data;
    uint8_t i2c_read_data;
    // [x] 设置为正常操作模式
    i2c_write_data = FT_DEVICE_MODE_WORKING_MODE;
    ESP_ERROR_CHECK(touch_i2c_write(FT_DEVIDE_MODE, &i2c_write_data, 1));
    // [x] 设置触摸有效值22 越小越灵敏
    i2c_write_data = 22;
    ESP_ERROR_CHECK(touch_i2c_write(FT_ID_G_THGROUP, &i2c_write_data, 1));
    // 读取触摸有效值
    i2c_read_data = 0;
    ESP_ERROR_CHECK(touch_i2c_write_read(FT_ID_G_THGROUP, &i2c_read_data, 1));
    DBG_LOGI("TH_GROUP = %d", i2c_read_data);
    // [x] 设置激活周期 不能小于12 最大14
    i2c_write_data = 14;
    ESP_ERROR_CHECK(touch_i2c_write(FT_ID_G_PERIODACTIVE, &i2c_write_data, 1));
    // 读取激活周期
    i2c_read_data = 0;
    ESP_ERROR_CHECK(touch_i2c_write_read(FT_ID_G_PERIODACTIVE, &i2c_read_data, 1));
    DBG_LOGI("PERIODACTIVE = %d", i2c_read_data);
    // [x] 设置中断产生方式 触摸时持续低电平
    i2c_write_data = FT_INTERRUPT_POLLING_MODE;
    ESP_ERROR_CHECK(touch_i2c_write(FT_ID_G_MODE, &i2c_write_data, 1));
    // 读取中断产生方式
    i2c_read_data = 0;
    ESP_ERROR_CHECK(touch_i2c_write_read(FT_ID_G_MODE, &i2c_read_data, 1));
    DBG_LOGI("G_MODE = %d", i2c_read_data);
}