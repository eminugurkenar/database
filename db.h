#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "db_error.h"

#define MAX_key		4096	// maximum key size in bytes

//	types of handles/arenas

typedef enum {
	Hndl_newarena = 0,
	Hndl_catalog,
	Hndl_database,
	Hndl_docStore,
	Hndl_artIndex,
	Hndl_btree1Index,
	Hndl_btree2Index,
	Hndl_colIndex,
	Hndl_iterator,
	Hndl_cursor,
	Hndl_txn
} HandleType;

//	general object pointer

typedef union {
	struct {
		uint32_t offset;	// offset in the segment
		uint16_t segment;	// arena segment number
		union {
			struct {
				uint8_t type:6;		// object type
				uint8_t kill:1;		// kill entry
				uint8_t mutex:1;	// mutex bit
			};
			volatile char latch[1];
		};
		union {
			uint8_t nbyte;		// number of bytes in a span node
			uint8_t nslot;		// number of frame slots in use
			uint8_t maxidx;		// maximum slot index in use
			uint8_t ttype;		// index transaction type
			int8_t rbcmp;		// red/black comparison
		};
	};
	uint64_t bits;
	struct {
		uint64_t addr:48;		// segment/offset
		uint64_t coll:16;		// collection  number
	};
} DbAddr;

#define MUTEX_BIT  0x80
#define KILL_BIT   0x40
#define TYPE_BITS  0x3f

#define ADDR_MUTEX_SET 0x80000000000000ULL
#define ADDR_KILL_SET  0x40000000000000ULL

typedef union {
	struct {
		uint32_t index;		// record ID in the segment
		uint16_t seg:10;	// arena segment number
		uint16_t cmd:6;		// for use in txn
		uint16_t idx;		// document store arena idx
	};
	uint64_t bits;
} ObjId;

typedef struct DbArena_ DbArena;
typedef struct Handle_ Handle;
typedef struct DbMap_ DbMap;

//	param slots

typedef enum {
	Size = 0,		// total Params structure size
	OnDisk,			// Arena resides on disk
	InitSize,		// initial arena size
	UseTxn,			// transactions used
	NoDocs,			// no documents, just indexes
	DropDb,			// drop the database

	IdxKeySpec = 10,	// offset of key document
	IdxKeyUnique,
	IdxKeySparse,
	IdxKeyPartial,		// offset of partial document
	IdxBinary,			// treat string fields as binary
	IdxType,			// 0 for artree, 1 & 2 for btree

	Btree1Bits = 20,	// Btree1 set
	Btree1Xtra,

	CursorTxn = 25,
	CursorStart,
	CursorEnd,

	MaxParam = 30	// param array size
} ParamSlot;

typedef union {
	uint64_t intVal;
	uint32_t offset;
	double dblVal;
	bool boolVal;
} Params;

//	Param key docs and partial docs

typedef struct {
	uint32_t len;
	char val[];
} ParamVal;

typedef enum {
	DocUnused = 0,
	DocActive,
	DocInsert,
	DocDelete,
	DocDeleted
} DocState;

typedef struct {
	DbAddr verKeys[1];	// skiplist of versions with Id key
	uint64_t version;	// document version
	uint32_t offset;	// offset from origin
	uint32_t size;		// version size
	ObjId docId;		// document ObjId
	ObjId txnId;		// insert/version txn ID
} Ver;

typedef struct {
	uint32_t refCnt[1];	// active references to the document
	DbAddr prevDoc[1];	// previous versions of doc
	DbAddr addr;		// doc arena address
	ObjId delId;		// delete txn ID
	uint32_t verCnt;	// number of versions
	uint32_t size;		// total size
	DocState state;		// document state
	Ver ver[1];			// base version
} Doc;

// user's DbHandle
//	contains the HandleId ObjId bits

typedef struct {
	uint64_t hndlBits;
} DbHandle;

// cursor positioning operations

typedef enum {
	OpLeft	= 'l',
	OpRight = 'r',
	OpNext	= 'n',
	OpPrev	= 'p',
	OpFind	= 'f',
	OpOne	= 'o'
} CursorOp;

uint32_t get64(char *key, uint32_t len, uint64_t *result);
uint32_t store64(char *key, uint32_t keylen, uint64_t what);
void *getParamOff(Params *params, uint32_t off);
void *getParamIdx(Params *params, uint32_t idx);
