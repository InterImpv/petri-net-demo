#include "main.h"
#include "action.h"

#include <cstdint>
#include <stdint.h>
#include <stdbool.h>

/* function table init */
void function_table_init(void)
{
	for (uint32_t i = 0; i < MAX_FUNCS; i++) {
		funcs[i] = empty_action_false;
	}

    const uint32_t offset_lights = 0;
    const uint32_t offset_climate = 5;
    const uint32_t offset_firectrl = 11;

	funcs[offset_lights + 0] = is_empty;
	funcs[offset_lights + 1] = is_full;
	funcs[offset_lights + 2] = lights_autooff;
	funcs[offset_lights + 3] = lights_off;
	funcs[offset_lights + 4] = lights_on;

	funcs[offset_climate + 0] = climate_init;
	funcs[offset_climate + 1] = climate_off;
	funcs[offset_climate + 2] = climate_on;
	funcs[offset_climate + 3] = climate_standby;
	funcs[offset_climate + 4] = is_empty;
	funcs[offset_climate + 5] = is_full;

	funcs[offset_firectrl + 0] = fire_on;
	funcs[offset_firectrl + 1] = fire_off;
	funcs[offset_firectrl + 2] = fire_temp_nominal;
	funcs[offset_firectrl + 3] = fire_temp_rising;
	funcs[offset_firectrl + 4] = fire_temp_rate;
	funcs[offset_firectrl + 5] = fire_stop;
	funcs[offset_firectrl + 6] = fire_smoke;
	funcs[offset_firectrl + 7] = fire_timeout;
	funcs[offset_firectrl + 8] = fire_timeclear;
	funcs[offset_firectrl + 9] = fire_done;
}

/* user-defined */
	/* LIGHTNING SYSTEM */
int is_empty(void *data)
{
	room_t *room = (room_t *)data;
	return (room->people < 1);
}

int is_full(void *data)
{
	return !is_empty(data);
}

int lights_on(void *data)
{
	room_t *room = (room_t *)data;
	/* turn on the light automatically if possible */
	if (room->ldata->state == C_ON) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
		return 0;
	}
	return -1;
}

int lights_off(void *data)
{
	room_t *room = (room_t *)data;
	/* turn off the light if it is on */
	if (room->ldata->state == C_OFF) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
		return 0;
	}
	return -1;
}

int lights_autooff(void *data)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	return 0;
}

	/* CLIMATE SYSTEM */
int climate_on(void *data)
{
	room_t *room = (room_t *)data;
	/* turn on automatically if possible */
	if (room->cdata->curr_temp < room->cdata->target_temp) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
		return 0;
	}
	return -1;
}

int climate_off(void *data)
{
	room_t *room = (room_t *)data;
	/* turn off the climate control if it is on */
	if (room->cdata->state == C_ON) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
		return 0;
	}
	return -1;
}

int climate_init(void *data)
{
	room_t *room = (room_t *)data;
	if (room->cdata->state == C_OFF) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
		return 0;
	}
	return -1;
}

int climate_standby(void *data)
{
	room_t *room = (room_t *)data;
	/* dec temp every time */
	if (rand() % 100 < 20) {
		room->cdata->curr_temp = room->cdata->prev_temp;
		room->cdata->curr_temp += 1;
	}
	/* wait until temperature drops */
	if (room->cdata->curr_temp >= room->cdata->target_temp) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
		return 0;
	}
	return -1;
}

	/* FIRE ALARM SYSTEM */
#define MAX_TEMPERATURE 35
#define MAX_TEMP_RATE 8
/* for demo only */
#define STOP_PROBABILITY 99

int fire_on(void *data)
{
	room_t *room = (room_t *)data;
	/* turn on the light automatically if possible */
	if (room->fdata->state == C_ON) {
		HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
		return 0;
	}
	return -1;
}

int fire_off(void *data)
{
	room_t *room = (room_t *)data;
	/* turn off the light if it is on */
	if (room->fdata->state == C_OFF) {
		HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_RESET);
		return 0;
	}
	return -1;
}

int fire_temp_nominal(void *data)
{
	room_t *room = (room_t *)data;
	/* if temp returned to normal */
	if (room->cdata->curr_temp < MAX_TEMPERATURE) {
		return 0;
	}
	return -1;
}

int fire_temp_rising(void *data)
{
	room_t *room = (room_t *)data;
	/* if temp above threshold start being suspicious */
	if (room->cdata->curr_temp >= MAX_TEMPERATURE) {
		return 0;
	}
	return -1;
}

int fire_temp_rate(void *data)
{
	room_t *room = (room_t *)data;
	/* turn off the light if it is on */
	room->fdata->rate = room->cdata->curr_temp - room->cdata->prev_temp;
	if (room->fdata->rate >= MAX_TEMP_RATE) {
		/* for demo purposes only */
		room->fdata->smoke = rand() % 2;
		/* start alarm */
		room->fdata->alarm = true;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
		/* start timer */
		if (!room->fdata->timer) {
			room->fdata->timer = true;
			room->fdata->timer_start = HAL_GetTick();
			HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);
		}
		return 0;
	}
	return -1;
}

int fire_smoke(void *data)
{
	room_t *room = (room_t *)data;
	/* if smoke detected turn on water sprinklers */
	if (room->fdata->smoke) {
		HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);
		return 0;
	}
	return -1;
}

int fire_timeout(void *data)
{
	room_t *room = (room_t *)data;
	/* timer fired */
	uint32_t dt = abs((int32_t)HAL_GetTick() - (int32_t)room->fdata->timer_start);

	if (dt > 5000 && room->fdata->timer) {
		room->fdata->timer = false;
		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
		/* sprinklers */
		HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);
		return 0;
	}
	return -1;
}

int fire_timeclear(void *data)
{
	room_t *room = (room_t *)data;
	/* timer fired */
	uint32_t dt = abs((int32_t)HAL_GetTick() - (int32_t)room->fdata->timer_start);

	if (dt > 10000 && room->fdata->timer && !room->fdata->alarm) {
		room->fdata->timer = false;
		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
		return 0;
	}
	return -1;
}

int fire_stop(void *data)
{
	room_t *room = (room_t *)data;
	/* for demo purposes only */
	if (rand() % 100 >= STOP_PROBABILITY) {
		room->fdata->timer = false;
		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);

		room->fdata->alarm = false;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

		room->fdata->smoke = false;
		HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);

		room->cdata->curr_temp = 20;
		room->cdata->prev_temp = 20;
		return 0;
	}
	return -1;
}

int fire_done(void *data)
{
	room_t *room = (room_t *)data;
	/* if temp above threshold start being sus */
	if (rand() % 100 >= STOP_PROBABILITY) {
		room->fdata->timer = false;

		room->fdata->alarm = false;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

		room->fdata->smoke = false;
		HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);

		room->cdata->curr_temp = 20;
		room->cdata->prev_temp = 20;
		return 0;
	}
	return -1;
}

