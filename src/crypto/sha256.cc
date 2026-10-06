#include "crypto.hh"

static const uint32 SHA256IV[8] = {
	0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
	0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19,
};

static const uint32 SHA256K[64] = {
	0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
	0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
	0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
	0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
	0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
	0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
	0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
	0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
	0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
	0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
	0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
	0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
	0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
	0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
	0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
	0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,
};

static uint32 RotR32(uint32 Value, int N){
	ASSERT(N >= 0 && N < 32);
	return (Value >> N) | (Value << (32 - N));
}

static void SHA256Compress(uint32 *H, const uint8 *Block){
	uint32 W[64];

	for(int i = 0; i < 16; i += 1){
		W[i] = BufferRead32BE(&Block[i * 4]);
	}

	for(int i = 16; i < 64; i += 1){
		uint32 S0 = RotR32(W[i - 15],  7) ^ RotR32(W[i - 15], 18) ^ (W[i - 15] >>  3);
		uint32 S1 = RotR32(W[i -  2], 17) ^ RotR32(W[i -  2], 19) ^ (W[i -  2] >> 10);
		W[i] = W[i - 16] + S0 + W[i - 7] + S1;
	}

	uint32 Aux[8];
	memcpy(Aux, H, sizeof(uint32) * 8);
	for(int i = 0; i < 64; i += 1){
		uint32 S1 = RotR32(Aux[4], 6) ^ RotR32(Aux[4], 11) ^ RotR32(Aux[4], 25);
		uint32 Ch = (Aux[4] & Aux[5]) ^ (~Aux[4] & Aux[6]);
		uint32 T1 = Aux[7] + S1 + Ch + SHA256K[i] + W[i];

		uint32 S0 = RotR32(Aux[0], 2) ^ RotR32(Aux[0], 13) ^ RotR32(Aux[0], 22);
		uint32 Maj = (Aux[0] & Aux[1]) ^ (Aux[0] & Aux[2]) ^ (Aux[1] & Aux[2]);
		uint32 T2 = S0 + Maj;

		Aux[7] = Aux[6];
		Aux[6] = Aux[5];
		Aux[5] = Aux[4];
		Aux[4] = Aux[3] + T1;
		Aux[3] = Aux[2];
		Aux[2] = Aux[1];
		Aux[1] = Aux[0];
		Aux[0] = T1 + T2;
	}

	H[0] += Aux[0];
	H[1] += Aux[1];
	H[2] += Aux[2];
	H[3] += Aux[3];
	H[4] += Aux[4];
	H[5] += Aux[5];
	H[6] += Aux[6];
	H[7] += Aux[7];
}

void SHA256(const uint8 *Input, int InputBytes, uint8 *Digest){
	ASSERT(Input != NULL && InputBytes >= 0 && Digest != NULL);
	uint32 H[8];
	memcpy(H, SHA256IV, sizeof(uint32) * 8);

	const uint8 *InputPtr = Input;
	int InputRem = InputBytes;
	while(InputRem >= 64){
		SHA256Compress(H, InputPtr);
		InputPtr += 64;
		InputRem -= 64;
	}

	ASSERT(InputRem < 64);
	uint8 Block[64] = {};
	memcpy(Block, InputPtr, InputRem);
	BufferWrite8(&Block[InputRem], 0x80);
	if(InputRem > 55){
		SHA256Compress(H, Block);
		memset(Block, 0, sizeof(Block));
	}
	BufferWrite64BE(&Block[56], ((uint64)InputBytes * 8));
	SHA256Compress(H, Block);

	BufferWrite32BE(&Digest[ 0], H[0]);
	BufferWrite32BE(&Digest[ 4], H[1]);
	BufferWrite32BE(&Digest[ 8], H[2]);
	BufferWrite32BE(&Digest[12], H[3]);
	BufferWrite32BE(&Digest[16], H[4]);
	BufferWrite32BE(&Digest[20], H[5]);
	BufferWrite32BE(&Digest[24], H[6]);
	BufferWrite32BE(&Digest[28], H[7]);
}

void SHA256Calculate(const void *Data, size_t Size, uint8 *Digest){
	SHA256((const uint8*)Data, (int)Size, Digest);
}

bool TestPassword(const uint8 *Auth, int AuthSize, const char *Password){
	if(AuthSize != 64){
		LOG_ERR("Expected 64 bytes of authentication data (got %d)", AuthSize);
		return false;
	}

	// NOTE(fusion): Constant time comparison to check whether the authentication
	// data is set. I'm considering all zeros to be NOT set.
	bool IsSet = false;
	for(int i = 0; i < AuthSize; i += 1){
		if(Auth[i] != 0){
			IsSet = true;
		}
	}

	if(!IsSet){
		LOG_ERR("Authentication data not set");
		return false;
	}

	const uint8 *Hash = &Auth[ 0];
	const uint8 *Salt = &Auth[32];

	// TODO(fusion): It's probably not the best way to mix the salt but should
	// be better than using plaintext or non-salted hashing schemes.
	uint8 Digest[32];
	SHA256((const uint8*)Password, (int)strlen(Password), Digest);
	for(int i = 0; i < 32; i += 1){
		Digest[i] ^= Salt[i];
	}
	SHA256(Digest, 32, Digest);

	// NOTE(fusion): Constant time comparison.
	uint8 Result = 0;
	for(int i = 0; i < 32; i += 1){
		Result |= Digest[i] ^ Hash[i];
	}
	return Result == 0;
}

bool GenerateAuth(const char *Password, uint8 *Auth, int AuthSize){
	if(AuthSize != 64){
		LOG_ERR("Expected 64 bytes buffer for authentication data (got %d)", AuthSize);
		return false;
	}

	uint8 *Hash = &Auth[ 0];
	uint8 *Salt = &Auth[32];
	CryptoRandom(Salt, 32);
	SHA256((const uint8*)Password, (int)strlen(Password), Hash);
	for(int i = 0; i < 32; i += 1){
		Hash[i] ^= Salt[i];
	}
	SHA256(Hash, 32, Hash);
	return true;
}

// CheckSHA256
//==============================================================================
bool CheckSHA256(void){
	// NOTE(fusion): We're using only a few NIST test vectors. This is to make
	// sure there are no blatant implementation errors but we'd ideally run it
	// against all of them to be sure.
	struct{
		const char *Input;
		const char *Expected;
	} Tests[] = {
		{
			"",
			"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
		},
		{
			"5738c929c4f4ccb6",
			"963bb88f27f512777aab6c8b1a02c70ec0ad651d428f870036e1917120fb48bf",
		},
		{
			"1b503fb9a73b16ada3fcf1042623ae7610",
			"d5c30315f72ed05fe519a1bf75ab5fd0ffec5ac1acb0daf66b6b769598594509",
		},
		{
			"09fc1accc230a205e4a208e64a8f204291f581a12756392da4b8c0cf5ef02b95",
			"4f44c1c7fbebb6f9601829f3897bfd650c56fa07844be76489076356ac1886a4",
		},
		{
			"03b264be51e4b941864f9b70b4c958f5355aac294b4b87cb037f11f85f07eb57"
			"b3f0b89550",
			"d1f8bd684001ac5a4b67bbf79f87de524d2da99ac014dec3e4187728f4557471",
		},
		{
			"d1be3f13febafefc14414d9fb7f693db16dc1ae270c5b647d80da8583587c1ad"
			"8cb8cb01824324411ca5ace3ca22e179a4ff4986f3f21190f3d7f3",
			"02804978eba6e1de65afdbc6a6091ed6b1ecee51e8bff40646a251de6678b7ef",
		}
	};

	uint8 Input[64];
	uint8 Expected[32];
	uint8 Digest[32];
	for(int i = 0; i < NARRAY(Tests); i += 1){
		int InputBytes = ParseHexStringBuf(Input, Tests[i].Input);
		int ExpectedBytes = ParseHexStringBuf(Expected, Tests[i].Expected);
		if(InputBytes == -1 || ExpectedBytes != sizeof(Expected)){
			LOG_ERR("Invalid test vector %d", i);
			return false;
		}

		SHA256(Input, InputBytes, Digest);
		if(memcmp(Expected, Digest, 32) != 0){
			LOG_ERR("Test vector %d failed", i);
			return false;
		}
	}

	return true;
}
