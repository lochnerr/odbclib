/**/ 
#include <stdio.h>

#include "funs.h"

void check_fun(SQLHDBC Conn, SQLSMALLINT fun, const char *name)
{
	SQLRETURN rc;
	SQLUSMALLINT supported;

	rc = SQLGetFunctions(Conn, fun, &supported);
	if (!SQL_SUCCEEDED(rc))
		supported = 0;

	if (!supported) {
		fprintf(stderr, "Function '%s' is not supported.\n", name);
	} /***
	else { 
		fprintf(stderr, "Function '%s' is supported.\n", name);
	} ***/
}

typedef struct SQLSupported {
char AllocConnect;
char AllocEnv;
char AllocHandle;
} SQLSupported;

typedef struct abc {
SQLUSMALLINT FunctionID;
SQLUSMALLINT Supported;
char        *Text;
} abc;

abc list[] = {
	{ SQL_API_SQLALLOCCONNECT,  0, "AllocConnect" },
	{ SQL_API_SQLALLOCENV,      0, "AllocEnv" },
};

static char check_ones(SQLHDBC Conn, SQLUSMALLINT FunctionID, char YES, char NO)
{
	SQLRETURN rc;
	SQLUSMALLINT supported;

	if (FunctionID == 9999)
		return NO;

	rc = SQLGetFunctions(Conn, FunctionID, &supported);
	if (!SQL_SUCCEEDED(rc))
		return YES;

	return NO;
}

void check_all(SQLHDBC Conn, SQLSupported *list, char YES, char NO)
{
	list->AllocConnect  = check_ones(Conn, SQL_API_SQLALLOCCONNECT, YES, NO);
	
}

#define CHECK_FUNCTION(fun) check_fun(Conn, fun, #fun) 

void check_em(SQLHDBC Conn)
{

#ifdef             SQL_API_SQLALLOCCONNECT
	CHECK_FUNCTION(SQL_API_SQLALLOCCONNECT);
#endif
#ifdef             SQL_API_SQLALLOCENV
	CHECK_FUNCTION(SQL_API_SQLALLOCENV);
#endif
#ifdef             SQL_API_SQLALLOCHANDLE
	CHECK_FUNCTION(SQL_API_SQLALLOCHANDLE);
#endif
#ifdef             SQL_API_SQLALLOCSTMT
	CHECK_FUNCTION(SQL_API_SQLALLOCSTMT);
#endif
#ifdef             SQL_API_SQLBINDCOL
	CHECK_FUNCTION(SQL_API_SQLBINDCOL);
#endif
#ifdef             SQL_API_SQLBINDPARAM
	CHECK_FUNCTION(SQL_API_SQLBINDPARAM);
#endif
#ifdef             SQL_API_SQLCANCEL
	CHECK_FUNCTION(SQL_API_SQLCANCEL);
#endif
#ifdef             SQL_API_SQLCLOSECURSOR
	CHECK_FUNCTION(SQL_API_SQLCLOSECURSOR);
#endif
#ifdef             SQL_API_SQLCOLATTRIBUTE
	CHECK_FUNCTION(SQL_API_SQLCOLATTRIBUTE);
#endif
#ifdef             SQL_API_SQLCOLUMNS
	CHECK_FUNCTION(SQL_API_SQLCOLUMNS);
#endif
#ifdef             SQL_API_SQLCONNECT
	CHECK_FUNCTION(SQL_API_SQLCONNECT);
#endif
#ifdef             SQL_API_SQLCOPYDESC
	CHECK_FUNCTION(SQL_API_SQLCOPYDESC);
#endif
#ifdef             SQL_API_SQLDATASOURCES
	CHECK_FUNCTION(SQL_API_SQLDATASOURCES);
#endif
#ifdef             SQL_API_SQLDESCRIBECOL
	CHECK_FUNCTION(SQL_API_SQLDESCRIBECOL);
#endif
#ifdef             SQL_API_SQLDISCONNECT
	CHECK_FUNCTION(SQL_API_SQLDISCONNECT);
#endif
#ifdef             SQL_API_SQLENDTRAN
	CHECK_FUNCTION(SQL_API_SQLENDTRAN);
#endif
#ifdef             SQL_API_SQLERROR
	CHECK_FUNCTION(SQL_API_SQLERROR);
#endif
#ifdef             SQL_API_SQLEXECDIRECT
	CHECK_FUNCTION(SQL_API_SQLEXECDIRECT);
#endif
#ifdef             SQL_API_SQLEXECUTE
	CHECK_FUNCTION(SQL_API_SQLEXECUTE);
#endif
#ifdef             SQL_API_SQLFETCH
	CHECK_FUNCTION(SQL_API_SQLFETCH);
#endif
#ifdef             SQL_API_SQLFETCHSCROLL
	CHECK_FUNCTION(SQL_API_SQLFETCHSCROLL);
#endif
#ifdef             SQL_API_SQLFREECONNECT
	CHECK_FUNCTION(SQL_API_SQLFREECONNECT);
#endif
#ifdef             SQL_API_SQLFREEENV
	CHECK_FUNCTION(SQL_API_SQLFREEENV);
#endif
#ifdef             SQL_API_SQLFREEHANDLE
	CHECK_FUNCTION(SQL_API_SQLFREEHANDLE);
#endif
#ifdef             SQL_API_SQLFREESTMT
	CHECK_FUNCTION(SQL_API_SQLFREESTMT);
#endif
#ifdef             SQL_API_SQLGETCONNECTATTR
	CHECK_FUNCTION(SQL_API_SQLGETCONNECTATTR);
#endif
#ifdef             SQL_API_SQLGETCONNECTOPTION
	CHECK_FUNCTION(SQL_API_SQLGETCONNECTOPTION);
#endif
#ifdef             SQL_API_SQLGETCURSORNAME
	CHECK_FUNCTION(SQL_API_SQLGETCURSORNAME);
#endif
#ifdef             SQL_API_SQLGETDATA
	CHECK_FUNCTION(SQL_API_SQLGETDATA);
#endif
#ifdef             SQL_API_SQLGETDESCFIELD
	CHECK_FUNCTION(SQL_API_SQLGETDESCFIELD);
#endif
#ifdef             SQL_API_SQLGETDESCREC
	CHECK_FUNCTION(SQL_API_SQLGETDESCREC);
#endif
#ifdef             SQL_API_SQLGETDIAGFIELD
	CHECK_FUNCTION(SQL_API_SQLGETDIAGFIELD);
#endif
#ifdef             SQL_API_SQLGETDIAGREC
	CHECK_FUNCTION(SQL_API_SQLGETDIAGREC);
#endif
#ifdef             SQL_API_SQLGETENVATTR
	CHECK_FUNCTION(SQL_API_SQLGETENVATTR);
#endif
#ifdef             SQL_API_SQLGETFUNCTIONS
	CHECK_FUNCTION(SQL_API_SQLGETFUNCTIONS);
#endif
#ifdef             SQL_API_SQLGETINFO
	CHECK_FUNCTION(SQL_API_SQLGETINFO);
#endif
#ifdef             SQL_API_SQLGETSTMTATTR
	CHECK_FUNCTION(SQL_API_SQLGETSTMTATTR);
#endif
#ifdef             SQL_API_SQLGETSTMTOPTION
	CHECK_FUNCTION(SQL_API_SQLGETSTMTOPTION);
#endif
#ifdef             SQL_API_SQLGETTYPEINFO
	CHECK_FUNCTION(SQL_API_SQLGETTYPEINFO);
#endif
#ifdef             SQL_API_SQLNUMRESULTCOLS
	CHECK_FUNCTION(SQL_API_SQLNUMRESULTCOLS);
#endif
#ifdef             SQL_API_SQLPARAMDATA
	CHECK_FUNCTION(SQL_API_SQLPARAMDATA);
#endif
#ifdef             SQL_API_SQLPREPARE
	CHECK_FUNCTION(SQL_API_SQLPREPARE);
#endif
#ifdef             SQL_API_SQLPUTDATA
	CHECK_FUNCTION(SQL_API_SQLPUTDATA);
#endif
#ifdef             SQL_API_SQLROWCOUNT
	CHECK_FUNCTION(SQL_API_SQLROWCOUNT);
#endif
#ifdef             SQL_API_SQLSETCONNECTATTR
	CHECK_FUNCTION(SQL_API_SQLSETCONNECTATTR);
#endif
#ifdef             SQL_API_SQLSETCONNECTOPTION
	CHECK_FUNCTION(SQL_API_SQLSETCONNECTOPTION);
#endif
#ifdef             SQL_API_SQLSETCURSORNAME
	CHECK_FUNCTION(SQL_API_SQLSETCURSORNAME);
#endif
#ifdef             SQL_API_SQLSETDESCFIELD
	CHECK_FUNCTION(SQL_API_SQLSETDESCFIELD);
#endif
#ifdef             SQL_API_SQLSETDESCREC
	CHECK_FUNCTION(SQL_API_SQLSETDESCREC);
#endif
#ifdef             SQL_API_SQLSETENVATTR
	CHECK_FUNCTION(SQL_API_SQLSETENVATTR);
#endif
#ifdef             SQL_API_SQLSETPARAM
	CHECK_FUNCTION(SQL_API_SQLSETPARAM);
#endif
#ifdef             SQL_API_SQLSETSTMTATTR
	CHECK_FUNCTION(SQL_API_SQLSETSTMTATTR);
#endif
#ifdef             SQL_API_SQLSETSTMTOPTION
	CHECK_FUNCTION(SQL_API_SQLSETSTMTOPTION);
#endif
#ifdef             SQL_API_SQLSPECIALCOLUMNS
	CHECK_FUNCTION(SQL_API_SQLSPECIALCOLUMNS);
#endif
#ifdef             SQL_API_SQLSTATISTICS
	CHECK_FUNCTION(SQL_API_SQLSTATISTICS);
#endif
#ifdef             SQL_API_SQLTABLES
	CHECK_FUNCTION(SQL_API_SQLTABLES);
#endif
#ifdef             SQL_API_SQLTRANSACT
	CHECK_FUNCTION(SQL_API_SQLTRANSACT);
#endif

}