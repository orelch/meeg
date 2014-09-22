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
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <glob.h>
#include <time.h>
#include <sys/time.h>

#include "edf.h"

/* \brief returns int by parsing len char and skip it
 *
 */
static inline int get_int(char**q, size_t len)
{
    char old;
    int res;
    char *p = *q;

    old = p[8];
    p[8] = '\0';
    res = atoi(p);
    p[8] = old;
    p += 8;
    *q = p;

    return res;
}

int edf_file_parse(edf_t *edf, const char *filename)
{
    FILE *file;
    char buf[256];
    char *p = buf;

    fprintf(stdout, "Parsing file <%s>\n", filename);
    /*
     * 8 ascii : version of this data format (0)
     * 80 ascii : local patient identification (mind item 3 of the additional
     * EDF+ specs)
     * 80 ascii : local recording identification (mind item 4 of the
     * additional EDF+ specs)
     * 8 ascii : startdate of recording (dd.mm.yy) (mind item 2 of the
     * additional EDF+ specs)
     * 8 ascii : starttime of recording (hh.mm.ss)
     * 8 ascii : number of bytes in header record
     * 44 ascii : reserved
     * 8 ascii : number of data records (-1 if unknown, obey item 10 of the
     * additional EDF+ specs)
     * 8 ascii : duration of a data record, in seconds
     * 4 ascii : number of signals (ns) in data record
     *
     * 256 ascii
     */
    file = fopen(filename, "r");
    fread(&buf, sizeof(buf), 1, file);
    fprintf(stderr, "buf: %s\n", buf);

    edf->version = get_int(&p, 8);

    /* -1 for \0
     */
    strncpy(edf->local_patient_id, p, sizeof(edf->local_patient_id) - 1);
    p += 80;

    strncpy(edf->local_record_id, p, sizeof(edf->local_record_id) - 1);
    p += 80;


    strncpy(edf->start_date, p, sizeof(edf->start_date) - 1);
    p += 8;

    strncpy(edf->start_time, p, sizeof(edf->start_time) - 1);
    p += 8;

    edf->header_size = get_int(&p, 8);

    p += 44;

    edf->nb_record = get_int(&p, 8);
    edf->duration = get_int(&p, 8);
    edf->nb_signals = get_int(&p, 4);

    /* TODO check malloc result */
    edf->signal_infos = calloc(edf->nb_signals, sizeof(signal_info_t));


    p = malloc(edf->nb_signals * 256);
    fread(p, edf->nb_signals * 256, 1, file);
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        strncpy(s->label, p, sizeof(s->label) - 1);
        p += 16;
    }
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        strncpy(s->transducer, p, sizeof(s->transducer) - 1);
        p += 80;
    }
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        strncpy(s->physical_dimension, p, sizeof(s->physical_dimension) - 1);
        p += 8;
    }
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        s->physical_min = get_int(&p, 8);
    }
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        s->physical_max = get_int(&p, 8);
    }
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        s->digital_min = get_int(&p, 8);
    }
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        s->digital_max = get_int(&p, 8);
    }
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        strncpy(s->prefiltering, p, sizeof(s->prefiltering) - 1);
        p += 80;
    }
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        s->nb_samples = get_int(&p, 8);
    }

    return 0;
}

/*
 *
 */

void signal_info_print(signal_info_t *s)
{
fprintf(stderr,
        "label:%s\n"
        "transducer:%s\n"
        "physical_dimension:%s\n"
        "physical_min:%d\n"
        "physical_max:%d\n"
        "digital_min:%d\n"
        "digital_max:%d\n"
        "prefiltering:%s\n"
        "nb_samples:%d\n",

        s->label,
        s->transducer,
        s->physical_dimension,
        s->physical_min,
        s->physical_max,
        s->digital_min,
        s->digital_max,
        s->prefiltering,
        s->nb_samples);

}

void edf_print(edf_t *e)
{
    fprintf(stderr, "version: %d\n"
    "local_patient_id:%s\n"
    "local_record_id:%s\n"
    "start_date:%s\n"
    "start_time:%s\n"
    "header_size:%d\n"
    "nb_record:%d\n"
    "duration:%d\n"
    "nb_signals:%d\n",
    e->version,
    e->local_patient_id,
    e->local_record_id,
    e->start_date,
    e->start_time,
    e->header_size,
    e->nb_record,
    e->duration,
    e->nb_signals);
    for (int i = 0; i < e->nb_signals; i++) {
        signal_info_print(&e->signal_infos[i]);
    }
}
