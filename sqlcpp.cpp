/**/ 

#include <iostream>
#include <cstring>

#include "sqlcpp.h"
#include "sqllib.h"

struct SQLEnvironment::SQLEnvironmentImpl
{
//SQLHENV Handle;
};

SQLEnvironment::SQLEnvironment() : imp(new SQLEnvironmentImpl)
{
//	allocEnv(&imp->Handle);
};

SQLEnvironment::~SQLEnvironment() 
{
//	freeEnv(imp->Handle);
	delete imp;
	std::cout << "SQLEnvironment destroyed" << std::endl;
};


struct SQLConnection::SQLConnectionImpl
{
//SQLHDBC Handle;
char ServerName[128];
char UserName[64];
char Authentication[64];
bool    isConnected;
};

SQLConnection::SQLConnection(SQLEnvironment &env) : imp(new SQLConnectionImpl)
{
//	allocConn(env.imp->Handle, &imp->Handle);
};

SQLConnection::~SQLConnection() 
{
	if (imp->isConnected) {
		std::cout << "Automatic disconnect" << std::endl;
		disconnect();
	}
		
//	freeConn(imp->Handle);
	delete imp;
	std::cout << "SQLConnection destroyed" << std::endl;
};

int SQLConnection::connect(const char *Server, const char *UserName, const char *Authentication)
{
//	SQLRETURN rc;

	strncpy((char *)imp->ServerName,     Server,         sizeof(imp->ServerName));
	strncpy((char *)imp->UserName,       UserName,       sizeof(imp->UserName));
	strncpy((char *)imp->Authentication, Authentication, sizeof(imp->Authentication));

//	rc = ::SQLLibConnect(imp->Handle, imp->ServerName, imp->UserName, imp->Authentication);

//	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
//		return 1;

	imp->isConnected = true;

	return 0;
};

int SQLConnection::disconnect(void )
{
//	SQLRETURN rc;

//	rc = ::SQLDisconnect(imp->Handle);

	imp->isConnected = false;

//	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
//		return 1;

	return 0;
};

