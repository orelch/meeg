/****************************************************************************/
/*  Copyright © 2014, Utix SAS                                              */
/*  All rights reserved.                                                    */
/*                                                                          */
/*  Redistribution and use in source and binary forms, with or without      */
/*  modification, are permitted provided that the following conditions are  */
/*  met:                                                                    */
/*      * Redistributions of source code must retain the above copyright    */
/*        notice, this list of conditions and the following disclaimer.     */
/*      * Redistributions in binary form must reproduce the above           */
/*        copyright notice, this list of conditions and the following       */
/*        disclaimer in the documentation and/or other materials provided   */
/*        with the distribution.                                            */
/*      * Neither the names of Utix employees nor the names of any          */
/*        contributors may be used to endorse or promote products derived   */
/*        from this software without specific prior written permission.     */
/*                                                                          */
/*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     */
/*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       */
/*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   */
/*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT OWNER  */
/*  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,         */
/*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        */
/*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   */
/*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   */
/*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     */
/*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   */
/*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    */
/****************************************************************************/

#include <stdlib.h>
#include <sysexits.h>
#include <ncurses.h>

#include "edf.h"

#define MARGIN 2

typedef struct signal_display_t {
    flag_t  show:1;
    int     color;
} signal_display_t;

static struct {
    edf_t            *edf;
    WINDOW           *help_win;
    double            row_scale;        /* scale for data feeting screen */
    double            samples_scale;    /* scale for time feeting screen */

    int               sig_id;           /* current signal id             */
    int               zoom;             /* vertical zoom                 */
    int               scroll;           /* time scroll                   */
    int               color;            /* use to rotate between color   */
    signal_display_t *signals_display;  /* display info for each signal  */
} _G;


/*
 * \brief draw axes
 */
static void nc_axes(void)
{
    attron(COLOR_PAIR(2));
    for (int i = 0; i < COLS - 2 * MARGIN; i++) {
        mvaddch(LINES / 2, i + MARGIN, ACS_HLINE);
    }
    mvaddch(LINES / 2, COLS - MARGIN, ACS_RARROW);
    for (int i = 0; i < LINES / 2; i++) {
        mvaddch( MARGIN + i, MARGIN, ACS_VLINE);
    }
    mvaddch(LINES / 2, MARGIN, ACS_LTEE);
    mvaddch(MARGIN, MARGIN, ACS_UARROW);
    attroff(COLOR_PAIR(2));

}

/*
 * \brief draw a signal
 *
 * \param[in] sig_id the id of signal to be printed
 * \param[in] color  the color used to display signal
 */
static void edf_nc_print_signal(int sig_id, int color)
{
    signal_info_t *s = &_G.edf->signal_infos[sig_id];

    attron(color);
    for (int i = 0; i < COLS - 2 * MARGIN; i++) {
        int k = _G.scroll + (i * _G.samples_scale);
        if (k >= s->nb_samples)
            break;

        mvaddch(LINES / 2 - _G.row_scale * _G.zoom / 128 *
                (s->offset + s->scale * s->data[k]),
                i + MARGIN,
                _G.sig_id == sig_id ? 'x' : '.');
    }
    attroff(color);
}


static void help_hidde(void)
{
    if (_G.help_win) {
        wclear(_G.help_win);
        wrefresh(_G.help_win);
    }
}

static void help_show(void)
{
    int row = 3;
    if (_G.help_win) {
        wclear(_G.help_win);
    } else {
        _G.help_win = newwin(LINES / 2, COLS / 2, LINES / 4, COLS / 4);
    }
    box(_G.help_win, 0 , 0);
    mvwprintw(_G.help_win, row++, 4, "h: display/hide this help");
    mvwprintw(_G.help_win, row++, 4, "+: vertical zoom in");;
    mvwprintw(_G.help_win, row++, 4, "-: vertical zoom out");;
    mvwprintw(_G.help_win, row++, 4, "=: back to initial zoom");;
    mvwprintw(_G.help_win, row++, 4, "KEY_UP/KEY_DOWN: browse signals");;
    mvwprintw(_G.help_win, row++, 4, "SPACE: toggle keep signal or not");;
    wrefresh(_G.help_win);
}

static int init_ncurses(edf_t *edf)
{
    int max = 0;
    double max_scale = 0;
    int max_samples = 0;
    initscr();
    if (LINES < 20) {
        fprintf(stderr, "You need a terminal with at least 20 lines.");
        return -1;
    }
    if (COLS < 100) {
        fprintf(stderr, "You need a terminal with at least 100 columns.");
        return -1;
    }
    keypad(stdscr, TRUE);
    nonl();
    cbreak();
    noecho();
    /* Init color */
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
    }
    _G.zoom = 128;
    _G.scroll = 0;
    _G.edf = edf;
    _G.signals_display = calloc(edf->nb_signals, sizeof(signal_display_t));
    for (int i = 0;i < _G.edf->nb_signals; i++) {
        signal_info_t *s = &_G.edf->signal_infos[_G.sig_id];
        /* Set white as default color for signal */
        _G.signals_display[i].color = COLOR_PAIR(7);

        /* search absolute max value */
        max = MAX(max, MAX(s->data_max, -1 * s->data_min));
        /* search max scale, all signal will feet on screen */
        max_scale = MAX(max_scale, s->scale);
        max_samples = MAX(max_samples, s->nb_samples);
    }
    /* Compute row and sample scales */
    _G.row_scale = (double)(LINES / 2  - MARGIN) /
        max / max_scale;
    _G.samples_scale = (double)max_samples / (COLS - 2 * MARGIN);

    return 0;
}

/*
 * \brief display information about current signal
 */
static void nc_current_sig(void) {
    signal_info_t *s = &_G.edf->signal_infos[_G.sig_id];

    attron(_G.signals_display[_G.sig_id].color);
    mvprintw(1, 120, "scale %g", _G.samples_scale);
    mvprintw(0, 1, "%d/%d", _G.sig_id + 1, _G.edf->nb_signals);
    mvprintw(1, 1, s->label);
    mvprintw(1, 50, "physical min %d max %d",
             s->physical_min, s->physical_max);
    mvprintw(0, 50, "digital min %d max %d",
             s->digital_min, s->digital_max);
    mvprintw(0, 120, "data min %d max %d",
             s->data_min, s->data_max);
    mvprintw(1, 120, "scale %g", _G.samples_scale);
    attroff(_G.signals_display[_G.sig_id].color);
}

/*
 * \brief main drawing function
 */
static void nc_main_print(void) {
    clear();
    nc_axes();
    for (int i = 0;i < _G.edf->nb_signals; i++) {
        if (i == _G.sig_id || _G.signals_display[i].show)
            edf_nc_print_signal(i, _G.signals_display[i].color);
    }
    nc_current_sig();
}


/*
 * see edf.h
 */
int edf_nc_display(edf_t *edf)
{
    if (init_ncurses(edf))
        return -1;
    while (42) {
        nc_main_print();
        switch (getch()) {
          case 'q':
            goto end;
          case KEY_DOWN:
            _G.sig_id --;
            if (_G.sig_id < 0) {
                _G.sig_id = 0;
            }
            break;
          case KEY_UP:
            _G.sig_id ++;
            if (_G.sig_id >= edf->nb_signals) {
                _G.sig_id = edf->nb_signals - 1;
            }
            break;
          case '-':
            _G.zoom /= 2;
            break;
          case '=':
            _G.zoom = 128;
            break;
          case '+':
            _G.zoom *= 2;
            break;
          case ' ':
            if (_G.signals_display[_G.sig_id].show) {
                _G.signals_display[_G.sig_id].color = COLOR_PAIR(7);
                _G.signals_display[_G.sig_id].show = false;
            } else {
                _G.signals_display[_G.sig_id].color =
                    COLOR_PAIR(_G.color++ % 5 + 1);
                _G.signals_display[_G.sig_id].show = true;
            }
            break;
          case 'h':
            help_show();
            while (getch() != 'h') {}
            help_hidde();
            break;
          case KEY_RIGHT:
            _G.scroll += 10;
            break;
          case KEY_LEFT:
            _G.scroll -= 10;
            _G.scroll = MAX(_G.scroll, 0);
            break;
        }
    }

end:
    endwin();
    return 0;
}
