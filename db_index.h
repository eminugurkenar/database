#pragma once

//	Index data structure after DbArena object

typedef struct {
	uint64_t numEntries[1];	// number of keys in index
	char noDocs;			// no document ID's on keys
} DbIndex;

// database index cursor

typedef enum {
	CursorNone,
	CursorLeftEof,
	CursorRightEof,
	CursorPosAt
} PosState;

typedef struct {
	uint64_t ver;		// cursor doc version
	uint64_t ts;		// cursor timestamp
	ObjId txnId;		// cursor transaction
	ObjId docId;		// current doc ID
	void *key;
	Doc *doc;			// current document
	uint32_t keyLen;	// raw key length
	uint32_t userLen;	// user's key length
	char *minKey;	// minimum key value
	char *maxKey;	// maximum key value
	uint32_t minKeyLen;
	uint32_t maxKeyLen;
	PosState state:8;	// cursor position state enum
	char foundKey;		// cursor position found the key
	char useTxn;		// txn being used
	char noDocs;		// no document ID's on keys
} DbCursor;

typedef struct {
	SkipHead indexes[1];	// index handles by Id
	uint64_t childId;		// highest child installed
	uint32_t idxCnt;		// number of indexes
} DocStore;

#define dbindex(map) ((DbIndex *)(map->arena + 1))

DbStatus installIndexes(Handle *docHndl);
DbStatus installIndexKeys(Handle *docHndl, Doc *doc);

DbStatus dbFindKey(DbCursor *cursor, DbMap *map, char *key, uint32_t keyLen, bool onlyOne);
DbStatus dbNextKey(DbCursor *cursor, DbMap *map);
DbStatus dbPrevKey(DbCursor *cursor, DbMap *map);

DbStatus dbNextDoc(DbCursor *cursor, DbMap *map);
DbStatus dbPrevDoc(DbCursor *cursor, DbMap *map);
DbStatus dbRightKey(DbCursor *cursor, DbMap *map);
DbStatus dbLeftKey(DbCursor *cursor, DbMap *map);
DbStatus dbCloseCursor(DbCursor *cursor, DbMap *map);
