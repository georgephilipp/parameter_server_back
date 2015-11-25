/* 
 * File:   Stat.h
 * Author: gschoenh
 *
 * Created on November 14, 2013, 8:14 PM
 */

#include "stdafx.h"

#ifndef STAT_H
#define	STAT_H
#include"standard.h"
#include"Rand.h"


namespace msii810161816
{   
    namespace gstd
    {   
        namespace stat
        {                    
            double MSII810161816_GSTD_API normcum(double x);  
            double MSII810161816_GSTD_API normcuminv(double p);
            double MSII810161816_GSTD_API randnorm(int mode);
            std::vector<double> MSII810161816_GSTD_API covarianceMatrix(int dim1, int dim2, std::vector<double>& input);
            std::pair<double,double> MSII810161816_GSTD_API meanAndVar(std::vector<double> input);
			double MSII810161816_GSTD_API meanPval(double targetMean, std::vector<double> vals);
			double MSII810161816_GSTD_API varPval(double targetVariance, std::vector<double> vals);
			double MSII810161816_GSTD_API combineTestPs(std::vector<double> inputs);

            bool MSII810161816_GSTD_API test();
        }
    }
}

#endif	/* STAT_H */

