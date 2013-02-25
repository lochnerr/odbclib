/* sqllibver.c */  

/*lint -e759 */ /* header declaration for symbol 'xxx()' defined at (line nnn, file file.c) could be moved from header to module */
/*lint -e830 */ /* file.c(nnn) : Info 830: Location cited in prior message */
/*lint -e14 */  /* Fixes a winnt.h error */ 

/*lint -esym(765,allocDesc) */
/*lint -esym(714,allocDesc) */
/*lint -esym(765,freeDesc) */
/*lint -esym(714,freeDesc) */
/*lint -esym(765,getMsgEnv) */
/*lint -esym(714,getMsgEnv) */
/*lint -esym(765,getMsgConnect) */
/*lint -esym(714,getMsgConnect) */
/*lint -esym(765,getMsgStmt) */
/*lint -esym(714,getMsgStmt) */
/*lint -esym(765,getMsgDesc) */
/*lint -esym(714,getMsgDesc) */
/*lint -esym(765,logMsgDesc) */
/*lint -esym(714,logMsgDesc) */

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


#include "sqllibver.h"

#if (ODBCVER >= 0x0300) && (defined(WIN32) || defined(_WIN64))
/* Windows ODBC version compliance - May add a function to set this at some point. */
static SQLPOINTER gODBCVersion = (SQLPOINTER)SQL_OV_ODBC3;
#endif

/**********************************************************************************
 *Portable (Version Neutral) SQL CLI Functions 
 **********************************************************************************/

SQLRETURN allocEnv(SQLHENV *Handle)
{
	SQLRETURN rc;
#if (ODBCVER >= 0x0300)
	SQLINTEGER Indicator;
#endif

	if (!Handle) return SQL_ERROR;

	*Handle = SQL_NULL_HENV;

#if (ODBCVER >= 0x0300)
	rc =  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, Handle);
	if (!SQL_SUCCEEDED(rc)) return rc;
#if defined(WIN32) || defined(_WIN64)
	/* This is manditory with Windoze...Must be ODBC2 or ODBC3 */
    rc = SQLSetEnvAttr(*Handle, SQL_ATTR_ODBC_VERSION, 
		(SQLPOINTER)gODBCVersion, SQL_IS_INTEGER);
	if (!SQL_SUCCEEDED(rc)) return rc;
#endif /* defined(WIN32) || defined(_WIN64) */	
	/* Get the null terminated string attribute. */
	Indicator = SQL_FALSE;
	rc = SQLGetEnvAttr(*Handle, SQL_ATTR_OUTPUT_NTS, 
		&Indicator, SQL_IS_INTEGER, NULL);
	if (!SQL_SUCCEEDED(rc)) return rc;
	/* If the null terminated string attribute is not set, set it. */
	if (Indicator != SQL_TRUE) {
		Indicator = SQL_TRUE;
	    rc = SQLSetEnvAttr(*Handle, SQL_ATTR_OUTPUT_NTS, 
			(SQLPOINTER)Indicator, 0);
	}
#else
    rc = SQLAllocEnv(Handle);
#endif

	return rc;
}

SQLRETURN allocConn(SQLHENV Input, SQLHDBC *Handle)
{

	if (!Handle) return SQL_ERROR;

	*Handle = SQL_NULL_HDBC;

#if (ODBCVER >= 0x0300)
    return SQLAllocHandle(SQL_HANDLE_DBC, Input, Handle);
#else
    return SQLAllocConnect(Input, Handle);
#endif
}

SQLRETURN allocStmt(SQLHDBC Input, SQLHSTMT *Handle)
{

	if (!Handle) return SQL_ERROR;

	*Handle = SQL_NULL_HSTMT;

#if (ODBCVER >= 0x0300)
    return SQLAllocHandle(SQL_HANDLE_STMT, Input, Handle);
#else
    return SQLAllocStmt(Input, Handle);
#endif
}

#if (ODBCVER >= 0x0300)
SQLRETURN allocDesc(SQLHSTMT Input, SQLHDESC *Handle)
{

	if (!Handle) return SQL_ERROR;

	*Handle = SQL_NULL_HDESC;

    return SQLAllocHandle(SQL_HANDLE_DESC, Input, Handle);
}
#endif

SQLRETURN freeEnv(SQLHENV Handle)
{

#if (ODBCVER >= 0x0300)
	return SQLFreeHandle(SQL_HANDLE_ENV, Handle);
#else
	return SQLFreeEnv(Handle);
#endif
}

SQLRETURN freeConn(SQLHDBC Handle)
{

#if (ODBCVER >= 0x0300)
	return SQLFreeHandle(SQL_HANDLE_DBC, Handle);
#else
	return SQLFreeConnect(Handle);
#endif
}

SQLRETURN freeStmt(SQLHSTMT Handle)
{

#if (ODBCVER >= 0x0300)
	return SQLFreeHandle(SQL_HANDLE_STMT, Handle);
#else
	return SQLFreeStmt(Handle, SQL_DROP);
#endif
}

#if (ODBCVER >= 0x0300)
SQLRETURN freeDesc(SQLHDESC Handle)
{

	return SQLFreeHandle(SQL_HANDLE_DESC, Handle);
}
#endif

SQLRETURN closeCursor(SQLHSTMT Handle)
{

#if (ODBCVER >= 0x0300)
	return SQLCloseCursor(Handle);
#else
	return SQLFreeStmt(Handle, SQL_CLOSE);
#endif
}

SQLRETURN getRowCount(SQLHSTMT Handle, SQLINTEGER *RowCount)
{
	SQLRETURN rc;
	
	if (!RowCount) return SQL_ERROR;

	*RowCount = -1;

#if (ODBCVER >= 0x0300)
    rc = SQLGetDiagField(SQL_HANDLE_STMT, Handle, 0, SQL_DIAG_ROW_COUNT,
		RowCount, sizeof(SQLINTEGER), NULL);
#else
    rc = SQLRowCount(Handle, RowCount);
#endif

	return rc;
}

SQLRETURN endTransaction(SQLHDBC Handle)
{
	SQLRETURN rc;

#if (ODBCVER >= 0x0300)
	rc = SQLEndTran(SQL_HANDLE_DBC, Handle, SQL_COMMIT);
#else
	rc = SQLTransact(gEnvironment, Handle, SQL_COMMIT);
#endif

	return rc;
}

/***************************************************************************************************
 * OLD STUFF
 ***************************************************************************************************/

static SQLRETURN getMsg(
	short       type, 
	SQLHANDLE   Handle, 
	SQLSMALLINT RecNumber,
	SQLTCHAR   *State,
	SQLTCHAR   *Msg,   
	SQLSMALLINT MsgLen,
	SQLINTEGER *NativeError
	)
{
	SQLRETURN    rc;
#if (ODBCVER >= 0x0300)
	SQLSMALLINT  HandleType;
#else
	SQLHENV      EnvironmentHandle;
	SQLHDBC      ConnectionHandle;
	SQLHSTMT     StatementHandle;
#endif

#if (ODBCVER >= 0x0300)
	switch (type) {
		case 1:  HandleType = SQL_HANDLE_ENV; break;
		case 2:  HandleType = SQL_HANDLE_DBC; break;
		case 3:  HandleType = SQL_HANDLE_STMT; break;
		default: HandleType = SQL_HANDLE_DESC; break;
	}
	rc = SQLGetDiagRec(
		HandleType, 
		Handle,
		RecNumber, 
		State,
		NativeError, 
		Msg,
		MsgLen, 
		NULL
	);
#else
	switch (type) {
		case 1:
			EnvironmentHandle = Handle;
			ConnectionHandle  = SQL_NULL_HDBC;
			StatementHandle   = SQL_NULL_HSTMT;
			break;
		case 2:
			EnvironmentHandle = SQL_NULL_HENV;
			ConnectionHandle  = Handle;
			StatementHandle   = SQL_NULL_HSTMT;
			break;
		case 3:
		case 4:
		default:
			EnvironmentHandle = SQL_NULL_HENV;
			ConnectionHandle  = SQL_NULL_HDBC;
			StatementHandle   = Handle;
			break;
		}
	rc = SQLError(
		EnvironmentHandle,
		ConnectionHandle, 
		StatementHandle,
		State, 
		NativeError,
		Msg, 
		MsgLen,
		NULL
		);
#endif
	/************
	if (rc == SQL_NO_DATA) return rc;

	tempMsg = 0;
	switch (rc) {
		case SQL_INVALID_HANDLE:
			tempMsg = "Invalid handle in checkError()";
			break;
		case SQL_ERROR:
			tempMsg = "Invalid parameter to checkError()";
			break;
		default:
			tempMsg = "Unexpected return code from SQLGetDiagRec()";
			break;
	}
	if (rc != SQL_SUCCESS && rc != SQL_NO_DATA) {
		fprintf(stderr, "%s\n", tempMsg);
		break;
	}
	************/

	return rc;
}

SQLRETURN getMsgEnv(SQLHENV Handle, SQLSMALLINT RecNumber, SQLTCHAR *State, 
	SQLTCHAR *Msg, SQLSMALLINT MsgLen, SQLINTEGER *NativeError)
{
	return getMsg(1, Handle, RecNumber, State, Msg, MsgLen, NativeError);
}

SQLRETURN getMsgConnect(SQLHDBC Handle, SQLSMALLINT RecNumber, SQLTCHAR *State, 
	SQLTCHAR *Msg, SQLSMALLINT MsgLen, SQLINTEGER *NativeError)
{
	return getMsg(2, Handle, RecNumber, State, Msg, MsgLen, NativeError);
}

SQLRETURN getMsgStmt(SQLHSTMT Handle, SQLSMALLINT RecNumber, SQLTCHAR *State, 
	SQLTCHAR *Msg, SQLSMALLINT MsgLen, SQLINTEGER *NativeError)
{
	return getMsg(3, Handle, RecNumber, State, Msg, MsgLen, NativeError);
}

SQLRETURN getMsgDesc(SQLHDESC Handle, SQLSMALLINT RecNumber, SQLTCHAR *State, 
	SQLTCHAR *Msg, SQLSMALLINT MsgLen, SQLINTEGER *NativeError)
{
	return getMsg(4, Handle, RecNumber, State, Msg, MsgLen, NativeError);
}

static void logMsg(short type, SQLHANDLE Handle, FILE *log, SQLRETURN rc)
{
	SQLSMALLINT RecNumber;  
	SQLRETURN   lrc;
	SQLTCHAR    Sqlstate[5+1]; 
	SQLTCHAR    MessageText[SQL_MAX_MESSAGE_LENGTH+1];
	SQLINTEGER  NativeError;

	if (rc == SQL_SUCCESS) return ;

	if (log == NULL) log = stderr;

	for (RecNumber = 1; ; RecNumber++) {

		lrc = getMsg(type, Handle, RecNumber, Sqlstate, 
			MessageText, sizeof(MessageText)/sizeof(MessageText[0]),
			&NativeError
			);

		if (lrc != SQL_SUCCESS) break;

		/* print the message data */

#ifdef UNICODE
		fwprintf(log, L"SQL State: %s, Message: %s.\n", Sqlstate, MessageText);
		if (NativeError)
			fwprintf(log, L"Native Error Code: 0x%08x.\n", (int)NativeError);
#else
		fprintf(log, "SQL State: %s, Message: %s.\n", Sqlstate, MessageText);
		if (NativeError)
			fprintf(log, "Native Error Code: 0x%08x.\n", (int)NativeError);
#endif
	}

	return ;
}

void logMsgEnv(SQLHENV Handle, FILE *log, SQLRETURN rc)
{
	logMsg(1, Handle, log, rc);
	return;
}

void logMsgConnect(SQLHDBC Handle, FILE *log, SQLRETURN rc)
{
	logMsg(2, Handle, log, rc);
	return;
}

void logMsgStmt(SQLHSTMT Handle, FILE *log, SQLRETURN rc)
{
	logMsg(3, Handle, log, rc);
	return;
}

void logMsgDesc(SQLHDESC Handle, FILE *log, SQLRETURN rc)
{
	logMsg(4, Handle, log, rc);
	return;
}

/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/

#ifdef  COMPILE_DUMMY

int dummy() {
	SQLRETURN    rc;
	SQLHDBC      ConnectionHandle;
	SQLHSTMT     StatementHandle;
	SQLUSMALLINT ColumnNumber; 
	SQLSMALLINT  TargetType;
	SQLLEN       BufferLengthLen;
	SQLLEN       StrLen_or_Ind;
	SQLUSMALLINT ParameterNumber;
	SQLSMALLINT  ValueType;
	SQLSMALLINT  ParameterType;
	SQLULEN      LengthPrecision;
	SQLSMALLINT  ParameterScale;
	SQLPOINTER   ParameterValue;
	SQLUSMALLINT FieldIdentifier;
	SQLPOINTER   CharacterAttribute; 
	SQLSMALLINT  StringLength; 
	SQLPOINTER   NumericAttribute; /* spec says (SQLPOINTER) not (SQLEN*) - PAH */
	SQLHDESC     SourceDescHandle;
	SQLHDESC     TargetDescHandle;
	SQLSMALLINT  DataType;  
	SQLSMALLINT  Nullable;
	SQLSMALLINT  FetchOrientation; 
	SQLROWOFFSET FetchOffset;
	SQLUSMALLINT Option;
	SQLINTEGER   Attribute; 
	SQLPOINTER   Value;
	SQLINTEGER   StringLengthInt; 
	SQLSMALLINT  BufferLength;
	SQLHDESC     DescriptorHandle;
	SQLSMALLINT  RecNumber; 
	SQLINTEGER   BufferLengthInt; 
	SQLTCHAR      Name[100];
	SQLSMALLINT  Type; 
	SQLSMALLINT  SubType;
	SQLLEN       Length; 
	SQLSMALLINT  Precision;
	SQLSMALLINT  Scale; 
	SQLPOINTER   Data;
	SQLULEN      ValueULen;
	SQLINTEGER   Indicator;
	SQLROWCOUNT  ValueRowCount;

#if (ODBCVER >= 0x0300)
    rc = SQLBindParam(
		StatementHandle,
		ParameterNumber,
		ValueType,
		ParameterType, 
		LengthPrecision,
		ParameterScale, 
		ParameterValue,
		&StrLen_or_Ind
		);
#endif

    rc = SQLCancel(
		StatementHandle
		);

#if (ODBCVER >= 0x0300)
    rc = SQLColAttribute (
		StatementHandle,
		ColumnNumber, 
		FieldIdentifier,
		CharacterAttribute, 
		BufferLength,
		&StringLength, 
		NumericAttribute /* spec says (SQLPOINTER) not (SQLEN*) - PAH */ 
		);
#endif

#if (ODBCVER >= 0x0300)
    rc = SQLCopyDesc(
		SourceDescHandle,
		TargetDescHandle
		);
#endif

#if (ODBCVER >= 0x0300)
    rc = SQLFetchScroll(
		StatementHandle,
		FetchOrientation, 
		FetchOffset
		);
#endif

#if (ODBCVER >= 0x0300)
    rc = SQLGetConnectAttr(
		ConnectionHandle,
		Attribute, 
		Value,
		BufferLength, 
		&StringLengthInt
		);
#endif

    rc = SQLGetConnectOption(
		ConnectionHandle,
		Option, 
		Value
		);

    rc = SQLGetData(
		StatementHandle,
		ColumnNumber, 
		TargetType,
		Value, 
		BufferLengthLen,
		&StrLen_or_Ind
		);

#if (ODBCVER >= 0x0300)
    rc =  SQLGetDescField(
		DescriptorHandle,
		RecNumber, 
		FieldIdentifier,
		Value, 
		BufferLengthInt,
		&StringLengthInt
		);
//
    rc = SQLGetDescRec(
		DescriptorHandle,
		RecNumber, 
		Name,
		BufferLength, 
		&StringLength,
		&Type, 
		&SubType,
		&Length, 
		&Precision,
		&Scale, 
		&Nullable
		);
#endif  /* ODBCVER >= 0x0300 */

#if (ODBCVER >= 0x0300)
    rc = SQLGetStmtAttr(
		StatementHandle,
		Attribute, 
		Value,
		BufferLengthInt, 
		&StringLengthInt
		);
#endif  /* ODBCVER >= 0x0300 */

    rc = SQLGetStmtOption(
		StatementHandle,
		Option, 
		Value
		);

    rc = SQLGetTypeInfo(
		StatementHandle,
		DataType
		);

    rc = SQLParamData(
		StatementHandle,
		&Value
		);

    rc = SQLPutData(
		StatementHandle,
		Data, 
		StrLen_or_Ind
		);

#if (ODBCVER >= 0x0300)
    rc = SQLSetConnectAttr(
		ConnectionHandle,
		Attribute, 
		Value,
		StringLengthInt
		);
#endif /* ODBCVER >= 0x0300 */

    rc = SQLSetConnectOption(
		ConnectionHandle,
		Option, 
		ValueULen
		);

#if (ODBCVER >= 0x0300)
    rc = SQLSetDescField(
		DescriptorHandle,
		RecNumber, 
		FieldIdentifier,
		Value, 
		BufferLengthInt
		);
//
    rc = SQLSetDescRec(
		DescriptorHandle,
		RecNumber, 
		Type,
		SubType, 
		Length,
		Precision, 
		Scale,
		Data, 
		&StringLengthInt,
		&Indicator
		);
#endif /* ODBCVER >= 0x0300 */

    rc = SQLSetParam(
		StatementHandle,
		ParameterNumber, 
		ValueType,
		ParameterType, 
		LengthPrecision,
		ParameterScale, 
		ParameterValue,
		&StrLen_or_Ind
		);

#if (ODBCVER >= 0x0300)
    rc = SQLSetStmtAttr(
		StatementHandle,
		Attribute, 
		Value,
		StringLengthInt
		);
#endif

    rc = SQLSetStmtOption(
		StatementHandle,
		Option, 
		ValueRowCount
		);

	return 0;
};

#endif /* COMPILE_DUMMY */

//#define DO_CURSOR
#ifdef  DO_CURSOR
	trace("Allocating SQL Statement Handle.");
	rc = allocStmt(ConnectionHandle, &StatementHandle);
	checkErrorDBC(ConnectionHandle, rc);

	trace("Getting Cursor Name.");
	TSTRCPY(CursorName, "Undefined");
    rc = SQLGetCursorName(/* Raw */ StatementHandle, CursorName, 
		sizeof(CursorName)/sizeof(CursorName[0]), NULL);
	checkErrorStmt(StatementHandle, rc);
	fprintf(stderr, "Cursor name is: %s.\n",CursorName);

	trace("Setting Cursor Name.");
	TSTRCPY(CursorName, "TestCursorName");
    rc = SQLSetCursorName(/* Raw */ StatementHandle, CursorName, SQL_NTS);
	checkErrorStmt(StatementHandle, rc);

	trace("Getting Cursor Name.");
	TSTRCPY(CursorName, "Undefined");
    rc = SQLGetCursorName(/* Raw */ StatementHandle, CursorName, 
		sizeof(CursorName)/sizeof(CursorName[0]), NULL);
	checkErrorStmt(StatementHandle, rc);
	fprintf(stderr, "Cursor name is: %s.\n",CursorName);
#endif /* DO_CURSOR */
