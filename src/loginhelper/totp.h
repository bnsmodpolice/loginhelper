#pragma once
#include "libcppotp/bytes.h"
#include "libcppotp/otp.h"
#include <time.h>

std::string normalizedBase32String(const std::string& unnorm)
{
	std::string ret;

	for (char c : unnorm)
	{
		if (c == ' ' || c == '\n' || c == '-')
		{
			// skip separators
		}
		else if (std::islower(c))
		{
			// make uppercase
			char u = std::toupper(c);
			ret.push_back(u);
		}
		else
		{
			ret.push_back(c);
		}
	}

	return ret;
}

uint32_t generateTotp(const char* key)
{
	std::string normalizedKey = normalizedBase32String(key);
	CppTotp::Bytes::ByteString qui = CppTotp::Bytes::fromUnpaddedBase32(normalizedKey);
	uint32_t p = CppTotp::totp(qui, time(NULL), 0, 30, 6);
#ifdef DEBUG
	printf("%06u (%2us remain)\r", p, 30 - (time(NULL) % 30));
#endif
	return p;
}