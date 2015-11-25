/* 
 * File:   Double.h
 * Author: gschoenh
 *
 * Created on November 15, 2013, 3:10 PM
 */

#include "stdafx.h"
#include "standard.h"

#ifndef PRIMITIVES_H
#define	PRIMITIVES_H

namespace msii810161816
{   
    namespace gstd
    {   
        namespace Double
        {
            double MSII810161816_GSTD_API abs(double x);
            bool MSII810161816_GSTD_API equals(double x, double y, double margin = 1e-15, bool relative = true, bool doc = false);
            double MSII810161816_GSTD_API clipPrecision(double x);
            
            bool MSII810161816_GSTD_API test();
        }

		namespace Int
		{
			int MSII810161816_GSTD_API create(double x, std::string mode);
			std::pair<int, double> splitReal(double x);

			bool MSII810161816_GSTD_API test();
		}
    }
}

#endif	/* PRIMITIVES_H */

