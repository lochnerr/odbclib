
/*lint -esym(715, argc) */

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#if (defined(WIN32) || defined(_WIN64)) && !defined(lint)
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqllib.h"

#include "ValuesMap.h"

static void trace(const char *msg)
{
	fprintf(stderr,"%s\n",msg);
}

/*******************************/
/* qsort example */

int values[] = { 40, 10, 100, 90, 20, 25, 30 };

char *vals[] = { "9","8","1","2","0","3","4" };

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int compstr (const void * a, const void * b)
{
  return strcmp( *(char**)a, *(char **)b );
}

int sortem ()
{
  int n;
  //qsort (values, 6, sizeof(int), compare);
  qsort (vals, 7, sizeof(char*), compstr);
  for (n=0; n</*6*/7; n++)
	  fprintf (stderr,"%s ",vals[n]);
     //printf ("%d ",values[n]);
  return 0;
}

//void *get_work(void *q);
//int put_work(void *q, void *wunit);

/*******************************/

typedef struct source_info source_info;

typedef struct column_info {
	long nulls;
	long unique;
	string_map *values;
} column_info;

typedef struct source_info {
	int columns;
	long rows;
	column_info *column;
} source_info;

void free_source_info(source_info *source) 
{
	int col;

	if (!source) return;

	if (source->column) {

		for (col=0; col<source->columns; col++) {
			value_map_string_destroy(source->column[col].values);
		}

		free(source->column);

	}

	free(source);
}

char *getQualifiedName(const char *catalog, const char *schema, const char *name, char *buff, size_t blen) 
{
	/* Prepare the name */

	buff[0] = 0; // make empty

	if (!catalog) catalog = "";
	if (!schema)  schema  = "";
	if (!name)    name    = "null";

	if (catalog[1] == ':') {
		/* MS-Access Table --- ignore catalog and schema */
	} else if (catalog[0] == 0) {
		/* null catalog name */
		if (schema[0] != 0) {
			/* null catalog and non-null schema name */
			//sprintf(buff, "\"%s\".", schema);
			strncat(buff, "\"", blen);
			strncat(buff, schema, blen);
			strncat(buff, "\".", blen);
		}
	} else {
		/* non-null catalog name */
		if (schema[0] == 0) {
			/* null schema name */
			//sprintf(sql, "\"%s\"..", catalog);
			strncat(buff, "\"", blen);
			strncat(buff, catalog, blen);
			strncat(buff, "\"..", blen);
		} else {
			/* non-null schema name */
			//sprintf(sql, "\"%s\".\"%s\".", catalog, schema);
			strncat(buff, "\"", blen);
			strncat(buff, catalog, blen);
			strncat(buff, "\".\"", blen);
			strncat(buff, schema, blen);
			strncat(buff, "\".", blen);
		}
	}
	strncat(buff, "\"", blen);
	strncat(buff, name, blen);
	strncat(buff, "\"", blen);

	return buff;
}


source_info *process_table(SQLLibConnection conn, const char *catalog, const char *schema, const char *name) 
{
	long recs;
	SQLLibResults Results;
	char sql[1024];
	int  err;
	int  col;
	int  minl, maxl;
	const char *pk;
	source_info *source;
	long count;

	source = malloc(sizeof(source_info));
	if (!source) return source;

	source->rows = 0;
	source->columns = 0;
	source->column = 0;

	recs = 0;

	/* Prepare the SELECT * FROM statement */

	strcpy(sql, "SELECT * FROM ");
	if (catalog[1] == ':') {
		/* MS-Access Table --- ignore catalog and schema */
	} else if (catalog[0] == 0) {
		/* null catalog name */
		if (schema[0] != 0) {
			/* null catalog and non-null schema name */
			sprintf(sql, "SELECT * FROM \"%s\".", schema);
		}
	} else {
		/* non-null catalog name */
		if (schema[0] == 0) {
			/* null schema name */
			sprintf(sql, "SELECT * FROM \"%s\"..", catalog);
		} else {
			/* non-null schema name */
			sprintf(sql, "SELECT * FROM \"%s\".\"%s\".", catalog, schema);
		}
	}
	strcat(sql, "\"");
	strcat(sql, name);
	strcat(sql, "\";");
	fprintf(stderr, "%s ", sql);

	/* Execute the SELECT statement */

	Results = SQLLibExecute(conn, sql);

	/* Save the number of columns */

	source->columns = SQLLibColumnCount(Results);

	/* Allocate the column maps */

	source->column = malloc(sizeof(column_info) * source->columns);
	if (!source->column) return source;

	for (col=0; col<source->columns; col++) {
		source->column[col].nulls = 0;
		source->column[col].unique = 0;
		source->column[col].values = value_map_string_create();
	}

	/* Bind the results to character fields (for now) */

	SQLLibBind(Results,SQLLibBindToChar);

	/* Process each row */

	for (;;) {
		err = SQLLibNext(Results);
		if (err) break;
		recs++;

		/* increment the value count for each value in each column */

		for (col=0; col<source->columns; col++) {
			if (SQLLibIsNull(Results, col+1)) {
				source->column[col].nulls++;
			} else {
				count = value_map_string_increment(source->column[col].values, SQLLibColumnChar(Results, col+1));
				if (count == 1) source->column[col].unique++; /* a new value was added */
			}
		}
	}

	/* Free the results structure */

	//SQLLibFreeResults(&Results);

	/* Store the row count */

	source->rows = recs;

	fprintf(stderr,"%d %ld\n", (int)source->columns,(long)source->rows);
	
	for (col=0; col<source->columns; col++) {
		pk = "";
		if (source->column[col].unique == source->rows && source->rows > 1) pk = "(possible primary key)";
		value_map_string_lengths(source->column[col].values, &minl, &maxl);
		fprintf(stderr, "column %2d has\t%6ld\tnulls and\t%6ld\tunique len\t%3d-%-3d\t%s\t%s\t%s\n", 
			(int)col, 
			(long)source->column[col].nulls, 
			(long)source->column[col].unique,
			(int)minl,
			(int)maxl,
			name,
			SQLLibColumnName(Results,col+1), 
			pk
			);
	}

	// need to delete info and init values[] -- at some point, not in here

	free_source_info(source);  // TESTING ONLY

	/* Free the results structure */

	SQLLibFreeResults(&Results);

	return source;
}

long get_table_rows(SQLLibConnection conn, const char *catalog, const char *schema, const char *name, const char *type) 
{
	long recs;
	SQLLibResults Results;
	char sql[1024];
	char fqn[512];
	int  err;

	recs = -1;

	if (strcmp(type, "TABLE") == 0) recs = 0;
	if (recs < 0) return recs;

	getQualifiedName(catalog, schema, name, fqn, sizeof(fqn));

	sprintf(sql, "SELECT COUNT(*) FROM %s;", fqn);

	Results = SQLLibExecute(conn, sql);
	SQLLibBind(Results,SQLLibBindToChar);

	err = SQLLibNext(Results);
	if (!err) {
		if (!SQLLibIsNull(Results, 1)) {
			recs = atol(SQLLibColumnChar(Results, 1));
		}
	}

	SQLLibFreeResults(&Results);

	return recs;
}

const char *fix_quotes(const char *val, char *scratch, size_t size) {
	size_t i;
	if (!val) return "";
	for (i=0; ; i++) {
		if (i >= size) { // buffer overflow
			if (size < 1 || !scratch) return "";
			else scratch[size-1] = '\0';
			break;
		}
		if (val[i] == '"') {
			scratch[i] = '\'';
		} else {
			scratch[i] = val[i];
		}
		if (val[i] == '\0') break;
	}
	return scratch;
}

int get_table_columns(SQLLibConnection conn, const char *catalog, const char *schema, 
					  const char *name, const char *type, FILE *output, long TableID) 
{
	char scratch[1024];
	SQLLibResults Results;
	SQLLibColumnInfo coldef;
	int i;
	int err;

	if (strcmp(type, "TABLE") != 0) 
		return 0;
		
	Results = SQLLibColumns(conn, catalog, schema, name, "%", &coldef);
	for (i=1; ;i++) {
		//memset(&coldef,0,sizeof(coldef));
		SQLLibColumnInfoClear(&coldef);
		err = SQLLibNext(Results);
		if (err) break;
		fprintf(output,"%d,\"%s\",%d,\"%s\",%d,%d,%d,%d,%d,\"%s\",\"%s\",%d,%ld,%ld,\"%s\"\n",
			(int)TableID, 
			coldef.Name,
			(int)coldef.DataType,
			coldef.TypeName,
			(int)coldef.ColumnSize,
			(int)coldef.BufferLength,
			(int)coldef.DecimalDigits,
			(int)coldef.Precision,
			(int)coldef.Nullable,
			fix_quotes(coldef.Remarks, scratch, sizeof(scratch)),
			coldef.Default,
			(int)coldef.DateTime,
			(long)coldef.OctetLength,
			(long)coldef.OrdinalPosition,
			coldef.IsNullable
			);
	}
	SQLLibFreeResults(&Results);

	return 0;
}

int get_table_key(SQLLibConnection conn, const char *catalog, const char *schema, 
					  const char *name, const char *type, FILE *output, long TableID) 
{
	SQLLibResults Results;
	SQLLibIdentInfo def;
	int i;
	int err;

	if (strcmp(type, "TABLE") != 0) 
		return 0;
		
	Results = SQLLibIdentRows(conn, catalog, schema, name, &def);
	for (i=1; ;i++) {
		SQLLibIdentInfoClear(&def);
		err = SQLLibNext(Results);
		if (err) break;
		fprintf(output,"%d,%d,\"%s\",%d,\"%s\",%d,%d,%d,%d,%d\n",
			(int)TableID, 
			(int)i, 
			def.Name,
			(int)def.DataType,
			def.TypeName,
			(int)def.ColumnSize,
			(int)def.BufferLength,
			(int)def.DecimalDigits,
			(int)def.Pseudo,
			(int)def.Scope
			);
	}
	SQLLibFreeResults(&Results);

	return 0;
}

int get_table_indexes(SQLLibConnection conn, const char *catalog, const char *schema, 
					  const char *name, const char *type, FILE *output, long TableID) 
{
	SQLLibResults Results;
	SQLLibIndexInfo def;
	int i;
	int err;

	if (strcmp(type, "TABLE") != 0) 
		return 0;
		
	Results = SQLLibIndexes(conn, catalog, schema, name, &def);
	for (i=1; ;i++) {
		SQLLibIndexInfoClear(&def);
		err = SQLLibNext(Results);
		if (err) break;
		if (def.Name[0] == 0) continue;
		fprintf(output,"%d,%d,\"%s\",\"%s\",%d,%d,\"%s\",\"%s\"\n",
			(int)TableID,
			(int)def.NonUnique,
			def.Qualifier,
			def.IndexName,
			(int)def.IndexType,
			(int)def.OrdinalPosition,
			def.Name,
			def.Asc_Or_Desc
			);
	}
	SQLLibFreeResults(&Results);

	return 0;
}

static int main_impl(int argc, const char * const argv[]) 
{
	int err;
	int TableID;
	const char *msg;
	long recs;
	FILE *ftables;
	FILE *fcolumns;
	FILE *findexes;
	FILE *fkeys;
	char temp[1024];
	char *path;
	SQLLibResults Results;
	SQLLibSourceInfo sources;
	SQLLibTableInfo  table_entry;
	SQLLibConnectionInfo ConnInfo;
	SQLLibConnection Conn;
	SQLLibConnection Conn2;

//	path = "";
	path = "c:\\odbc\\";

	fprintf(stderr, "\t\t\t\t\t\t\t\t\t\t\n");
#ifdef UNICODE
	trace("Using UNICODE libraries.");
#else
	trace("Using ASCII libraries.");
#endif

//	fprintf(stderr, "ODBC Version is 0x%04lx.\n",(long)ODBCVER);
	
#ifndef lint
	msg = malloc(128); /* Test memory leak */
#endif

	trace("Getting Sources Function.");
	Results = SQLLibSources(&sources /*NULL*/);
	fprintf(stderr,"\n%-30s %s\n\n","Data Source","Description");
	do {
		err = SQLLibNext(Results);
		if (err) break;
//		fprintf(stderr, "%-30s", SQLLibColumnChar(Results,1));
//		fprintf(stderr, " %s\n", SQLLibColumnChar(Results,2));
		fprintf(stderr, "%-30s", sources.Name);
		fprintf(stderr, " %s\n", sources.Description);
		SQLLibSourceInfoClear(&sources);
	} while (!err);
	fprintf(stderr,"\n");
	SQLLibFreeResults(&Results);

	(void)sortem();
	fprintf(stderr,"\n");

	if (argc < 4) {
		fprintf(stderr, "Format: %s DSN Username Passcode\n\n", argv[0]);
		return 1;
	}

	trace("Allocating SQL Connection Handle(s).");

	Conn  = SQLLibConnectionCreate();
	Conn2 = SQLLibConnectionCreate();
	
	trace("Connecting to Datasource.");
//	"PT3 DB", "postgres", "clone15654!"
//	"w2k3ee64-dev", "sa", "clone15654!"
//	"TeleLink", "sa", "clone15654!"
//	"LERG", "admin", ""
	err = SQLLibConnect(Conn, argv[1], argv[2], argv[3]);
	if (err) {
		fprintf(stderr,"Connection to Datasource '%s' failed.", argv[1]);
		return 1;
	}

	trace("Connecting to Datasource again.");	
	(void)SQLLibConnect(Conn2, argv[1], argv[2], argv[3]);

	trace("Get Functions.");
//	if (isSupported(ConnectionHandle, SQL_API_SQLFETCH)) 
//		msg = "Function SQL_API_SQLFETCH Supported";
//	else 
//		msg = "Function SQL_API_SQLFETCH Not Supported";
//	fprintf(stderr,"%s\n", msg);
////	check_em(ConnectionHandle);

	trace("Get Connection Info.");
	SQLLibConnectInfo(Conn, &ConnInfo);
	fprintf(stderr,"Server is %s ", ConnInfo.DBMSName);
	fprintf(stderr,"Version %s.\n", ConnInfo.DBMSVersion);
	fprintf(stderr,"X/Open Specification Year is %s.\n", ConnInfo.XOpenYear);

	strcpy(temp, path);
	strncat(temp, argv[1], sizeof(temp));
	strncat(temp, "_tables.csv", sizeof(temp));
	ftables = fopen(temp, "w");
	if (!ftables) {
		fprintf(stderr,"Open of file '%s' for output failed.\n", temp);
		return 1;
	}

	strcpy(temp, path);
	strncat(temp, argv[1], sizeof(temp));
	strncat(temp, "_columns.csv", sizeof(temp));
	fcolumns = fopen(temp, "w");
	if (!fcolumns) {
		fprintf(stderr,"Open of file '%s' for output failed.\n", temp);
		return 1;
	}

	strcpy(temp, path);
	strncat(temp, argv[1], sizeof(temp));
	strncat(temp, "_keys.csv", sizeof(temp));
	fkeys = fopen(temp, "w");
	if (!fkeys) {
		fprintf(stderr,"Open of file '%s' for output failed.\n", temp);
		return 1;
	}

	strcpy(temp, path);
	strncat(temp, argv[1], sizeof(temp));
	strncat(temp, "_indexes.csv", sizeof(temp));
	findexes = fopen(temp, "w");
	if (!findexes) {
		fprintf(stderr,"Open of file '%s' for output failed.\n", temp);
		return 1;
	}
	
	fprintf(ftables,"\"TableID\",\"Catalog\",\"Schema\",\"Name\",\"Type\",\"Remarks\",\"Rows\"\n");

	fprintf(fcolumns,"\"TableID\",\"Name\",\"DataType\",\"TypeName\",\"ColumnSize\","
		"\"BufferLength\",\"Digits\",\"Precision\",\"Nullable\",\"Remarks\",\"Default\","
		"\"DateTime\",\"OctetLen\",\"Ordinal\",\"IsNullable\"\n");

	fprintf(fkeys,"\"TableID\",\"SeqNo\",\"Name\",\"DataType\",\"TypeName\",\"ColumnSize\","
		"\"BufferLength\",\"Digits\",\"Pseudo\",\"Scope\"\n");

	fprintf(findexes,"\"TableID\",\"NonUnique\",\"Qualifier\",\"IndexName\",\"IndexType\","
		"\"Ordinal\",\"Name\",\"Order\"\n");

	trace("Execute Tables Function.");
	msg = "TABLE"; msg = NULL;
	Results = SQLLibTables(Conn, NULL, NULL, NULL, msg, &table_entry);
	
	for (TableID=1; ; TableID++) {
		err = SQLLibNext(Results);
		if (err) break;
		//if (TableID > 30) break;  // TESTING 1 2 3
//4-22-11		process_table(Conn2, table_entry.Catalog, table_entry.Schema, table_entry.Name);
		recs = get_table_rows(Conn2, table_entry.Catalog, table_entry.Schema, table_entry.Name, table_entry.Type);
		
		fprintf(stderr, "%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%ld\n", TableID,
			table_entry.Catalog, table_entry.Schema, table_entry.Name,
			table_entry.Type,    table_entry.Remarks, recs
			);

		fprintf(ftables,"%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%ld\n", TableID,
			table_entry.Catalog, table_entry.Schema, table_entry.Name,
			table_entry.Type,    table_entry.Remarks, recs
			);
		
		get_table_columns(Conn2, table_entry.Catalog, table_entry.Schema, 
			table_entry.Name, table_entry.Type, fcolumns, TableID); 
		
		get_table_key(Conn2, table_entry.Catalog, table_entry.Schema, 
			table_entry.Name, table_entry.Type, fkeys, TableID); 
		
		get_table_indexes(Conn2, table_entry.Catalog, table_entry.Schema, 
			table_entry.Name, table_entry.Type, findexes, TableID); 
	}
	SQLLibFreeResults(&Results);

	fclose(ftables);
	fclose(fcolumns);
	fclose(fkeys);
	fclose(findexes);

//4-22-11

	trace("Disconnecting From Datasource.");
	SQLLibDisconnect(Conn); // Disconnect is (or should be) optional...

	trace("Freeing SQL Connection Handle(s).");
	SQLLibConnectionDestroy(&Conn);
	SQLLibConnectionDestroy(&Conn2);

//	trace("Freeing SQL Environment Handle.");
//	rc = freeEnv(gEnvironment);
//	checkErrorEnv(gEnvironment, rc);

	trace("Done Temporary!");

	return 0;
// end 4-22-11

	trace("Getting Columns Function.");
	Results = SQLLibColumns(Conn, NULL, NULL, "LERGDATE", "%", 0 /* &column_info */);
	SQLLibPrint(Results, stderr, '\t', 1);
	SQLLibFreeResults(&Results); 

	trace("Getting Special Columns Function.");
	Results = SQLLibIdentRows(Conn, NULL, NULL, "LERGDATE", 0);
	SQLLibPrint(Results, stderr, '\t', 1);
	SQLLibFreeResults(&Results);

	trace("Getting Statistics(Indexes) Function.");
	Results = SQLLibIdentRows(Conn, NULL, NULL, "LERGDATE", 0);
	SQLLibPrint(Results, stderr, '\t', 1);
	SQLLibFreeResults(&Results);

	trace("Execute SQL");
	Results = SQLLibExecute(Conn, "SELECT * FROM LERGDATE;");
	SQLLibBind(Results,1);
	//SQLLibPrint(Results, stderr, '\t', 1);
	(void)SQLLibNext(Results);
	msg = SQLLibColumnChar(Results, 1);
	fprintf(stderr,"%s\n", msg);
	SQLLibFreeResults(&Results);

/***
	trace("Getting Types Function.");
    rc = SQLGetTypeInfo(// Raw  
		StatementHandle,
		SQL_ALL_TYPES // DataType
		);
***/

	trace("Disconnecting From Datasource.");
	SQLLibDisconnect(Conn); // Disconnect is (or should be) optional...

	trace("Freeing SQL Connection Handle(s).");
	SQLLibConnectionDestroy(&Conn);
	SQLLibConnectionDestroy(&Conn2);

//	trace("Freeing SQL Environment Handle.");
//	rc = freeEnv(gEnvironment);
//	checkErrorEnv(gEnvironment, rc);

	trace("Done!");

	return 0;
}

typedef int (*write_file)(const char *buff, long len);

write_file x = NULL;


int main(int argc, const char *const argv[])
{
	int rc;
#if (defined(WIN32) || defined(_WIN64)) && !defined(lint)
	// Get current debug flag
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
#endif

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