/* 
 * File:   Linalg.h
 * Author: gschoenh
 *
 * Created on November 15, 2013, 10:30 AM
 */

#include "stdafx.h"

#ifndef LINALG_H
#define	LINALG_H
#include"standard.h"
#include <tuple>


namespace msii810161816
{   
    namespace gstd
    {   
        namespace Linalg
        {
            bool MSII810161816_GSTD_API mequals(std::vector<double> m1, std::vector<double> m2, double margin = 1e-15, bool relative = true); //check whether 2 matrices are equal
            std::vector<double> MSII810161816_GSTD_API mabs(std::vector<double>& m1); //absolute values of entries
            std::vector<double> MSII810161816_GSTD_API mneg(std::vector<double>& m1); //negative of a matrix
            std::vector<double> MSII810161816_GSTD_API madd(std::vector<double>& m1, std::vector<double>& m2); //add 2 matrices
            std::vector<double> MSII810161816_GSTD_API msub(std::vector<double>& m1, std::vector<double>& m2); //subtract 2 matrices
            std::vector<double> MSII810161816_GSTD_API mmult(int dim1, int dim2, int dim3, std::vector<double>& m1, std::vector<double>& m2); //multiply two matrices
            std::vector<double> MSII810161816_GSTD_API mprod(std::vector<double>& m1, std::vector<double>& m2); //element-wise product
			std::vector<double> MSII810161816_GSTD_API mscale(std::vector<double>& m1, double scale); //element-wise product
            std::vector<double> MSII810161816_GSTD_API msumrows(int dim1, int dim2, std::vector<double>& m1); //sum each row (results in x*1 matrix)
            std::vector<double> MSII810161816_GSTD_API msumcols(int dim1, int dim2, std::vector<double>& m1); //sum each column (result in 1*x matrix)
            std::vector<double> MSII810161816_GSTD_API msumall(std::vector<double>& m1); //sum all elements (results in 1*1 matrix)
            std::vector<double> MSII810161816_GSTD_API mdotprod(std::vector<double>& m1, std::vector<double>& m2); //dot product
            std::vector<double> MSII810161816_GSTD_API submatrix(int dim1, int dim2, std::vector<double>& m1, int startdim1, int startdim2, int lengthdim1, int lengthdim2); //submatrix. argument -1 indicates "until the end"                             
            std::tuple<int, int, std::vector<double>> MSII810161816_GSTD_API submatrix(int dim1, int dim2, std::vector<double>& m1, int startdim1, int startdim2, int lengthdim1, int lengthdim2, bool allowSmall); //submatrix. argument -1 indicates "until the end"                             
            std::vector<double> MSII810161816_GSTD_API mcoljoin(std::vector<std::vector<double>> ms);
            double MSII810161816_GSTD_API minftynorm(std::vector<double>& m1);
			std::vector<double> MSII810161816_GSTD_API transpose(int dim1, int dim2, std::vector<double> m);
			std::vector<double> MSII810161816_GSTD_API cholesky(std::vector<double>&m1);
            bool MSII810161816_GSTD_API test();
        
        }
    }
}

#endif	/* LINALG_H */

