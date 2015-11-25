/* 
 * File:   Linalg.cpp
 * Author: gschoenh
 * 
 * Created on November 15, 2013, 10:30 AM
 */

#include "stdafx.h"

#include "ex.h"
#include "Linalg.h"
#include "Primitives.h"
#include "Printer.h"
#include "Rand.h"
#include <math.h>       

namespace msii810161816
{   
    namespace gstd
    {   
        namespace Linalg
        {
            std::vector<double> mabs(std::vector<double>& m1)
            {
                int size = m1.size();
                std::vector<double> res(size);
                for(int i=0;i<size;i++)
                {
                    if(m1[i] < 0)
                        res[i] = -m1[i];
                    else
                        res[i] = m1[i];
                }          
                return res;
            }            
            
            std::vector<double> mneg(std::vector<double>& m1)
            {
                int size = m1.size();
                std::vector<double> res = std::vector<double>(size);
                for(int i=0;i<size;i++)
                    res[i] = -m1[i];
                return res;
            }
            
            std::vector<double> madd(std::vector<double>& m1, std::vector<double>& m2)
            {
                int size = m1.size();
                if((int)m2.size() != size)
                    gstd::error("Cannot add vectors of unequal sizes");
                std::vector<double> res = std::vector<double>(size);
                for(int i=0;i<size;i++)
                    res[i] = m1[i] + m2[i];
                return res;
            }

            std::vector<double> msub(std::vector<double>& m1, std::vector<double>& m2)
            {
                int size = m1.size();
                if((int)m2.size() != size)
                    gstd::error("Cannot add vectors of unequal sizes");
                std::vector<double> res = std::vector<double>(size);
                for(int i=0;i<size;i++)
                    res[i] = m1[i] - m2[i];
                return res;
            }
             
            std::vector<double> mmult(int dim1, int dim2, int dim3, std::vector<double>& m1, std::vector<double>& m2)
            {
                if( (int)m1.size() != dim1*dim2 )
                    gstd::error("Size mismatch");
                if( (int)m2.size() != dim2*dim3 )
                    gstd::error("Size mismatch");
                std::vector<double> res = std::vector<double>(dim1*dim3);
                for(int i=0;i<dim1;i++)
                    for(int k=0;k<dim3;k++)
                        for(int j=0;j<dim2;j++)
                            res[i*dim3+k] += m1[i*dim2+j]*m2[j*dim3+k];
                return res;
            }    

            bool mequals(std::vector<double> m1, std::vector<double> m2, double margin /*= 1e-15*/, bool relative /*= true*/)
            {
                int size = m1.size();
                if( (int)m2.size() != size )
                    gstd::error("cannot compare vectors of unequal size for equality");
                for(int i=0;i<size;i++)
                    if(!gstd::Double::equals(m1[i], m2[i], margin, relative))
                        return false;
                return true;              
            }
            
            std::vector<double> mprod(std::vector<double>& m1, std::vector<double>& m2)
            {
                int size = m1.size();
                if( (int)m2.size() != size )
                    gstd::error("cannot multiply vectors of unequal size for equality");
                std::vector<double> res = std::vector<double>(size);
                for(int i=0;i<size;i++)
                    res[i] = m1[i]*m2[i];
                return res;
            }

			std::vector<double> mscale(std::vector<double>& m1, double scale)
			{
				int size = m1.size();
				std::vector<double> res(size);
				for (int i = 0; i < size; i++)
					res[i] = scale*m1[i];
				return res;
			}
            
            std::vector<double> msumrows(int dim1, int dim2, std::vector<double>& m1)
            {
                if((int)m1.size() != dim1*dim2)
                    gstd::error("size mismatch");
                std::vector<double> res = std::vector<double>(dim1);
                for(int i=0;i<dim1;i++)
                {
                    double newval = 0;
                    for(int j=0;j<dim2;j++)
                        newval += m1[i*dim2+j];
                    res[i] = newval;
                }
                return res;
            }
            
            std::vector<double> msumcols(int dim1, int dim2, std::vector<double>& m1)
            {
                if((int)m1.size() != dim1*dim2)
                    gstd::error("size mismatch");
                std::vector<double> res = std::vector<double>(dim2);
                for(int j=0;j<dim2;j++)
                {
                    double newval = 0;
                    for(int i=0;i<dim1;i++)
                        newval += m1[i*dim2+j];
                    res[j] = newval;
                }
                return res;
            }
            
            std::vector<double> msumall(std::vector<double>& m1)
            {
                std::vector<double> res = std::vector<double>(1);
                double total = 0;
                int size = m1.size();
                for(int i=0;i<size;i++)
                    total += m1[i];
                res[0] = total;
                return res;
            }
            
            std::vector<double> mdotprod(std::vector<double>& m1, std::vector<double>& m2)
            {
                std::vector<double> prod = mprod(m1, m2);
                return msumall(prod);
            }    
            
            std::vector<double> submatrix(int dim1, int dim2, std::vector<double>& m1, int startdim1, int startdim2, int lengthdim1, int lengthdim2)
            {               
                if((int)m1.size() != dim1*dim2)
                    gstd::error("size mismatch");
                
                gstd::check(lengthdim1 <= dim1 - startdim1, "cannot extract large from small matrix");
                gstd::check(lengthdim2 <= dim2 - startdim2, "cannot extract large from small matrix");
                
                if(lengthdim1 == -1)
                    lengthdim1 = dim1 - startdim1;
                if(lengthdim2 == -1)
                    lengthdim2 = dim2 - startdim2;
                
                std::vector<double> res = std::vector<double>(lengthdim1*lengthdim2);
                for(int i=0;i<lengthdim1;i++)
                    for(int j=0;j<lengthdim2;j++)
                        res[i*lengthdim2 + j] = m1[(i+startdim1)*dim2+j+startdim2];

                return res;            
            }
            
            std::tuple<int, int, std::vector<double> > submatrix(int dim1, int dim2, std::vector<double>& m1, int startdim1, int startdim2, int lengthdim1, int lengthdim2, bool allowSmall)
            {
                if(!allowSmall)
                    gstd::error("use other overload");
                
                if(lengthdim1 > dim1 - startdim1)
                    lengthdim1 = dim1 - startdim1;
                if(lengthdim2 > dim2 - startdim2)
                    lengthdim2 = dim2 - startdim2;
                
                std::tuple<int, int, std::vector<double> > res = std::make_tuple(lengthdim1, lengthdim2, submatrix(dim1, dim2, m1, startdim1, startdim2, lengthdim1, lengthdim2));
                return res;                
            }
            
            std::vector<double> mcoljoin(std::vector<std::vector<double>> ms)
            {
                int cols = ms.size();
                gstd::check(cols > 0, "cannot join 0 columns");
                int rows = ms[0].size();
                for(int i=0;i<cols;i++)
                    gstd::check((int)ms[i].size() == rows, "cannot joint columns of different length");
                std::vector<double> res;
                for(int j=0;j<rows;j++)
                    for(int i=0;i<cols;i++)
                        res.push_back(ms[i][j]);
                return res;                
            }
            
            double minftynorm(std::vector<double>& m1)
            {
                double res = 0;
                int size = m1.size();
                for(int i=0;i<size;i++)
                {
                    double absval = m1[i];
                    if(absval < 0)
                        absval = -absval;
                    if(res < absval)
                        res = absval;
                }   
                return res;
            }
            
			std::vector<double> transpose(int dim1, int dim2, std::vector<double> m)
			{
				gstd::check((int)m.size() == dim1*dim2, "incorrect input dimension");
				std::vector<double> res = std::vector<double>(dim1*dim2, 0);
				for (int i = 0; i < dim1; i++)
				{
					for (int j = 0; j < dim2; j++)
					{
						res[j*dim1 + i] = m[i*dim2 + j];
					}
				}
				return res;
			}

			std::vector<double> cholesky(std::vector<double>&m)
			{
				int size = m.size();
				int dim = (int)sqrt((double)size);
				gstd::check(dim*dim == size, "input matrix is not square");

				std::vector<double> res(size, 0);

				for (int i = 0; i<dim; i++)
				{
					res[i*dim + i] = m[i*dim + i];
					for (int k = 0; k < i; k++)
						res[i*dim + i] -= res[k*dim + i] * res[k*dim + i];
					gstd::check(res[i*dim + i] > 0, "input matrix is not symmetric positive definite");
					res[i*dim + i] = sqrt(res[i*dim + i]);
					for (int j = i + 1; j < dim; j++)
					{
						res[i*dim + j] = m[i*dim + j];
						for (int k = 0; k < i; k++)
							res[i*dim + j] -= res[k*dim + i] * res[k*dim + j];
						res[i*dim + j] /= res[i*dim + i];
					}
				}

				return res;
			}

            bool test()
            {
                gstd::Printer::c("This is the test for the gstd::Linalg group of functions");
                
                //equals
                {
                    std::vector<double> input1 = {1};
                    std::vector<double> input2 = {1.0000000000000002};
                    std::vector<double> input3 = {1.000000000000002};
                    if(!mequals(input1, input2) || mequals(input1, input3))
                    {
                        gstd::reportFailure("gstd::Linalg::mequals");
                        return false;
                    }
                }
                
                //mabs
                {
                    std::vector<double> input = {0,1,-2,3,-4};
                    std::vector<double> target = {0,1,2,3,4};
                    std::vector<double> out = mabs(input);
                    if(!mequals(target,out))
                    {
                        gstd::reportFailure("gstd::Linalg::mabs");
                        return false;
                    }
                }
 
                //mneg
                {
                    std::vector<double> start = {1,-2,3};
                    std::vector<double> target = {-1,2,-3};
                    std::vector<double> neg = mneg(start);
                    if(!mequals(neg,target) || !mequals(mneg(neg),start))
                    {
                        gstd::reportFailure("gstd::Linalg::mneg");
                        return false;
                    }
                }

                //madd
                {
                    std::vector<double> start1 = {1,-2,3};
                    std::vector<double> start2 = {10,-20,30};                    
                    std::vector<double> target = {11,-22,33};
                    if(!mequals(madd(start1,start2),target))
                    {
                        gstd::reportFailure("gstd::Linalg::madd");
                        return false;
                    }
                }

                //msub
                {
                    std::vector<double> start1 = {1,-2,3};
                    std::vector<double> start2 = {10,-20,30};                    
                    std::vector<double> target = {-9,18,-27};
                    if(!mequals(msub(start1,start2),target))
                    {
                        gstd::reportFailure("gstd::Linalg::msub");
                        return false;
                    }
                }
                
                //mmult
                {
                    std::vector<double> start1 = {2, 1, 1, 2};
                    std::vector<double> start2 = {3, 4};                    
                    std::vector<double> target = {10,11};
                    if(!mequals(mmult(2,2,1,start1,start2),target))
                    {
                        gstd::reportFailure("gstd::Linalg::mmult");
                        return false;
                    }
				}

				//mprod
				{
					std::vector<double> start1 = { 1, -2, 3 };
					std::vector<double> start2 = { 10, -20, 30 };
					std::vector<double> target = { 10, 40, 90 };
					if (!mequals(mprod(start1, start2), target))
					{
						gstd::reportFailure("gstd::Linalg::mprod");
						return false;
					}
				}

				//mscale
				{
					std::vector<double> start = { 1, 2, 3 };
					std::vector<double> target = { -0.5, -1, -1.5 };
					if (!mequals(mscale(start, -0.5), target))
					{
						gstd::reportFailure("gstd::Linalg::mscale");
						return false;
					}
				}
                
                //msumrows
                {
                    std::vector<double> start = {1, 2, 3, 4, 5, 6, 7, 8};
                    std::vector<double> target = {3, 7, 11, 15};
                    if(!mequals(msumrows(4,2,start),target))
                    {
                        gstd::reportFailure("gstd::Linalg::msumrows");
                        return false;
                    }
                }

                //msumcols
                {
                    std::vector<double> start = {1, 2, 3, 4, 5, 6, 7, 8};
                    std::vector<double> target = {16, 20};
                    if(!mequals(msumcols(4,2,start),target))
                    {
                        gstd::reportFailure("gstd::Linalg::msumcols");
                        return false;
                    }
                }
                
                //msumall
                {
                    std::vector<double> start = {1, 2, 3, 4, 5, 6, 7, 8};
                    std::vector<double> target = {36};
                    if(!mequals(msumall(start),target))
                    {
                        gstd::reportFailure("gstd::Linalg::msumall");
                        return false;
                    }
                }

                //mdotprod
                {
                    std::vector<double> start1 = {1,-2,3};
                    std::vector<double> start2 = {10,-20,30};                    
                    std::vector<double> target = {140};
                    if(!mequals(mdotprod(start1,start2),target))
                    {
                        gstd::reportFailure("gstd::Linalg::mdotprod");
                        return false;
                    }
                }

                //submatrix
                {
                    std::vector<double> start = {11,12,13,14,21,22,23,24,31,32,33,34,41,42,43,44,51,52,53,54};
                    std::vector<double> target = {32,33,42,43,52,53};
                    if(!mequals(submatrix(5,4,start,2,1,-1,2),target))
                    {
                        gstd::reportFailure("gstd::Linalg::submatrix");
                        return false;
                    }
                }

                //submatrix (overload)
                {
                    std::vector<double> start = {11,12,13,14,21,22,23,24,31,32,33,34,41,42,43,44,51,52,53,54};
                    std::tuple<int, int, std::vector<double>> target = std::make_tuple(3, 2, std::vector<double>({33, 34, 43, 44, 53, 54}));
                    std::tuple<int, int, std::vector<double>> res = submatrix(5,4,start,2,2,10,10,true);
                    if(res != target)
                    {
                        gstd::reportFailure("gstd::Linalg::submatrix (overload)");
                        return false;
                    }
                }
                
                //mcoljoin
                {
                    std::vector<double> input1 = {1,2,3,4};
                    std::vector<double> input2 = {5,6,7,8};
                    std::vector<double> target = {1,5,2,6,3,7,4,8};
                    if(mcoljoin({input1, input2}) != target)
                    { 
                        gstd::reportFailure("gstd::Linalg::mcoljoin");
                        return false;
                    }
				}

				//minftynorm
				{
					std::vector<double> input = { 0, 1, -2, 3, -4 };
					double target = 4;
					double out = minftynorm(input);
					if (!gstd::Double::equals(target, out))
					{
						gstd::reportFailure("gstd::Linalg::minftynorm");
						return false;
					}
				}

				//transpose
				{
					std::vector<double> input = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
					std::vector<double> target = { 1, 4, 7, 10, 2, 5, 8, 11, 3, 6, 9, 12 };
					if (transpose(4, 3, input) != target)
					{
						gstd::reportFailure("gstd::Linalg::transpose");
						return false;
					}
				}

				//cholesky
				{
					int dim = 7;
					std::vector<double> target(dim*dim, 0);
					for (int i = 0; i < dim; i++)
					{
						for (int j = 0; j < dim; j++)
						{
							if (j > i)
								target[i*dim + j] = gstd::Rand::d(3) - 0.5;
							else if (j == i)
								target[i*dim + j] = gstd::Rand::d(3);
							else
								target[i*dim + j] = 0;
						}
					}
                                        std::vector<double> transposed = transpose(dim, dim, target);
					std::vector<double> input = mmult(dim, dim, dim, transposed, target);
					std::vector<double> out = cholesky(input);
					if (!mequals(out, target, 1e-10, false))
					{
						gstd::reportFailure("gstd::Linalg::cholesky");
						return false;
					}
				}

               
                return true;
            }
        }
        
        
    }
}

