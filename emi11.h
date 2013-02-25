/*-----------------------------------------------------------------------------
 * Exchange Message Interface (EMI) Type 11 Record Header 
 *
 * $Id: emi11.h 12 2009-01-19 20:33:05Z lochnerr $
 * $HeadURL: https://svn.clone1.com/svn/dev/main/trunk/AMADecode/emi11.h $
 *
 * Copyright (C) 2008-2009 Clone Research Corporation 
 * All Rights Reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef emi11_included
#define emi11_included 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
char rec_type			[6];
char date				[6];
char from_no_len		[2];
char from_no			[10];
char overflow			[3];
char to_no_len			[2];
char to_no				[10];
char orig_term_ind		[1];
char trunk_group		[4];
char reserved1			[1];
char cic				[4];
char access_method		[1];
char routing_method		[1];
char dialing_method		[1];
char ani_ind			[1];
char ncta				[1];
char time				[6];	/* hhmmss */
char bill_elapsed		[7];	/* mmmmsst */
char method_of_recording[2];
char return_code		[2];
char from_rao			[3];
char cust_bill_format	[1];
char conference_leg		[2];
char type_of_access		[2];
char reserved2			[1];
char method_of_signaling[1];
char indicators			[20];
char operator_unit		[2];
char recording_point	[6];
char billing_rao		[3];
char indicators2		[10];
char fga_access_no		[10];
char reserved_for_local	[3];
char reserved3			[2];
char neca_company		[4];
char call_event_status	[2];
char reserved4			[2];
char feature_group_id	[1];
char library_code		[2];
char settlement			[1];
char conv_elapsed		[7];	/* mmmmsst */
char orig_lrn			[10];
char orig_ocn			[4];
char orig_lrn_src		[1];
char term_lrn			[10];
char term_ocn			[4];
char term_lrn_src		[1];
char send_ocn			[4];
//char reserved6			[20];
char call_type          [3];
char str_code           [5];
char orig_tgn           [4];
char orig_net           [1];
char term_tgn           [4];
char term_net           [1];
char reserved6			[2];
} EMI11;

#ifdef EMI11_OFFSETS
/*
 * EMI Field Offsets
 */
#define EMI_REC_TYPE_OFFSET				0
#define EMI_DATE_OFFSET					6
#define EMI_FROM_NO_LEN					12
#define EMI_FROM_NO_OFFSET				14
#define EMI_OVERFLOW_OFFSET				24
#define EMI_TO_NO_LEN					27
#define EMI_TO_NO_OFFSET				29
#define EMI_ORIG_TERM_IND_OFFSET		39
#define EMI_TRUNK_GROUP_OFFSET			40
#define EMI_RESERVED1_OFFSET			44
#define EMI_CIC_OFFSET					45
#define EMI_ACCESS_METHOD_OFFSET		49
#define EMI_ROUTING_METHOD_OFFSET		50
#define EMI_DIALING_METHOD_OFFSET		51
#define EMI_ANI_IND_OFFSET				52
#define EMI_NCTA_OFFSET					53
#define EMI_TIME_OFFSET					54
#define EMI_BILL_ELAPSED_OFFSET			60
#define EMI_BILL_MINS_OFFSET			60
#define EMI_BILL_SECS_OFFSET			64
#define EMI_BILL_TENTHS_OFFSET			66
#define EMI_METHOD_OF_RECORDING_OFFSET	67
#define EMI_RETURN_CODE_OFFSET			69
#define EMI_FROM_RAO_OFFSET				71
#define EMI_CUST_BILL_FORMAT_OFFSET		74
#define EMI_CONFERENCE_LEG_OFFSET		75
#define EMI_TYPE_OF_ACCESS_OFFSET		77
#define EMI_RESERVED2_OFFSET			79
#define EMI_METHOD_OF_SIGNALING_OFFSET	80
#define EMI_INDICATORS_OFFSET			81
#define EMI_IND_1_OFFSET				EMI_INDICATORS_OFFSET + 0
#define EMI_IND_2_OFFSET				EMI_INDICATORS_OFFSET + 1
#define EMI_IND_3_OFFSET				EMI_INDICATORS_OFFSET + 2
#define EMI_IND_4_OFFSET				EMI_INDICATORS_OFFSET + 3
#define EMI_IND_5_OFFSET				EMI_INDICATORS_OFFSET + 4
#define EMI_IND_6_OFFSET				EMI_INDICATORS_OFFSET + 5
#define EMI_IND_7_OFFSET				EMI_INDICATORS_OFFSET + 6
#define EMI_IND_8_OFFSET				EMI_INDICATORS_OFFSET + 7
#define EMI_IND_9_OFFSET				EMI_INDICATORS_OFFSET + 8
#define EMI_IND_10_OFFSET				EMI_INDICATORS_OFFSET + 9
#define EMI_IND_11_OFFSET				EMI_INDICATORS_OFFSET + 10
#define EMI_IND_12_OFFSET				EMI_INDICATORS_OFFSET + 11
#define EMI_IND_13_OFFSET				EMI_INDICATORS_OFFSET + 12
#define EMI_IND_14_OFFSET				EMI_INDICATORS_OFFSET + 13
#define EMI_IND_15_OFFSET				EMI_INDICATORS_OFFSET + 14
#define EMI_IND_16_OFFSET				EMI_INDICATORS_OFFSET + 15
#define EMI_IND_17_OFFSET				EMI_INDICATORS_OFFSET + 16
#define EMI_IND_18_OFFSET				EMI_INDICATORS_OFFSET + 17
#define EMI_IND_19_OFFSET				EMI_INDICATORS_OFFSET + 18
#define EMI_IND_20_OFFSET				EMI_INDICATORS_OFFSET + 19
#define EMI_OPERATOR_UNIT_OFFSET		101
#define EMI_RECORDING_POINT_OFFSET		103
#define EMI_BILLING_RAO_OFFSET			109
#define EMI_INDICATORS2_OFFSET			112
#define EMI_IND_21_OFFSET				EMI_INDICATORS2_OFFSET + 0
#define EMI_IND_22_OFFSET				EMI_INDICATORS2_OFFSET + 1
#define EMI_IND_23_OFFSET				EMI_INDICATORS2_OFFSET + 2
#define EMI_IND_24_OFFSET				EMI_INDICATORS2_OFFSET + 3
#define EMI_IND_25_OFFSET				EMI_INDICATORS2_OFFSET + 4
#define EMI_IND_26_OFFSET				EMI_INDICATORS2_OFFSET + 5
#define EMI_IND_27_OFFSET				EMI_INDICATORS2_OFFSET + 6
#define EMI_IND_28_OFFSET				EMI_INDICATORS2_OFFSET + 7
#define EMI_IND_29_OFFSET				EMI_INDICATORS2_OFFSET + 8
#define EMI_IND_30_OFFSET				EMI_INDICATORS2_OFFSET + 9
#define EMI_FGA_ACCESS_NO_OFFSET		122
#define EMI_RESERVED_FOR_LOCAL_OFFSET	132
#define EMI_RESERVED3_OFFSET			135
#define EMI_NECA_COMPANY_OFFSET			137
#define EMI_CALL_EVENT_STATUS_OFFSET	141
#define EMI_RESERVED4_OFFSET			143
#define EMI_FEATURE_GROUP_ID_OFFSET		145
#define EMI_LIBRARY_CODE_OFFSET			146
#define EMI_SETTLEMENT_OFFSET			148
#define EMI_CONV_ELAPSED_OFFSET			149
#define EMI_CONV_MINS_OFFSET			149
#define EMI_CONV_SECS_OFFSET			153
#define EMI_CONV_TENTHS_OFFSET			155
#define EMI_ORIG_LRN_OFFSET				156
#define EMI_ORIG_OCN_OFFSET				166
#define EMI_ORIG_LRN_SRC_OFFSET			170
#define EMI_TERM_LRN_OFFSET				171
#define EMI_TERM_OCN_OFFSET				181
#define EMI_TERM_LRN_SRC_OFFSET			185
#define EMI_SEND_OCN_OFFSET				186
#define EMI_RESERVED6_OFFSET			190
#define EMI_CRLF_OFFSET					210
#define EMI_TERM_NULL_OFFSET			212
#define EMI_STD_210						213

#define EMI_CALL_TYPE_OFFSET			EMI_RESERVED6_OFFSET+0
#define EMI_STRCUTURE_CODE_OFFSET		EMI_RESERVED6_OFFSET+3
#define EMI_ORIG_TGN_OFFSET				EMI_RESERVED6_OFFSET+8
#define EMI_ORIG_NET_OFFSET				EMI_RESERVED6_OFFSET+12
#define EMI_TERM_TGN_OFFSET				EMI_RESERVED6_OFFSET+13
#define EMI_TERM_NET_OFFSET				EMI_RESERVED6_OFFSET+17

#endif /* EMI11_OFFSETS  */

#ifdef EMI11_RETURN_CODES

// Standard EMI Return Codes

#define EMI_INVALID_CIC					"72"
#define EMI_DUPLICATE_MESSAGE 			"80"
#define EMI_INVALID_INDICATOR_19		"AS"
#define EMI_INVALID_SEND_OCN			"BM"

// Non-Standard EMI Return Codes

#define EMI_INVALID_ORIG_TERM_IND		"81"

#endif /* EMI11_RETURN_CODES */

int setEMI(
	EMI11 *emi,
	const char *rec_type,
	const char *date,
	const char *from_no_len,
	const char *from_no,
	const char *to_no_len,
	const char *to_no,
	const char *orig_term_ind,
	const char *trunk_group,
	const char *cic,
	const char *routing_method,
	const char *dialing_method,
	const char *ani_ind,
	const char *time,         /* hhmmss */
	const char *bill_elapsed, /* mmmmsst */
	const char *method_of_signaling,
	const char *indicators,
	const char *recording_point,
	const char *indicators2,
	const char *call_event_status,
	const char *conv_elapsed, /* mmmmsst */
	const char *orig_lrn,
	const char *orig_ocn,
	const char *orig_lrn_src,
	const char *term_lrn,
	const char *term_ocn,
	const char *term_lrn_src,
	const char *reserved6
	); 

#ifdef __cplusplus
}
#endif

#endif /* emi11_included */
