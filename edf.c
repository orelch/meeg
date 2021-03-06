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
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <glob.h>
#include <time.h>
#include <sys/time.h>

#include "edf.h"


edf_t* edf_init(edf_t *edf)
{
    memset(edf, 0, sizeof(*edf));
}

void edf_wipe(edf_t *edf)
{
    if (!edf)
        return;
    if (edf->signal_infos) {
        for (int i = 0; i < edf->nb_signals; i++) {
            signal_info_wipe(&edf->signal_infos[i]);
        }
        free (edf->signal_infos);
    }
    edf = NULL;
}


void signal_info_wipe(signal_info_t *signal_info)
{
    if (!signal_info)
        return;
    if (signal_info->data) {
        free(signal_info->data);
        signal_info->data = NULL;
    }
}

/* \brief returns int by parsing len char and skip it
 *
 */
static inline int get_int(char**q, size_t len)
{
    char old;
    int res;
    char *p = *q;

    old = p[len];
    p[len] = '\0';
    res = atoi(p);
    p[len] = old;
    p += len;
    *q = p;

    return res;
}

static inline void compute_scale_offset(signal_info_t *s)
{
       s->scale = (double)(s->physical_max - s->physical_min)/
           (s->digital_max - s->digital_min);
       s->offset = (double)s->physical_max - s->scale * s->digital_max;
}


int edf_file_parse(edf_t *edf, const char *filename)
{
    FILE *file;
    char buf[256];
    char *p = buf;

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

    edf->version = get_int(&p, 8);

    /* -1 for final \0
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

    edf->nb_records = get_int(&p, 8);
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
        compute_scale_offset(s);
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

    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        s->data = malloc(s->nb_samples * edf->nb_records * sizeof(int16_t));
    }
    /*XXX: kept i for nb_signals to be coherent with previous loop */
    for (int k = 0; k < edf->nb_records; k++) {
        for (int i = 0; i < edf->nb_signals; i++) {
            signal_info_t *s = &edf->signal_infos[i];
            fread(&s->data[k * s->nb_samples], s->nb_samples,
                  sizeof(int16_t), file);
        }
    }
    /* Lookup to find real max and real min, we can't trust header */
    for (int i = 0; i < edf->nb_signals; i++) {
        signal_info_t *s = &edf->signal_infos[i];
        s->data_min = INT16_MAX;
        s->data_max = INT16_MIN;
        for (int j = 0; j < s->nb_samples * edf->nb_records; j++) {
            s->data_min = MIN(s->data_min, s->data[j]);
            s->data_max = MAX(s->data_max, s->data[j]);
        }
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
            "nb_records:%d\n"
            "duration:%d\n"
            "nb_signals:%d\n",
            e->version,
            e->local_patient_id,
            e->local_record_id,
            e->start_date,
            e->start_time,
            e->header_size,
            e->nb_records,
            e->duration,
            e->nb_signals);
    for (int i = 0; i < e->nb_signals; i++) {
        signal_info_print(&e->signal_infos[i]);
    }
}
