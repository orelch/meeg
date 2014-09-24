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

#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <getopt.h>
#include <string.h>


#include "edf.h"


static void usage(int full, const char *arg0)
{
    printf("%s - Version " EDF_VERSION_STRING "\n"
           "Copyright (C) 2014 UTIX SAS \n"
           "\n", arg0);

    if (!full)
        return;

    printf("usage: %s [options] filename\n"
           "\n"
           "    filename       path to edf file\n"
           "\n"
           "Options:\n"
           "    -h             show this help\n"
           "    -V             show version\n", arg0
    );
}

int main(int argc, char *argv[])
{
    const char *arg0 = argv[0];

    int c;
    edf_t edf;

    memset(&edf, 0, sizeof(edf));
    while ((c = getopt(argc, argv, "hV")) >= 0) {
        switch (c) {
          case 'h':
          case 'V':
            usage(c == 'h', arg0);
            return EX_OK;

          default:
            usage(1, arg0);
            return EX_USAGE;
        }
    }
    if (optind < argc - 1) {
        fprintf(stderr, "Too many arguments (see %s -h)\n", arg0);
        return EX_USAGE;
    }
    if (optind >= argc) {
        fprintf(stderr, "Missing arguments (see %s -h)\n", arg0);
        return EX_USAGE;
    }

    edf_file_parse(&edf, argv[argc - 1]);
    edf_print(&edf);
    return EX_OK;
}
