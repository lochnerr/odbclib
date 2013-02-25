// START:main

#include <stdio.h>

#include <windows.h>

//#include <sqlext.h>

//#define ODBCVER 0x0200
#include <sql.h>

/***********************************************************************************************

' Set Connection
    
ConnStr = "Driver={SQL Native Client};Server=192.168.33.12;Database=CABS;Uid=sa;Pwd=clone15654!;"

Set Conn = New ADODB.Connection
Conn.ConnectionTimeout = 3600 * 8
Conn.CommandTimeout = Conn.ConnectionTimeout
Conn.Open ConnStr

' Clear the match table...

Conn.Execute ("DELETE FROM ToMatchMatches")

' Open input recordsets

Query = "SELECT * FROM viewToMatchSwitch ORDER BY Year, Month, Day, ConnectSecs, DurationSecs"
Set Switch = New ADODB.Recordset
Switch.CursorLocation = adUseServer
Switch.Open Query, Conn, adOpenForwardOnly, adLockReadOnly

***********************************************************************************************/

/*
 * The old problems still exist...
 *
 * You want to remove odbc dependencies, if possible
 * But, you don't want to introduce tons of new repetitive stuff (e.g. SQLCHAR vs. ODBCCHAR)
 *
 * Alternative is embracing the library, not hiding it, but composing it.
 * That seemed to be better for s.100 - although I never really used it...
 *
 * In any case the C composition(s) should be automatically wrappable by C++
 *
 * odbcConnection server;
 * server.ConnectionTimeout = 60;
 *
 */


typedef struct odbc_env
{
	SQLHENV henv;
} odbc_env;

static odbc_env odbc_global;

odbc_env *odbcInit(void )
{
	SQLRETURN rc;
	
#if (ODBCVER < 0x0300) 
	rc = SQLAllocEnv(&odbc_global.henv);
#else /* >= 0x0300 */
	rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HENV, &odbc_global.henv);
#endif
	if (rc == SQL_SUCCESS_WITH_INFO) {
        ;
    } else if (rc != SQL_SUCCESS) {
        ;
    }

	return &odbc_global;
}

void odbcTerm(odbc_env *odbc_globals)
{
	SQLRETURN rc;

#if (ODBCVER < 0x0300) 
	rc = SQLFreeEnv(odbc_globals->henv);
#else /* >= 0x0300 */
	rc = SQLFreeHandle(SQL_HANDLE_ENV, odbc_globals->henv);
#endif
    
	if (rc == SQL_SUCCESS_WITH_INFO) {
        ;
    } else if (rc != SQL_SUCCESS) {
        ;
    }

	odbc_globals->henv = SQL_NULL_HENV;

	return;
}

typedef struct xyz *xyz;

int mainX(int argc, const char *const argv[])
{	
	odbc_env *odbc;
//	int rc;
//	int env;
//	int conn;
//	int stmt;
//	int i;

	if (argc < 2) { 
		printf("format: %s\n", argv[0]);
		return 1;
	}	 

	odbc = odbcInit();

	printf("Hello...\n");

	odbcTerm(odbc);


/* Function to start getting the data: SQLLibTables( STMT, CAT, SCHEM, NAME, TYPE, *STRUCT )
 * Function to get next etry:          SQLLibNext
 * Information Entry Structure:        SQLLibTable
 * Information Collection:             SQLLibTables
 */
/***********************************/
 /*
  * There are really 3 (5?) levels...
  *
  * 0) The library itself
  * 1) Library Helper Routines
  *    These need to know about the library internals and headers.
  *    These use layer 0.
  * 2) Abstraction Routines
  *    These use library headers internally but do not expose them to clients.
  *    These should mirror C++ functions.
  *    These use layer 1 and/or layer 0.
  * 3) C++ Classes
  *    C++ wrappers for layer 2.
  *    These use layer 2.
  * 4) Applications.
  *    Applications use layer 3 (C++) or layer 2 (C).
  */
/***********************************/
	/***
	rc = allocEnv(&env);
	if (logMsgenv(env, stderr, rc)) return 1;
	
	rc = allocConnect(env, &conn);
	if (logMsgConnect(env, stderr, rc)) return 1;
	
	rc = allocStmt(conn, &stmt);
	if (logMsgConnect(conn, stderr, rc)) return 1;

	rc = SQLLibConnect(conn, "db", "user", "pass");
	if (logMsgConnect(conn, stderr, rc)) return 1;	

	// Perhaps have a SQLLibExecuteSQLNoBind 
	rc = SQLLibExecuteSQL(stmt, "SELECT * FROM TABLE;");
	if (logMsgStmt(stmt, stderr, rc)) return 1;

	for (i = 0; SQLLibNext(stmt) != EOF; i++) {
	}

	rc = freeStmt(stmt);
	logMsgStmt(stmt, stderr, rc);

	rc = SQLLibDisconnect(conn);
	if (logMsgConnect(conn, stderr, rc)) return 1;

	freeStmt(stmt);
	freeConnect(conn);
	freeEnv(env);
	***/
	return 1;
}
// END:main
