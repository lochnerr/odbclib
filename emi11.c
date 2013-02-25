/**/ 
#include <string.h>
#include <ctype.h>

#if defined(WIN32) || defined(_WIN64)
#include "/rick/projects/AMADecode/emi11.h"
#else
#include "emi11.h"
#endif

int emicpy(char *dst, const char *src, int len)
{
	/* This function copies the leftmost "len" bytes of the source 
	 * to the destination.  
	 */
	//int srclen;

	int i;
	char pad;

	if (src == 0 || dst == 0 || len < 1) return 0;

	pad = 0;
	for (i=0; i<len; i++) {
		if (src[i] == 0) pad = '0';
		if (pad) {
			dst[i] = pad;
		} else {
			if (isprint(src[i])) dst[i] = src[i];
			else dst[i] = ' ';
		}
	}

	//srclen = (int)strlen(src);
	//if (srclen < len) len = srclen; 
	//(void)emicpy(dst,src,len); 

	return len;
}

int setEMI(
//	char *emi,
	EMI11 *emi,
	const char *rec_type,
	const char *date,
	const char *from_no_len,
	const char *from_no,
//	const char *overflow,
	const char *to_no_len,
	const char *to_no,
	const char *orig_term_ind,
	const char *trunk_group,
//	const char *reserved1,
	const char *cic,
//	const char *access_method,
	const char *routing_method,
	const char *dialing_method,
	const char *ani_ind,
//	const char *ncta,
	const char *time,         /* hhmmss */
	const char *bill_elapsed, /* mmmmsst */
//	const char *method_of_recording,
//	const char *return_code,
//	const char *from_rao,
//	const char *cust_bill_format,
//	const char *conference_leg,
//	const char *type_of_access,
//	const char *reserved2,
	const char *method_of_signaling,
	const char *indicators,
//	const char *operator_unit,
	const char *recording_point,
//	const char *billing_rao,
	const char *indicators2,
//	const char *fga_access_no,
//	const char *reserved_for_local,
//	const char *reserved3,
//	const char *neca_company,
	const char *call_event_status,
//	const char *reserved4,
//	const char *feature_group_id,
//	const char *library_code,
//	const char *settlement,
	const char *conv_elapsed, /* mmmmsst */
	const char *orig_lrn,
	const char *orig_ocn,
	const char *orig_lrn_src,
	const char *term_lrn,
	const char *term_ocn,
	const char *term_lrn_src,
//	const char *send_ocn,
	const char *reserved6
	) 
{

	memset(emi, '-', sizeof(EMI11));

	/* fill in the emi */

	(void)emicpy(emi->rec_type,                           rec_type,            sizeof(emi->rec_type) );
	(void)emicpy(emi->date,                               date,                sizeof(emi->date) );
	(void)emicpy(emi->from_no_len,                        from_no_len,         sizeof(emi->from_no_len) );
	(void)emicpy(emi->from_no,                            from_no,             sizeof(emi->from_no) );
	(void)emicpy(emi->overflow,                           "000",               sizeof(emi->overflow) );
	(void)emicpy(emi->to_no_len,                          to_no_len,           sizeof(emi->to_no_len) );
	(void)emicpy(emi->to_no,                              to_no,               sizeof(emi->to_no) );
	(void)emicpy(emi->orig_term_ind,                      orig_term_ind,       sizeof(emi->orig_term_ind) );
	(void)emicpy(emi->trunk_group,                        trunk_group,         sizeof(emi->trunk_group) );
	(void)emicpy(emi->reserved1,                          "0",                 sizeof(emi->reserved1) );
	(void)emicpy(emi->cic,                                cic,                 sizeof(emi->cic) );
	(void)emicpy(emi->access_method,                      "1",                 sizeof(emi->access_method) );
	(void)emicpy(emi->routing_method,                     routing_method,      sizeof(emi->routing_method) );
	(void)emicpy(emi->dialing_method,                     dialing_method,      sizeof(emi->dialing_method) );
	(void)emicpy(emi->ani_ind,                            ani_ind,             sizeof(emi->ani_ind) );
	(void)emicpy(emi->ncta,                               "0",                 sizeof(emi->ncta) );
	(void)emicpy(emi->time,                               time,                sizeof(emi->time) );
	(void)emicpy(emi->bill_elapsed,                       bill_elapsed,        sizeof(emi->bill_elapsed) );
	(void)emicpy(emi->method_of_recording,                "02",                sizeof(emi->method_of_recording) );
	(void)emicpy(emi->return_code,                        "00",                sizeof(emi->return_code) );
	(void)emicpy(emi->from_rao,                           "000",               sizeof(emi->from_rao) );
	(void)emicpy(emi->cust_bill_format,                   "0",                 sizeof(emi->cust_bill_format) );
	(void)emicpy(emi->conference_leg,                     "00",                sizeof(emi->conference_leg) );
	(void)emicpy(emi->type_of_access,                     "00",                sizeof(emi->type_of_access) );
	(void)emicpy(emi->reserved2,                          "0",                 sizeof(emi->reserved2) );
	(void)emicpy(emi->method_of_signaling,                method_of_signaling, sizeof(emi->method_of_signaling) );
	(void)emicpy(emi->indicators,                         indicators,          sizeof(emi->indicators) );
	(void)emicpy(emi->operator_unit,                      "00",                sizeof(emi->operator_unit) );
	(void)emicpy(emi->recording_point,                    recording_point,     sizeof(emi->recording_point) );
	(void)emicpy(emi->billing_rao,                        "000",               sizeof(emi->billing_rao) );
	(void)emicpy(emi->indicators2,                        indicators2,         sizeof(emi->indicators2) );
	(void)emicpy(emi->fga_access_no,                      "0000000000",        sizeof(emi->fga_access_no) );
	(void)emicpy(emi->reserved_for_local,                 "   ",               sizeof(emi->reserved_for_local) );
	(void)emicpy(emi->reserved3,                          "00",                sizeof(emi->reserved3) );
	(void)emicpy(emi->neca_company,                       "0000",              sizeof(emi->neca_company) );
	(void)emicpy(emi->call_event_status,                  call_event_status,   sizeof(emi->call_event_status) );
	(void)emicpy(emi->reserved4,                          "00",                sizeof(emi->reserved4) );
	(void)emicpy(emi->feature_group_id,                   "D",                 sizeof(emi->feature_group_id) );
	(void)emicpy(emi->library_code,                       "  ",                sizeof(emi->library_code) );
	(void)emicpy(emi->settlement,                         "Z",                 sizeof(emi->settlement) );
	(void)emicpy(emi->conv_elapsed,                       conv_elapsed,        sizeof(emi->conv_elapsed) );
	(void)emicpy(emi->orig_lrn,                           orig_lrn,            sizeof(emi->orig_lrn) );
  	(void)emicpy(emi->orig_ocn,                           orig_ocn,            sizeof(emi->orig_ocn) );
	(void)emicpy(emi->orig_lrn_src,                       orig_lrn_src,        sizeof(emi->orig_lrn_src) );
	(void)emicpy(emi->term_lrn,                           term_lrn,            sizeof(emi->term_lrn) );
  	(void)emicpy(emi->term_ocn,                           term_ocn,            sizeof(emi->term_ocn) );
	(void)emicpy(emi->term_lrn_src,                       term_lrn_src,        sizeof(emi->term_lrn_src) );
  	(void)emicpy(emi->send_ocn,                           "0000",              sizeof(emi->send_ocn) );
	(void)emicpy(emi->call_type,                          "000",               sizeof(emi->call_type) );
	(void)emicpy(emi->str_code,                           "00000",             sizeof(emi->str_code) );
	(void)emicpy(emi->orig_tgn,                           "0000",              sizeof(emi->orig_tgn) );
	(void)emicpy(emi->orig_net,                           "0",                 sizeof(emi->orig_net) );
	(void)emicpy(emi->term_tgn,                           "0000",              sizeof(emi->term_tgn) );
	(void)emicpy(emi->term_net,                           "0",                 sizeof(emi->term_net) );
	(void)emicpy(emi->reserved6,                          "00",                sizeof(emi->reserved6) );

	/* If this is a toll-free or an 800 database access record, */
	/* copy the to number to the fga access number field. */

	if ((memcmp(emi->rec_type,"110105",6) == 0) ||
		(memcmp(emi->rec_type,"110125",6) == 0))
		memcpy(emi->fga_access_no,emi->to_no,10);

	//emi[EMI_CRLF_OFFSET]      = '\n'; /* add a new line */
	//emi[EMI_CRLF_OFFSET+1]    = '\0'; 
	//emi[EMI_TERM_NULL_OFFSET] = '\0'; /* string terminator */

	return 0;
}
