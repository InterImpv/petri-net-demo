/* ----------------------------------------------------
 * File:       	Library for HD44780 compatible displays
 * Version:	   	v2.01
 * Author:     	GrAnd/www.MakeSystem.net
 * Tested on:  	AVR, STM32F10X
 * License:		GNU LGPLv2.1
 * ----------------------------------------------------
 *
 * -------------------------------------------
 * Copyright (C)2014 GrAnd. All right reserved
 * -------------------------------------------
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Contact information :
 * 		mail@makesystem.net
 * 		http://makesystem.net/?page_id=2
 *
 * lcd_driver.c
 * Reworked by: InterImpv
 *
 * Comment (17.12.2021):
 * as I could not trace back where did I get both of these .h and .c files
 * to give proper credit I will be copypasting the original license that was
 * provided by the author and reworking previous code, because makesystem.net
 * website is no longer functioning as I am writing this comment statement
 */

#include "lcd_driver.h"

extern I2C_HandleTypeDef hi2c1;

/* LCD ADAPTER 4-BIT MODE ONLY:
 * 		board
 * P11	P10	P9	P8	P7	P6	P5	P4
 * D4	D3	D2	D1	D0	EN	RW	RS
 * 		byte:
 * 7	6	5	4	3	2	1	0
 * dd	dd	dd	dd	1?	en	0?	rs
 */
// ??	EN	RW	RS
#define LCD_EN0_RS0 0x08	// 1	0	0	0
#define LCD_EN1_RS0 0x0C	// 1	1	0	0
#define LCD_EN0_RS1 0x09	// 1	0	0	1
#define LCD_EN1_RS1 0x0D	// 1	1	0	1

/* private lcd constants BEGIN */
static const uint32_t LCDT_1MS = 1;
static const uint32_t LCDT_10MS = 10;
static const uint8_t __LCD_CURS_L = '<';
static const uint8_t __LCD_CURS_R = '>';

enum LCD_CMD
{
	__LCD_CLS = 0x01,
	__LCD_RET = 0x02,
	__LCD_CSL = 0x10,
	__LCD_CSR = 0x14,
	__LCD_DSL = 0x18,
	__LCD_DSR = 0x1C,
	__LCD_FST = 0x30
};

/* private lcd constants END */

static void lcd_send_cmd(uint8_t cmd)
{
	uint8_t data_u, data_l;
	uint8_t buf[4];

	data_u = cmd & 0xF0;
	data_l = (cmd << 4) & 0xF0;

	buf[0] = data_u | LCD_EN1_RS0;  // en = 1, rs = 0
	buf[1] = data_u | LCD_EN0_RS0;  // en = 0, rs = 0
	buf[2] = data_l | LCD_EN1_RS0;  // en = 1, rs = 0
	buf[3] = data_l | LCD_EN0_RS0;  // en = 0, rs = 0

	HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, buf, 4, HAL_MAX_DELAY);
}

static void lcd_send_data(uint8_t data)
{
	uint8_t data_u, data_l;
	uint8_t buf[4];

	data_u = data & 0xF0;
	data_l = (data << 4) & 0xF0;

	buf[0] = data_u | LCD_EN1_RS1;  // en = 1, rs = 1
	buf[1] = data_u | LCD_EN0_RS1;  // en = 0, rs = 1
	buf[2] = data_l | LCD_EN1_RS1;  // en = 1, rs = 1
	buf[3] = data_l | LCD_EN0_RS1;  // en = 0, rs = 1

	HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, buf, 4, HAL_MAX_DELAY);
}

/* sending instruction: RS <= 1'b0 */
/* sending data: RS <= 1'b1 */

/* interface functions BEGIN */
void lcd_cls(void)
{
	/* RS <= 1'b0 */
	lcd_send_cmd(__LCD_CLS);
	HAL_Delay(LCDT_1MS);
}

void lcd_ret(void)
{
	/* RS <= 1'b0 */
	lcd_send_cmd(__LCD_RET);
	HAL_Delay(LCDT_1MS);
}

enum LCD_ERR lcd_shift_curs(const uint8_t direction)
{
	/* RS <= 1'b0 */
	if (direction ==__LCD_CURS_L) {			// '<'
		lcd_send_cmd(__LCD_CSL);
		return LCD_EOK;
	} else if (direction ==__LCD_CURS_R) {	// '>'
		lcd_send_cmd(__LCD_CSR);
		return LCD_EOK;
	}
	return LCD_EARG;
}

/* prints a given char at current cursor position */
void lcd_putch(const uint8_t byte)
{
	lcd_send_data(byte);
	HAL_Delay(LCDT_1MS);
}

/* moves cursor to given X: @addr, Y: @line coordinates */
enum LCD_ERR lcd_goto(const uint8_t line, const uint8_t addr)
{
    uint8_t xpos = addr & 0x0F;

    /* RS <= 1'b0 */
	switch (line)
	{
	/* set DDRAM address. */
	case 0:
		lcd_send_cmd(0x80 | 0x00 | xpos);
		return LCD_EOK;
		break;
	case 1:
		lcd_send_cmd(0x80 | 0x40 | xpos);
		return LCD_EOK;
		break;

	default:
		return LCD_EARG;
		break;
	}
}

/* erases a char on current cursor position and moves cursor back once */
void lcd_bckspc(void)
{
	lcd_shift_curs(__LCD_CURS_L);
	lcd_putch(' ');
	lcd_shift_curs(__LCD_CURS_L);
}

enum LCD_ERR lcd_scroll(uint8_t direction)
{
	/* RS <= 1'b0 */
	/* shift whole display */
	if (direction ==__LCD_CURS_L) {			// '<'
		lcd_send_cmd(__LCD_DSL);
		return LCD_EOK;
	} else if (direction ==__LCD_CURS_R) {	// '>'
		lcd_send_cmd(__LCD_DSR);
		return LCD_EOK;
	}
	return LCD_EARG;
}

/* initializes lcd display */
enum LCD_ERR lcd_init(void)
{
	lcd_send_cmd(__LCD_FST);
	HAL_Delay(LCDT_10MS);
	lcd_send_cmd(__LCD_RET);
	HAL_Delay(LCDT_10MS);
	lcd_send_cmd(LCD_CFG_DISPLAY1_BLINK0_CURSOR1);
	HAL_Delay(LCDT_10MS);
	lcd_send_cmd(__LCD_CLS);
	HAL_Delay(LCDT_10MS);

	return LCD_EOK;
}

/* interface functions END */

