#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include "totp.hpp"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32")

using namespace std;

// Base32 decoding table
/* clang-format off */
const int base32_reverse_map[128] = 
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 15
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 31
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 47
 //      (49)   2    3    4    5    6    7  (56)
    255, 255,  26,  27,  28,  29,  30,  31, 255, 255, 255, 255, 255, 255, 255, 255, // 63
 // (64)   A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
    255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, // 79
 //   P    Q    R    S    T    U    V    W    X    Y    Z  (91)
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255, // 95
 // (96)   A    B    C    D    E    F    G    H    I    J    K    L    M    N    O  // lower case instead
    255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, // 111
 //   P    Q    R    S    T    U    V    W    X    Y    Z (123)                     // lower case instead
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255  // 127
};
/* clang-format on */

// Base32 decoding function
vector<unsigned char> base32_decode(string base32) {
	queue<int> q_rev;
	for(char& c: base32) {
		if(c <= 49 || (c >= 56 && c <= 64) || (c >= 91 && c <= 96) || c >= 123) {		// illegal lower case will be treat as upper case thanks to base32_reverse_map
			if(c == '=')
				break;
			else
				return {};		 // meet illegal base32 char, return empty
		}
		else {
			q_rev.push(base32_reverse_map[(int)c]);		  // get integers of base32
		}
	}
	queue<bool> q_bin;
	while(!q_rev.empty())		// transform the integers into bits and queue them up
	{
		int now = q_rev.front();	   // get a integer from queue
		q_rev.pop();
		stack<bool> sb;
		for(int i = 0; i < 5; i++)		 // push the bits of this integer into stack
		{								 // 5 bits into 1 group
			sb.push(now & 1);			 // push the last bit of this integer into stack
			now >>= 1;					 // flush the last bit
		}
		while(!sb.empty())		 // flush the bits of this integer into binary queue
		{
			q_bin.push(sb.top());		// pop bits from stack into binary queue
			sb.pop();
		}
	}
	vector<unsigned char> uc;
	while(!q_bin.empty())		// transform the bits into bytes
	{
		int thehex = 0;
		for(int i = 0; i < 8; i++)		 // read a byte from the bits queue
		{
			if(q_bin.empty()) {			// if the bits queue is empty now
				thehex <<= 8 - i;		// set 0 on the last bits of a byte
				break;
			}
			thehex <<= 1;						// make the space for the new bit
			thehex |= (int)q_bin.front();		// push this bit into byte
			q_bin.pop();
		}
		uc.push_back(thehex);
	}
	if(uc.back() == 0)
		uc.pop_back();
	return uc;
}

// Base32 encoding table
/* clang-format off */
const char base32_map[32] = 
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', // 15
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5', '6', '7'  // 31
};
/* clang-format on */

// Base32 encoding function
string base32_encode(vector<unsigned char> bytes) {
	string base32;
	int	   i = 0;
	while(i < bytes.size()) {
		int now = 0;
		for(int j = 0; j < 8; j++) {
			now <<= 1;
			now |= (int)bytes.data()[i + j];
		}
		i += 8;
		for(int j = 0; j < 5; j++) {
			base32 += base32_map[now & 31];
			now >>= 5;
		}
	}
	if(base32.back() == '=')
		base32.pop_back();
	return base32;
}

// transform time_t aka <long long> to bytes
vector<unsigned char> llong2bytes(time_t tt) {
	vector<unsigned char> bytes;
	unsigned char*		  emp = (unsigned char*)&tt;
	for(int i = sizeof(tt) - 1; i >= 0; i--)
		bytes.push_back(emp[i]);
	return bytes;
}

// Fast Exponentiation algorithm
unsigned fastpow(unsigned base, unsigned index) {
	if(index == 0)
		return 1;
	unsigned t = 1;
	while(index != 0) {
		if(index & 1)
			t *= base;
		index >>= 1;
		base *= base;
	}
	return t;
}

// calculate TOTP according to RFC-6238/4226 standard
string hash_to_TOTP(unsigned char* hmac, unsigned length, unsigned digit) {
	stringstream ss;
	for(size_t i = 0; i < length; i++)		 // get hash string from hmac
	{
		ss << setfill('0') << setw(2) << hex << (int)hmac[i];
	}

	string raw_key;
	ss >> raw_key;
	ss.str("");
	ss.clear();

	unsigned overflow_key;
	ss << hex << raw_key.substr(static_cast<size_t>((int)hmac[19] % 16) * 2, 8);
	ss >> overflow_key;
	ss.str("");
	ss.clear();

	int safe_key = overflow_key &= 0x7fffffff;		 // abandon bits over signed int
	int out_key	 = safe_key % fastpow(10, digit);

	string result;
	ss << setw(6) << setfill('0') << to_string(out_key);
	ss >> result;
	ss.str("");
	ss.clear();

	return result;
}

//random_bytes
vector<unsigned char> random_bytes(size_t length) {
	vector<unsigned char> result(length);
	for(size_t i = 0; i < length; i++)
		result[i] = (unsigned char)rand();
	return result;
}

string totp_secret_key_gen(size_t length) {
	vector<unsigned char> key = random_bytes(length);
	return base32_encode(key);
}
