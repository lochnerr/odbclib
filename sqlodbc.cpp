//
//  $Id: sqlodbc.cpp,v 1.17 2001/10/14 15:50:37 erngui Exp $
//
#define	SQL_NOUNICODEMAP
#include "sqlodbc.h"
#include <odbcinst.h>
#include <assert.h>
#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

namespace odbc_5189169245 {


SQLHENV DataSource::g_henv = SQL_NULL_HENV;
long DataSource::g_envCount = -1;

SqlStatement::SqlStatement(DataSource& db) 
  : m_dataSource(db), m_boundCols(0), m_hstmt(SQL_NULL_HSTMT)
{
    CheckStatus(::SQLAllocHandle(SQL_HANDLE_STMT, db.m_hdbc, 
        &m_hstmt));
}

SqlStatement::~SqlStatement()
{
    ::SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
}

bool SqlStatement::IsValid() const
{
    return m_hstmt != SQL_NULL_HSTMT;
}

void SqlStatement::CheckStatus(RETCODE ret)
{
    m_dataSource.CheckStatus(ret, m_hstmt);
}

void DataSource::CheckStatus(RETCODE ret, HSTMT st)
{
    if (ret == SQL_SUCCESS_WITH_INFO) {
        TraceError(st);
    } else if (ret != SQL_SUCCESS) {
        ThrowError(st);
    }
}

void SqlStatement::Execute(const string& sql)
{
    assert(IsValid());
    CheckStatus(::SQLExecDirect(m_hstmt, (UCHAR*)sql.c_str(),
        SQL_NTS));
}

void SqlStatement::Tables(const string& type)
{
	//SQLCHAR *       CatalogName = "";
	//SQLCHAR *       SchemaName = "";
	//SQLCHAR *       TableName = "";
	//SQLCHAR *       TableType = "";

    assert(IsValid());
    CheckStatus(
		::SQLTables(
			m_hstmt, 
			(SQLCHAR *)NULL, //"%", //CatalogName, 
			SQL_NTS,
			(SQLCHAR *)NULL, //"%", //SchemaName, 
			SQL_NTS,
			(SQLCHAR *)NULL, //"%", //TableName, 
			SQL_NTS,
			type.length() > 0 ? (SQLCHAR*)type.c_str() : NULL,
			SQL_NTS
			)
		);
	/***
    CheckStatus(::SQLTables(m_hstmt, NULL, SQL_NTS,
        NULL, SQL_NTS,
        NULL, SQL_NTS,
        type.length() > 0? (SQLCHAR*)type.c_str() : NULL, 
        SQL_NTS));
	***/
}


void SqlStatement::Columns(const string& table)
{
    assert(IsValid());
    CheckStatus(::SQLColumns(m_hstmt, NULL, SQL_NTS,
        NULL, SQL_NTS,
        table.length() > 0? (SQLCHAR*)table.c_str() : NULL, 
        SQL_NTS,
        NULL, SQL_NTS));
}
    
void SqlStatement::DescribeCol(USHORT number, UCHAR *name,
       USHORT BufferLength, SHORT *NameLength,
       SHORT *DataType, ULONG *ColumnSize,
       SHORT *DecimalDigits, SHORT *Nullable)
{
	SQLSMALLINT nameLength;
    SQLSMALLINT dataType;
	SQLUINTEGER columnSize;
    SQLSMALLINT decimalDigits;
	SQLSMALLINT nullable;

    assert(IsValid());
    CheckStatus(
		::SQLDescribeCol(
			m_hstmt, 
			number, 
			name,
			BufferLength, 
			&nameLength,
			&dataType, 
			&columnSize,
			&decimalDigits, 
			&nullable
			)
		);
	*NameLength    = nameLength;
	*DataType      = dataType;
	*ColumnSize    = columnSize;
	*DecimalDigits = decimalDigits;
	*Nullable      = nullable;
}


SWORD SqlStatement::NumResultCols()
{
    assert(IsValid());
    SWORD cols;
    CheckStatus(::SQLNumResultCols(m_hstmt, &cols));
    return cols;
}

SDWORD SqlStatement::RowCount()
{
    assert(IsValid());
    SDWORD rows;
    CheckStatus(::SQLRowCount(m_hstmt, &rows));
    return rows;
}


bool SqlStatement::Next()
{
    assert(IsValid());
    RETCODE ret = ::SQLFetch(m_hstmt);
    if (ret == SQL_NO_DATA_FOUND)
        return false;
    CheckStatus(ret);
    return true;
}

void SqlStatement::GetData(WORD col, DWORD maxlen, void* data,
    SDWORD* len)
{
    assert(IsValid());
    CheckStatus(::SQLGetData(m_hstmt, col, SQL_C_DEFAULT,
        data, maxlen, len));
}

void SqlStatement::BindCol(void* rgbValue, SDWORD cbValueMax, 
    SDWORD* pcbValue, SQLSMALLINT TargetType)
{
	SQLINTEGER len;
    assert(IsValid());
    RETCODE ret = ::SQLBindCol(
		m_hstmt, 
		m_boundCols+1, 
        TargetType, 
		rgbValue, 
		cbValueMax, 
		&len //pcbValue
		);
	*pcbValue = len;
    CheckStatus(ret);
    m_boundCols++;
}

UWORD SqlStatement::NumBoundCols()
{
    return m_boundCols;
}


DataSource::DataSource() : m_hdbc(SQL_NULL_HDBC)
{    
}

void DataSource::Connect(const std::string& dsn, 
    const std::string& user, const std::string& pwd, 
    DWORD timeout)
{
    InitEnv();
    assert(g_henv != SQL_NULL_HENV);

    CheckStatus(::SQLAllocHandle(SQL_HANDLE_DBC, g_henv, 
        &m_hdbc));
    assert(m_hdbc != SQL_NULL_HDBC);

    if (timeout != INFINITE) {
        RETCODE ret = ::SQLSetConnectOption(m_hdbc, 
            SQL_LOGIN_TIMEOUT, timeout);
        if (!SQL_SUCCEEDED(ret))
            TraceError();
    }
    
    RETCODE ret = ::SQLConnect(m_hdbc, 
        (UCHAR*)dsn.c_str(),  SQL_NTS, 
        (UCHAR*)user.c_str(), SQL_NTS, 
        (UCHAR*)pwd.c_str(),  SQL_NTS);
    if (ret != SQL_SUCCESS) {
        if (ret != SQL_SUCCESS_WITH_INFO) {
            Exception ex = GetError();
            ::SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
            m_hdbc = SQL_NULL_HDBC;
            throw ex;
        } else {
            TraceError();
        }
    }
}

void DataSource::Disconnect()
{
    if (IsConnected()) {
        RETCODE ret1, ret2;
        ret1 = ::SQLDisconnect(m_hdbc);
        ret2 = ::SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
        CheckStatus(ret1);
        CheckStatus(ret2);
        m_hdbc = SQL_NULL_HDBC;
    }
}


Exception DataSource::GetError(HSTMT hstmt)
{
    /*SWORD*/  SQLSMALLINT nOutlen;
    /*UCHAR*/  SQLCHAR lpszMsg[SQL_MAX_MESSAGE_LENGTH];
    /*UCHAR*/  SQLCHAR lpszState[SQL_MAX_MESSAGE_LENGTH /*SQL_SQLSTATE_SIZE*/];
    /*SDWORD*/ SQLINTEGER lNative;

	lpszState[0] = '\0';
	lNative      = 0;
	lpszMsg[0]   = '\0';
	
/***
    ::SQLError(g_henv, m_hdbc, hstmt, lpszState, &lNative,
        lpszMsg, SQL_MAX_MESSAGE_LENGTH-1, &nOutlen);
***/
	::SQLGetDiagRec(
		SQL_HANDLE_STMT, 
		hstmt, 
		0, //RecNumber, 
		lpszState, 
		&lNative, 
		lpszMsg, 
		SQL_MAX_MESSAGE_LENGTH-1, 
		&nOutlen
		);

    // START:ansioem
#ifdef _CONSOLE
    ::AnsiToOem(
        (const char*)lpszMsg, 
        (char*)lpszMsg);
#endif
    // END:ansioem

	stringstream out;

#define IWANTTHIS2
#ifdef  IWANTTHIS2
    out << "'"   << (const char*)lpszState << "', "  << lNative 
        << ", '" << (const char*)lpszMsg   << "'."   << endl;
#endif

    return Exception(out.str(), (char*)lpszState, lNative, 
        (char*)lpszMsg);
}


void DataSource::TraceError(HSTMT hstmt)
{
    Exception ex = GetError(hstmt);
///    OutputDebugString(ex.what());
}

void DataSource::ThrowError(HSTMT hstmt)
{
    throw GetError(hstmt);
}


DataSource::~DataSource()
{
    Disconnect();
    FreeEnv();
}

bool DataSource::IsConnected() const
{
    return m_hdbc != SQL_NULL_HDBC;
}


void DataSource::InitEnv()
{
    if (InterlockedIncrement(&g_envCount) == 0) {
        ::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HENV, 
            &g_henv);
        ::SQLSetEnvAttr(g_henv, SQL_ATTR_ODBC_VERSION,       (SQLPOINTER)SQL_OV_ODBC2,          SQL_IS_INTEGER);
    }
}

void DataSource::FreeEnv()
{
    if (InterlockedDecrement(&g_envCount) == -1) {
        ::SQLFreeHandle(SQL_HANDLE_ENV, g_henv);
        g_henv = SQL_NULL_HENV;
    }
}


void DataSource::Commit()
{
	/*HRESULT*/ /*SQLRETURN*/ RETCODE ret = ::SQLTransact(g_henv, m_hdbc, SQL_COMMIT);
    CheckStatus(ret);
}

void SqlStatement::WriteResultColumns(ostream& out, 
    size_t columns, size_t dataLen, const string& sep)
{
   string line;
   vector<char> colName(dataLen+1);
   for (int i = 1; i <= (int)columns; i++) {
      SWORD  dataType;
      UDWORD columnSize;
	  SWORD  decimalDigits;
	  SWORD  nullable;
      SWORD  nameLen = (SWORD)colName.size();
      DescribeCol(i, (UCHAR*)&colName[0], (USHORT)colName.size(),
          &nameLen, &dataType, &columnSize, &decimalDigits, &nullable);
	  cerr << &colName[0] << '\t' << dataType << '\t' << columnSize << '\t' << decimalDigits << '\t' << nullable << endl;
      line += &colName[0] + sep;
   }
   if (!line.empty()) {
      line[line.length()-1]='\0'; // delete last tab
   }
   out << line << endl;
}

void SqlStatement::InitResultRows(size_t columns, 
    size_t dataLen, Row& row)
{
    row.resize(columns);
    for (int i = 0; i < (int)row.size(); i++) {
        row[i].length = 0;
        row[i].data.resize(dataLen);
        row[i].data[0] = '\0';
    }

    // bind column data to the result set
    for (int i = 0; i < (int)columns; i++) {
		SDWORD len = row[i].length;
        BindCol(&row[i].data[0], (SDWORD)dataLen, &len, SQL_C_CHAR);
    }
}


void SqlStatement::WriteResultRows(ostream& out, 
    size_t columns, Row& row, const string& null, 
    const string& sep)
{
    while (Next()) {
        string line;
        for (int i = 0; i < (int)columns; i++) {
            if (row[i].length == SQL_NULL_DATA) {
                line += null;
            } else {
                line += (char*)&row[i].data[0];
            }
            line += sep;
        }
        if (!line.empty()) {
            line[line.length()-1]='\0'; // delete last tab
        } else {
            break;
        }
        out << line << endl;
   }
}


void SqlStatement::WriteResultSet(ostream& out, 
    size_t dataLen, const string& null, const string& sep)
{
    // see how many columns we have in the result set
    // zero columns means nothing to show
    SWORD nCols = NumResultCols();
    if (nCols != 0) {
        WriteResultSet(out, nCols, dataLen, null, sep);
    }
}

void SqlStatement::WriteResultSet(ostream& out, size_t cols,
    size_t length, const string& null, const std::string& sep)
{
    WriteResultColumns(out, cols, length, sep);

    Row row;
    InitResultRows(cols, length, row);
    WriteResultRows(out, cols, row, null, sep);
}

bool DataSource::Add(const char* driver, const char* attr)
{
    return SQLConfigDataSource(NULL, ODBC_ADD_DSN, driver, 
        attr) != FALSE;
}

bool DataSource::Remove(const char* driver, const char* attr)
{
    return SQLConfigDataSource(NULL, ODBC_REMOVE_DSN, driver, 
        attr) != FALSE;
}

} // namespace odbc
