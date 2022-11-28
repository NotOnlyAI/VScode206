#include "Base64.h"
#include <iostream>
#include <ctype.h>


#ifndef UCHAR_MAX
#define UCHAR_MAX 0xff
#endif

static const std::string base64_chars = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

unsigned char*Base64::decode_table = NULL;
char*Base64::encode_table = NULL;

Base64::Base64(void)
{
	encode_table = new char[64];

	encode_table[0] = 'A';
	encode_table[17] = 'R';
	encode_table[34] = 'i';
	encode_table[51] = 'z';

	encode_table[1] = 'B';
	encode_table[18] = 'S';
	encode_table[35] = 'j';
	encode_table[52] = '0';

	encode_table[2] = 'C';
	encode_table[19] = 'T';
	encode_table[36] = 'k';
	encode_table[53] = '1';

	encode_table[3] = 'D';
	encode_table[20] = 'U';
	encode_table[37] = 'l';
	encode_table[54] = '2';

	encode_table[4] = 'E';
	encode_table[21] = 'V';
	encode_table[38] = 'm';
	encode_table[55] = '3';

	encode_table[5] = 'F';
	encode_table[22] = 'W';
	encode_table[39] = 'n';
	encode_table[56] = '4';

	encode_table[6] = 'G';
	encode_table[23] = 'X';
	encode_table[40] = 'o';
	encode_table[57] = '5';

	encode_table[7] = 'H';
	encode_table[24] = 'Y';
	encode_table[41] = 'p';
	encode_table[58] = '6';

	encode_table[8] = 'I';
	encode_table[25] = 'Z';
	encode_table[42] = 'q';
	encode_table[59] = '7';

	encode_table[9] = 'J';
	encode_table[26] = 'a';
	encode_table[43] = 'r';
	encode_table[60] = '8';

	encode_table[10] = 'K';
	encode_table[27] = 'b';
	encode_table[44] = 's';
	encode_table[61] = '9';

	encode_table[11] = 'L';
	encode_table[28] = 'c';
	encode_table[45] = 't';
	encode_table[62] = '+';

	encode_table[12] = 'M';
	encode_table[29] = 'd';
	encode_table[46] = 'u';
	encode_table[63] = '/';

	encode_table[13] = 'N';
	encode_table[30] = 'e';
	encode_table[47] = 'v';

	encode_table[14] = 'O';
	encode_table[31] = 'f';
	encode_table[48] = 'w';

	encode_table[15] = 'P';
	encode_table[32] = 'g';
	encode_table[49] = 'x';

	encode_table[16] = 'Q';
	encode_table[33] = 'h';
	encode_table[50] = 'y';

	decode_table = new unsigned char[UCHAR_MAX];
	for (int i = 0; i < UCHAR_MAX; ++i)
	{
		decode_table[i] = 100;
	}
	for (unsigned char i = 0; i < 64; ++i)
	{
		decode_table[(unsigned char)encode_table[i]] = i;
	}
}
Base64::~Base64(void)
{
	delete encode_table;
	encode_table = NULL;

	delete decode_table;
	decode_table = NULL;
}
std::string Base64::base64_encode(const std::string &s)
{
	return base64_encode((const char *)s.c_str(), s.length());
}

std::string Base64::base64_decode(unsigned char const* encoded_string, unsigned int in_len,char*outstr)
{
	int i = 0;
	int j = 0;
	int in_ = 0;
	int offset = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;
	
	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
	{
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) 
		{
			for (i = 0; i <4; i++)
				char_array_4[i] = decode_table[char_array_4[i]];
			
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
			
			for (i = 0; (i < 3); i++)
			{
 				*(outstr+offset) = char_array_3[i];
 				offset++;
			}
			
			i = 0;
		}
	}
	
	if (i) 
	{
		for (j = i; j <4; j++)
			char_array_4[j] = 0;
		
		for (j = 0; j <4; j++)
			char_array_4[j] = decode_table[char_array_4[j]];
		
		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
		
		for (j = 0; (j < i - 1); j++)
		{ 
			*(outstr+offset) = char_array_3[j];
			offset++;
		}
	}
	
	return ret;
}


std::string Base64::base64_decode_string(unsigned char const* encoded_string, unsigned int in_len)
{
	int i = 0;
	int j = 0;
	int in_ = 0;
	int offset = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
	{
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) 
		{
			for (i = 0; i <4; i++)
				char_array_4[i] = decode_table[char_array_4[i]];

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) 
	{
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = decode_table[char_array_4[j]];

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
			ret += char_array_3[j];

	}

	return ret;
}
std::string Base64::base64_encode( char const* bytes_to_encode, unsigned int in_len)
{
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	
	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			
			for(i = 0; (i <4) ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}
	
	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';
		
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
		
		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];
		
		while((i++ < 3))
			ret += '=';
		
	}
	
	return ret;
}

std::string Base64::base64_decode(std::string const& decode_string)
{
	return base64_decode_string((const unsigned char *)decode_string.c_str(), decode_string.size());
/*
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;
	
	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);
			
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
			
			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}
	
	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;
		
		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);
		
		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
		
		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}
	
	return ret;*/
}

