/*-----------------------------------------------------------------------------
 * SQL CLI (ODBC) Library Implementation 
 *
 * $Id$
 * $HeadURL$
 *
 * Copyright (C) 2008-2009 Clone Research Corporation 
 * All Rights Reserved.
 *-----------------------------------------------------------------------------
 */

/*lint -e818 */ /* Note: Pointer parameter 'xxx' (line nnn) could be declared as pointing to const */

//#define SQL_NOUNICODEMAP

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#if defined(WIN32) || defined(_WIN64)
#include <crtdbg.h>
#endif

#if defined(WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sqlext.h>
#else
#include <sql.h>
#endif

#include <stdio.h>
#include <string.h>

#include "sqllib.h"
#include "sqllibver.h"

/***************************************************************************************************
 * Global Variables
 ***************************************************************************************************/

static SQLHENV gEnvironment = SQL_NULL_HENV;

static int LOGALLOC = 0;

/***************************************************************************************************
 * Local (static) Function Declarations
 ***************************************************************************************************/

static SQLLibResults SQLLibDescribe (SQLLibConnection Conn, SQLHSTMT Handle);  

/***************************************************************************************************
 * Local (Opaque) Structure Definitions
 ***************************************************************************************************/

struct SQLLibConnectionTag {
	long          Signature;
	short         ObjectType;
	SQLHDBC       Handle;
	SQLRETURN     rc;
	int           Connected;
	FILE         *log;
	int           CatalogSupport;
};

typedef struct SQLLibColDefTag
{
	SQLCHAR      ColumnName[129];
	SQLSMALLINT  NameLength;
	SQLSMALLINT  DataType; 
	SQLULEN      ColumnSize;
	SQLSMALLINT  DecimalDigits; 
	SQLSMALLINT  Nullable;

	SQLSMALLINT  TargetType;
	SQLINTEGER   BufferLength;
	SQLINTEGER   StrLen_or_Ind;

	int          NoFree;
	union {
	SQLCHAR     *SqlChar;	/* Always bind SQLCHAR */
	SQLINTEGER   SqlInteger;
#if (ODBCVER >= 0x0300)
	SQLDOUBLE    SqlDouble;
#else
	double       SqlDouble;
#endif
	}            TargetValue;
} SQLLibColDef;

struct SQLLibResultsTag {
	long          Signature;
	short         ObjectType;
	SQLHSTMT      Handle;
	SQLRETURN     rc;
	int           CursorOpen;
//	int           Connected;
//	FILE         *log;

	SQLHDBC       ConnectionHandle;
	SQLINTEGER    RowCount;
	SQLSMALLINT   ColumnCount;
	SQLLibColDef *Column;
};

/***************************************************************************************************
 * Main Library Functions 
 ***************************************************************************************************/

/**********************************************************************************/
/**********************************************************************************/

static void print_one(const SQLLibResults Results, FILE *out, char sep)
{
	int i;

	for (i = 0; i < Results->ColumnCount; i++) {

		if (i > 0) fprintf(out,"%c",sep);

		if (Results->Column[i].StrLen_or_Ind == SQL_NULL_DATA) {
			fprintf(out,"<null>");
		} else if (Results->Column[i].TargetType == SQL_CHAR) {
			fprintf(out,"%s",Results->Column[i].TargetValue.SqlChar);
		} else if (Results->Column[i].TargetType == SQL_INTEGER) {
			fprintf(out,"%ld",(long)Results->Column[i].TargetValue.SqlInteger);
#if (ODBCVER >= 0x0300)
		} else if (Results->Column[i].TargetType == SQL_DOUBLE) {
			fprintf(out,"%g",(double)Results->Column[i].TargetValue.SqlDouble);
#endif
		} else { 
			fprintf(out,"<unknown type>");
		}
	}
	fprintf(out,"\n");

	return;
}

void SQLLibPrint(SQLLibResults Results, FILE *out, char sep, int headings)
{
	int i;
	int err;

	if (!Results) return ;

	if (headings) {
		for (i = 0; i < Results->ColumnCount; i++) {
			if (i > 0) fprintf(out, "%c", sep);
			fprintf(out,"%s",Results->Column[i].ColumnName);
		}
		fprintf(out,"\n");
	}

	do {
		err = SQLLibNext(Results);
		//checkErrorStmt(Results->Handle, rc);

		if (!err)
			print_one(Results, out, sep);

	} while (!err);

	return ;
}

/**********************************************************************************/
/**********************************************************************************/

void SQLLibBindByName(SQLLibResults Results, 
			   const char *Name, short Type, void *Target, long Length)
{
	unsigned short i;

	if (!Results) return ;

	Results->rc = SQL_ERROR;

	for (i=0; i<Results->ColumnCount; i++)
	{
		if (strcmp((const char *)Results->Column[i].ColumnName, Name) == 0) {
			Results->Column[i].TargetType   = Type;
			Results->Column[i].BufferLength = Length;
			Results->rc = SQLBindCol(Results->Handle, i+1, 
				Type, Target, Length, 
				&Results->Column[i].StrLen_or_Ind
				);
			break;
		}
	}

	return ;
}

static int dataSourceNext(SQLLibResults Results)
{
	SQLRETURN rc;
	SQLSMALLINT Direction;

	/* Allocate Environment Handle, if Necessary */

	if (gEnvironment == SQL_NULL_HENV) {
		Results->rc = allocEnv(&gEnvironment);
		if (!SQL_SUCCEEDED(Results->rc)) /* If no environment, must bail */
			return 0 /*Results*/;
	}

	Direction = SQL_FETCH_NEXT;
	if (Results->RowCount == 1) Direction = SQL_FETCH_FIRST;
	Results->RowCount = 0;

	rc = SQLDataSources(gEnvironment, Direction, 
		Results->Column[0].TargetValue.SqlChar, (SQLSMALLINT)Results->Column[0].ColumnSize, NULL, 
		Results->Column[1].TargetValue.SqlChar, (SQLSMALLINT)Results->Column[1].ColumnSize, NULL
		);
	if (!SQL_SUCCEEDED(rc))
		return 1;

	return 0;
}

int SQLLibNext (SQLLibResults Results)
{
	SQLRETURN rc;
	int i;

	if (!Results) 
		return 1;

	if (Results->CursorOpen == 2)
		return dataSourceNext(Results);

	rc = SQLFetch(Results->Handle);
	if (rc == SQL_NO_DATA) {
		Results->CursorOpen = 0;
		(void)closeCursor(Results->Handle);
	}
	if (!SQL_SUCCEEDED(rc)) return 1;

	for (i=0; i<Results->ColumnCount; i++) 
	{
		if (Results->Column[i].StrLen_or_Ind == SQL_NULL_DATA) {
			switch (Results->Column[i].TargetType) {
				case SQL_CHAR:
					if (Results->Column[i].TargetValue.SqlChar)
						Results->Column[i].TargetValue.SqlChar[0] = 0;
					break;
				case SQL_INTEGER:
					break;  ////////////////// FIXME????
				case SQL_DOUBLE:
					break;
				default:
					break;
			}
		}
	}

	return 0;
}

SQLRETURN SQLLibGetLong    (SQLHSTMT Handle, unsigned short Column, long *Value, unsigned *IsNull)
{
	SQLRETURN  rc;
	SQLINTEGER StrLen_or_Ind;

	rc = SQLGetData(Handle, Column, SQL_INTEGER, (SQLPOINTER)Value, sizeof(long), &StrLen_or_Ind);
	if (!SQL_SUCCEEDED(rc)) /* Failed to get the data, set default values */
		StrLen_or_Ind = SQL_NULL_DATA;

	if (StrLen_or_Ind == SQL_NULL_DATA) 
		*Value = 0;

	if (IsNull != NULL) 
		if (StrLen_or_Ind == SQL_NULL_DATA) *IsNull = 1;
		else *IsNull = 0;

	return rc;
}

SQLRETURN SQLLibGetDouble  (SQLHSTMT Handle, unsigned short Column, double *Value, unsigned *IsNull)
{
	SQLRETURN  rc;
	SQLINTEGER StrLen_or_Ind;

	rc = SQLGetData(Handle, Column, SQL_DOUBLE, (SQLPOINTER)Value, sizeof(double), &StrLen_or_Ind);
	if (!SQL_SUCCEEDED(rc)) /* Failed to get the data, set default values */
		StrLen_or_Ind = SQL_NULL_DATA;

	if (StrLen_or_Ind == SQL_NULL_DATA) 
		*Value = 0.0;

	if (IsNull != NULL) 
		if (StrLen_or_Ind == SQL_NULL_DATA) *IsNull = 1;
		else *IsNull = 0;

	return rc;
}

SQLRETURN SQLLibGetChar    (SQLHSTMT Handle, unsigned short Column, char *Value, long Length, unsigned *IsNull)
{
	SQLRETURN  rc;
	SQLINTEGER StrLen_or_Ind;

	rc = SQLGetData(Handle, Column, SQL_CHAR, (SQLPOINTER)Value, Length, &StrLen_or_Ind);
	if (!SQL_SUCCEEDED(rc)) /* Failed to get the data, set default values */
		StrLen_or_Ind = SQL_NULL_DATA;

	if (StrLen_or_Ind == SQL_NULL_DATA) 
		memset(Value, 0, (size_t)Length);

	if (IsNull != NULL) 
		if (StrLen_or_Ind == SQL_NULL_DATA) *IsNull = 1;
		else *IsNull = 0;

	return rc;
}

const char *SQLLibColumnChar (SQLLibResults Results, unsigned short Column)
{
	const char *Value = "";
	
	if (!Results) return Value;
	
	if (Column < 1 || Column > Results->ColumnCount)
		return Value;

	if (Results->Column[Column-1].TargetType != SQL_CHAR)
		return Value;

	if (Results->Column[Column-1].StrLen_or_Ind != SQL_NULL_DATA)
		Value = (const char *)Results->Column[Column-1].TargetValue.SqlChar;

	return Value;
}

long        SQLLibColumnLong   (SQLLibResults Results, unsigned short Column)
{
	long Value = 0;
	
	if (!Results) return Value;
	
	if (Column < 1 || Column > Results->ColumnCount)
		return Value;

	if (Results->Column[Column-1].TargetType != SQL_INTEGER)
		return Value;

	if (Results->Column[Column-1].StrLen_or_Ind != SQL_NULL_DATA)
		Value = Results->Column[Column-1].TargetValue.SqlInteger;

	return Value;
}

double      SQLLibColumnDouble (SQLLibResults Results, unsigned short Column)
{
	double Value = 0.0;
	
	if (!Results) return Value;
	
	if (Column < 1 || Column > Results->ColumnCount)
		return Value;

	if (Results->Column[Column-1].TargetType != SQL_DOUBLE)
		return Value;

	if (Results->Column[Column-1].StrLen_or_Ind != SQL_NULL_DATA)
		Value = Results->Column[Column-1].TargetValue.SqlDouble;

	return Value;
}

int         SQLLibIsNull       (SQLLibResults Results, unsigned short Column)
{
	
	if (!Results) return 1;
	
	if (Column < 1 || Column > Results->ColumnCount)
		return 1;
	
	if (Results->Column[Column-1].StrLen_or_Ind == SQL_NULL_DATA)
		return 1;
	
	return 0;
}

/**********************************************************************************
 * ??? 
 **********************************************************************************/

int isSupported(SQLHDBC Connection, SQLUSMALLINT FunctionId)
{
	SQLRETURN rc;
	SQLUSMALLINT Supported;

    rc = SQLGetFunctions(Connection, FunctionId, &Supported);
	
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
		return 0;

	return Supported;
}
/**********************************************************************************
 * Common Utility Functions 
 **********************************************************************************/

#define SQLLIB_SIGNATURE  0x1234

static int badConnection(SQLLibConnection Conn)
{

	if (!Conn) return 1;
	if (Conn->Signature != SQLLIB_SIGNATURE) return 1;

	return 0;
}

static int badResults(SQLLibResults Results)
{

	if (!Results) return 1;
	if (Results->Signature != SQLLIB_SIGNATURE) return 1;

	return 0;
}

/**********************************************************************************/
/**********************************************************************************/

static void handleNonSelect(SQLLibResults Results, SQLINTEGER type)
{
	int         do_end_tran = 0;

	switch (type) {
#if (ODBCVER >= 0x0300)
		case SQL_DIAG_DELETE_WHERE:
		case SQL_DIAG_UPDATE_WHERE:
#endif
		case SQL_DIAG_INSERT:
//			rc = getRowCount(Handle, &RowCount);
			do_end_tran = 1;
			break;
		case SQL_DIAG_SELECT_CURSOR:
			/* This should never happen */
			break;
#if (ODBCVER >= 0x0300)
		case SQL_DIAG_ALTER_TABLE:
		case SQL_DIAG_CREATE_INDEX:
		case SQL_DIAG_CREATE_TABLE:
		case SQL_DIAG_CREATE_VIEW:
		case SQL_DIAG_DROP_INDEX:
		case SQL_DIAG_DROP_TABLE:
		case SQL_DIAG_DROP_VIEW:
		case SQL_DIAG_DYNAMIC_DELETE_CURSOR:
//		case SQL_DIAG_DYNAMIC_INSERT_CURSOR:
		case SQL_DIAG_GRANT:
		case SQL_DIAG_REVOKE:
		/* Possible MS extensions */
		case SQL_DIAG_ALTER_DOMAIN:
		case SQL_DIAG_CREATE_ASSERTION:
		case SQL_DIAG_CREATE_CHARACTER_SET:
		case SQL_DIAG_CREATE_COLLATION:
		case SQL_DIAG_CREATE_DOMAIN:
		case SQL_DIAG_CREATE_SCHEMA:
		case SQL_DIAG_CREATE_TRANSLATION:
		case SQL_DIAG_DROP_ASSERTION:
		case SQL_DIAG_DROP_CHARACTER_SET:
		case SQL_DIAG_DROP_COLLATION:
		case SQL_DIAG_DROP_DOMAIN:
		case SQL_DIAG_DROP_SCHEMA:
		case SQL_DIAG_DROP_TRANSLATION:
		case SQL_DIAG_DYNAMIC_UPDATE_CURSOR:
			do_end_tran = 1;
			break;
		/* Possible MS extensions */
		case SQL_DIAG_CALL:
		case SQL_DIAG_UNKNOWN_STATEMENT:
#endif
		default:
			break;
	}

	if (do_end_tran)
		(void)endTransaction(Results->ConnectionHandle);

	return ;
}

static SQLLibResults allocResults(int columns)
{
	SQLLibResults Results;
	int i;

	/* Allocate the Results structure */

	Results = malloc(sizeof(struct SQLLibResultsTag));
	if (!Results)
		return Results;

	Results->Signature = SQLLIB_SIGNATURE;
	Results->ObjectType = 2;
	Results->Handle = SQL_NULL_HSTMT;
	Results->ColumnCount = 0;
	Results->Column = 0;
	Results->CursorOpen = 0;
	Results->RowCount = 0;
	Results->ConnectionHandle = SQL_NULL_HDBC;
	Results->CursorOpen = 0;

	Results->ColumnCount = columns;

	Results->Column = 0;

	if (columns < 1) 
		return Results;

	/* If we have result columns, we must have a cursor */

	Results->CursorOpen = 1;

	/* Allocate the binding column entries */

	Results->Column = malloc( sizeof(SQLLibColDef) * Results->ColumnCount );
	if (!Results->Column)
		return Results;

	for (i=0; i < Results->ColumnCount; i++) {
		Results->Column[i].ColumnName[0] = 0;
		Results->Column[i].NameLength = 0;
		Results->Column[i].DataType = 0;
		Results->Column[i].ColumnSize = 0;
		Results->Column[i].DecimalDigits = 0;
		Results->Column[i].Nullable = 0;
		Results->Column[i].TargetType = 0;
		Results->Column[i].BufferLength = 0;
		Results->Column[i].StrLen_or_Ind = 0;
		Results->Column[i].TargetValue.SqlInteger = 0;
		Results->Column[i].NoFree = 0;
	}

	return Results;
}

static SQLLibResults SQLLibDescribe(SQLLibConnection Conn, SQLHSTMT StatementHandle)
{
	SQLRETURN      rc;
	SQLSMALLINT    ColumnCount;
	SQLUSMALLINT   ColumnNumber;  
	SQLSMALLINT    BufferLength;
	SQLLibResults  Results;
	SQLLibColDef  *Column;
	SQLINTEGER     type;
	SQLINTEGER     RowCount;

	/* Get the dynamic function code */

	type = SQL_DIAG_SELECT_CURSOR;

#if (ODBCVER >= 0x0300)
	rc = SQLGetDiagField(SQL_HANDLE_STMT, StatementHandle, 0, SQL_DIAG_DYNAMIC_FUNCTION_CODE,
		(SQLPOINTER)&type, SQL_IS_INTEGER, (SQLSMALLINT *)NULL);
	if (!SQL_SUCCEEDED(rc))
		type = SQL_DIAG_UNKNOWN_STATEMENT;
#endif

	/* Get the number of result columns */

	rc = SQLNumResultCols(StatementHandle, &ColumnCount);

	Results = allocResults(ColumnCount);
	if (!Results)
		return Results;

	Results->ObjectType = 2;
	Results->Handle = StatementHandle;
	Results->ConnectionHandle = Conn->Handle;

	if (SQL_SUCCEEDED(rc) && ColumnCount > 0) {
		/* if we have results columns, must be a select */
		type = SQL_DIAG_SELECT_CURSOR;
	} else {
		rc = getRowCount(StatementHandle, &RowCount);
		if (SQL_SUCCEEDED(rc) && RowCount > 0) {
			type = SQL_DIAG_INSERT;
			Results->RowCount = RowCount;
		} else {
			type = SQL_DIAG_UNKNOWN_STATEMENT;
		}
	}

	/* If it is not a select statement, we need to figure out what to do with it. */

	if (type != SQL_DIAG_SELECT_CURSOR) {
		handleNonSelect(Results, type);
		return Results;
	}

	if (!Results->Column)
		return Results;

	Column = Results->Column;

	for (ColumnNumber = 0; ColumnNumber < ColumnCount; ColumnNumber++)
	{
		Column[ColumnNumber].TargetType    = SQL_CHAR;
		Column[ColumnNumber].StrLen_or_Ind = SQL_NULL_DATA;
		Column[ColumnNumber].TargetValue.SqlChar = NULL;
		
		/* Get the description of each result column */

		BufferLength = sizeof(Column[ColumnNumber].ColumnName)/sizeof(Column[ColumnNumber].ColumnName[0]);
	    rc = SQLDescribeCol(
			StatementHandle,
			ColumnNumber+1, 
			Column[ColumnNumber].ColumnName,
			BufferLength, 
			&Column[ColumnNumber].NameLength,
			&Column[ColumnNumber].DataType, 
			&Column[ColumnNumber].ColumnSize,
			&Column[ColumnNumber].DecimalDigits, 
			&Column[ColumnNumber].Nullable
			);
		if (!SQL_SUCCEEDED(rc)) break;

#ifdef DESCRIBE_DEBUG
		/* Print the column description */

#ifdef UNICODE
		fwprintf(stderr, L"%-30s\t%d\t%d\t%d\t%d\n", 
			Column[ColumnNumber].ColumnName, 
#else
		fprintf(stderr, "%-30s\t%d\t%d\t%d\t%d\n", 
			Column[ColumnNumber].ColumnName, 
#endif
			(int)Column[ColumnNumber].DataType, 
			(int)Column[ColumnNumber].ColumnSize, 
			(int)Column[ColumnNumber].DecimalDigits, 
			(int)Column[ColumnNumber].Nullable);
#endif
	}

	return Results; 
}

SQLLibResults SQLLibExecute (SQLLibConnection Conn, const char *Text)
{
	SQLHSTMT  Handle;
	SQLRETURN rc;

	rc = allocStmt(Conn->Handle, &Handle);
	if (!SQL_SUCCEEDED(rc)) 
		return (SQLLibResults)0;

    rc = SQLExecDirect(Handle, (SQLCHAR *)Text, SQL_NTS);
	if (!SQL_SUCCEEDED(rc)) 
		return (SQLLibResults)0;
	
#define USE_DIRECT
#ifdef  USE_DIRECT
#else
	trace("Prepare.");
    rc = SQLPrepare( /* Raw */
		StatementHandle,
		sql, // StatementText, 
		SQL_NTS
		);
	checkErrorStmt(StatementHandle, rc);
	trace("Execute.");
    rc = SQLExecute( /* Raw */
		StatementHandle
		);
#endif

	return SQLLibDescribe(Conn, Handle);
}

int SQLLibColumnCount(SQLLibResults Results)
{

	return (int)Results->ColumnCount;
}

const char *SQLLibColumnName   (SQLLibResults Results, int col)
{

	if (col > Results->ColumnCount || col < 1)
		return "<bad column #>";

	return (const char *)Results->Column[col-1].ColumnName;
}

/**********************************************************************************
 * Metadata Functions 
 **********************************************************************************/

SQLLibResults SQLLibTables (SQLLibConnection Conn, const char *CatalogName, const char *SchemaName,
						    const char *TableName, const char *TableType,   SQLLibTableInfo *Result)
{
	SQLLibResults Results;
	SQLHSTMT  Handle;
	SQLRETURN rc;

	if (Result) SQLLibTableInfoClear(Result);

	rc = allocStmt(Conn->Handle, &Handle);
	if (!SQL_SUCCEEDED(rc)) return (SQLLibResults)0;

    rc = SQLTables(Handle, 
		(SQLCHAR *)CatalogName, SQL_NTS, 
		(SQLCHAR *)SchemaName,  SQL_NTS,
		(SQLCHAR *)TableName,   SQL_NTS,
		(SQLCHAR *)TableType,   SQL_NTS
		);
	if (!SQL_SUCCEEDED(rc)) return (SQLLibResults)0;

	Results = SQLLibDescribe(Conn, Handle);

	if (!Result) { /* If no output buffer specified, bind to character */
		SQLLibBind(Results, 1);
		return Results;
	}

	SQLLibBindByName(Results, "TABLE_CAT",        SQL_CHAR,      Result->Catalog,         sizeof(Result->Catalog));
	SQLLibBindByName(Results, "TABLE_SCHEM",      SQL_CHAR,      Result->Schema,          sizeof(Result->Schema));
	SQLLibBindByName(Results, "TABLE_NAME",       SQL_CHAR,      Result->Name,            sizeof(Result->Name));
	SQLLibBindByName(Results, "TABLE_TYPE",       SQL_CHAR,      Result->Type,            sizeof(Result->Type));
	SQLLibBindByName(Results, "REMARKS",          SQL_CHAR,      Result->Remarks,         sizeof(Result->Remarks));

//	SQLBindCol(Handle, 5, SQL_CHAR, Result->Remarks, sizeof(Result->Remarks), &Result->RemarksLen);

	return Results;
}

SQLLibResults SQLLibColumns (SQLLibConnection Conn, const char *CatalogName, const char *SchemaName,
						     const char *TableName, const char *ColumnName,  SQLLibColumnInfo *Result)
{
	SQLLibResults Results;
	SQLHSTMT  Handle;
	SQLRETURN rc;

	if (Result) SQLLibColumnInfoClear(Result);

	rc = allocStmt(Conn->Handle, &Handle);
	if (!SQL_SUCCEEDED(rc)) return (SQLLibResults)0;

	for ( ; Conn->CatalogSupport < 3; Conn->CatalogSupport++) {
		/* Some databases don't support catalogs or schemas, so try with and without */
		if (Conn->CatalogSupport >= 1) CatalogName = NULL;
		if (Conn->CatalogSupport >= 2) SchemaName = NULL;

	    rc = SQLColumns(Handle, 
			(SQLCHAR *)CatalogName, SQL_NTS, 
			(SQLCHAR *)SchemaName,  SQL_NTS,
			(SQLCHAR *)TableName,   SQL_NTS,
			(SQLCHAR *)ColumnName,  SQL_NTS
			);
		if (SQL_SUCCEEDED(rc)) break;
	}
		
	if (!SQL_SUCCEEDED(rc)) {
		logMsgStmt(Handle, stderr, rc);
		return (SQLLibResults)0;
	}

	Results = SQLLibDescribe(Conn, Handle);

	if (!Result) { /* If no output buffer specified, bind to character */
		SQLLibBind(Results, 1);
		return Results;
	}

	SQLLibBindByName(Results, "TABLE_CAT",        SQL_CHAR,      Result->Catalog,         sizeof(Result->Catalog));
	SQLLibBindByName(Results, "TABLE_SCHEM",      SQL_CHAR,      Result->Schema,          sizeof(Result->Schema));
	SQLLibBindByName(Results, "TABLE_NAME",       SQL_CHAR,      Result->Table,           sizeof(Result->Table));
	SQLLibBindByName(Results, "COLUMN_NAME",      SQL_CHAR,      Result->Name,            sizeof(Result->Name));
	SQLLibBindByName(Results, "DATA_TYPE",        SQL_SMALLINT, &Result->DataType,        sizeof(Result->DataType));
	SQLLibBindByName(Results, "TYPE_NAME",        SQL_CHAR,      Result->TypeName,        sizeof(Result->TypeName));
	SQLLibBindByName(Results, "COLUMN_SIZE",      SQL_INTEGER,  &Result->ColumnSize,      sizeof(Result->ColumnSize));
	SQLLibBindByName(Results, "BUFFER_LENGTH",    SQL_INTEGER,  &Result->BufferLength,    sizeof(Result->BufferLength));
	SQLLibBindByName(Results, "DECIMAL_DIGITS",   SQL_SMALLINT, &Result->DecimalDigits,   sizeof(Result->DecimalDigits));
	SQLLibBindByName(Results, "NUM_PREC_RADIX",   SQL_SMALLINT, &Result->Precision,       sizeof(Result->Precision));
	SQLLibBindByName(Results, "NULLABLE",         SQL_SMALLINT, &Result->Nullable,        sizeof(Result->Nullable));
	SQLLibBindByName(Results, "REMARKS",          SQL_CHAR,      Result->Remarks,         sizeof(Result->Remarks));
	SQLLibBindByName(Results, "COLUMN_DEF",       SQL_CHAR,      Result->Default,         sizeof(Result->Default));
	SQLLibBindByName(Results, "DATETIME_CODE",    SQL_INTEGER,  &Result->DateTime,        sizeof(Result->DateTime));
	SQLLibBindByName(Results, "CHAR_OCTET_LENGTH",SQL_INTEGER,  &Result->OctetLength,     sizeof(Result->OctetLength));
	SQLLibBindByName(Results, "ORDINAL_POSITION", SQL_INTEGER,  &Result->OrdinalPosition, sizeof(Result->OrdinalPosition));
	SQLLibBindByName(Results, "IS_NULLABLE",      SQL_CHAR,      Result->IsNullable,      sizeof(Result->IsNullable));
	
	SQLLibBindByName(Results, "SQL_DATETIME_SUB", SQL_INTEGER,  &Result->DateTime,        sizeof(Result->DateTime));
	SQLLibBindByName(Results, "SQL_DATA_TYPE",    SQL_SMALLINT, &Result->SQLType,         sizeof(Result->SQLType));
	SQLLibBindByName(Results, "ORDINAL",          SQL_INTEGER,  &Result->Ordinal,         sizeof(Result->Ordinal));

	return Results;
}

SQLLibResults SQLLibIndexes    (SQLLibConnection Conn, const char *CatalogName, const char *SchemaName,
							    const char *TableName, SQLLibIndexInfo *Result)
{
	SQLLibResults Results;
	SQLHSTMT  Handle;
	SQLRETURN rc;

	if (Result) SQLLibIndexInfoClear(Result);

	rc = allocStmt(Conn->Handle, &Handle);
	if (!SQL_SUCCEEDED(rc)) return (SQLLibResults)0;

	for ( ; Conn->CatalogSupport < 3; Conn->CatalogSupport++) {
		/* Some databases don't support catalogs or schemas, so try with and without */
		if (Conn->CatalogSupport >= 1) CatalogName = NULL;
		if (Conn->CatalogSupport >= 2) SchemaName = NULL;

		rc = SQLStatistics(Handle, 
			(SQLCHAR *)CatalogName, SQL_NTS, 
			(SQLCHAR *)SchemaName,  SQL_NTS,
			(SQLCHAR *)TableName,   SQL_NTS,
			SQL_INDEX_ALL, // Unique 
			0 // Accuracy?
			);
		if (SQL_SUCCEEDED(rc)) break;
	}

	if (!SQL_SUCCEEDED(rc)) return (SQLLibResults)0;

	Results = SQLLibDescribe(Conn, Handle);

	if (!Result) { /* If no output buffer specified, bind to character */
		SQLLibBind(Results, 1);
		return Results;
	}

	SQLLibBindByName(Results, "TABLE_CAT",        SQL_CHAR,      Result->Catalog,         sizeof(Result->Catalog));
	SQLLibBindByName(Results, "TABLE_SCHEM",      SQL_CHAR,      Result->Schema,          sizeof(Result->Schema));
	SQLLibBindByName(Results, "TABLE_NAME",       SQL_CHAR,      Result->Table,           sizeof(Result->Table));
	SQLLibBindByName(Results, "NON_UNIQUE",       SQL_SMALLINT, &Result->NonUnique,       sizeof(Result->NonUnique));
	SQLLibBindByName(Results, "INDEX_QUALIFIER",  SQL_CHAR,      Result->Qualifier,       sizeof(Result->Qualifier));
	SQLLibBindByName(Results, "INDEX_NAME",       SQL_CHAR,      Result->IndexName,       sizeof(Result->IndexName));
	SQLLibBindByName(Results, "TYPE",             SQL_SMALLINT, &Result->IndexType,       sizeof(Result->IndexType));
	SQLLibBindByName(Results, "ORDINAL_POSITION", SQL_SMALLINT, &Result->OrdinalPosition, sizeof(Result->OrdinalPosition));
	SQLLibBindByName(Results, "COLUMN_NAME",      SQL_CHAR,      Result->Name,            sizeof(Result->Name));
	SQLLibBindByName(Results, "ASC_OR_DESC",      SQL_CHAR,      Result->Asc_Or_Desc,     sizeof(Result->Asc_Or_Desc));

	return Results;
}

SQLLibResults SQLLibIdentRows (SQLLibConnection Conn, const char *CatalogName, const char *SchemaName,
							   const char *TableName, SQLLibIdentInfo *Result)
{
	SQLLibResults Results;
	SQLHSTMT  Handle;
	SQLRETURN rc;

	if (Result) SQLLibIdentInfoClear(Result);

	rc = allocStmt(Conn->Handle, &Handle);
	if (!SQL_SUCCEEDED(rc)) return (SQLLibResults)0;

	for ( ; Conn->CatalogSupport < 3; Conn->CatalogSupport++) {
		/* Some databases don't support catalogs or schemas, so try with and without */
		if (Conn->CatalogSupport >= 1) CatalogName = NULL;
		if (Conn->CatalogSupport >= 2) SchemaName = NULL;

		rc = SQLSpecialColumns(Handle, SQL_ROW_IDENTIFIER, 
			(SQLCHAR *)CatalogName, SQL_NTS, 
			(SQLCHAR *)SchemaName,  SQL_NTS,
			(SQLCHAR *)TableName,   SQL_NTS,
			SQL_SCOPE_CURROW, 
			SQL_NULLABLE 
			);
		if (SQL_SUCCEEDED(rc)) break;
	}

	if (!SQL_SUCCEEDED(rc)) return (SQLLibResults)0;

	Results = SQLLibDescribe(Conn, Handle);

	if (!Result) { /* If no output buffer specified, bind to character */
		SQLLibBind(Results, 1);
		return Results;
	}

	SQLLibBindByName(Results, "SCOPE",            SQL_SMALLINT, &Result->Scope,           sizeof(Result->Scope));
	SQLLibBindByName(Results, "COLUMN_NAME",      SQL_CHAR,      Result->Name,            sizeof(Result->Name));
	SQLLibBindByName(Results, "DATA_TYPE",        SQL_SMALLINT, &Result->DataType,        sizeof(Result->DataType));
	SQLLibBindByName(Results, "TYPE_NAME",        SQL_CHAR,      Result->TypeName,        sizeof(Result->TypeName));
	SQLLibBindByName(Results, "COLUMN_SIZE",      SQL_INTEGER,  &Result->ColumnSize,      sizeof(Result->ColumnSize));
	SQLLibBindByName(Results, "BUFFER_LENGTH",    SQL_INTEGER,  &Result->BufferLength,    sizeof(Result->BufferLength));
	SQLLibBindByName(Results, "DECIMAL_DIGITS",   SQL_SMALLINT, &Result->DecimalDigits,   sizeof(Result->DecimalDigits));
	SQLLibBindByName(Results, "PSEUDO_COLUMN",    SQL_SMALLINT, &Result->Pseudo,          sizeof(Result->Pseudo));

	return Results;
}

SQLLibResults SQLLibSources (SQLLibSourceInfo *Result)
{
	SQLLibResults Results;

	/* Clear the result structure, if one specified. */

	if (Result) SQLLibSourceInfoClear(Result);

	/* Allocate the result set. */

	Results = allocResults(2);
	if (!Results) /* malloc failed */
		return Results; 

	/* Sources do not have a cursor (2 - indicates data sources). */

	Results->CursorOpen = 2;

	if (!Results->Column) /* malloc failed on columns */
		return Results;

	/* Set the column sizes and data types. */

	Results->Column[0].ColumnSize = sizeof(Result->Name);
	Results->Column[1].ColumnSize = sizeof(Result->Description);
	Results->Column[0].TargetType = SQL_CHAR;
	Results->Column[1].TargetType = SQL_CHAR;

	/* Set the column names. */

	strcpy(Results->Column[0].ColumnName, "SERVER");
	strcpy(Results->Column[1].ColumnName, "DESCRIPTION");

	/* Set the data pointers. */

	if (Result) {
		Results->Column[0].TargetValue.SqlChar = Result->Name;
		Results->Column[0].NoFree = 1;
		Results->Column[1].TargetValue.SqlChar = Result->Description;
		Results->Column[1].NoFree = 1;
	} else {
		Results->Column[0].TargetValue.SqlChar = malloc( Results->Column[0].ColumnSize );
		Results->Column[1].TargetValue.SqlChar = malloc( Results->Column[1].ColumnSize );
	}

	/* Set the first flag. */

	Results->RowCount = 1;

	return Results;
}

/**********************************************************************************
 * ??? 
 **********************************************************************************/

void SQLLibBind(SQLLibResults Results, int BindMode)
{
	SQLRETURN    rc;
	SQLSMALLINT  ColumnCount;
	SQLUSMALLINT ColumnNumber;
	SQLPOINTER   TargetValue;
	void        *mem;
	SQLSMALLINT  DataType;
	SQLLibColDef *Column;

	if (badResults(Results)) return ;

	Column = Results->Column;
	ColumnCount = Results->ColumnCount;

	//Column[ColumnCount].TargetType = -1; /* This should be obsolete */ /* AND, IT IS A MEMORY ERROR - UNLESS FIXED */

	for (ColumnNumber = 0; ColumnNumber < ColumnCount; ColumnNumber++)
	{
		/* Bind the value based on its type to its column entry */

		Column[ColumnNumber].BufferLength = (SQLINTEGER)Column[ColumnNumber].ColumnSize;

		DataType = Column[ColumnNumber].DataType;
		if (BindMode == SQLLibBindToChar) DataType = SQL_CHAR;

		switch (DataType) {
			case SQL_INTEGER:
			case SQL_SMALLINT:
				Column[ColumnNumber].TargetType   = SQL_INTEGER;
				Column[ColumnNumber].BufferLength = sizeof(SQLINTEGER);
				TargetValue = &Column[ColumnNumber].TargetValue.SqlInteger;
				break;
#if (ODBCVER > 0x0300)
			case SQL_DOUBLE:
			case SQL_REAL:
			case SQL_FLOAT:
				Column[ColumnNumber].TargetType   = SQL_DOUBLE;
				Column[ColumnNumber].BufferLength = sizeof(SQLDOUBLE);
				TargetValue = &Column[ColumnNumber].TargetValue.SqlDouble;
				break;
			case SQL_DATETIME:
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIME:
			case SQL_TYPE_TIMESTAMP:
			case SQL_DECIMAL:
			case SQL_NUMERIC:
#endif
			case SQL_UNKNOWN_TYPE:
			case SQL_CHAR:
			case SQL_VARCHAR:
			default:
				Column[ColumnNumber].BufferLength += 1; /* Fix Right Truncation Errors */
				if (LOGALLOC) fprintf(stderr,"allocated (Bind) %ld bytes at ", (long)Column[ColumnNumber].BufferLength);
				mem = malloc( (size_t)Column[ColumnNumber].BufferLength );
				Column[ColumnNumber].TargetValue.SqlChar = mem;
				if (Column[ColumnNumber].TargetValue.SqlChar) {
					strncpy( (char *)Column[ColumnNumber].TargetValue.SqlChar, "<not set>",
						(size_t)Column[ColumnNumber].BufferLength);
				}
				Column[ColumnNumber].TargetType   = SQL_CHAR;
				TargetValue = Column[ColumnNumber].TargetValue.SqlChar;
				break;				
		}

	    rc = SQLBindCol(
			Results->Handle,
			ColumnNumber+1, 
			Column[ColumnNumber].TargetType, 
			TargetValue, 
			Column[ColumnNumber].BufferLength, 
			&Column[ColumnNumber].StrLen_or_Ind
			);
		if (!SQL_SUCCEEDED(rc)) 
			return ;
	}

	return ;
}

void SQLLibFreeResults(SQLLibResults *Results)
{
	int i;

	if (!Results)  return;
	if (!*Results) return;

	/* If there is an open cursor, close it. */

	if ((*Results)->CursorOpen == 1) {
		(void)closeCursor((*Results)->Handle);
	}
	(*Results)->CursorOpen = 0;

	/* If a statement handle was allocated, free it. */

	if ((*Results)->Handle != SQL_NULL_HSTMT) {
		(void)freeStmt((*Results)->Handle);
		(*Results)->Handle = SQL_NULL_HSTMT;
	}

	/* Free any allocated memory for bound columns. */

	for (i = 0; i<(*Results)->ColumnCount; i++)
	{
		if ((*Results)->Column[i].TargetType == SQL_CHAR && (*Results)->Column[i].NoFree == 0) {
			if ((*Results)->Column[i].TargetValue.SqlChar != NULL) {
				free((*Results)->Column[i].TargetValue.SqlChar);
			}
			(*Results)->Column[i].TargetValue.SqlChar = 0;
		}
	}

	/* Free the column entries. */

	free((*Results)->Column);

	/* Free the results structure. */

	free((*Results));

	/* Zap the handle. */

	*Results = NULL;

	return;
}

/**********************************************************************************
 * Metadata Buffer Functions 
 **********************************************************************************/
			
void SQLLibTableInfoClear (SQLLibTableInfo  *Info)
{
	Info->Catalog[0] = 0;
	Info->Name[0] = 0;
	Info->Remarks[0] = 0;
	Info->Schema[0] = 0;
	Info->Type[0] = 0;
}

void SQLLibColumnInfoClear(SQLLibColumnInfo *Info)
{
	Info->BufferLength = 0;
	Info->Catalog[0] = 0;
	Info->ColumnSize = 0;
	Info->DataType = 0;
	Info->DateTime = 0;
	Info->DecimalDigits = 0;
	Info->Default[0] = 0;
	Info->IsNullable[0] = 0;
	Info->Name[0] = 0;
	Info->Nullable = 0;
	Info->OctetLength = 0;
	Info->Ordinal = 0;
	Info->OrdinalPosition = 0;
	Info->Precision = 0;
	Info->Remarks[0] = 0;
	Info->Schema[0] = 0;
	Info->SQLType = 0;
	Info->Table[0] = 0;
	Info->TypeName[0] = 0;
}

void SQLLibIndexInfoClear (SQLLibIndexInfo  *Info)
{
	Info->Asc_Or_Desc[0] = 0;
	Info->Catalog[0] = 0;
	Info->IndexName[0] = 0;
	Info->IndexType = 0;
	Info->Name[0] = 0;
	Info->NonUnique = 0;
	Info->OrdinalPosition = 0;
	Info->Qualifier[0] = 0;
	Info->Schema[0] = 0;
	Info->Table[0] = 0;
}

void SQLLibIdentInfoClear (SQLLibIdentInfo  *Info)
{
	Info->BufferLength = 0;
	Info->ColumnSize = 0;
	Info->DataType = 0;
	Info->DecimalDigits = 0;
	Info->Name[0] = 0;
	Info->Pseudo = 0;
	Info->Scope = 0;
	Info->TypeName[0] = 0;
}

void SQLLibSourceInfoClear(SQLLibSourceInfo *Info)
{
	Info->Description[0] = 0;
	Info->Name[0] = 0;
}

/**********************************************************************************
 * Connection Functions 
 **********************************************************************************/

SQLLibConnection SQLLibConnectionCreate (void )
{
	SQLLibConnection Conn;

	/* Allocate the Connection Structure and set defaults */

	Conn = (SQLLibConnection)malloc(sizeof(struct SQLLibConnectionTag));
	if (!Conn) /* Out of memory, only option is to bail */
		return Conn;

	Conn->Signature = SQLLIB_SIGNATURE;
	Conn->ObjectType = 1;
	Conn->Handle    = SQL_NULL_HDBC;
	Conn->rc        = SQL_SUCCESS;
	Conn->Connected = 0;
	Conn->CatalogSupport = 0;

	Conn->log = stderr;  // FIXME

	/* Allocate Environment Handle, if Necessary */

	if (gEnvironment == SQL_NULL_HENV) {
		Conn->rc = allocEnv(&gEnvironment);
		logMsgEnv(gEnvironment, Conn->log, Conn->rc);
		if (!SQL_SUCCEEDED(Conn->rc)) /* If no environment, must bail */
			return Conn;
	}

	/* Allocate Connection Handle */

	Conn->rc = allocConn(gEnvironment, &Conn->Handle);
	logMsgConnect(Conn->Handle, Conn->log, Conn->rc);

	return Conn;
}

int SQLLibConnect (SQLLibConnection Conn, const char *Server, const char *UserName, const char *Authentication)
{

	/* Make sure the connection handle is ok. */

	if (badConnection(Conn)) return 1;

	/* Try to connect */

    Conn->rc = SQLConnect(Conn->Handle,
		(SQLCHAR *)Server,         SQL_NTS,
		(SQLCHAR *)UserName,       SQL_NTS,
		(SQLCHAR *)Authentication, SQL_NTS
		);
	if (SQL_SUCCEEDED(Conn->rc)) 
		Conn->Connected = 1;
	logMsgConnect(Conn->Handle, Conn->log, Conn->rc);

	if (Conn->Connected != 0) return 0;

	return 1;
}

void SQLLibDisconnect (SQLLibConnection Conn)
{

	/* Make sure the connection handle is ok. */

	if (badConnection(Conn)) return ;

	/* if not connected, nothing to do */
	
	if (Conn->Connected == 0) return ;

	/* Try to disconnect */

    Conn->rc = SQLDisconnect(Conn->Handle);

	/* Set disconnect flag, even if error. */

	Conn->Connected = 0;

	logMsgConnect(Conn->Handle, Conn->log, Conn->rc);

	if (SQL_SUCCEEDED(Conn->rc)) return ;

	return ;
}

void SQLLibConnectionDestroy (SQLLibConnection *Conn)
{

	/* Make sure the connection handle is ok. */

	if (badConnection(*Conn)) return;

	/* If connected, disconnect. */

	if ((*Conn)->Connected) {
		(*Conn)->rc = SQLDisconnect((*Conn)->Handle);
		logMsgConnect((*Conn)->Handle, (*Conn)->log, (*Conn)->rc);
	}

	/* Free the connection */

	(*Conn)->rc = freeConn((*Conn)->Handle);
	if (!SQL_SUCCEEDED((*Conn)->rc)) 
		logMsgConnect((*Conn)->Handle, (*Conn)->log, (*Conn)->rc);

	/* Invalidate the signature. */

	(*Conn)->Signature = ~SQLLIB_SIGNATURE;

	/* Free the Connection Structure. */

	free(*Conn);

	/* Set the handle to NULL. */

	*Conn = NULL;

	/* Decrement the global reference count? */

	/* Free the global environment, if necessary? */

	return;
}

void SQLLibConnectInfo (SQLLibConnection Conn, SQLLibConnectionInfo *Result)
{

	if (!Result) return;

	memset(Result, '\0', sizeof(SQLLibConnectionInfo));

	strncpy(Result->DBMSName,    "Unknown", sizeof(Result->DBMSName));
	strncpy(Result->DBMSVersion, "Unknown", sizeof(Result->DBMSVersion));
	strncpy(Result->XOpenYear,   "Unknown", sizeof(Result->XOpenYear));

    (void)SQLGetInfo(Conn->Handle, SQL_DBMS_NAME,        Result->DBMSName,     sizeof(Result->DBMSName),    NULL);
    (void)SQLGetInfo(Conn->Handle, SQL_DBMS_VER,         Result->DBMSVersion,  sizeof(Result->DBMSVersion), NULL);
#if (ODBCVER >= 0x0300)
    (void)SQLGetInfo(Conn->Handle, SQL_XOPEN_CLI_YEAR,   Result->XOpenYear,    sizeof(Result->XOpenYear),   NULL);
#endif

	return ;
}

void *SQLLibConnectionHandle (SQLLibConnection Conn)
{

	return Conn->Handle;
}
