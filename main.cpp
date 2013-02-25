// START:main
#include "sqlodbc.h"

#include "sqlcpp.h"

#include <iostream>

using namespace std;
using namespace odbc;

/***
typedef unsigned char   SQLCHAR;
typedef signed char     SQLSCHAR;
typedef unsigned char   SQLDATE;
typedef unsigned char   SQLDECIMAL;
typedef double          SQLDOUBLE;
typedef double          SQLFLOAT;
typedef long            SQLINTEGER;
typedef unsigned long   SQLUINTE
***/

typedef union {
	SQLCHAR     sqlchar_;
	SQLSCHAR    sqlschar_;
	SQLDATE     saldate_;
	SQLDECIMAL  sqldec;
	SQLFLOAT    sqlfloat;
	SQLINTEGER  sqlint;
	SQLUINTEGER sqluint;
	int int_;
	char char_;
	char *cp;
} xyz;

typedef struct table_entry {
	std::string catalog;
	std::string schema;
	std::string name;
	std::string type;

	int  columns;
	long rows;
} table_entry;

typedef struct column_entry {
	SQLCHAR     ColumnName_[256];
	SQLSMALLINT NameLength;
    SQLSMALLINT DataType;
	SQLUINTEGER ColumnSize;
    SQLSMALLINT DecimalDigits;
	SQLSMALLINT Nullable;

	long        NullValues;
	long        UniqueValues;
//	values_set *Values;
} column_entry;

/*
 * Steps (for each data source):
 *
 * 1) Connect to the data source
 * 2) Get list of tables
 * 3) Do "pass 1" on the tables on x threads.
 *   a) Determine unique (key fields)
 *   b) Determine single-value (irrelevant) fields.
 *   c) Save table info: row count, column count, irrelevant count, table type (transaction/master/parameter/etc).
 * 4) Do "pass 2" on the tables that need it. 
 *   a) Determine alternate keys columns.
 * 5) Disconnect from the data source.
 * 
 */

int mainXY(int argc, char** argv)
{
	SQLEnvironment env;
	SQLConnection  conn(env);

	conn.connect("LERG", "admin", "");


  return 1;

  if (argc < 4) {
    cerr << "Usage: " << argv[0]
         << " DSN User Password [Sql Statement] " << endl;
    return 1;
  }

  try {
    DataSource db;
    db.Connect(argv[1],	argv[2], argv[3]);

    SqlStatement sql(db);
    if (argc < 5) {
      sql.Tables();
    } else {
      sql.Execute(argv[4]);
    }
    sql.WriteResultSet(cout, 256,
      "NULL", "\t");
    return 0;
  } catch (const exception& ex) {
    cerr << argv[0] << ": " 
         << ex.what() << endl;
  } catch (...) {
    cerr << "Unknown exception." 
         << endl;
  }
  return 1;
}
// END:main
