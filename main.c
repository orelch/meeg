/**************************************************************************/
/*                                                                        */
/*  Copyright (C) 2014 Utix                                               */
/*                                                                        */
/*                                                                        */
/*  Should you receive a copy of this source code, you must check you     */
/*  have a proper, written authorization of Utix to hold it. If you       */
/*  don't have such an authorization, you must DELETE all source code     */
/*  files in your possession, and inform Utix of the fact you obtain      */
/*  these files. Should you not comply to these terms, you can be         */
/*  prosecuted in the extent permitted by applicable law.                 */
/*                                                                        */
/*   contact@utix.fr                                                      */
/*                                                                        */
/**************************************************************************/

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
