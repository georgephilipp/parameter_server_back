#include "stdafx.h"

#pragma once

#include"standard.h"
#include"Base.h"
#include<iostream>

namespace msii810161816
{
	namespace gstd
	{
		namespace file
		{
			extern int MSII810161816_GSTD_API fileOpenBufferReps;
			extern double MSII810161816_GSTD_API fileOpenBufferDelay;
			std::string MSII810161816_GSTD_API joinpaths(std::string path1, std::string path2);			
			/*new*/ bool MSII810161816_GSTD_API open(std::ifstream *stream, std::string location, int reps = fileOpenBufferReps, double delay = fileOpenBufferDelay);
			/*new*/ bool MSII810161816_GSTD_API writeopen(std::ofstream *stream, std::string location, std::ios_base::openmode mode, int reps = fileOpenBufferReps, double delay = fileOpenBufferDelay);
			/*new*/ bool MSII810161816_GSTD_API close(std::ifstream *stream, int reps = fileOpenBufferReps, double delay = fileOpenBufferDelay);
			/*new*/ bool MSII810161816_GSTD_API writeclose(std::ofstream *stream, int reps = fileOpenBufferReps, double delay = fileOpenBufferDelay);
			/*new*/ std::vector<double> MSII810161816_GSTD_API existsProbabilities(std::vector<std::string> locations, int reps = fileOpenBufferReps, double delay = fileOpenBufferDelay);
			/*new*/ std::vector<bool> MSII810161816_GSTD_API exists(std::vector<std::string> locations, int reps = fileOpenBufferReps, double delay = fileOpenBufferDelay);
			std::vector<bool> MSII810161816_GSTD_API remove(std::vector<std::string> paths, bool enforceBefore, bool enforceAfter, int reps = fileOpenBufferReps, double delay = fileOpenBufferDelay);
			bool MSII810161816_GSTD_API test();
		}



	}
}
