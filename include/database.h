/*
	File:           database.h
	Project:        PROG2111 - Assignment 1
	Programmer:     Aidan Eastcott
	Last Update:    2020-10-06
	Description:
		This file contains the prototypes and macros for the main database API
*/

#pragma once
#ifndef DATABASE_H
#define DATABASE_H


// Compiler guard for external C code in C++
#ifdef __cplusplus
extern "C" {
#endif


#include "socket_base.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


// The access mode to use with stdio file handles
#define DB_STDIO_FILE_MODE  "rb+"


// A type big enough to hold the maximum database entry
typedef uint16_t DBIndex;

// The minimum and maximum entry ID allowed in the database
#define DB_MIN_ENTRY  ((DBIndex)1)
#define DB_MAX_ENTRY  ((DBIndex)40'000)


// The type used to hold the database flag codes
typedef uint16_t DBCode;

// Database bit flags to communicate behavior and error codes
#define DB_SUCCESS          ((DBCode)0)

#define DB_FILE_ERROR       ((DBCode)0b0001)
#define DB_FILE_FORMAT      ((DBCode)0b0010)

#define DB_SOCKET_ERROR     ((DBCode)0b0100)
#define DB_SOCKET_MISMATCH  ((DBCode)0b1000)

#define DB_REQUEST_SUCCESS  DB_SUCCESS
#define DB_REQUEST_INSERT   ((DBCode)0b0001'0000'0000)
#define DB_REQUEST_UPDATE   ((DBCode)0b0010'0000'0000)
#define DB_REQUEST_FIND     ((DBCode)0b0100'0000'0000)
#define DB_REQUEST_QUERY    ((DBCode)0b1000'0000'0000)

#define DB_REQUEST_DENIED   ((DBCode)0b0001'0000'0000'0000)


// The size of the name fields in the Record struct
#define DB_RECORD_NAME_SIZE 29


typedef uint16_t DBDateYear;
typedef uint8_t DBDateMonth;
typedef uint8_t DBDateDay;


typedef enum Months {
	JAN = 1, FEB, MAR,
	APR, MAY, JUN,
	JUL, AUG, SEP,
	OCT, NOV, DEC
} Months;


// A struct to store date information
typedef struct DBDate {
	DBDateYear year;
	DBDateMonth day;
	DBDateDay month;
} DBDate;


// A struct to store Record information
typedef struct DBRecord {

	DBIndex memberId;

	char firstName[DB_RECORD_NAME_SIZE];
	char lastName[DB_RECORD_NAME_SIZE];

	struct DBDate birthDate;
} DBRecord;


// Prototypes for reading and writing records to files
DBCode writeRecord(FILE *, const DBRecord *);
DBCode readRecord(FILE *, DBRecord *);

// Prototypes for sending and receiving records to sockets
DBCode sendRecord(SOCKET, const DBRecord *, bool);
DBCode receiveRecord(SOCKET, DBRecord *, bool);

// Prototypes for sending and receiving codes to sockets
DBCode sendCode(SOCKET, DBCode);
DBCode receiveCode(SOCKET, DBCode *);

// Prototype for server-side request handling
DBCode handleRequest(FILE *, SOCKET, DBCode, DBIndex *);

// Prototypes for client-size request handling
DBCode sendInsertRequest(SOCKET, const DBRecord *);
DBCode sendUpdateRequest(SOCKET, const DBRecord *);
DBCode sendFindRequest(SOCKET, DBRecord *);
DBCode sendQueryRequest(SOCKET, DBIndex *);

// Prototype for reading the database size
DBCode readEntryCount(FILE *, DBIndex *);


#ifdef __cplusplus // extern "C"
}
#endif


#endif // DATABASE_H
