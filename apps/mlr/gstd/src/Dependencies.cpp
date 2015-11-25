#include "stdafx.h"

#include "Dependencies.h"

namespace msii810161816
{
	namespace gstd
	{
		namespace dependencies
		{
			namespace cpp
			{
				const std::array<std::string, 5> home = { {
					"/usr0/home/gschoenh/Dropbox/Code/cpp/",
					"C:/Users/gschoenh/Dropbox/Code/cpp/",
					"/home/gschoenh/Dropbox/Code/cpp/",
					"/Users/georgschoenherr/Dropbox/Code/cpp/",
					"Z:/Dropbox/Code/cpp/"
				} };

				std::string getHome()
				{
					return getDirectory(home, "libs/gstd/makefile");
				}
			}

			namespace matlab
			{
				const std::array<std::string, 5> objRoots = { {
					"/usr0/home/gschoenh/Dropbox/Code_obj/Libraries/Matlab/",
					"C:/Users/gschoenh/Dropbox/Code_obj/Libraries/Matlab/",
					"/home/gschoenh/Dropbox/Code_obj/Libraries/Matlab/",
					"/Users/georgschoenherr/Dropbox/Code_obj/Libraries/Matlab/",
					"Z:/Dropbox/Code_obj/Libraries/Matlab/"
				} };

				std::string getObjRoot()
				{
					return getDirectory(objRoots, "SLEP_wrappers/Lasso.m");
				}

				const std::array<std::string, 2> startCommands = { {
					"/opt/matlab/8.1/bin/matlab",
					"/Applications/MATLAB_R2014a.app/bin/matlab"
				} };
			}

			bool test()
			{
				gstd::Printer::c("This is a gstd::Dependencies group of function test");

				//cpp::getHome
				{
					try
					{
						cpp::getHome();
					}
					catch(std::exception e)
					{
						gstd::reportFailure("Failed on function gstd::dependencies::cpp::getHome");
						return false;
					}
				}

				//matlab::getObjRoot
				{
					try
					{
						matlab::getObjRoot();
					}
					catch (std::exception e)
					{
						gstd::reportFailure("Failed on function gstd::dependencies::matlab::getObjRoot");
						return false;
					}
				}

				return true;
			}
		}
	}
}
