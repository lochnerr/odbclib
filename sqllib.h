/*-----------------------------------------------------------------------------
 * SQL CLI (ODBC) Library Header 
 *
 * $Id$
 * $HeadURL$
 *
 * Copyright (C) 2008-2009 Clone Research Corporation 
 * All Rights Reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef sqllib_h_included
#define sqllib_h_included 1

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Opaque object handles 
 */
typedef struct SQLLibResultsTag    *SQLLibResults;
typedef struct SQLLibConnectionTag *SQLLibConnection;

/* Field size constants */

#define SQLLIB_CATALOG_LEN	256
#define SQLLIB_SCHEMA_LEN	129
#define SQLLIB_NAME_LEN		129
#define SQLLIB_TYPE_LEN		129
#define SQLLIB_REMARKS_LEN	255
#define SQLLIB_TYPENAME_LEN	65
#define SQLLIB_DEFAULT_LEN	129

/* Constants */

enum {
	SQLLibBindToNumeric = 0,
	SQLLibBindToChar    = 1
} SQLLibBindMode;

/* Data returned by SQLLibTables function */

typedef struct SQLLibTableInfoTag {
	char      Catalog [SQLLIB_CATALOG_LEN];
	char      Schema  [SQLLIB_SCHEMA_LEN];
	char      Name    [SQLLIB_NAME_LEN];
	char      Type    [SQLLIB_TYPE_LEN];
	char      Remarks [SQLLIB_REMARKS_LEN];
} SQLLibTableInfo;

/* Data returned by SQLLibColumns function */

typedef struct SQLLibColumnInfoTag {
	char      Catalog [SQLLIB_CATALOG_LEN];
	char      Schema  [SQLLIB_SCHEMA_LEN];
	char      Table   [SQLLIB_NAME_LEN];
	char      Name    [129]; /* ColumnName */
	short     DataType;
	char      TypeName[SQLLIB_TYPENAME_LEN];
	long      ColumnSize;
	long      BufferLength;
	short     DecimalDigits;
	short     Precision;
	short     Nullable;
	char      Remarks [SQLLIB_REMARKS_LEN];
	char      Default [SQLLIB_DEFAULT_LEN];
	short     DateTime;
	long      OctetLength;     
	long      OrdinalPosition; 
	char      IsNullable[255];
	/* Microsoft Extensions */
	short     SQLType;
	long      Ordinal;
} SQLLibColumnInfo;

/* Data returned by SQLLibIndexes function */

typedef struct SQLLibIndexInfoTag {
	char      Catalog     [SQLLIB_CATALOG_LEN];
	char      Schema      [SQLLIB_SCHEMA_LEN];
	char      Table       [SQLLIB_NAME_LEN];
	short     NonUnique;
	char      Qualifier   [129];
	char      IndexName   [129];
	short     IndexType;     
	short     OrdinalPosition;
	char      Name        [129]; /* ColumnName */
	char      Asc_Or_Desc [2]; /* A-Ascending D-Descending */
} SQLLibIndexInfo;

/* Data returned by SQLLibXXX function */

typedef struct SQLLibIdentInfoTag {
	short     Scope;
	char      Name        [129]; /* ColumnName */
	short     DataType;
	char      TypeName    [SQLLIB_TYPENAME_LEN];
	long      ColumnSize;
	long      BufferLength;
	short     DecimalDigits;
	short     Pseudo;
} SQLLibIdentInfo;

/* Data returned by SQLLibXXX function */

typedef struct SQLLibConnectionInfoTag {
	char      DBMSName    [129];
	char      DBMSVersion [129];
	char      XOpenYear   [8];
} SQLLibConnectionInfo;

/* Data returned by SQLLibSources function */

typedef struct SQLLibSourceInfoTag {
	char      Name        [129];
	char      Description [256];
} SQLLibSourceInfo;

/*
 * SQL CLI (ODBC) Library API declarations
 */

/*
 * Allocate and Deallocate Functions
 */

/*
 * Get and Set Attribute Functions
 */

/*
 * Connect and Disconnect Functions
 */

SQLLibConnection SQLLibConnectionCreate  (void );
void             SQLLibConnectionDestroy (SQLLibConnection *Conn);

int              SQLLibConnect           (SQLLibConnection  Conn, const char *Server, const char *UserName, const char *Authentication);
void             SQLLibDisconnect        (SQLLibConnection  Conn);

void             SQLLibConnectInfo       (SQLLibConnection  Conn, SQLLibConnectionInfo *Result);

void *           SQLLibConnectionHandle  (SQLLibConnection  Conn);

/*
 * Execution Functions
 */ 

SQLLibResults SQLLibExecute    (SQLLibConnection Conn, const char *SQL);
int           SQLLibColumnCount(SQLLibResults Results);

/*
 * Metadata Functions
 */
								
SQLLibResults SQLLibTables     (SQLLibConnection Conn, const char *CatalogName, const char *SchemaName,
							    const char *TableName, const char *TableType,   SQLLibTableInfo *Result);

SQLLibResults SQLLibColumns    (SQLLibConnection Conn, const char *CatalogName, const char *SchemaName,
							    const char *TableName, const char *ColumnName,  SQLLibColumnInfo *Result);

SQLLibResults SQLLibIndexes    (SQLLibConnection Conn, const char *CatalogName, const char *SchemaName,
							    const char *TableName, SQLLibIndexInfo *Result);

SQLLibResults SQLLibIdentRows  (SQLLibConnection Conn, const char *CatalogName, const char *SchemaName,
							    const char *TableName, SQLLibIdentInfo *Result);

SQLLibResults SQLLibSources    (SQLLibSourceInfo *Result);

/*
 * Metadata Buffer Functions
 */
								
void SQLLibTableInfoClear (SQLLibTableInfo  *Info);
void SQLLibColumnInfoClear(SQLLibColumnInfo *Info);
void SQLLibIndexInfoClear (SQLLibIndexInfo  *Info);
void SQLLibIdentInfoClear (SQLLibIdentInfo  *Info);
void SQLLibSourceInfoClear(SQLLibSourceInfo *Info);

/*
 * Retrieving Result Functions
 */

void          SQLLibFreeResults(SQLLibResults *Results);

/*
 * Descriptor Functions
 */

/*
 * Introspection Functions
 */

/*
 * Transaction Control Functions
 */

/*
 * Other Functions
 */

int         SQLLibNext         (SQLLibResults Results);
void        SQLLibPrint        (SQLLibResults Results, FILE *out, char sep, int headings);

void        SQLLibBindByName   (SQLLibResults Results, const char *Name, short Type, void *Target, long Length);
const char *SQLLibColumnChar   (SQLLibResults Results, unsigned short Column);
long        SQLLibColumnLong   (SQLLibResults Results, unsigned short Column);
double      SQLLibColumnDouble (SQLLibResults Results, unsigned short Column);

int         SQLLibIsNull       (SQLLibResults Results, unsigned short Column);

const char *SQLLibColumnName   (SQLLibResults Results, int col);

void        SQLLibBind         (SQLLibResults Results, int BindMode); 

//SQLRETURN   SQLLibGetLong      (SQLHSTMT Handle, unsigned short Column, long   *Value, unsigned *IsNull);
//SQLRETURN   SQLLibGetDouble    (SQLHSTMT Handle, unsigned short Column, double *Value, unsigned *IsNull);
//SQLRETURN   SQLLibGetChar      (SQLHSTMT Handle, unsigned short Column, char   *Value, long  Length, unsigned *IsNull);

/*
 * Diagnostic Functions
 */

//int isSupported            (SQLHDBC Connection, SQLUSMALLINT FunctionId);


#ifdef __cplusplus
}
#endif

#endif
