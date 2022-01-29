#pragma once
#include <string>


static int count_utf8(const std::string& str)
{
	int count = 0;
	for(size_t i = 0; i < str.size(); i++)
	{
		if((str[i] & 0xC0) != 0x80)
		{
			count++;
		}
	}
	return count;
}