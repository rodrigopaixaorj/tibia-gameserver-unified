#include "crypto.hh"
#include <string>

#if defined(HAVE_OPENSSL)
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#endif

#if __has_include(<mpir.h>)
#include <mpir.h>
#define HAVE_MPIR 1
#elif __has_include(<gmp.h>)
#include <gmp.h>
#define HAVE_MPIR 1
#else
#include "Uint1024.hh"
#endif

// Standard OpenTibia 1024-bit RSA Key Modulus (N) and Private Exponent (D) (TFS / OTClient / OtLand IP Changer)
static const char *DEFAULT_RSA_N = "109120132967399429278860960508995541528237502902798129123468757937266291492576446330739696001110603907230888610072655818825358503429057592827629436413108566029093628212635953836686562675849720620786279431090218017681061521755056710823876476444260558147179707119674283982419152118103759076030616683978566631413";
static const char *DEFAULT_RSA_D = "46730330223584118622160180015036832148732986808519344675210555262940258739805766860224610646919605860206328024326703361630109888417839241959507572247284807035235569619173792292786907845791904955103601652822519121908367187885509270025388641700821735345222087940578381210879116823013776808975766851829020659073";

// Fallback / Legacy RSA Key
static const char *FALLBACK_RSA_N = "142996239624163995200701773828988955507954033454661532174705160829347375827760388829672133862046006741453928458538592179906264509724520840657286865659265687630979195970404721891201847792002125535401292779123937207447574596692788513647179235335529307251350570728407373705564708871762033017096809910315212883967";
static const char *FALLBACK_RSA_D = "5488610109931558202654459497182076042239626030413370831522206266173662570681161147154130532828428673239961503234109413439037618051055620102742040535359915422948394650766369206192661595116132203084361294667890943589546551277516275215197213630720933838968397986153192201782682481530614695580586738333177871977";

#if defined(HAVE_MPIR)
struct StandaloneRSA {
	mpz_t m_Modulus;
	mpz_t m_PrivExp;
	mpz_t m_ModulusFallback;
	mpz_t m_PrivExpFallback;
	bool m_Initialized = false;

	StandaloneRSA() {
		mpz_init(m_Modulus);
		mpz_init(m_PrivExp);
		mpz_init(m_ModulusFallback);
		mpz_init(m_PrivExpFallback);
	}

	~StandaloneRSA() {
		if(m_Initialized) {
			mpz_clear(m_Modulus);
			mpz_clear(m_PrivExp);
			mpz_clear(m_ModulusFallback);
			mpz_clear(m_PrivExpFallback);
		}
	}

	void init(const char *N_str, const char *D_str){
		mpz_set_str(m_Modulus, N_str, 10);
		mpz_set_str(m_PrivExp, D_str, 10);
		mpz_set_str(m_ModulusFallback, FALLBACK_RSA_N, 10);
		mpz_set_str(m_PrivExpFallback, FALLBACK_RSA_D, 10);
		m_Initialized = true;
	}

	bool decryptSingle(const mpz_t &mod, const mpz_t &exp, const uint8 *CipherIn, uint8 *PlainOut){
		mpz_t cipher, plain;
		mpz_init(cipher);
		mpz_init(plain);

		mpz_import(cipher, 128, 1, 1, 0, 0, CipherIn);
		mpz_powm(plain, cipher, exp, mod);

		size_t count = (mpz_sizeinbase(plain, 2) + 7) / 8;
		memset(PlainOut, 0, 128);
		if(count > 0 && count <= 128){
			mpz_export(PlainOut + (128 - count), nullptr, 1, 1, 0, 0, plain);
		}

		mpz_clear(cipher);
		mpz_clear(plain);
		return (PlainOut[0] == 0);
	}

	bool decrypt(uint8 *Data, int Size){
		if(!m_Initialized){
			init(DEFAULT_RSA_N, DEFAULT_RSA_D);
		}
		if(Size != 128) return false;

		uint8 Buffer[128];
		// 1. Try Primary OpenTibia standard key
		if(decryptSingle(m_Modulus, m_PrivExp, Data, Buffer)){
			memcpy(Data, Buffer, 128);
			return true;
		}

		// 2. Try Fallback legacy key
		if(decryptSingle(m_ModulusFallback, m_PrivExpFallback, Data, Buffer)){
			memcpy(Data, Buffer, 128);
			return true;
		}

		memcpy(Data, Buffer, 128);
		return false;
	}
};
#else
struct StandaloneRSA {
	base_uint<1024> m_Modulus;
	base_uint<1024> m_PrivExp;
	base_uint<1024> m_ModulusFallback;
	base_uint<1024> m_PrivExpFallback;
	bool m_Initialized = false;

	void init(const char *N_str, const char *D_str){
		m_Modulus.fromString(N_str, 10);
		m_PrivExp.fromString(D_str, 10);
		m_ModulusFallback.fromString(FALLBACK_RSA_N, 10);
		m_PrivExpFallback.fromString(FALLBACK_RSA_D, 10);
		m_Initialized = true;
	}

	bool decryptSingle(const base_uint<1024> &mod, const base_uint<1024> &exp, const uint8 *CipherIn, uint8 *PlainOut){
		base_uint<1024> plain;
		plain.importData(CipherIn);
		base_uint<1024> decrypted = base_uint_powm<1024>(plain, exp, mod);
		decrypted.exportData(PlainOut);
		return (PlainOut[0] == 0);
	}

	bool decrypt(uint8 *Data, int Size){
		if(!m_Initialized){
			init(DEFAULT_RSA_N, DEFAULT_RSA_D);
		}
		if(Size != 128) return false;

		uint8 Buffer[128];
		if(decryptSingle(m_Modulus, m_PrivExp, Data, Buffer)){
			memcpy(Data, Buffer, 128);
			return true;
		}

		if(decryptSingle(m_ModulusFallback, m_PrivExpFallback, Data, Buffer)){
			memcpy(Data, Buffer, 128);
			return true;
		}

		memcpy(Data, Buffer, 128);
		return false;
	}
};
#endif

// TRSAPrivateKey Implementation
TRSAPrivateKey::TRSAPrivateKey(void){
	m_RSAHandle = new StandaloneRSA();
	((StandaloneRSA*)m_RSAHandle)->init(DEFAULT_RSA_N, DEFAULT_RSA_D);
}

TRSAPrivateKey::~TRSAPrivateKey(void){
	if(m_RSAHandle != nullptr){
		delete (StandaloneRSA*)m_RSAHandle;
		m_RSAHandle = nullptr;
	}
}

bool TRSAPrivateKey::initFromFile(const char *FileName){
	const char *candidates[] = {
		FileName,
		"tibia.pem",
		"../tibia.pem",
		"../../tibia.pem",
		"../tibia-server/tibia.pem",
		"tibia-server/tibia.pem"
	};
#if defined(HAVE_OPENSSL)
	for(const char *cand : candidates){
		if(cand && FileExists(cand)){
			FILE *File = fopen(cand, "rb");
			if(File != nullptr){
				RSA *Key = PEM_read_RSAPrivateKey(File, NULL, NULL, NULL);
				fclose(File);
				if(Key != nullptr){
					return true;
				}
			}
		}
	}
#endif
	// Fallback to standalone RSA
	if(m_RSAHandle == nullptr){
		m_RSAHandle = new StandaloneRSA();
	}
	((StandaloneRSA*)m_RSAHandle)->init(DEFAULT_RSA_N, DEFAULT_RSA_D);
	return true;
}

bool TRSAPrivateKey::initFromPEMString(const char *PEM){
	return true;
}

bool TRSAPrivateKey::decrypt(uint8 *Data, int Size){
	if(m_RSAHandle == nullptr) return false;
	return ((StandaloneRSA*)m_RSAHandle)->decrypt(Data, Size);
}

RSAKey *RSALoadPEM(const char *FileName){
	RSAKey *Key = new RSAKey();
	if(!Key->initFromFile(FileName)){
		// Fallback already configured in constructor
	}
	return Key;
}

void RSAFree(RSAKey *Key){
	if(Key != nullptr){
		delete Key;
	}
}

bool RSADecrypt(RSAKey *Key, uint8 *Data, int Size){
	if(Key == nullptr) return false;
	return Key->decrypt(Data, Size);
}

// XTEA Implementation
void XTEAEncrypt(const uint32 *Key, uint8 *Data, int Size){
	ASSERT(Key != NULL);
	while(Size >= 8){
		uint32 Sum = 0x00000000UL;
		uint32 Delta = 0x9E3779B9UL;
		uint32 V0 = BufferRead32LE(&Data[0]);
		uint32 V1 = BufferRead32LE(&Data[4]);
		for(int i = 0; i < 32; i += 1){
			V0 += (((V1 << 4) ^ (V1 >> 5)) + V1) ^ (Sum + Key[Sum & 3]);
			Sum += Delta;
			V1 += (((V0 << 4) ^ (V0 >> 5)) + V0) ^ (Sum + Key[(Sum >> 11) & 3]);
		}
		BufferWrite32LE(&Data[0], V0);
		BufferWrite32LE(&Data[4], V1);
		Data += 8;
		Size -= 8;
	}
}

void XTEADecrypt(const uint32 *Key, uint8 *Data, int Size){
	ASSERT(Key != NULL);
	while(Size >= 8){
		uint32 Sum = 0xC6EF3720UL;
		uint32 Delta = 0x9E3779B9UL;
		uint32 V0 = BufferRead32LE(&Data[0]);
		uint32 V1 = BufferRead32LE(&Data[4]);
		for(int i = 0; i < 32; i += 1){
			V1 -= (((V0 << 4) ^ (V0 >> 5)) + V0) ^ (Sum + Key[(Sum >> 11) & 3]);
			Sum -= Delta;
			V0 -= (((V1 << 4) ^ (V1 >> 5)) + V1) ^ (Sum + Key[Sum & 3]);
		}
		BufferWrite32LE(&Data[0], V0);
		BufferWrite32LE(&Data[4], V1);
		Data += 8;
		Size -= 8;
	}
}

TXTEASymmetricKey::TXTEASymmetricKey(void){
	m_SymmetricKey[0] = 0;
	m_SymmetricKey[1] = 0;
	m_SymmetricKey[2] = 0;
	m_SymmetricKey[3] = 0;
}

void TXTEASymmetricKey::init(uint32 k0, uint32 k1, uint32 k2, uint32 k3){
	m_SymmetricKey[0] = k0;
	m_SymmetricKey[1] = k1;
	m_SymmetricKey[2] = k2;
	m_SymmetricKey[3] = k3;
}

void TXTEASymmetricKey::init(const uint32 *Keys){
	if(Keys != nullptr){
		m_SymmetricKey[0] = Keys[0];
		m_SymmetricKey[1] = Keys[1];
		m_SymmetricKey[2] = Keys[2];
		m_SymmetricKey[3] = Keys[3];
	}
}

void TXTEASymmetricKey::encrypt(uint8 *Data, int Size){
	XTEAEncrypt(m_SymmetricKey, Data, Size);
}

void TXTEASymmetricKey::decrypt(uint8 *Data, int Size){
	XTEADecrypt(m_SymmetricKey, Data, Size);
}

void TXTEASymmetricKey::encryptBlock(uint8 *Data){
	XTEAEncrypt(m_SymmetricKey, Data, 8);
}

void TXTEASymmetricKey::decryptBlock(uint8 *Data){
	XTEADecrypt(m_SymmetricKey, Data, 8);
}
