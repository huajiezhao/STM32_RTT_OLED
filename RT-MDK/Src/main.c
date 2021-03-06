/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"
#include "app_rtthread.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED_I2C.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/*创建静态任务，不使用堆区*/
//创建第一个句柄
static struct rt_thread led_thread;
//设定任务的栈区大小
static char led_thread_stack[256];


//创建第二个任务
static struct rt_thread uart_thread;
static char uart_thread_stack[512];

//创建第三个任务
static struct rt_thread oled_thread;
static char oled_thread_stack[256];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//任务一
static void led_thread_entry(void *parameter){
	MX_GPIO_Init();
	while(1){
		HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);
		rt_thread_mdelay(500);
		HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);
		rt_thread_mdelay(500);
	}
}

//任务二子函数
void frame_prase(uint8_t *data,uint8_t *pluse,uint8_t *spo2,uint8_t *dbm,uint8_t *PI,uint8_t *bm,uint8_t *tmm){
	uint8_t length = sizeof(data);
	//int x = 0;
	for(int x=0;x<length;x++){
		//如果不是帧头，就进入下一次循环
		if(data[x]&0x80 == 0) continue;
		//如果是帧头，就读取七个字节的数据
			*pluse = (data[x+2]&0x40)*2+data[x+3];
			*spo2 = data[x+4];
			*dbm = data[x+0]&0x07;
			*PI = (data[x+0]&0x08)*16+data[x+5];
			*bm = data[x+2]&0x0f;
			*tmm = data[x+1];
			break;
		}
}


//任务二
static void uart_thread_entry(void *parameter){
	MX_USART1_UART_Init();
	unsigned char spo2_str[10];
	unsigned char pluse_str[10];
	uint8_t frame[] = {0x82,0x64,0x2a,0x46,0x62,0x64,0x00};
	uint8_t pluse = 0;
	uint8_t spo2 = 0;
	uint8_t dbm = 0;
	uint8_t PI = 0;
	uint8_t bm = 0;
	uint8_t tmm = 0;
	frame_prase(frame,&pluse,&spo2,&dbm,&PI,&bm,&tmm);
	while(1){
		printf("pluse:%d\r\nspo2:%d\r\ndbm:%d\r\nPI:%d\r\nbm:%d\r\ntmm:%d\r\n",pluse,spo2,dbm,PI,bm,tmm);
		sprintf(spo2_str,"spo2   %d",spo2);
		sprintf(pluse_str,"pluse  %d",pluse);
		OLED_ShowStr(24,2,spo2_str,1);
		OLED_ShowStr(24,3,pluse_str,1);
		rt_thread_mdelay(1000);
	}
}

//任务三
static void oled_thread_entry(void *parameter){
	I2C_Configuration();
	OLED_Init();
	OLED_Fill(0);
	unsigned char ch[] = "BLOOD OXYGEN MINITER";
	while(1){
		OLED_ShowStr(0,0,ch,1);
		rt_thread_mdelay(500);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	rt_err_t task1;
	task1 = rt_thread_init(&oled_thread,							//句柄地址
												 "oled_display",						//任务名
												 oled_thread_entry,					//任务函数
												 RT_NULL,										//参数，一般为null
												 &oled_thread_stack,				//任务栈首地址
												 sizeof(oled_thread_stack),	//任务栈区大小
												 RT_THREAD_PRIORITY_MAX -3,	//任务优先级
												 20);												//分配时间片
	if(task1 == RT_EOK){
		rt_thread_startup(&oled_thread);
	}
	rt_err_t task2;
	task2 = rt_thread_init(&led_thread,
												 "led_blinky",
												 led_thread_entry,
												 RT_NULL,
												 &led_thread_stack,
												 sizeof(led_thread_stack),
												 RT_THREAD_PRIORITY_MAX -1,
												 20);
	if(task2 == RT_EOK){
		rt_thread_startup(&led_thread);
	}
	rt_err_t task3;
	task3 = rt_thread_init(&uart_thread,
												 "uart_task",
												 uart_thread_entry,
												 RT_NULL,
												 &uart_thread_stack,
												 sizeof(uart_thread_stack),
												 RT_THREAD_PRIORITY_MAX -2,
												 20);
	if(task3 == RT_EOK){
		rt_thread_startup(&uart_thread);
	}
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

    /* USER CODE END WHILE */


    /* USER CODE BEGIN 3 */
  
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
