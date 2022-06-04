#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include "csaori.h"
#include "totp.hpp"
#include <openssl/hmac.h>

/*---------------------------------------------------------
	‰Šú‰»
---------------------------------------------------------*/
bool CSAORI::load()
{
	return true;
}

/*---------------------------------------------------------
	‰ð•ú
---------------------------------------------------------*/
bool CSAORI::unload()
{
	return true;
}

/*---------------------------------------------------------
	ŽÀs
---------------------------------------------------------*/
void CSAORI::exec(const CSAORIInput& in, CSAORIOutput& out) {
	out.result_code = SAORIRESULT_BAD_REQUEST;
	if(in.args.size() == 0)
		return;
	if(in.args[0] == L"keygen") {
		//get length form args[1]
		size_t length=30;
		if(in.args.size()>1) {
			length = _wtoi(in.args[1].c_str());
		}
		//generate key
		string key		= totp_secret_key_gen(length);
		out.result_code = SAORIRESULT_OK;
		out.result  = wstring(key.begin(),key.end());
	}
	else if(in.args[0] == L"totp") {
		//get key form args[1]
		if(in.args.size() == 1)
			return;
		const string_t&keyw		= in.args[1];
		string key		= string(keyw.begin(),keyw.end());
		//get time from args[2]
		time_t time;
		if(in.args.size()>2) {
			time = _wtoi(in.args[2].c_str());
		}
		else {
			time = std::time(NULL);
		}
		//get digit from args[3]
		unsigned digit = 6;
		if(in.args.size()>3) {
			digit = _wtoi(in.args[3].c_str());
		}
		//get refresh seconds from args[4]
		unsigned refreshSeconds = 30;
		if(in.args.size()>4) {
			refreshSeconds = _wtoi(in.args[4].c_str());
		}
		//Calculating hmac
		vector<unsigned char> h_msg = llong2bytes(getTimeSign(refreshSeconds,time));  // get time signature and transform time_t aka <long long> to bytes
		vector<unsigned char> h_key = base32_decode(key);							  // transform base32 encoded secret key to decoded bytes
		
		unsigned char out_hmac[UINT_LENGTH_SHA1];		// perform openssl function to calculate HMAC-SHA1 hash
		unsigned int  out_hmac_length;
		HMAC(EVP_sha1(), h_key.data(), (int)h_key.size(), h_msg.data(), (int)h_msg.size(), out_hmac, &out_hmac_length);
		
		//Calculating TOTP
		auto result  = hash_to_TOTP(out_hmac, out_hmac_length, digit);
		out.result=wstring(result.begin(),result.end());
		out.result_code = SAORIRESULT_OK;
	}
	return;
}
