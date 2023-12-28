/*
 * terminal.h
 *
 *  Created on: Nov 24, 2021
 *      Author: InterImpv
 */

#ifndef INC_TERMINAL_H_
#define INC_TERMINAL_H_

#include <stdint.h>

#define TERM1602_1L_SIZE 16u
#define TERM1602_1L_ROWS 1u
#define TERM1602_1L_COLS 16u

#define TERM1602_2L_SIZE 32u
#define TERM1602_2L_ROWS 2u
#define TERM1602_2L_COLS 16u

typedef struct
{
    /* settings */
    uint8_t update_req : 1;
    uint8_t h_overflow : 1;		// if a line is overflown
    uint8_t v_overflow : 1;		// ~ buffer overflow


} term_flags;

/* terminal screen structure */
typedef struct
{
	/* primary screen buffer */
    char win[TERM1602_2L_SIZE];
    uint8_t size;				// = TERM_2L_SIZE;
    uint8_t rows;				// = 2;
    uint8_t cols;				// = 16;

    /* cursor position */
    uint8_t cx;
    uint8_t cy;

    /* settings */
    term_flags flags;

} term_win;

#define term_refresh(term) term->flags.update_req = true
#define term_skip(term) term->flags.update_req = true

//void term_request_update(term_win *term);
//void term_reject_update(term_win *term);

/* sets cursor xpos to @x, ypos to @y */
void term_goto(term_win *term, const uint8_t x, const uint8_t y);

/* clears terminal primary buffer */
void term_cls(term_win *term);

/* sends whole primary buffer to lcd display
 * this function is somewhat slow and sometimes mcu can't keep up with it
 * so it actually redraws the screen only if @updateRequest flag of @std_win is
 * set  * if you need to actively redraw lcd screen every tick use @term_req_upd
 * before calling this */
void term_draw(term_win *term);

/* prints a @ch at current cursor position and advances cursor once */
void term_putch(term_win *term, const char ch);

/* prints given @ch on coordinates @x, @y */
void term_putchyx(term_win *term, const char ch, const uint8_t y, const uint8_t x);

void term_putsyx(term_win *term, const char *str, const uint8_t y, const uint8_t x);
void term_putsl(term_win *term, const char *str, const uint8_t line);

/* erases char at current cursor position and backs cursor once*/
void term_erasech(term_win *term);

/* initializes std_win terminal */
void term_init(term_win *term);

#endif /* INC_TERMINAL_H_ */
