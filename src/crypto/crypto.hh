#ifndef TIBIA_CRYPTO_HH_
#define TIBIA_CRYPTO_HH_ 1

#include "../compat/compat.hh"

// Byte buffer reading/writing utilities
inline uint16 BufferRead16LE(const void *Buffer){
	const uint8 *P = (const uint8*)Buffer;
	return (uint16)(P[0] | (P[1] << 8));
}

inline uint32 BufferRead32LE(const void *Buffer){
	const uint8 *P = (const uint8*)Buffer;
	return (uint32)(P[0] | (P[1] << 8) | (P[2] << 16) | (P[3] << 24));
}

inline uint32 BufferRead32BE(const void *Buffer){
	const uint8 *P = (const uint8*)Buffer;
	return (uint32)((P[0] << 24) | (P[1] << 16) | (P[2] << 8) | P[3]);
}

inline void BufferWrite16LE(void *Buffer, uint16 Value){
	uint8 *P = (uint8*)Buffer;
	P[0] = (uint8)(Value & 0xFF);
	P[1] = (uint8)((Value >> 8) & 0xFF);
}

inline void BufferWrite32LE(void *Buffer, uint32 Value){
	uint8 *P = (uint8*)Buffer;
	P[0] = (uint8)(Value & 0xFF);
	P[1] = (uint8)((Value >> 8) & 0xFF);
	P[2] = (uint8)((Value >> 16) & 0xFF);
	P[3] = (uint8)((Value >> 24) & 0xFF);
}

inline void BufferWrite8(void *Buffer, uint8 Value){
	((uint8*)Buffer)[0] = Value;
}

inline void BufferWrite32BE(void *Buffer, uint32 Value){
	uint8 *P = (uint8*)Buffer;
	P[0] = (uint8)((Value >> 24) & 0xFF);
	P[1] = (uint8)((Value >> 16) & 0xFF);
	P[2] = (uint8)((Value >> 8) & 0xFF);
	P[3] = (uint8)(Value & 0xFF);
}

inline void BufferWrite64BE(void *Buffer, uint64 Value){
	uint8 *P = (uint8*)Buffer;
	P[0] = (uint8)((Value >> 56) & 0xFF);
	P[1] = (uint8)((Value >> 48) & 0xFF);
	P[2] = (uint8)((Value >> 40) & 0xFF);
	P[3] = (uint8)((Value >> 32) & 0xFF);
	P[4] = (uint8)((Value >> 24) & 0xFF);
	P[5] = (uint8)((Value >> 16) & 0xFF);
	P[6] = (uint8)((Value >> 8) & 0xFF);
	P[7] = (uint8)(Value & 0xFF);
}

inline void CryptoRandom(uint8 *Buffer, int Size){
	for(int i = 0; i < Size; i++){
		Buffer[i] = (uint8)(rand() & 0xFF);
	}
}

inline int ParseHexStringBuf(uint8 *Buffer, const char *Hex){
	if(!Hex || !Buffer) return -1;
	size_t len = strlen(Hex);
	if(len % 2 != 0) return -1;
	for(size_t i = 0; i < len / 2; i++){
		char byteString[3] = { Hex[i*2], Hex[i*2+1], 0 };
		Buffer[i] = (uint8)strtoul(byteString, nullptr, 16);
	}
	return (int)(len / 2);
}

// XTEA Functions
void XTEAEncrypt(const uint32 *Key, uint8 *Data, int Size);
void XTEADecrypt(const uint32 *Key, uint8 *Data, int Size);

struct TXTEASymmetricKey {
	TXTEASymmetricKey(void);
	void init(uint32 k0, uint32 k1, uint32 k2, uint32 k3);
	void init(const uint32 *Keys);
	void encrypt(uint8 *Data, int Size);
	void decrypt(uint8 *Data, int Size);
	void encrypt(uint8 *Data) { encryptBlock(Data); }
	void decrypt(uint8 *Data) { decryptBlock(Data); }
	void encryptBlock(uint8 *Data);
	void decryptBlock(uint8 *Data);

	uint32 m_SymmetricKey[4];
};

// RSA Key
struct TRSAPrivateKey {
	TRSAPrivateKey(void);
	~TRSAPrivateKey(void);

	bool initFromFile(const char *FileName);
	bool initFromPEMString(const char *PEM);
	bool decrypt(uint8 *Data, int Size = 128);

	void *m_RSAHandle;
};

typedef TRSAPrivateKey RSAKey;

RSAKey *RSALoadPEM(const char *FileName);
void RSAFree(RSAKey *Key);
bool RSADecrypt(RSAKey *Key, uint8 *Data, int Size = 128);

// SHA256 Functions
void SHA256Calculate(const void *Data, usize Size, uint8 *OutHash);
bool SHA256CalculateHex(const void *Data, usize Size, char *OutHex, usize OutHexCapacity);

#endif // TIBIA_CRYPTO_HH_
