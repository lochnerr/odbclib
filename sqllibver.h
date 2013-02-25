/* sqllibver.h */ 

#if (ODBCVER < 0x0300)
typedef void *              SQLHANDLE;
typedef SQLHANDLE           SQLHDESC;
#define SQL_NO_DATA 100
#define SQL_ROW_IDENTIFIER 1
#define SQL_DIAG_SELECT_CURSOR 1
#define SQL_DIAG_INSERT 2
#define SQL_DIAG_UNKNOWN_STATEMENT 111
#endif

/**********************************************************************************
 * Static Portability (Version Neutral) Functions 
 **********************************************************************************/

SQLRETURN allocEnv(SQLHENV *Handle);
SQLRETURN allocConn(SQLHENV Input, SQLHDBC *Handle);
SQLRETURN allocStmt(SQLHDBC Input, SQLHSTMT *Handle);
#if (ODBCVER >= 0x0300)
SQLRETURN allocDesc(SQLHSTMT Input, SQLHDESC *Handle);
#endif
SQLRETURN freeEnv(SQLHENV Handle);
SQLRETURN freeConn(SQLHDBC Handle);
SQLRETURN freeStmt(SQLHSTMT Handle);
#if (ODBCVER >= 0x0300)
SQLRETURN freeDesc(SQLHDESC Handle);
#endif
SQLRETURN closeCursor(SQLHSTMT Handle);
SQLRETURN getRowCount(SQLHSTMT Handle, SQLINTEGER *RowCount);
SQLRETURN endTransaction(SQLHDBC Handle);


SQLRETURN getMsgEnv       (SQLHENV  Handle, SQLSMALLINT RecNumber, SQLTCHAR *State, 
	                       SQLTCHAR *Msg, SQLSMALLINT MsgLen, SQLINTEGER *NativeError);
SQLRETURN getMsgConnect   (SQLHDBC  Handle, SQLSMALLINT RecNumber, SQLTCHAR *State, 
	                       SQLTCHAR *Msg, SQLSMALLINT MsgLen, SQLINTEGER *NativeError);
SQLRETURN getMsgStmt      (SQLHSTMT Handle, SQLSMALLINT RecNumber, SQLTCHAR *State, 
	                       SQLTCHAR *Msg, SQLSMALLINT MsgLen, SQLINTEGER *NativeError);
//SQLRETURN getMsgDesc      (SQLHDESC Handle, SQLSMALLINT RecNumber, SQLTCHAR *State, 
//	                       SQLTCHAR *Msg, SQLSMALLINT MsgLen, SQLINTEGER *NativeError);

void logMsgEnv       (SQLHENV  Handle, FILE *log, SQLRETURN rc);
void logMsgConnect   (SQLHDBC  Handle, FILE *log, SQLRETURN rc);
void logMsgStmt      (SQLHSTMT Handle, FILE *log, SQLRETURN rc);
//void logMsgDesc      (SQLHDESC Handle, FILE *log, SQLRETURN rc);

//#define checkErrorEnv(Handle, rc)   logMsgEnv(Handle, stderr, rc);
//#define checkErrorDBC(Handle, rc)   logMsgConnect(Handle, stderr, rc);
//#define checkErrorStmt(Handle, rc)  logMsgStmt(Handle, stderr, rc);
