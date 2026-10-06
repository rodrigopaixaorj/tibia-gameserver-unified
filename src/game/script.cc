#include "script.hh"

// NOTE(fusion): Throwing a character array on the stack as an exception won't
// work because it implicitly decays into a character pointer which would then
// point to invalid data when actually parsed. It is the reason `ErrorString` is
// declared statically, or else you'd need to throw some heap allocated string
// which then becomes ambiguous whether you should free it or not. Used in:
//	- TReadScriptFile::error
//	- TWriteScriptFile::error
//	- TReadBinaryFile::open
//	- TReadBinaryFile::error
//	- TWriteBinaryFile::open
//	- TWriteBinaryFile::error
static char ErrorString[100];

// Helper Functions
// =============================================================================
static void SaveFile(const char *Filename){
	if(Filename == NULL){
		error("SaveFile: Filename ist NULL.\n");
		return;
	}

	char BackupFilename[4096];

	strcpy(BackupFilename, Filename);
	strcat(BackupFilename, "#");

	FILE *Source = fopen(Filename, "rb");
	if(Source == NULL){
		error("SaveFile: Quelldatei %s existiert nicht.\n", Filename);
		return;
	}

	FILE *Dest = fopen(BackupFilename, "wb");
	if(Dest == NULL){
		error("SaveFile: Kann Zieldatei %s nicht anlegen.\n", BackupFilename);
		fclose(Source);
		return;
	}

	// NOTE(fusion): This function was in pretty bad shape after this point. The
	// reason is probably because GHIDRA didn't make sense of it using `alloca`
	// or some other mechanism to allocate a buffer on the stack and copy the whole
	// file in one go. It's probably not a good idea to mimic that here so we'll
	// do it gradually.

	char Buffer[KB(16)];
	while(true){
		usize n = fread(Buffer, 1, sizeof(Buffer), Source);
		if(n == 0){
			if(ferror(Source)){
				error("SaveFile: Fehler beim Lesen der Quelldatei %s.\n", Filename);
			}
			break;
		}

		if(fwrite(Buffer, 1, n, Dest) != n){
			error("SaveFile: Fehler beim Schreiben der Zieldatei %s.\n", BackupFilename);
			break;
		}
	}

	if(fclose(Source) != 0){
		error("SaveFile: Fehler %d beim Schließen der Quelldatei.\n", errno);
	}

	if(fclose(Dest) != 0){
		error("SaveFile: Fehler %d beim Schließen der Zieldatei.\n", errno);
	}
}

// TReadScriptFile
//==============================================================================
TReadScriptFile::TReadScriptFile(void){
	this->RecursionDepth = -1;
	this->Bytes = (uint8*)this->String;
}

TReadScriptFile::~TReadScriptFile(void){
	if(this->RecursionDepth != -1){
		::error("TReadScriptFile::~TReadScriptFile: Datei ist noch offen.\n");
		for(int Depth = this->RecursionDepth; Depth >= 0; Depth -= 1){
			// TODO(fusion): Probably `TReadScriptFile::close` inlined?
			if(fclose(this->File[Depth]) != 0){
				::error("TReadScriptFile::close: Fehler %d beim Schließen der Datei.\n", errno);
			}
		}
		this->RecursionDepth = -1;
	}
}

void TReadScriptFile::open(const char *FileName){
	int Depth = this->RecursionDepth + 1;
	if(Depth >= NARRAY(this->File)){
		::error("TReadScriptFile::open: Rekursionstiefe zu groß.\n");
		throw "Recursion depth too high";
	}

	ASSERT(Depth >= 0);

	// TODO(fusion): More `strcpy`s...
	if(Depth > 0 && FileName[0] != '/'){
		strcpy(this->Filename[Depth], this->Filename[Depth - 1]);
		if(char *Slash = findLast(this->Filename[Depth], '/')){
			strcpy(Slash + 1, FileName);
		}else{
			strcpy(this->Filename[Depth], FileName);
		}
	}else{
		strcpy(this->Filename[Depth], FileName);
	}

	this->File[Depth] = fopen(this->Filename[Depth], "rb");
	if(this->File[Depth] == NULL){
		int ErrCode = errno;
		::error("TReadScriptFile::open: Kann Datei %s nicht öffnen.\n", this->Filename[Depth]);
		::error("Fehler %d: %s.\n", ErrCode, GetErrorDescription(ErrCode));
		throw "Cannot open script-file";
	}

	this->Line[Depth] = 1;
	this->RecursionDepth = Depth;
}

void TReadScriptFile::close(void){
	int Depth = this->RecursionDepth;
	if(Depth <= -1){
		::error("TReadScriptFile::close: Keine Datei offen.\n");
		return;
	}

	ASSERT(Depth < NARRAY(this->File));
	if(fclose(this->File[Depth]) != 0){
		::error("TReadScriptFile::close: Fehler %d beim Schließen der Datei.\n", errno);
	}
	this->RecursionDepth -= 1;
}

void TReadScriptFile::error(const char *Text){
	int Depth = this->RecursionDepth;
	ASSERT(Depth >= 0 && Depth < NARRAY(this->File));

	const char *Filename = this->Filename[Depth];
	if(const char *Slash = findLast(this->Filename[Depth], '/')){
		Filename = Slash + 1;
	}

	snprintf(ErrorString, sizeof(ErrorString),
			"error in script-file \"%s\", line %d: %s",
			Filename, this->Line[Depth], Text);

	// TODO(fusion): Reset? Also seems like `TReadScriptFile::close` was inlined.
	for(; Depth >= 0; Depth -= 1){
		if(fclose(this->File[Depth]) != 0){
			::error("TReadScriptFile::close: Fehler %d beim Schließen der Datei.\n", errno);
		}
	}
	this->RecursionDepth = -1;

	throw ErrorString;
}

// NOTE(fusion): This function in particular was in pretty bad shape. It should
// be one of the few that doesn't roughly match the original version. Nevertheless,
// I think we should properly split this into a lexer and parser. We find ourselves
// parsing strings and numbers multiple times here, whereas a simple lexer and
// parser would simplify the work tremendously.
// TODO(fusion): This needs to be tested to make sure it behaves like the original.
void TReadScriptFile::nextToken(void){
	if(this->RecursionDepth == -1){
		::error("TReadScriptFile::nextToken: Kein Skript zum Lesen geöffnet.\n");
		this->Token = ENDOFFILE;
		return;
	}

	// NOTE(fusion): Reset any previous token state.
	memset(this->String, 0, sizeof(this->String));
	this->Number = 0;
	this->CoordX = 0;
	this->CoordY = 0;
	this->CoordZ = 0;
	this->Special = 0;

	// NOTE(fusion): `TReadScriptFile::error` will format and throw an error
	// string that must be handled up the call stack. It must be considered
	// an exit point for parsing errors in this function.

	while(true){
		int Depth = this->RecursionDepth;
		ASSERT(Depth >= 0 && Depth < NARRAY(this->File));
		FILE *File = this->File[Depth];

		int c;
		do{
			c = getc(File);
			if(c == '\n'){
				this->Line[Depth] += 1;
			}
		}while(isSpace(c));

		if(c == EOF){
			if(Depth == 0){
				this->Token = ENDOFFILE;
				return;
			}

			this->close();
			continue;
		}

		switch(c){
			case '#':{ // COMMENT
				while(true){
					int next = getc(File);
					if(next == '\n' || next == EOF){
						if(next == '\n'){
							this->Line[Depth] += 1;
						}
						break;
					}
				}
				break;
			}

			case '@':{ // INCLUDE
				int next = getc(File);
				if(next == EOF){
					this->error("unexpected end of file");
				}else if(next != '"'){
					this->error("syntax error");
				}

				int StringLength = 0;
				while(true){
					// TODO(fusion): I don't think new lines are even allowed in
					// file names but we should keep track of the line number even
					// if they happen here.
					next = getc(File);
					if(next == EOF){
						this->error("unexpected end of file");
					}else if(next == '"'){
						break;
					}

					if(StringLength >= (NARRAY(this->String) - 1)){
						this->error("string too long");
					}
					this->String[StringLength] = (char)next;
					StringLength += 1;
				}

				// TODO(fusion): Maybe check if the path is empty?
				this->open(this->String);

				// NOTE(fusion): This is the only place we parse a string without
				// returning it as a token. We need to reset it to make sure any
				// subsequent string-like token will be properly parsed.
				memset(this->String, 0, sizeof(this->String));

				break;
			}

			case '"':{ // STRING
				int StringLength = 0;
				while(true){
					int next = getc(File);
					if(next == EOF){
						this->error("unexpected end of file");
					}else if(next == '\\'){
						next = getc(File);
						if(next == EOF){
							this->error("unexpected end of file");
						}else if(next == 'n'){
							next = '\n';
						}
					}else if(next == '\n'){
						// TODO(fusion): Shouldn't we prevent non-escaped new
						// lines inside strings?
						this->Line[Depth] += 1;
					}else if(next == '"'){
						break;
					}

					if(StringLength >= (NARRAY(this->String) - 1)){
						this->error("string too long");
					}
					this->String[StringLength] = (char)next;
					StringLength += 1;
				}
				this->Token = STRING;
				return;
			}

			case '[':{ // COORDINATE
				// NOTE(fusion): X-Coordinate or SPECIAL '['.
				int Sign = -1;
				int Coord = 0;
				int next = getc(File);
				if(isDigit(next)){
					Sign = 1;
					Coord = next - '0';
				}else if(next != '-'){
					this->Token = SPECIAL;
					this->Special = '[';
					if(next != EOF){
						ungetc(next, File);
					}
					return;
				}

				while(true){
					next = getc(File);
					if(next == EOF){
						this->error("unexpected end of file");
					}else if(isDigit(next)){
						Coord *= 10;
						Coord += next - '0';
					}else if(next == ','){
						break;
					}else{
						this->error("syntax error");
					}
				}

				this->CoordX = Sign * Coord;

				// NOTE(fusion): Y-Coordinate.
				Sign = -1;
				Coord = 0;
				next = getc(File);
				if(isDigit(next)){
					Sign = 1;
					Coord = next - '0';
				}else if(next != '-'){
					this->error("syntax error");
				}

				while(true){
					next = getc(File);
					if(next == EOF){
						this->error("unexpected end of file");
					}else if(isDigit(next)){
						Coord *= 10;
						Coord += next - '0';
					}else if(next == ','){
						break;
					}else{
						this->error("syntax error");
					}
				}

				this->CoordY = Sign * Coord;

				// NOTE(fusion): Z-Coordinate.
				Sign = -1;
				Coord = 0;
				next = getc(File);
				if(isDigit(next)){
					Sign = 1;
					Coord = next - '0';
				}else if(next != '-'){
					this->error("syntax error");
				}

				while(true){
					next = getc(File);
					if(next == EOF){
						this->error("unexpected end of file");
					}else if(isDigit(next)){
						Coord *= 10;
						Coord += next - '0';
					}else if(next == ']'){
						break;
					}else{
						this->error("syntax error");
					}
				}

				this->CoordZ = Sign * Coord;
				this->Token = COORDINATE;
				return;
			}

			case '<':{
				int next = getc(File);
				if(next == '='){
					this->Special = 'L';
				}else if(next == '>'){
					this->Special = 'N';
				}else{
					this->Special = '<';
					if(next != EOF){
						ungetc(next, File);
					}
				}
				this->Token = SPECIAL;
				return;
			}

			case '>':{
				int next = getc(File);
				if(next == '='){
					this->Special = 'G';
				}else{
					this->Special = '>';
					if(next != EOF){
						ungetc(next, File);
					}
				}
				this->Token = SPECIAL;
				return;
			}

			case '-':{
				int next = getc(File);
				if(next == '>'){
					this->Special = 'I';
				}else{
					this->Special = '-';
					if(next != EOF){
						ungetc(next, File);
					}
				}
				this->Token = SPECIAL;
				return;
			}

			default:{
				if(isAlpha(c)){
					int IdentLength = 1;
					this->String[0] = (char)c;
					while(true){
						int next = getc(File);
						if(isAlpha(next) || isDigit(next) || next == '_'){
							if(IdentLength >= (MAX_IDENT_LENGTH - 1)){
								this->error("identifier too long");
							}
							this->String[IdentLength] = (char)next;
							IdentLength += 1;
						}else{
							if(next != EOF){
								ungetc(next, File);
							}
							break;
						}
					}
					this->Token = IDENTIFIER;
				}else if(isDigit(c)){
					// NOTE(fusion): `this->Bytes` points to `this->String` (set
					// in the constructor) which is why we check against the size
					// of the `this->String` array. It doesn't seem to store the
					// number of bytes but I've only seen it used with fixed size
					// arrays like outfit colors or sector offsets.
					//	Also, it doesn't make sense to have a null terminator in
					// a byte string so the capacity check doesn't include it as
					// in the case of regular strings.

					// TODO(fusion): We're not checking for integer overflow at all.
					int BytesLength = 0;
					int Number = c - '0';
					while(true){
						int next = getc(File);
						if(isDigit(next)){
							Number *= 10;
							Number += next - '0';
						}else if(next == '-'){
							if(BytesLength >= NARRAY(this->String)){
								this->error("too many bytes");
							}
							this->Bytes[BytesLength] = (uint8)Number;
							BytesLength += 1;

							// NOTE(fusion): If there is a '-' after a number, there
							// better be a second number.
							next = getc(File);
							if(next == EOF){
								this->error("unexpected end of file");
							}else if(!isDigit(next)){
								this->error("syntax error");
							}
							Number = next - '0';
						}else{
							if(next != EOF){
								ungetc(next, File);
							}

							if(BytesLength <= 0){
								this->Token = NUMBER;
								this->Number = Number;
							}else{
								if(BytesLength >= NARRAY(this->String)){
									this->error("too many bytes");
								}
								this->Token = BYTES;
								this->Bytes[BytesLength] = (uint8)Number;
								BytesLength += 1;
							}
							break;
						}
					}
				}else{
					this->Token = SPECIAL;
					this->Special = (char)c;
				}
				return;
			}
		}
	}
}

char *TReadScriptFile::getIdentifier(void){
	if(this->Token != IDENTIFIER){
		this->error("identifier expected");
	}
	strLower(this->String);
	return this->String;
}

int TReadScriptFile::getNumber(void){
	if(this->Token != NUMBER){
		this->error("number expected");
	}
	return this->Number;
}

char *TReadScriptFile::getString(void){
	if(this->Token != STRING){
		this->error("string expected");
	}
	return this->String;
}

uint8 *TReadScriptFile::getBytesequence(void){
	if(this->Token != BYTES){
		this->error("byte-sequence expected");
	}
	return this->Bytes;
}

void TReadScriptFile::getCoordinate(int *x, int *y, int *z){
	if(this->Token != COORDINATE){
		this->error("coordinates expected");
	}
	*x = this->CoordX;
	*y = this->CoordY;
	*z = this->CoordZ;
}

char TReadScriptFile::getSpecial(void){
	if(this->Token != SPECIAL){
		this->error("special-char expected");
	}
	return this->Special;
}

// TWriteScriptFile
//==============================================================================
TWriteScriptFile::TWriteScriptFile(void){
	this->File = NULL;
}

TWriteScriptFile::~TWriteScriptFile(void){
	if(this->File != NULL){
		::error("TWriteScriptFile::~TWriteScriptFile: Datei %s ist noch offen.\n", this->Filename);
		if(fclose(this->File) != 0){
			::error("TWriteScriptFile::~TWriteScriptFile: Fehler %d beim Schließen der Datei.\n", errno);
		}
	}
}

void TWriteScriptFile::open(const char *FileName){
	if(this->File != NULL){
		::error("TWriteScriptFile::open: Altes Skript ist noch offen.\n");
		if(fclose(this->File) != 0){
			::error("TWriteScriptFile::open: Fehler %d beim Schließen der Datei.\n", errno);
		}
		this->File = NULL;
	}

	this->File = fopen(FileName, "wb");
	if(this->File == NULL){
		int ErrCode = errno;
		::error("TWriteScriptFile: Kann Datei %s nicht anlegen.\n", FileName);
		::error("Fehler %d: %s.\n", ErrCode, GetErrorDescription(ErrCode));
		throw "Cannot create script-file";
	}

	strcpy(this->Filename, FileName);
	this->Line = 0;
}

void TWriteScriptFile::close(void){
	if(this->File == NULL){
		::error("TWriteScriptFile::close: Kein Skript offen.\n");
		return;
	}

	if(fclose(this->File) != 0){
		::error("TWriteScriptFile::close: Fehler %d beim Schließen der Datei.\n", errno);
	}
	this->File = NULL;
}

void TWriteScriptFile::error(const char *Text){
	if(this->File != NULL){
		if(fclose(this->File) != 0){
			::error("TWriteScriptFile::error: Fehler %d beim Schließen der Datei.\n", errno);
		}
		this->File = NULL;
	}

	snprintf(ErrorString, sizeof(ErrorString),
			"error in script-file \"%s\", line %d: %s",
			this->Filename, this->Line, Text);

	throw ErrorString;
}

void TWriteScriptFile::writeLn(void){
	if(this->File == NULL){
		::error("TWriteScriptFile::writeLn: Kein Skript zum Schreiben geöffnet.\n");
		throw "Cannot write linefeed";
	}

	putc('\n', this->File);
}

void TWriteScriptFile::writeText(const char *Text){
	if(this->File == NULL){
		::error("TWriteScriptFile::writeText: Kein Skript zum Schreiben geöffnet.\n");
		throw "Cannot write text";
	}

	if(Text == NULL){
		::error("TWriteScriptFile::writeText: Text ist NULL.\n");
		throw "Cannot write text";
	}

	for(int i = 0; Text[i] != 0; i += 1){
		putc(Text[i], this->File);
	}
}

void TWriteScriptFile::writeNumber(int Number){
	if(this->File == NULL){
		::error("TWriteScriptFile::writeNumber: Kein Skript zum Schreiben geöffnet.\n");
		throw "Cannot write number";
	}

	char s[32];
	snprintf(s, sizeof(s), "%d", Number);
	this->writeText(s);
}

void TWriteScriptFile::writeString(const char *Text){
	if(this->File == NULL){
		::error("TWriteScriptFile::writeString: Kein Skript zum Schreiben geöffnet.\n");
		throw "Cannot write string";
	}

	if(Text == NULL){
		::error("TWriteScriptFile::writeString: Text ist NULL.\n");
		throw "Cannot write string";
	}

	putc('"', this->File);
	for(int i = 0; Text[i] != 0; i += 1){
		if(Text[i] == '\"' || Text[i] == '\\'){
			putc('\\', this->File);
			putc(Text[i], this->File);
		}else if(Text[i] == '\n'){
			putc('\\', this->File);
			putc('n', this->File);
		}else{
			putc(Text[i], this->File);
		}
	}
	putc('"', this->File);
}

void TWriteScriptFile::writeCoordinate(int x ,int y ,int z){
	if(this->File == NULL){
		::error("TWriteScriptFile::writeCoordinate: Kein Skript zum Schreiben geöffnet.\n");
		throw "Cannot write coordinate";
	}

	// TODO(fusion): This is weird because we support loading negative coordinates values.
	if(x < 0 || y < 0 || z < 0){
		::error("TWriteScriptFile::writeCoordinate: Ungültige Koordinaten [%d,%d,%d].\n", x, y, z);
		throw "Invalid coordinates";
	}

	char s[64];
	snprintf(s, sizeof(s), "[%u,%u,%u]", x, y, z);
	this->writeText(s);
}

void TWriteScriptFile::writeBytesequence(const uint8 *Sequence, int Length){
	if(this->File == NULL){
		::error("TWriteScriptFile::writeBytesequence: Kein Skript zum Schreiben geöffnet.\n");
		throw "Cannot write bytesequence";
	}

	if(Sequence == NULL){
		::error("TWriteScriptFile::writeBytesequence: Sequence ist NULL.\n");
		throw "Cannot write bytesequence";
	}

	if(Length <= 0){
		::error("TWriteScriptFile::writeBytesequence: Ungültige Sequenzlänge.\n");
		throw "Cannot write bytesequence";
	}

	for(int i = 0; i < Length; i += 1){
		if(i > 0){
			putc('-', this->File);
		}

		char s[32];
		snprintf(s, sizeof(s), "%u", Sequence[i]);
		this->writeText(s);
	}
}

// TReadBinaryFile REGULAR FUNCTIONS
//==============================================================================
TReadBinaryFile::TReadBinaryFile(void){
	this->File = NULL;
}

void TReadBinaryFile::open(const char *FileName){
	if(this->File != NULL){
		this->error("File still open");
	}

	this->File = fopen(FileName, "rb");
	if(this->File == NULL){
		snprintf(ErrorString, sizeof(ErrorString),
				"Cannot open file %s", FileName);
		throw ErrorString;
	}

	strcpy(this->Filename, FileName);
	this->FileSize = -1;
}

void TReadBinaryFile::close(void){
	// TODO(fusion): Check if file is NULL?
	if(fclose(this->File) != 0){
		int ErrCode = errno;
		::error("TReadBinaryFile::close: Fehler beim Schließen der Datei.\n");
		::error("# Datei: %s, Fehlercode: %d (%s)\n",
				this->Filename, ErrCode, GetErrorDescription(ErrCode));
	}
	this->File = NULL;
}

void TReadBinaryFile::error(const char *Text){
	if(this->File != NULL){
		if(fclose(this->File) != 0){
			::error("TReadBinaryFile::error: Fehler %d beim Schließen der Datei.\n", errno);
		}
		this->File = NULL;
	}

	snprintf(ErrorString, sizeof(ErrorString),
			"error in binary-file \"%s\": %s.",
			this->Filename, Text);

	throw ErrorString;
}

int TReadBinaryFile::getPosition(void){
	// TODO(fusion): Check if file is NULL?
	return (int)ftell(this->File);
}

int TReadBinaryFile::getSize(void){
	// TODO(fusion): Check if file is NULL?
	int Size = this->FileSize;
	if(Size == -1){
		long Position = ftell(this->File);
		fseek(this->File, 0, SEEK_END);
		Size = (int)ftell(this->File);
		fseek(this->File, Position, SEEK_SET);
		this->FileSize = Size;
	}
	return Size;
}

void TReadBinaryFile::seek(int Offset){
	if(this->File == NULL){
		this->error("File not open for seek");
	}

	if(Offset < 0){
		this->error("Negative offset for seek");
	}

	fseek(this->File, (long)Offset, 0);
}

// TReadBinaryFile VIRTUAL FUNCTIONS
//==============================================================================
uint8 TReadBinaryFile::readByte(void){
	uint8 Byte;
	int Result = (int)fread(&Byte, 1, 1, this->File);
	if(Result != 1){
		int ErrCode = errno;
		int Position = this->getPosition();
		::error("TReadBinaryFile::readByte: Fehler beim Lesen eines Bytes\n");
		::error("# Datei: %s, Position: %d, Rückgabewert: %d, Fehlercode: %d (%s)\n",
				this->Filename, Position, Result, ErrCode, GetErrorDescription(ErrCode));

		// NOTE(fusion): Close file and make a backup, possibly for further inspection.
		if(fclose(this->File) != 0){
			::error("TReadBinaryFile::readByte: Fehler %d beim Schließen der Datei.\n", errno);
		}
		this->File = NULL;
		SaveFile(this->Filename);

		this->error("Error while reading byte");
	}
	return Byte;
}

void TReadBinaryFile::readBytes(uint8 *Buffer, int Count){
	int Result = (int)fread(Buffer, 1, Count, this->File);
	if(Result != Count){
		int ErrCode = errno;
		int Position = this->getPosition();
		::error("TReadBinaryFile::readBytes: Fehler beim Lesen von %d Bytes\n", Count);
		::error("# Datei: %s, Position %d, Rückgabewert: %d, Fehlercode: %d (%s)\n",
				this->Filename, Position, Result, ErrCode, GetErrorDescription(ErrCode));

		// NOTE(fusion): Close file and make a backup, possibly for further inspection.
		if(fclose(this->File) != 0){
			::error("TReadBinaryFile::readBytes: Fehler %d beim Schließen der Datei.\n", errno);
		}
		this->File = NULL;
		SaveFile(this->Filename);

		this->error("Error while reading bytes");
	}
}

bool TReadBinaryFile::eof(void){
	if(this->File == NULL){
		this->error("File not open for eof check");
	}

	return this->getSize() <= this->getPosition();
}

void TReadBinaryFile::skip(int Count){
	if(this->File == NULL){
		this->error("File not open for skip");
	}

	this->seek(this->getPosition() + Count);
}

TReadBinaryFile::~TReadBinaryFile(void){
	if(this->File != NULL){
		::error("TReadBinaryFile::~TReadBinaryFile: Datei %s ist noch offen.\n", this->Filename);
		if(fclose(this->File) != 0){
			::error("TReadBinaryFile::~TReadBinaryFile: Fehler %d beim Schließen der Datei.\n", errno);
		}
	}
}

// TWriteBinaryFile
//==============================================================================
TWriteBinaryFile::TWriteBinaryFile(void){
	this->File = NULL;
}

void TWriteBinaryFile::open(const char *FileName){
	if(this->File != NULL){
		this->error("File still open");
	}

	this->File = fopen(FileName, "wb");
	if(this->File == NULL){
		int ErrCode = errno;
		::error("TWriteBinaryFile::open: Kann Datei %s nicht anlegen.\n", FileName);
		::error("Fehler %d: %s.\n", ErrCode, GetErrorDescription(ErrCode));

		snprintf(ErrorString, sizeof(ErrorString),
				"Cannot create file %s.", FileName);

		throw ErrorString;
	}

	strcpy(this->Filename, FileName);
}

void TWriteBinaryFile::close(void){
	// TODO(fusion): Check if file is NULL?
	if(fclose(this->File) != 0){
		int ErrCode = errno;
		::error("TWriteBinaryFile::close: Fehler beim Schließen der Datei.\n");
		::error("# Datei: %s, Fehlercode: %d (%s)\n",
				this->Filename, ErrCode, GetErrorDescription(ErrCode));
	}
	this->File = NULL;
}

void TWriteBinaryFile::error(const char *Text){
	if(this->File != NULL){
		if(fclose(this->File) != 0){
			::error("TWriteBinaryFile::error: Fehler %d beim Schließen der Datei.\n", errno);
		}
		this->File = NULL;
	}

	snprintf(ErrorString, sizeof(ErrorString),
			"error in binary-file \"%s\": %s.",
			this->Filename, Text);

	throw ErrorString;
}

void TWriteBinaryFile::writeByte(uint8 Byte){
	int Result = (int)fwrite(&Byte, 1, 1, this->File);
	if(Result != 1){
		int ErrCode = errno;
		::error("TWriteBinaryFile::writeByte: Fehler beim Schreiben eines Bytes\n");
		::error("# Datei: %s, Rückgabewert: %d, Fehlercode: %d (%s)\n",
				this->Filename, Result, ErrCode, GetErrorDescription(ErrCode));

		// NOTE(fusion): Close file and make a backup, possibly for further inspection.
		if(fclose(this->File) != 0){
			::error("TWriteBinaryFile::writeByte: Fehler %d beim Schließen der Datei.\n", errno);
		}
		this->File = NULL;
		SaveFile(this->Filename);

		this->error("Error while writing byte");
	}
}

void TWriteBinaryFile::writeBytes(const uint8 *Buffer, int Count){
	int Result = (int)fwrite(Buffer, 1, Count, this->File);
	if(Result != Count){
		int ErrCode = errno;
		::error("TWriteBinaryFile::writeBytes: Fehler beim Schreiben von %d Bytes\n", Count);
		::error("# Datei: %s, Rückgabewert: %d, Fehlercode: %d (%s)\n",
				this->Filename, Result, ErrCode, GetErrorDescription(ErrCode));

		// NOTE(fusion): Close file and make a backup, possibly for further inspection.
		if(fclose(this->File) != 0){
			::error("TWriteBinaryFile::writeBytes: Fehler %d beim Schließen der Datei.\n", errno);
		}
		this->File = NULL;
		SaveFile(this->Filename);

		this->error("Error while writing bytes");
	}
}

TWriteBinaryFile::~TWriteBinaryFile(void){
	if(this->File != NULL){
		::error("TWriteBinaryFile::~TWriteBinaryFile: Datei %s ist noch offen.\n", this->Filename);
		if(fclose(this->File) != 0){
			::error("TWriteBinaryFile::~TWriteBinaryFile: Fehler %d beim Schließen der Datei.\n", errno);
		}
	}
}
