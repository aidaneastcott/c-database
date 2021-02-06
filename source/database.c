
#include "extra.h"
#include "database.h"


// Disable MSVC specific compiler error
#ifdef _MSC_VER

#endif


// Conditional return macro for database handling functions
#define CONDITIONAL_RETURN(expr) \
do { \
	DBCode DBResult = (expr); \
	if (DBResult != DB_SUCCESS) { \
		return DBResult; \
	} \
} while(0)


// Macro for DBIndex conversion
#define htonDBIndex(expr)  htons(expr)
#define ntohDBIndex(expr)  ntohs(expr)

// Macro for DBCode conversion
#define htonDBCode(expr)  htons(expr)
#define ntohDBCode(expr)  ntohs(expr)


// Macros for easy access to type size information
#define DB_RECORD_SIZE  sizeof(DBRecord)
#define DB_CODE_SIZE    sizeof(DBCode)
#define DB_INDEX_SIZE   sizeof(DBIndex)


// Prototypes for DBDate conversion
struct DBDate htonDBDate(DBDate);
struct DBDate ntohDBDate(DBDate);


// Prototypes for server-side request handling
DBCode handleInsertRequest(FILE *, SOCKET, DBIndex *);
DBCode handleUpdateRequest(FILE *, SOCKET, DBIndex);
DBCode handleFindRequest(FILE *, SOCKET, DBIndex);
DBCode handleQueryRequest(FILE *, SOCKET, DBIndex);



/*	Name:           htonDBDate
	Description:    Converts a date struct from host to network byte order
	Parameters:     DBDate date:  The date to convert
	Returns:        DBDate:  The converted date
*/
struct DBDate htonDBDate(DBDate date) {
	date.year = htons(date.year);
	return date;
}


/*	Name:           ntohDBDate
	Description:    Converts a date struct from network to host byte order
	Parameters:     DBDate date:  The date to convert
	Returns:        DBDate:  The converted date
*/
struct DBDate ntohDBDate(DBDate date) {
	date.year = ntohs(date.year);
	return date;
}


/*	Name:           writeRecord
	Description:    Writes a database record to a file
	Parameters:     FILE *file:  The file to write the record to
	                DBRecord *record:  The record to write in the file
	Returns:        DBCode:  A return status code
*/
DBCode writeRecord(FILE *file, const DBRecord *record) {

	// Establish function preconditions
	assert_assume(record != NULL);
	assert_assume(file != NULL);

	// Temporary union for type-punning
	union {
		DBRecord record;
		char buffer[DB_RECORD_SIZE];
	} temp;

	// Copy the record into the temp variable
	temp.record = *record;

	// Convert integers to network byte order
	temp.record.memberId = htonDBIndex(temp.record.memberId);
	temp.record.birthDate = htonDBDate(temp.record.birthDate);

	// Write the temp variable buffer to the file
	if (fwrite(temp.buffer, sizeof(char), DB_RECORD_SIZE, file) != DB_RECORD_SIZE) {
		return DB_FILE_ERROR;
	}

	return DB_SUCCESS;
}


/*	Name:           readRecord
	Description:    Reads a database record from a file
	Parameters:     FILE *file:  The file to read the record from
	                DBRecord *record:  The record to read from the file
	Returns:        DBCode:  A return status code
*/
DBCode readRecord(FILE *file, DBRecord *record) {

	// Establish function preconditions
	assert_assume(record != NULL);
	assert_assume(file != NULL);

	// Temporary union for type-punning
	union {
		DBRecord record;
		char buffer[DB_RECORD_SIZE];
	} temp;

	// Read the file contents into the temp variable buffer
	if (fread(temp.buffer, sizeof(char), DB_RECORD_SIZE, file) != DB_RECORD_SIZE) {
		return DB_FILE_ERROR;
	}

	// Convert integers to host byte order
	temp.record.memberId = ntohDBIndex(temp.record.memberId);
	temp.record.birthDate = ntohDBDate(temp.record.birthDate);

	// Copy the temp variable into the record
	*record = temp.record;

	return DB_SUCCESS;
}


/*	Name:           sendRecord
	Description:    Sends a database record to a socket
	Parameters:     SOCKET socket:  The socket to send the record to
	                DBRecord *record:  The record to send to the socket
	                bool sendId:  Whether to send the memberId field
	Returns:        DBCode:  A return status code
*/
DBCode sendRecord(SOCKET socket, const DBRecord *record, bool sendId) {

	// Establish function preconditions
	assert_assume(record != NULL);
	assert_assume(socket != INVALID_SOCKET);

	// Temporary union for type-punning
	union {
		DBRecord record;
		char buffer[DB_RECORD_SIZE];
	} temp;

	// Copy the record into the temp variable
	temp.record = *record;

	// Convert integers to network byte order
	if (sendId) {
		temp.record.memberId = htonDBIndex(temp.record.memberId);
	}
	temp.record.birthDate = htonDBDate(temp.record.birthDate);

	int size = DB_RECORD_SIZE;
	char *address = temp.buffer;
	// Adjust the packet size and starting memory address appropriately
	if (!sendId) {
		address += DB_INDEX_SIZE;
		size -= DB_INDEX_SIZE;
	}

	// Send the temp variable buffer to the socket
	int result = send(socket, address, size, 0);
	if (result == SOCKET_ERROR) {
		return DB_SOCKET_ERROR;
	}
	else if (result != size) {
		return DB_SOCKET_MISMATCH;
	}

	return DB_SUCCESS;
}


/*	Name:           receiveRecord
	Description:    Receives a database record from a socket
	Parameters:     SOCKET socket:  The socket to receive the record from
	                DBRecord *record:  The record to receive from the socket
	                bool receiveId:  Whether to receive the memberId field
	Returns:        DBCode:  A return status code
*/
DBCode receiveRecord(SOCKET socket, DBRecord *record, bool receiveId) {

	// Establish function preconditions
	assert_assume(record != NULL);
	assert_assume(socket != INVALID_SOCKET);

	// Temporary union for type-punning
	union {
		DBRecord record;
		char buffer[DB_RECORD_SIZE];
	} temp;

	int size = DB_RECORD_SIZE;
	char *address = temp.buffer;
	// Adjust the packet size and starting memory address appropriately
	if (!receiveId) {
		address += DB_INDEX_SIZE;
		size -= DB_INDEX_SIZE;
	}

	// Receive the socket contents and store it in the temp variable
	int result = recv(socket, address, size, 0);
	if (result == SOCKET_ERROR) {
		return DB_SOCKET_ERROR;
	}
	else if (result != size) {
		return DB_SOCKET_MISMATCH;
	}

	// Convert integers to host byte order
	if (receiveId) {
		temp.record.memberId = ntohDBIndex(temp.record.memberId);
	}
	temp.record.birthDate = ntohDBDate(temp.record.birthDate);

	// Copy the temp variable into the record
	*record = temp.record;

	return DB_SUCCESS;
}


/*	Name:           sendCode
	Description:    Sends a database code to a socket
	Parameters:     SOCKET socket:  The socket to send the code to
	                DBCode code:  The code to send to the socket
	Returns:        DBCode:  A return status code
*/
DBCode sendCode(SOCKET socket, DBCode code) {

	// Establish function preconditions
	assert_assume(socket != INVALID_SOCKET);

	// Convert code to network byte order
	DBCode temp = htonDBCode(code);

	// Send the code to the socket
	int result = send(socket, (char *)&temp, DB_CODE_SIZE, 0);
	if (result == SOCKET_ERROR) {
		return DB_SOCKET_ERROR;
	}
	else if (result != DB_CODE_SIZE) {
		return DB_SOCKET_MISMATCH;
	}

	return DB_SUCCESS;
}


/*	Name:           receiveCode
	Description:    Receives a database code from a socket
	Parameters:     SOCKET socket:  The socket to receive the code from
	                DBCode *code:  The record to code from the socket
	Returns:        DBCode:  A return status code
*/
DBCode receiveCode(SOCKET socket, DBCode *code) {

	// Establish function preconditions
	assert_assume(code != NULL);
	assert_assume(socket != INVALID_SOCKET);

	DBCode temp;

	// Receive the code from the socket
	int result = recv(socket, (char *)&temp, DB_CODE_SIZE, 0);
	if (result == SOCKET_ERROR) {
		return DB_SOCKET_ERROR;
	}
	else if (result != DB_CODE_SIZE) {
		return DB_SOCKET_MISMATCH;
	}

	// Convert code to host byte order
	*code = ntohDBCode(temp);

	return DB_SUCCESS;
}


/*	Name:           sendIndex
	Description:    Sends a database code to a socket
	Parameters:     SOCKET socket:  The socket to send the code to
	                DBIndex code:  The code to send to the socket
	Returns:        DBCode:  A return status code
*/
DBIndex sendIndex(SOCKET socket, DBIndex index) {

	// Establish function preconditions
	assert_assume(socket != INVALID_SOCKET);

	// Convert code to network byte order
	DBIndex temp = htonDBIndex(index);

	// Send the code to the socket
	int result = send(socket, (char *)&temp, DB_INDEX_SIZE, 0);
	if (result == SOCKET_ERROR) {
		return DB_SOCKET_ERROR;
	}
	else if (result != DB_INDEX_SIZE) {
		return DB_SOCKET_MISMATCH;
	}

	return DB_SUCCESS;
}


/*	Name:           receiveIndex
	Description:    Receives a database code from a socket
	Parameters:     SOCKET socket:  The socket to receive the code from
	                DBIndex *code:  The record to code from the socket
	Returns:        DBCode:  A return status code
*/
DBIndex receiveIndex(SOCKET socket, DBIndex *index) {

	// Establish function preconditions
	assert_assume(index != NULL);
	assert_assume(socket != INVALID_SOCKET);

	DBIndex temp;

	// Receive the code from the socket
	int result = recv(socket, (char *)&temp, DB_INDEX_SIZE, 0);
	if (result == SOCKET_ERROR) {
		return DB_SOCKET_ERROR;
	}
	else if (result != DB_INDEX_SIZE) {
		return DB_SOCKET_MISMATCH;
	}

	// Convert code to host byte order
	*index = ntohDBIndex(temp);

	return DB_SUCCESS;
}


/*	Name:           handleInsertRequest
	Description:    Handles a database insert request from the server-side
	Parameters:     FILE *file:  The database file to handle the request with
	                SOCKET socket:  The database socket to handle the request with
	                DBIndex *entries:  The number of entries in the database
	Returns:        DBCode:  A return status code
*/
DBCode handleInsertRequest(FILE *file, SOCKET socket, DBIndex *entries) {

	// Establish function preconditions
	assert_assume(file != NULL);
	assert_assume(socket != INVALID_SOCKET);
	assert_assume(entries != NULL && 0 <= *entries && *entries <= DB_MAX_ENTRY);

	// Validate the database capacity
	if (*entries == DB_MAX_ENTRY) {
		return DB_REQUEST_DENIED;
	}

	// Send a confirmation code
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_SUCCESS));

	// Receive the record to insert
	DBRecord record;
	CONDITIONAL_RETURN(receiveRecord(socket, &record, false));

	// Seek to the end of the file
	if (fseek(file, 0, SEEK_END) != 0) {
		return DB_FILE_ERROR;
	}

	// Write the record to the file
	record.memberId = *entries + 1;
	CONDITIONAL_RETURN(writeRecord(file, &record));

	// Increment the database entry count
	++(*entries);

	// Send a confirmation code
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_SUCCESS));

	return DB_SUCCESS;
}


/*	Name:           handleUpdateRequest
	Description:    Handles a database update request from the server-side
	Parameters:     FILE *file:  The database file to handle the request with
	                SOCKET socket:  The database socket to handle the request with
	                DBIndex entries:  The number of entries in the database
	Returns:        DBCode:  A return status code
*/
DBCode handleUpdateRequest(FILE *file, SOCKET socket, DBIndex entries) {

	// Establish function preconditions
	assert_assume(file != NULL);
	assert_assume(socket != INVALID_SOCKET);
	assert_assume(0 <= entries && entries <= DB_MAX_ENTRY);

	// Send a confirmation code
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_SUCCESS));

	// Receive the record to update
	DBRecord record;
	CONDITIONAL_RETURN(receiveRecord(socket, &record, true));

	// Validate the record memberId
	if (record.memberId < DB_MIN_ENTRY) {
		return DB_REQUEST_DENIED;
	}
	if (record.memberId > entries) {
		return DB_REQUEST_DENIED;
	}

	// Seek the file to the correct position
	if (fseek(file, (record.memberId - DB_MIN_ENTRY) * DB_RECORD_SIZE, SEEK_SET) != 0) {
		return DB_FILE_ERROR;
	}

	// Write the record to the file
	CONDITIONAL_RETURN(writeRecord(file, &record));

	// Send a completion code
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_SUCCESS));

	return DB_SUCCESS;
}


/*	Name:           handleFindRequest
	Description:    Handles a database find request from the server-side
	Parameters:     FILE *file:  The database file to handle the request with
	                SOCKET socket:  The database socket to handle the request with
	                DBIndex entries:  The number of entries in the database
	Returns:        DBCode:  A return status code
*/
DBCode handleFindRequest(FILE *file, SOCKET socket, DBIndex entries) {

	// Establish function preconditions
	assert_assume(file != NULL);
	assert_assume(socket != INVALID_SOCKET);
	assert_assume(0 <= entries && entries <= DB_MAX_ENTRY);

	// Send a confirmation code
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_SUCCESS));

	DBIndex memberId;
	// Receive the memberId to find in the database
	int result = recv(socket, (char *)&memberId, DB_INDEX_SIZE, 0);
	if (result == SOCKET_ERROR) {
		return DB_SOCKET_ERROR;
	}
	if (result != DB_INDEX_SIZE) {
		return DB_SOCKET_MISMATCH;
	}

	// Convert the memberId to host byte order
	memberId = ntohDBIndex(memberId);

	// Validate the memberId
	if (memberId < DB_MIN_ENTRY) {
		return DB_REQUEST_DENIED;
	}
	if (memberId > entries) {
		return DB_REQUEST_DENIED;
	}

	// Seek the file to the correct position
	if (fseek(file, (memberId - DB_MIN_ENTRY) * DB_RECORD_SIZE, SEEK_SET) != 0) {
		return DB_FILE_ERROR;
	}

	// Read the record from the file
	DBRecord record;
	CONDITIONAL_RETURN(readRecord(file, &record));

	// Send a confirmation code
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_SUCCESS));

	// Send the record
	CONDITIONAL_RETURN(sendRecord(socket, &record, true));

	// Receive a completion code
	DBCode response;
	CONDITIONAL_RETURN(receiveCode(socket, &response));

	return DB_SUCCESS;
}


/*	Name:           handleQueryRequest
	Description:    Handles a database query request from the server-side
	Parameters:     FILE *file:  The database file to handle the request with
	                SOCKET socket:  The database socket to handle the request with
	                DBIndex entries:  The number of entries in the database
	Returns:        DBCode:  A return status code
*/
DBCode handleQueryRequest(FILE *file, SOCKET socket, DBIndex entries) {

	// Establish function preconditions
	assert_assume(file != NULL);
	assert_assume(socket != INVALID_SOCKET);
	assert_assume(0 <= entries && entries <= DB_MAX_ENTRY);

	// Send a confirmation code
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_SUCCESS));

	// Send the number of entries in the database
	CONDITIONAL_RETURN(sendIndex(socket, entries));

	// Receive a completion code
	DBCode response;
	CONDITIONAL_RETURN(receiveCode(socket, &response));

	return DB_SUCCESS;
}


/*	Name:           handleRequest
	Description:    Handles a database request from the server-side
	Parameters:     FILE *file:  The database file to handle the request with
	                SOCKET socket:  The database socket to handle the request with
	                DBCode command:  The request to handle
	                DBIndex *entries:  The number of entries in the database
	Returns:        DBCode:  A return status code
*/
DBCode handleRequest(FILE *file, SOCKET socket, DBCode command, DBIndex *entries) {

	// Establish function preconditions
	assert_assume(file != NULL);
	assert_assume(socket != INVALID_SOCKET);
	assert_assume(0 <= *entries && *entries <= DB_MAX_ENTRY);

	DBCode status;

	// Jump to the command to handle
	switch (command) {
	case DB_REQUEST_INSERT:
		status = handleInsertRequest(file, socket, entries);
		break;

	case DB_REQUEST_UPDATE:
		status = handleUpdateRequest(file, socket, *entries);
		break;

	case DB_REQUEST_FIND:
		status = handleFindRequest(file, socket, *entries);
		break;

	case DB_REQUEST_QUERY:
		status = handleQueryRequest(file, socket, *entries);
		break;

	default:
		// Do nothing if the command is not valid
		status = DB_REQUEST_DENIED;
		break;
	}

	// If there is an uncommunicated request fail state
	if (status != DB_SUCCESS
		&& status != DB_SOCKET_ERROR) {

		// Send the error code
		DBCode result = sendCode(socket, status);
		if (result != DB_SUCCESS) {
			status |= result;
		}
	}

	return status;
}


/*	Name:           sendInsertRequest
	Description:    Sends a database insert request from the client-side
	Parameters:     SOCKET socket:  The database socket to send the request with
	                DBRecord *record:  The record associated with the request
	Returns:        DBCode:  A return status code
*/
DBCode sendInsertRequest(SOCKET socket, const DBRecord *record) {

	// Establish function preconditions
	assert_assume(record != NULL);
	assert_assume(socket != INVALID_SOCKET);

	// Send the database command
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_INSERT));

	// Receive a confirmation code
	DBCode response;
	CONDITIONAL_RETURN(receiveCode(socket, &response));
	if (response != DB_REQUEST_SUCCESS) {
		return response;
	}

	// Send the record to insert in the database
	CONDITIONAL_RETURN(sendRecord(socket, record, false));

	// Receive a completion code
	CONDITIONAL_RETURN(receiveCode(socket, &response));

	return response;
}


/*	Name:           sendUpdateRequest
	Description:    Sends a database update request from the client-side
	Parameters:     SOCKET socket:  The database socket to send the request with
	                DBRecord *record:  The record associated with the request
	Returns:        DBCode:  A return status code
*/
DBCode sendUpdateRequest(SOCKET socket, const DBRecord *record) {

	// Establish function preconditions
	assert_assume(record != NULL);
	assert_assume(socket != INVALID_SOCKET);

	// Send the database command
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_UPDATE));

	// Receive a confirmation code
	DBCode response;
	CONDITIONAL_RETURN(receiveCode(socket, &response));
	if (response != DB_REQUEST_SUCCESS) {
		return response;
	}

	// Send the record to update in the database
	CONDITIONAL_RETURN(sendRecord(socket, record, true));

	// Receive a completion code
	CONDITIONAL_RETURN(receiveCode(socket, &response));

	return response;
}


/*	Name:           sendFindRequest
	Description:    Sends a database find request from the client-side
	Parameters:     SOCKET socket:  The database socket to send the request with
	                DBRecord *record:  The record associated with the request
	Returns:        DBCode:  A return status code
*/
DBCode sendFindRequest(SOCKET socket, DBRecord *record) {

	// Establish function preconditions
	assert_assume(record != NULL);
	assert_assume(socket != INVALID_SOCKET);

	// Send the database command
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_FIND));

	// Receive a confirmation code
	DBCode response;
	CONDITIONAL_RETURN(receiveCode(socket, &response));
	if (response != DB_SUCCESS) {
		return response;
	}

	DBIndex memberId = htonDBIndex(record->memberId);

	// Send the memberId to find in the database
	int result = send(socket, (char *)&memberId, DB_INDEX_SIZE, 0);
	if (result == SOCKET_ERROR || result != DB_INDEX_SIZE) {
		return DB_SOCKET_ERROR;
	}

	// Receive a confirmation code
	CONDITIONAL_RETURN(receiveCode(socket, &response));
	if (response != DB_REQUEST_SUCCESS) {
		return response;
	}

	// Receive the database record
	CONDITIONAL_RETURN(receiveRecord(socket, record, true));

	// Send a completion code
	CONDITIONAL_RETURN(sendCode(socket, DB_SUCCESS));

	return response;
}


/*	Name:           sendQueryRequest
	Description:    Sends a database query request from the client-side
	Parameters:     SOCKET socket:  The database socket to send the request with
	                DBRecord *record:  The record associated with the request
	Returns:        DBCode:  A return status code
*/
DBCode sendQueryRequest(SOCKET socket, DBIndex *entries) {

	// Establish function preconditions
	assert_assume(entries != NULL);
	assert_assume(socket != INVALID_SOCKET);

	// Send the database command
	CONDITIONAL_RETURN(sendCode(socket, DB_REQUEST_QUERY));

	// Receive a confirmation code
	DBCode response;
	CONDITIONAL_RETURN(receiveCode(socket, &response));
	if (response != DB_REQUEST_SUCCESS) {
		return response;
	}

	// Receive the number of entries in the database
	CONDITIONAL_RETURN(receiveIndex(socket, entries));

	// Send a completion code
	CONDITIONAL_RETURN(sendCode(socket, DB_SUCCESS));

	return response;
}


/*	Name:           readEntryCount
	Description:    Reads the number of entries in the database
	Parameters:     FILE *file:  The database file to read from
	                DBIndex *entries:  The number of entries in the database
	Returns:        DBCode:  A return status code
*/
DBCode readEntryCount(FILE *file, DBIndex *entries) {

	// Establish function preconditions
	assert_assume(file != NULL);
	assert_assume(entries != NULL);

	// Seek to the end of the file
	if (fseek(file, 0, SEEK_END) != 0) {
		return DB_FILE_ERROR;
	}
	// Get the file position offset
	long pos = ftell(file);
	if (pos % DB_RECORD_SIZE != 0) {
		return DB_FILE_ERROR;
	}

	// Calculate the number of entries in the file
	*entries = (DBIndex)(pos / DB_RECORD_SIZE);

	return DB_SUCCESS;
}
