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


#ifndef UTIX_EDF_H
#define UTIX_EDF_H


#define EDF_VERSION_STRING "alpha 0.0"

/* XXX: size are one more spec for \0 */
typedef struct signal_info_t {
    char label[17];
    char transducer[81];
    char physical_dimension[9];
    int  physical_min;
    int  physical_max;
    int  digital_min;
    int  digital_max;
    char prefiltering[81];
    int  nb_samples;
} signal_info_t;

typedef struct edf_t {
    int   version;
    char  local_patient_id[81];
    char  local_record_id[81];
    char  start_date[9];
    char  start_time[9];
    int   header_size;
    int   nb_record;
    int   duration;
    int   nb_signals;

    signal_info_t  *signal_infos;
    int            *data;
} edf_t;

int edf_file_parse(edf_t *edf, const char *filename);

/* \breif Print on stderr the edf_t struct
 *
 * \param[in]  *e  a pointer to the edf_t struct to be printed
 *
 * \return
 *             void
 *
 */
void edf_print(edf_t *edf);
void signal_info_print(signal_info_t *signal_info);

#endif
