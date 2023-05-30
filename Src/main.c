#include "main.h"
#include "rcc.h"
#include "lcd.h"
#include "tim.h"
#include "adc.h"
#include "i2c_hal.h"
#include "key_led.h"

void key_proc(void);
void led_proc(void);
void lcd_proc(void);
void adc_proc(void);
void pwm_proc(void);

//for delay 
__IO uint32_t uwTick_key, uwTick_led, uwTick_lcd, uwTick_adc, uwTick_pwm;

//for key
uint8_t key_val, key_old, key_down, key_up;

//for led
uint8_t led;

//for lcd
uint8_t string_lcd[21];
uint8_t lcd_index; //0-measure  1-setting
uint8_t which_para;

//for iic
uint8_t read_buf[3];
uint8_t write_buf[3] = {'4', 1, 1};
//for plus
float plus1_freq;
float plus2_freq;
uint8_t div_disp = 1;
uint8_t div_real = 1;
uint8_t mul_disp = 1;
uint8_t mul_real = 1;;

//for adc
float AO1_V;
float AO2_V;

int main(void)
{
  HAL_Init();
	HAL_Delay(1);

  SystemClock_Config();
	
	Key_Led_Init();
	
	ADC2_Init();
	
	I2CInit();
	HAL_Delay(10);
	eeprom_read(read_buf, 0, 3);
	HAL_Delay(10);
	if(read_buf[0] != '4')
	{
		eeprom_write(write_buf, 0, 3);
	}
	else 
	{
		eeprom_read(read_buf, 0, 3);
		div_real = read_buf[1];
		mul_real = read_buf[2];
		
		div_disp = read_buf[1];;
		mul_disp = read_buf[2];;
	}
		
		
	
	PLUS1_Init();
	PLUS2_Init();
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim15,TIM_CHANNEL_1);
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim15);
	
	PA6_Iint();
	PA7_Iint();
	//HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	//HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
	
	LCD_Init();
	LCD_SetTextColor(White);
	LCD_SetBackColor(Black);
	LCD_Clear(Black);

  while (1)
  {
		key_proc();
		led_proc();
		lcd_proc();
		adc_proc();
		pwm_proc();
  }

}

void key_proc(void)
{
	if(uwTick - uwTick_key < 50)	return;
	uwTick_key = uwTick;

	key_val = Read_Key();
	key_down = key_val & (key_val ^ key_old);
	key_up  = ~key_val & (key_val ^ key_old);
	key_old = key_val;
	
	if(key_down == 1)
	{
		lcd_index ^= 1;
		LCD_Clear(Black);
		LCD_SetTextColor(White);
		
		if(lcd_index == 0)
		{
			div_real = div_disp;
			mul_real = mul_disp;			
			write_buf[0] = div_real;
			write_buf[1] = mul_real;
			eeprom_write(write_buf, 1, 2);
		}
	}
	
	else if(key_down == 2)
	{
		if(which_para < 2)
			which_para++;
		if(which_para == 2)
			which_para = 0;
	
	
	}
	
	else if(key_down == 3)
	{
		if(which_para == 0)
		{
			if(div_disp < 4)
				div_disp++;
		}
		
		else if(which_para == 1)
		{
			if(mul_disp < 4)
				mul_disp++;		
		}
	}

	else if(key_down == 4)
	{
		if(which_para == 0)
		{
			if(div_disp > 1)
				div_disp--;
		}
		
		else if(which_para == 1)
		{
			if(mul_disp > 1)
				mul_disp--;		
		}	
	}
}

void led_proc(void)
{
	if(uwTick - uwTick_led < 50)	return;
	uwTick_led = uwTick;
	
	if(lcd_index == 1)
		led |= 0x01;
	else 
		led &= (~0x01);
	
	if(AO1_V > AO2_V)
		led |= 0x80;
	else 
		led &= (~0x80);	
	
	Led_Disp(led);
}


void adc_proc(void)
{
	if(uwTick - uwTick_adc < 50)	return;
	uwTick_adc = uwTick;

	get_adc_rp5();
	HAL_Delay(1);
	AO1_V = get_adc_rp5()*3.3/4096;
	HAL_Delay(1);
	get_adc_rp6();
	HAL_Delay(1);
	AO2_V = get_adc_rp6()*3.3/4096;
}



void pwm_proc(void)
{
	if(uwTick - uwTick_pwm < 50)	return;
	uwTick_pwm = uwTick;
	
	if(lcd_index == 0)
	{
		HAL_TIM_MspPostInit(&htim3);
		HAL_TIM_MspPostInit(&htim17);
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
		
		__HAL_TIM_SetAutoreload(&htim3, 1000000/(plus1_freq/div_real));
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, __HAL_TIM_GetAutoreload(&htim3)*0.5);
	
		__HAL_TIM_SetAutoreload(&htim17, 1000000/(plus2_freq*mul_real));
		__HAL_TIM_SetCompare(&htim17, TIM_CHANNEL_1, __HAL_TIM_GetAutoreload(&htim17)*0.5);
	}
	else 
	{
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim17, TIM_CHANNEL_1);
		GPIO_out_low();	
	}
}
void lcd_proc(void)
{
	if(uwTick - uwTick_lcd < 50)	return;
	uwTick_lcd = uwTick;

	if(lcd_index == 0)
	{
		sprintf((char *)string_lcd, "      Measure");
		LCD_DisplayStringLine(Line0, string_lcd);
		
		sprintf((char *)string_lcd, "PLUS1: %1.1fKHz  ", plus1_freq/1000);
		LCD_DisplayStringLine(Line2, string_lcd);

		sprintf((char *)string_lcd, "PLUS2: %1.1fKHz  ", plus2_freq/1000);
		LCD_DisplayStringLine(Line4, string_lcd);	
		
		//get_adc_rp5(); //加上不显示了？
		sprintf((char *)string_lcd, "AO1: %1.2fV  ", AO1_V);
		LCD_DisplayStringLine(Line6, string_lcd);

		//get_adc_rp6();
		sprintf((char *)string_lcd, "AO2: %1.2fV  ", AO2_V);
		LCD_DisplayStringLine(Line8, string_lcd);

		sprintf((char *)string_lcd, "                  1");
		LCD_DisplayStringLine(Line9, string_lcd);		
	}
	
	else if(lcd_index == 1)
	{
		LCD_SetTextColor(White);
		sprintf((char *)string_lcd, "      Setting");
		LCD_DisplayStringLine(Line0, string_lcd);
		
		if(which_para == 0)
			LCD_SetTextColor(Green);
		else
			LCD_SetTextColor(White);
		sprintf((char *)string_lcd, "div: %d  ", div_disp);
		LCD_DisplayStringLine(Line4, string_lcd);

		if(which_para == 1)
			LCD_SetTextColor(Green);
		else
			LCD_SetTextColor(White);
		sprintf((char *)string_lcd, "mul: %d  ", mul_disp);
		LCD_DisplayStringLine(Line6, string_lcd);	
	

		LCD_SetTextColor(White);		
		sprintf((char *)string_lcd, "                  2");
		LCD_DisplayStringLine(Line9, string_lcd);		
	}

}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			plus1_freq = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			plus1_freq = 1000000.0 / plus1_freq;
		}
	}

	if(htim->Instance == TIM15)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			plus2_freq = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			plus2_freq = 1000000.0 / plus2_freq;
		}
	}
}





void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

