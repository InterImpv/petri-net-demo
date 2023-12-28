/*
 *	terminal.c
 *
 *	Created on: Nov 24, 2021
 *		Author: InterImpv
 */

#include "terminal.h"
#include "lcd_driver.h"

#include <string.h>
#include <stdbool.h>

/* sets update request flag */
inline void term_request_update(term_win *term)
{
	term->flags.update_req = true;
}

/* resets update request flag */
inline void term_reject_update(term_win *term)
{
	term->flags.update_req = false;
}

/* sets cursor xpos to @x, ypos to @y */
void term_goto(term_win *term, const uint8_t x, const uint8_t y)
{
	term->cx = x;
	term->cy = y;
}

/* clears terminal primary buffer */
void term_cls(term_win *term)
{
	term_goto(term, 0, 0);	//return home
	memset(term->win, ' ', term->size);

	term_refresh(term);
}

/* sends  */
void term_draw(term_win *term)
{
	if (term->flags.update_req) {
		term_skip(term);

		for (uint8_t i = 0; i < term->rows; i++) {
			for (uint8_t j = 0; j < term->cols; j++) {
				lcd_goto(i, j);
				lcd_putch(term->win[i * term->cols + j]);
				//HAL_Delay(1);
			}
		}
	}
}

/* prints given @ch on coordinates @x, @y */
void term_putchyx(term_win *term, const char ch, const uint8_t y, const uint8_t x)
{
	if (term->cols > x && term->rows > y) {
		uint8_t cxprev = term->cx;
		uint8_t cyprev = term->cy;

		term_goto(term, x, y);				// move cursor
		term->win[y * term->cols + x] = ch;	// put char
		term_goto(term, cxprev, cyprev);	// reset cursor

		term_refresh(term);
	}
}

/* prints a @ch at current cursor position and advances cursor once */
void term_putch(term_win *term, const char ch)
{
	uint8_t x = term->cx;
	uint8_t y = term->cy;
	term->flags.h_overflow = false;
	term->flags.v_overflow = false;

	if (ch != '\n')
		term->win[y * term->cols + x] = ch;

	term->cx++;
	term_refresh(term);

	/* line overflow */
	if (term->cx > term->cols - 1 || ch == '\n') {
		term->cx = 0;
		term->cy++;
		term->flags.h_overflow = true;
	}
	/* vertical overflow */
	if (term->cy > term->rows) {
		term->cy = 0;
		term->flags.v_overflow = true;
	}
}

/* erases char at current cursor position and backs cursor once*/
void term_erasech(term_win *term)
{
	uint8_t x = term->cx;
	uint8_t y = term->cy;
	term->win[y * term->cols + x] = ' ';
	term_request_update(term);

	if ((term->cx == 0) && (term->cy == term->rows - 1)) {
		term->cx = term->cols;
		term->cy--;
	}
	if (term->cx > 0)
		term->cx--;
}

void term_putsyx(term_win *term, const char *str, const uint8_t y, const uint8_t x)
{
	term_goto(term, x, y);
	while (*str != '\0') {
		term_putch(term, *str);
		if (term->flags.h_overflow)
			break;

		str++;
	}
}

inline void term_putsl(term_win *term, const char *str, const uint8_t line)
{
	term_putsyx(term, str, line, 0);
}

/* initializes std_win terminal */
void term_init(term_win *term)
{
	term->cx = 0;   // set default cursor position
	term->cy = 0;
	term->size = TERM1602_2L_SIZE;
	term->rows = TERM1602_2L_ROWS;
	term->cols = TERM1602_2L_COLS;
	term->flags.update_req = true;
	term_cls(term);     //flush the screen buffer
}
