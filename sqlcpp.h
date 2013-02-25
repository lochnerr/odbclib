/**/ 

class SQLEnvironment {
public:
	SQLEnvironment();
	~SQLEnvironment();
private:
	friend class SQLConnection;
	// hidden implementation
	struct SQLEnvironmentImpl;
	SQLEnvironmentImpl *imp;

    // prevent copying and assignment
    SQLEnvironment (const SQLEnvironment &);
    SQLEnvironment & operator= (const SQLEnvironment &);
};

class SQLConnection {
public:
	SQLConnection(SQLEnvironment &env) throw();
	~SQLConnection() throw();
	int connect(const char *Server, const char *UserName, const char *Authentication);
	int disconnect(void );
private:
	// hidden implementation
	struct SQLConnectionImpl;
	SQLConnectionImpl *imp;

	// prevent default constructor
	SQLConnection();
    // prevent copying and assignment
    SQLConnection (const SQLConnection &);
    SQLConnection & operator= (const SQLConnection &);
};