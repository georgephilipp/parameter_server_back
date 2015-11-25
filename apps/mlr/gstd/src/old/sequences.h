/* 
 * File:   sequences.h
 * Author: gschoenh
 *
 * Created on April 11, 2013, 12:00 PM
 */

#ifndef msii810161816_gstd_SEQUENCES_H
#define	msii810161816_gstd_SEQUENCES_H

#include "standard.h"

namespace msii810161816
{   
    namespace gstd
    {
        namespace seq
        {
            std::vector<int> irange(int begin, int end, int returnpoint = 0);
            std::vector<std::vector<double> > d2range(double begin, int numrows, int numcols, int rowstep = 1, int colstep = 1);
            std::vector<int> randperm(int length);
        }
    }
}
#endif	/* msii810161816_gstd_SEQUENCES_H */

