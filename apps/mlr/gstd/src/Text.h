#include "stdafx.h"

#pragma once
#ifndef TEXT_H
#define TEXT_H
#include"standard.h"
#include<map>
#include<array>


namespace msii810161816
{
	namespace gstd
	{
		namespace text
		{
			extern MSII810161816_GSTD_API const std::vector<char> defaultBlanks;
			extern MSII810161816_GSTD_API const std::map<char, char> upperLower;
			extern MSII810161816_GSTD_API const std::map<char, char> lowerUpper;
			extern MSII810161816_GSTD_API const std::map<char, bool> letterMap;
			MSII810161816_GSTD_API std::string makeUpper(std::string input);
			MSII810161816_GSTD_API std::string makeLower(std::string input);
			MSII810161816_GSTD_API std::map<std::string, std::vector<int> > invert(std::vector<std::string> input, bool killAllCapitals, std::vector<char> blanks = defaultBlanks);
			MSII810161816_GSTD_API bool test();
		}
	}
}






#endif