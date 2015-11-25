#include "stdafx.h"

#pragma once
#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H
#include"standard.h"
#include "Reader.h"
#include "File.h"
#include<array>


namespace msii810161816
{
	namespace gstd
	{
		namespace dependencies
		{
			template<unsigned long numRoots>
			std::string getDirectory(std::array<std::string, numRoots> roots, std::string tail)
			{
				std::vector<std::string> vecRoots;
				for (int i = 0; i < (int)numRoots; i++)
					vecRoots.push_back(roots[i] + tail);
				std::vector<bool> res = gstd::file::exists(vecRoots);
				for (int i = 0; i < (int)numRoots; i++)
				{
					if (res[i])
						return roots[i];
				}
				gstd::error("dependency does not exist for tail " + tail);
				return "";
			}

			namespace cpp
			{
				const extern std::array<std::string, 5> home;
				MSII810161816_GSTD_API std::string getHome();
			}

			namespace matlab
			{
				const extern std::array<std::string, 5> objRoots;
				MSII810161816_GSTD_API std::string getObjRoot();
				const extern std::array<std::string, 2> startCommands;
			}

			MSII810161816_GSTD_API bool test();
		}
	}
}





#endif
