#ifndef __ACTION_H
#define __ACTION_H

#include "petri.h"

#include <stdint.h>
#include <stdbool.h>

enum BUTTON_PRESS {
	BP_NONE = 0,
	BP_SINGLE = 1,
	BP_DOUBLE = 2,
	BP_LONG = 9
};

enum CONTROL_STATE {
	C_ON = 0,
	C_OFF = 1,
	C_STANDBY = 2
};

typedef struct light_data {
	enum CONTROL_STATE state;
	uint32_t brightness;
} light_t;

typedef struct climate_control {
	enum CONTROL_STATE state;
	int32_t prev_temp;
	int32_t curr_temp;
	int32_t target_temp;
} climate_t;

typedef struct fire_control {
	enum CONTROL_STATE state;
	/* timer */
//	TIM_HandleTypeDef *tim;
	uint32_t timer_start;
	bool timer;
	/* fire control */
	bool smoke;
	bool alarm;
	/* misc */
	int32_t rate;
} firectrl_t;

typedef struct room_data {
	light_t *ldata;
	climate_t *cdata;
	firectrl_t *fdata;
	uint32_t people;
} room_t;

#define MAX_FUNCS 32u
int (*funcs[MAX_FUNCS]) (void *data);

void function_table_init(void);

int is_full(void *data);
int is_empty(void *data);

int lights_on(void *data);
int lights_off(void *data);
int lights_autooff(void *data);

int climate_on(void *data);
int climate_off(void *data);
int climate_init(void *data);
int climate_standby(void *data);

int fire_on(void *data);
int fire_off(void *data);
int fire_temp_nominal(void *data);
int fire_temp_rising(void *data);
int fire_temp_rate(void *data);
int fire_stop(void *data);
int fire_smoke(void *data);
int fire_timeout(void *data);
int fire_timeclear(void *data);
int fire_done(void *data);

#endif
