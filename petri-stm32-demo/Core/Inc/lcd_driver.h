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
 * lcd_driver.h
 * Reworked by: InterImpv
 *
 * Comment (17.12.2021):
 * as I could not trace back where did I get both of these .h and .c files
 * to give proper credit I will be copypasting the original license that was
 * provided by the author and reworking previous code, because makesystem.net
 * website is no longer functioning as I am writing this comment statement
 */

#ifndef INC_LCD_DRIVER_H_
#define INC_LCD_DRIVER_H_

#include "stm32f0xx_hal.h"

/* public lcd constants BEGIN */

#define LCD_ADDR 0x7E

#define LCD_CFG_DISPLAY1_BLINK1_CURSOR1 0x0Fu
#define LCD_CFG_DISPLAY1_BLINK0_CURSOR1 0x0Eu
#define LCD_CFG_DISPLAY1_BLINK1_CURSOR0 0x0Du
#define LCD_CFG_DISPLAY1_BLINK0_CURSOR0 0x0Cu
#define LCD_CFG_DISPLAY0_BLINK0_CURSOR0 0x08u

/* public lcd constants END */

enum LCD_ERR {
	LCD_EOK = 0,
	LCD_EARG = 1,
	LCD_ECGRAM = 2
};

/* interface functions BEGIN */

/* clears the display */
void lcd_cls(void);

/* returns cursor to (0, 0) */
void lcd_ret(void);

/* shifts cursor one position left(<) or right(>) */
enum LCD_ERR lcd_shift_curs(const uint8_t direction);

/* shifts whole display one position left(<) or right(>) */
enum LCD_ERR lcd_scroll(uint8_t direction);

/* prints a given char at current cursor position */
void lcd_putch(const uint8_t byte);

/* moves cursor to given X: @addr, Y: @line coordinates */
enum LCD_ERR lcd_goto(const uint8_t line, const uint8_t addr);

/* erases a char on current cursor position and moves cursor back once */
void lcd_bckspc(void);

/* initializes lcd display */
enum LCD_ERR lcd_init(void);
/* interface functions END */

#endif /* INC_LCD_DRIVER_H_ */
