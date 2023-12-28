/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file					 : main.c
* @brief					: Main program body
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
* All rights reserved.</center></h2>
*
* This software component is licensed by ST under BSD 3-Clause license,
* the "License"; You may not use this file except in compliance with the
* License. You may obtain a copy of the License at:
*												opensource.org/licenses/BSD-3-Clause
*
******************************************************************************
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd_driver.h"
#include "terminal.h"
#include "action.h"
#include "netjson.h"

#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct button_data {
	int val;
	bool press;
} btn_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define MAX_SYSTEMS 3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//#define ABS(x) ((x)<0 ? -(x) : (x))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim14;

TSC_HandleTypeDef htsc;

/* USER CODE BEGIN PV */
uint32_t memalloc;

const uint8_t *heap_start = (uint8_t *)0x200003d0;
uint8_t *heap_ptr;

static enum BUTTON_PRESS btn = BP_NONE;
static btn_t ubtn;

static uint32_t t_prev = 0, t_curr = 0, dt = 0;

static light_t light;
static climate_t climate;
static firectrl_t fire;
static room_t room;

//static lt_data_t ldata[MAX_SYSTEMS];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TSC_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM14_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* helper functions */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* get time delta in ms */
	t_prev = t_curr;
	t_curr = HAL_GetTick();
	dt = abs((int32_t)t_curr - (int32_t)t_prev);
	/* button */
	switch(GPIO_Pin)
	{
	case GPIO_PIN_0:
		/* check for multiple presses, with short & long double press*/
		if (dt < 400) {
			btn = BP_DOUBLE;
		} else if (dt >= 400 && dt < 800) {
			btn = BP_LONG;
		} else {
			btn = BP_SINGLE;
		}
		/* start a button get timer */
//		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, SET);
		HAL_TIM_Base_Start_IT(&htim6);
		break;

	default:
		break;
	};
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	if (&htim6 == htim) {
		HAL_TIM_Base_Stop_IT(&htim6);
//		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, RESET);

		ubtn.val = btn;
		ubtn.press = true;

		btn = 0;
	}
	if (&htim14 == htim) {
		HAL_TIM_Base_Stop_IT(&htim14);
		room.fdata->timer = false;
	}
}

void control_lights(room_t *room)
{
	switch (ubtn.val)
	{
	case BP_DOUBLE:
		room->ldata->state ^= 1;
		break;
	case BP_SINGLE:
		if (room->ldata->state == C_OFF)
			room->ldata->brightness++;
		else
			room->ldata->brightness--;
		break;

	default:
		break;
	};
}

void control_climate(room_t *room)
{
	switch (ubtn.val)
	{
	case BP_DOUBLE:
		room->cdata->state ^= 1;
		break;
	case BP_SINGLE:
		room->cdata->prev_temp = room->cdata->curr_temp;
		if (room->cdata->state == C_OFF)
			room->cdata->curr_temp++;
		else
			room->cdata->curr_temp--;
		break;

	default:
		break;
	};
}

void control_fire(room_t *room)
{
	switch (ubtn.val)
	{
	case BP_DOUBLE:
		room->fdata->state ^= 1;
		break;
	case BP_SINGLE:
		room->cdata->prev_temp = room->cdata->curr_temp;
		if (room->fdata->state == C_OFF)
			room->cdata->curr_temp--;
		else
			room->cdata->curr_temp++;
		break;

	default:
		break;
	};
}

void control_room(room_t *room)
{
	switch (ubtn.val)
	{
	case BP_SINGLE:
		room->people ^= 1;
		break;

	default:
		break;
	};
}

void control_by_idx(room_t *room, uint32_t idx)
{
	if (!room || !room->ldata || !room->ldata)
		return;

	switch (idx)
	{
	case 1:
		control_lights(room);
		break;
	case 2:
		control_climate(room);
		break;
	case 3:
		control_fire(room);
		break;

	default:
		control_room(room);
		break;
	};
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TSC_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */
	/* user systems */
		/* terminal data structs & buffers */
	char sbuf[17] = {'\0'};
	char tbuf[17] = {'\0'};
	term_win stdwin;
		/* i2c bus & lcd + terminal lib init */
	while (1) {
		if (HAL_I2C_IsDeviceReady(&hi2c1, LCD_ADDR, 1, HAL_MAX_DELAY) == HAL_OK) {
			lcd_init();
			term_init(&stdwin);
			break;
		}
	}
	/* status structures init */
	light.state = C_OFF;
	light.brightness = 8;

	climate.state = C_OFF;
	climate.prev_temp = 27;
	climate.curr_temp = 28;
	climate.target_temp = 22;

	fire.state = C_OFF;
//	fire.tim = &htim14;
	fire.timer = false;
	fire.alarm = false;
	fire.smoke = false;
	fire.rate = climate.curr_temp - climate.prev_temp;

	room.people = 0;
	room.ldata = &light;
	room.cdata = &climate;
	room.fdata = &fire;

	/* misc control data */
	int jstatus = 0;
	uint32_t sys_idx = 0, ctrl_idx = 0;
	/* for testing */
	srand(0);

	/* petri net init */
		/* define function table */
	function_table_init();
		/* function table offsets */
	uint32_t offset = 0;
	uint32_t offsets[MAX_SYSTEMS] = { 0, 5, 11 };
		/* system names */
	const char *sys_names[MAX_SYSTEMS] = {
		"LightSys",
		"ClimateSys",
		"FireSys"
	};
		/* petri net container */
	pnet_t *sys = pnet_create();
	/* load LightSys by default */
	pnet_init(sys, sys_names[sys_idx]);
	jstatus = pnet_deserialize(sys, jstr[sys_idx]);
	/* set transition functions */
	offset = offsets[sys_idx];
	for (uint32_t i = 0; i < sys->trans_count; i++) {
		const uint32_t x = offset + i;
		trans_set_action(sys->trans[i], funcs[x], &room);
	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	uint32_t delta = 0, i = 0;
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		delta = (uint32_t)heap_ptr - (uint32_t)heap_start;
		/* simulate current loaded system in sequence */
		pnet_fire_sequence(sys);

		/* draw clear */
		term_cls(&stdwin);

		switch (ctrl_idx)
		{
		case 1:
			snprintf(tbuf, 16, "s%i b%lu", room.ldata->state, room.ldata->brightness);
			break;
		case 2:
			snprintf(tbuf, 16, "s%1i c%03li t%03li", room.cdata->state, room.cdata->curr_temp, room.cdata->target_temp);
			break;
		case 3:
			snprintf(tbuf, 16, "s%1i p%03lu c%03lu %03li", room.fdata->state, room.cdata->prev_temp, room.cdata->curr_temp, room.fdata->rate);
			break;

		default:
			snprintf(tbuf, 16, "j%1u p%1lu %lu %lu", jstatus, room.people, delta, memalloc);
			break;
		};
		term_putsl(&stdwin, tbuf, 0);
		/* do not print anything if in room controls */
		if (ctrl_idx != 0) {
			pnet_dump_state_as_str(sys, sbuf);
			snprintf(tbuf, 16, "%.8s %04lu", sbuf, delta);
			term_putsl(&stdwin, tbuf, 1);
		}
		/* simulation for different systems */
		uint32_t chance = rand() % 100;
		room.cdata->prev_temp = room.cdata->curr_temp;
		switch (sys_idx) {
		case 1:
			/* random temperature decrement for heating */
			if (i % 25 == 0) {
				if (chance < 40) {
					room.cdata->curr_temp--;
				} else if (chance >= 40 && chance < 70) {
					room.cdata->curr_temp -= 5;
				} else if (chance >= 70 && chance < 100) {
					room.cdata->curr_temp -= 10;
				}
			}
			break;

		case 2:
			/* random temperature increment for fire alarm */
			if (i % 25 == 0) {
				room.cdata->prev_temp = room.cdata->curr_temp;
				if (chance < 40) {
					room.cdata->curr_temp++;
				} else if (chance >= 40 && chance < 70) {
					room.cdata->curr_temp += 5;
				} else if (chance >= 70 && chance < 100) {
					room.cdata->curr_temp += 10;
				}
			}
			break;

		default:
			break;
		};
		i++;

		/* draw render */
		term_draw(&stdwin);

		/* do something only if button was pressed */
		if (!ubtn.press)
			continue;
		/* room controls emulation */
		control_by_idx(&room, ctrl_idx);
		/* cycle through systems */
		if(ubtn.val == BP_LONG) {
			/* currently loaded system */
			if (ctrl_idx > 0)
				sys_idx = (sys_idx + 1) % MAX_SYSTEMS;
			/* 0 is room, others are system controls */
			ctrl_idx = (ctrl_idx + 1) % (MAX_SYSTEMS + 1);
			/* free previous & create next */
			pnet_free(sys);
			sys = pnet_create();
			/* reinit system */
			pnet_init(sys, sys_names[sys_idx]);
			jstatus = pnet_deserialize(sys, jstr[sys_idx]);
			/* set transition functions */
			offset = offsets[sys_idx];
			for (uint32_t i = 0; i < sys->trans_count; i++) {
				const uint32_t x = offset + i;
				trans_set_action(sys->trans[i], funcs[x], &room);
			}
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
		}
		/* clear button press */
		ubtn.val = BP_NONE;
		ubtn.press = false;
	}
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x20303E5D;
  hi2c1.Init.OwnAddress1 = 126;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 319;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x23;
  sTime.Minutes = 0x59;
  sTime.Seconds = 0x45;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_SUB1H;
  sTime.StoreOperation = RTC_STOREOPERATION_SET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = RTC_MONTH_DECEMBER;
  sDate.Date = 0x31;
  sDate.Year = 0x99;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 8191;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 5859;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 732;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 65535;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief TSC Initialization Function
  * @param None
  * @retval None
  */
static void MX_TSC_Init(void)
{

  /* USER CODE BEGIN TSC_Init 0 */

  /* USER CODE END TSC_Init 0 */

  /* USER CODE BEGIN TSC_Init 1 */

  /* USER CODE END TSC_Init 1 */
  /** Configure the TSC peripheral
  */
  htsc.Instance = TSC;
  htsc.Init.CTPulseHighLength = TSC_CTPH_2CYCLES;
  htsc.Init.CTPulseLowLength = TSC_CTPL_2CYCLES;
  htsc.Init.SpreadSpectrum = DISABLE;
  htsc.Init.SpreadSpectrumDeviation = 1;
  htsc.Init.SpreadSpectrumPrescaler = TSC_SS_PRESC_DIV1;
  htsc.Init.PulseGeneratorPrescaler = TSC_PG_PRESC_DIV4;
  htsc.Init.MaxCountValue = TSC_MCV_8191;
  htsc.Init.IODefaultMode = TSC_IODEF_OUT_PP_LOW;
  htsc.Init.SynchroPinPolarity = TSC_SYNC_POLARITY_FALLING;
  htsc.Init.AcquisitionMode = TSC_ACQ_MODE_NORMAL;
  htsc.Init.MaxCountInterrupt = DISABLE;
  htsc.Init.ChannelIOs = TSC_GROUP1_IO3|TSC_GROUP2_IO3|TSC_GROUP3_IO2;
  htsc.Init.ShieldIOs = 0;
  htsc.Init.SamplingIOs = TSC_GROUP1_IO4|TSC_GROUP2_IO4|TSC_GROUP3_IO3;
  if (HAL_TSC_Init(&htsc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TSC_Init 2 */

  /* USER CODE END TSC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, NCS_MEMS_SPI_Pin|EXT_RESET_Pin|LD3_Pin|LD6_Pin
                          |LD4_Pin|LD5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : NCS_MEMS_SPI_Pin EXT_RESET_Pin LD3_Pin LD6_Pin
                           LD4_Pin LD5_Pin */
  GPIO_InitStruct.Pin = NCS_MEMS_SPI_Pin|EXT_RESET_Pin|LD3_Pin|LD6_Pin
                          |LD4_Pin|LD5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : MEMS_INT1_Pin MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT1_Pin|MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : I2C2_SCL_Pin I2C2_SDA_Pin */
  GPIO_InitStruct.Pin = I2C2_SCL_Pin|I2C2_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF1_I2C2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI2_SCK_Pin SPI2_MISO_Pin SPI2_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI2_SCK_Pin|SPI2_MISO_Pin|SPI2_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

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
	__disable_irq();
	while (1)
	{
	}
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
 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
