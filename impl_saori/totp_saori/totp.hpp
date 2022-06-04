#include <vector>
#include <string>
#include <ctime>
using namespace std;
constexpr int UINT_LENGTH_SHA1 = 20;

// Base32 decoding function
vector<unsigned char> base32_decode(string base32);

// Base32 encoding function
string base32_encode(vector<unsigned char> bytes);

// Get Unix Time and culculate the TOTP time signature
inline time_t getTimeSign(unsigned refreshPeriod, time_t time) {
	time_t outTime = time / refreshPeriod;
	return outTime;
}
inline time_t getTimeSign(unsigned refreshPeriod) {
	time_t currentUnixTime = time(0);
	return getTimeSign(refreshPeriod, currentUnixTime);
}

// transform time_t aka <long long> to bytes
vector<unsigned char> llong2bytes(time_t tt);

// Fast Exponentiation algorithm
unsigned fastpow(unsigned base, unsigned index);

// calculate TOTP according to RFC-6238/4226 standard
string hash_to_TOTP(unsigned char* hmac, unsigned length, unsigned digit);

string totp_secret_key_gen(size_t length);
