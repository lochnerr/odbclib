
/*lint -esym(715, argc) */

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#if (defined(WIN32) || defined(_WIN64)) && !defined(lint)
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <string.h>

#include "sqllib.h"
#include "emi11.h"

static void cvtTime(char *out, const char *val)
{
	char temp[10];
	long value;
	int  t;
	int  ss;
	long mmmm;

	value = atol(val);

	t = value % 10;
	value = value / 10;
	ss = value % 60;
	mmmm = value / 60;

	value = 10000000 + mmmm*1000 + ss*10 + t;

	sprintf(temp, "%ld", value);

	memcpy(out, &temp[1], 8);
}

static void write_emi(FILE *out, SQLLibResults Results)
{
	const char *DateTemp;
	const char *ToNo;
	const char *FromNo;
	const char *TGN;
	const char *CIC;
	const char *BillDur;
	const char *Answered;
	const char *Source;
	const char *ConvDur;
	char emi[sizeof(EMI11)+3];
		
	char *RecPoint;
	char Date[7];
	char Time[7];
	char indicators[21]; 
	char BillDurOut[8];
	char ConvDurOut[8];

//	int          mycols[] = {1, 4, 7, 15, 13, 11, 17, 16, 10, -1};

	DateTemp = SQLLibColumnChar(Results, 1);
	ToNo     = SQLLibColumnChar(Results, 4);
	FromNo   = SQLLibColumnChar(Results, 7);
	TGN      = SQLLibColumnChar(Results, 15);
	CIC      = SQLLibColumnChar(Results, 13);
	BillDur  = SQLLibColumnChar(Results, 11);
	Answered = SQLLibColumnChar(Results, 17);
	Source   = SQLLibColumnChar(Results, 16);
	ConvDur  = SQLLibColumnChar(Results, 10);

	if (strcmp(Source, "LUCENT") == 0) RecPoint = "      ";
	else RecPoint = "999999";

	memcpy(&Date[0], &DateTemp[2], 2); /* YY */
	memcpy(&Date[2], &DateTemp[5], 2); /* MM */
	memcpy(&Date[4], &DateTemp[8], 2); /* DD */
	Date[6] = 0;

	memcpy(&Time[0], &DateTemp[11], 2); /* HH */
	memcpy(&Time[2], &DateTemp[14], 2); /* MM */
	memcpy(&Time[4], &DateTemp[17], 2); /* SS */
	Time[6] = 0;

	strcpy(indicators,"00000003000006200020");   
	if (Answered[0] != 'Y') indicators[18-1] = '2';

	cvtTime(BillDurOut, BillDur);
	cvtTime(ConvDurOut, ConvDur);

	(void)setEMI(
		(EMI11 *)emi,
		/* rec_type            */ "110105",
		/* date                */ Date,
		/* from_no_len         */ "10",
		/* from_no             */ FromNo,
		/* to_no_len           */ "10",
		/* to_no               */ ToNo,
		/* orig_term_ind       */ "1",
		/* trunk_group         */ TGN,
		/* cic                 */ CIC,
		/* routing_method      */ "0",
		/* dialing_method      */ "1",
		/* ani_ind             */ "1",
		/* time                */ Time,
		/* bill_elapsed        */ BillDurOut,
		/* method_of_signaling */ "3",
		/* indicators          */ indicators,
 		/* recording_point     */ RecPoint,
		/* indicators2         */ "1200000000",
		/* call_event_status   */ "10",
		/* conv_elapsed        */ ConvDurOut, /* mmmmsst */
		/* orig_lrn            */ "0000000000",
		/* orig_ocn            */ "5917",
		/* orig_lrn_src        */ "0",
		/* term_lrn            */ "0000000000",
		/* term_ocn            */ "0000",
		/* term_lrn_src        */ "0",
		/* reserved6           */ "00"   // This needs to be fixed...
		);
	
	emi[sizeof(EMI11)+0] = '\n';
	emi[sizeof(EMI11)+1] = 0;
	emi[sizeof(EMI11)+2] = 0;

	fputs(emi,out);

	return;
}

static void processTable(SQLLibConnection Conn, const char *table, FILE *out)
{
	int err;
	char *sql_fmt;
	char sql[2048];
	SQLLibResults Results;

	sql_fmt = 
		"SELECT "
		" ORIG_DATE, DISC_DATE, BILLING_NO, DIALED_NO, TERM_NO, CALLED_NO, "
		" FROM_NO, ORIG_NO, DURATION, BILL_DURATION, ORIG_DURATION, TERM_DURATION, "
		" CIC, ORIG_TRUNK, TERM_TRUNK, DATA_SOURCE, ANSTYPE, CALL_TYPE, "
		" STRUCTURE_CODE, PROCESSED_DATE, REF_NO, FILENAME, BYTE_COUNT, BYTE_OFFSET, "
		" BILLING_JURISDICTION, BILL_TYPE, COST_JURISDICTION, DOM_IDDD, "
		" INITIAL_CELL_SITE, INTL_REGION, JURISDICTION, LAST_CELL_SITE, "
		" OLI, ON_NET, ORIG_CARRIER, PAYPHONE_IND, RBOC, ROAMER, SEQ_NO, "
		" TERM_CARRIER, TRAFFIC_TYPE, REGION, REGION_IND, TRAN_NO, "
		" TREATMENT_CODE, ORIG_OCN, TERM_OCN "
		"FROM %s "
		"WHERE DATA_SOURCE In ('LUCCOM','LUCENT') "
		"AND   CALL_TYPE   In ('141','142'); ";

	sprintf(sql, sql_fmt, table);

	Results = SQLLibExecute(Conn, sql);
	SQLLibBind(Results, 1);

	for (;;) {
		err = SQLLibNext(Results);
		if (err) break;	
		write_emi(out, Results);
	}

	SQLLibFreeResults(&Results);

	return;
}

static int main_impl(int argc, const char * const argv[]) 
{
	int err;
	SQLLibResults Results;
	SQLLibTableInfo  table_entry;
	SQLLibConnection Conn;
	SQLLibConnection Conn2;
	FILE *out = stdout;
	static int TOLLFREE = 0;

	fprintf(stderr,"%s Started.\n", argv[0]);
	
	Conn  = SQLLibConnectionCreate();
	Conn2 = SQLLibConnectionCreate();
	
	(void)SQLLibConnect(Conn, argv[1], argv[2], argv[3]);
	(void)SQLLibConnect(Conn2, argv[1], argv[2], argv[3]);

	Results = SQLLibTables(Conn, NULL, "dbo", "Cimco_20%", "TABLE", &table_entry);  

	out = fopen("TollFreeEMI.txt", "w");
	if (!out) out = stdout;

	for (;;) {
		err = SQLLibNext(Results);
		if (err) break;
		if (strcmp(&table_entry.Name[6], argv[4]) < 0) continue;
		if (strcmp(&table_entry.Name[6], argv[5]) > 0) continue;
		fprintf(stderr, "Processing: %s\t%s\t%s\t%s\t%s\n",
			table_entry.Catalog, table_entry.Schema, table_entry.Name,
			table_entry.Type,    table_entry.Remarks
			);
			processTable(Conn2, table_entry.Name, out);
	}
	SQLLibFreeResults(&Results);
	fclose(out);

	SQLLibConnectionDestroy(&Conn);
	SQLLibConnectionDestroy(&Conn2);

	fprintf(stderr,"%s Finished.\n", argv[0]);

	return 0;
}

int main(int argc, const char *const argv[])
{
	int rc;
#if (defined(WIN32) || defined(_WIN64)) && !defined(lint)
	// Get current debug flag
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
#endif

	if (argc < 6) {
		fprintf(stderr, "Format: %s DSN      Username Passcode FromDate ToDate\n", argv[0]);
		fprintf(stderr, "e.g:    %s TeleLink sa       yourpass 20090218 20090317\n", argv[0]);
		return 1;
	}

#if (defined(WIN32) || defined(_WIN64)) && !defined(lint)
	// Turn on leak-checking bit.
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	// Turn off CRT block checking bit.
	//tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
	// Set flag to the new value.
	_CrtSetDbgFlag( tmpFlag );
	// Set the report mode and file
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE |  _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
#endif

	rc = main_impl(argc, argv);

#if (defined(WIN32) || defined(_WIN64)) && !defined(lint)
	//_CrtDumpMemoryLeaks(); /* This is automatic with report file set */
#endif

	return rc;
}
