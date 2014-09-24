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

#include <sysexits.h>
#include <ncurses.h>

#include "edf.h"

static struct {
    WINDOW *draw_win;
} _G;



void edf_nc_print_signal(edf_t *e, int sig_id)
{
    const int margin = 2;
    signal_info_t *s = &e->signal_infos[sig_id];
    /* XXX:  min - max to have a negative value for have a classic y axe */
    double row_scale = (double)LINES / (s->digital_min - s->digital_max) / 4;
    double samples_scale = (double)s->nb_samples / (COLS - 2 * margin);

    clear();
    mvprintw(1, 1, "%d/%d", sig_id + 1, e->nb_signals);
    mvprintw(2, 1, s->label);
    for (int i = 0; i < COLS - 2 * margin; i++) {
        mvaddch(LINES / 2 + row_scale * s->data [(int)(i * samples_scale)],
                i + margin,
                'x');
    }
    refresh();
}


static int init_ncurses(void)
{
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
    if (_G.draw_win) {
        wclear(_G.draw_win);
    } else {
        _G.draw_win = newwin(LINES - 10, COLS, 10, 0);
    }
    return 0;
}



int edf_nc_display(edf_t *edf)
{
    int sig_id = 0;
    if (init_ncurses())
        return -1;
    edf_nc_print_signal(edf, 0);
    while (42) {
        switch (getch()) {
          case 'q':
            goto end;
          case KEY_DOWN:
            sig_id --;
            if (sig_id < 0) {
               sig_id = 0;
            }
            edf_nc_print_signal(edf, sig_id);
            break;
          case KEY_UP:
            sig_id ++;
            if (sig_id >= edf->nb_signals) {
               sig_id = edf->nb_signals - 1;
            }
            edf_nc_print_signal(edf, sig_id);
            break;
        }
    }

end:
    endwin();
    return 0;
}
