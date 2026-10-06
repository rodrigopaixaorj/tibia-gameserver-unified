#ifndef TIBIA_SCRIPT_HH_
#define TIBIA_SCRIPT_HH_ 1

#include "common.hh"
#include "enums.hh"

#define MAX_IDENT_LENGTH 30

struct TReadScriptFile {
	TReadScriptFile(void);
	~TReadScriptFile(void);
	void open(const char *FileName);
	void close(void);
	void error(const char *Text);
	void nextToken(void);
	char *getIdentifier(void);
	int getNumber(void);
	char *getString(void);
	uint8 *getBytesequence(void);
	void getCoordinate(int *x, int *y, int *z);
	char getSpecial(void);

	char *readIdentifier(void){
		this->nextToken();
		return this->getIdentifier();
	}

	int readNumber(void){
		this->nextToken();

		int Sign = 1;
		if(this->Token == SPECIAL && this->Special == '-'){
			Sign = -1;
			this->nextToken();
		}

		return Sign * this->getNumber();
	}

	char *readString(void){
		this->nextToken();
		return this->getString();
	}

	uint8 *readBytesequence(void){
		this->nextToken();
		return this->getBytesequence();
	}

	void readCoordinate(int *x, int *y, int *z){
		this->nextToken();
		this->getCoordinate(x, y, z);
	}

	char readSpecial(void){
		this->nextToken();
		return this->getSpecial();
	}

	void readSymbol(char Symbol){
		if(this->readSpecial() != Symbol){
			this->error("symbol mismatch");
		}
	}

	// DATA
	// =================
	TOKEN Token;
	FILE *File[3];
	char Filename[3][4096];
	int Line[3];
	char String[4000];
	int RecursionDepth;
	uint8 *Bytes;
	int Number;
	int CoordX;
	int CoordY;
	int CoordZ;
	char Special;
};

struct TWriteScriptFile {
	TWriteScriptFile(void);
	~TWriteScriptFile(void);
	void open(const char *FileName);
	void close(void);
	void error(const char *Text);
	void writeLn(void);
	void writeText(const char *Text);
	void writeNumber(int Number);
	void writeString(const char *Text);
	void writeCoordinate(int x ,int y ,int z);
	void writeBytesequence(const uint8 *Sequence, int Length);

	// DATA
	// =================
	FILE *File;
	char Filename[4096];
	int Line;
};

struct TReadBinaryFile: TReadStream {
	TReadBinaryFile(void);
	void open(const char *FileName);
	void close(void);
	void error(const char *Text);
	int getPosition(void);
	int getSize(void);
	void seek(int Offset);

	// VIRTUAL FUNCTIONS
	// =================
	uint8 readByte(void) override;
	void readBytes(uint8 *Buffer, int Count) override;
	bool eof(void) override;
	void skip(int Count) override;

	// TODO(fusion): Appended virtual functions. These are not in the base class
	// VTABLE which can be problematic if we intend to use polymorphism, although
	// that doesn't seem to be case.
	virtual ~TReadBinaryFile(void);														// VTABLE[8]
	// Duplicate destructor that also calls operator delete.							// VTABLE[9]

	// DATA
	// =================
	FILE *File;
	char Filename[4096];
	int FileSize;
};

struct TWriteBinaryFile: TWriteStream {
	TWriteBinaryFile(void);
	void open(const char *FileName);
	void close(void);
	void error(const char *Text);

	// VIRTUAL FUNCTIONS
	// =================
	void writeByte(uint8 Byte) override;
	void writeBytes(const uint8 *Buffer, int Count) override;

	// TODO(fusion): Appended virtual functions. These are not in the base class
	// VTABLE which can be problematic if we intend to use polymorphism, although
	// that doesn't seem to be case.
	virtual ~TWriteBinaryFile(void);

	// DATA
	// =================
	FILE *File;
	char Filename[4096];
};

#endif //TIBIA_SCRIPT_HH_
