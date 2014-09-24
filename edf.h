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


#ifndef UTIX_EDF_H
#define UTIX_EDF_H


#define EDF_VERSION_STRING "alpha 0.0"

/* XXX: size are one more spec for final \0 */

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

    int16_t        *data;
} signal_info_t;

typedef struct edf_t {
    int   version;
    char  local_patient_id[81];
    char  local_record_id[81];
    char  start_date[9];
    char  start_time[9];
    int   header_size;
    int   nb_records;
    int   duration;
    int   nb_signals;

    signal_info_t  *signal_infos;
} edf_t;



/*
 * \brief Parse an edf file
 *
 * \param[in,out] *edf      a pointer to the edf_struct to fiel with file data
 * \param[in]     *filename the path to the edf file
 *
 * \return 0 on Succes
 */
int edf_file_parse(edf_t *edf, const char *filename);

/* \breif Print on stderr the edf_t struct
 *
 * \param[in]  *edf  a pointer to the edf_t struct to be printed
 *
 * \return
 *             void
 *
 */
void edf_print(edf_t *edf);

/* \breif Print on stderr the signal_info_t struct
 *
 * \param[in] *signal_info a pointer to the signal_info_t struct to be printed
 *
 * \return
 *             void
 *
 */
void signal_info_print(signal_info_t *signal_info);

#endif
