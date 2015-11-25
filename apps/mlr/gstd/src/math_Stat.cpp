/* 
 * File:   Stat.cpp
 * Author: gschoenh
 * 
 * Created on November 14, 2013, 8:14 PM
 */

#include "stdafx.h"

#define _USE_MATH_DEFINES
#include"standard.h"
#include"Printer.h"
#include "Stat.h"
#include "Rand.h"
#include "ex.h"
#include <math.h> 
#include<algorithm>
#include "Primitives.h"
#include "Linalg.h"
#include "Vector.h"

namespace msii810161816
{   
    namespace gstd
    {      
        namespace stat
        { 
            
            //****************************************************************************80

            double normcum ( double x )

            //****************************************************************************80
            //
            //  Purpose:
            //
            //    ALNORM computes the cumulative density of the standard normal distribution.
            //
            //  Licensing:
            //
            //    This code is distributed under the GNU LGPL license. 
            //
            //  Modified:
            //
            //    17 January 2008
            //
            //  Author:
            //
            //    Original FORTRAN77 version by David Hill.
            //    C++ version by John Burkardt.
            //
            //  Reference:
            //
            //    David Hill,
            //    Algorithm AS 66:
            //    The Normal Integral,
            //    Applied Statistics,
            //    Volume 22, Number 3, 1973, pages 424-427.
            //
            //  Parameters:
            //
            //    Input, double X, is one endpoint of the semi-infinite interval
            //    over which the integration takes place.
            //
            //    Input, bool UPPER, determines whether the upper or lower
            //    interval is to be integrated:
            //    .TRUE.  => integrate from X to + Infinity;
            //    .FALSE. => integrate from - Infinity to X.
            //
            //    Output, double ALNORM, the integral of the standard normal
            //    distribution over the desired interval.
            //
            {
              double a1 = 5.75885480458;
              double a2 = 2.62433121679;
              double a3 = 5.92885724438;
              double b1 = -29.8213557807;
              double b2 = 48.6959930692;
              double c1 = -0.000000038052;
              double c2 = 0.000398064794;
              double c3 = -0.151679116635;
              double c4 = 4.8385912808;
              double c5 = 0.742380924027;
              double c6 = 3.99019417011;
              double con = 1.28;
              double d1 = 1.00000615302;
              double d2 = 1.98615381364;
              double d3 = 5.29330324926;
              double d4 = -15.1508972451;
              double d5 = 30.789933034;
              double ltone = 7.0;
              double p = 0.398942280444;
              double q = 0.39990348504;
              double r = 0.398942280385;
              bool up;
              double utzero = 18.66;
              double value;
              double y;
              double z;

              up = false;
              z = x;

              if ( z < 0.0 )
              {
                up = !up;
                z = - z;
              }

              if ( ltone < z && ( ( !up ) || utzero < z ) )
              {
                if ( up )
                {
                  value = 0.0;
                }
                else
                {
                  value = 1.0;
                }
                return value;
              }

              y = 0.5 * z * z;

              if ( z <= con )
              {
                value = 0.5 - z * ( p - q * y 
                  / ( y + a1 + b1 
                  / ( y + a2 + b2 
                  / ( y + a3 ))));
              }
              else
              {
                value = r * exp ( - y ) 
                  / ( z + c1 + d1 
                  / ( z + c2 + d2 
                  / ( z + c3 + d3 
                  / ( z + c4 + d4 
                  / ( z + c5 + d5 
                  / ( z + c6 ))))));
              }

              if ( !up )
              {
                value = 1.0 - value;
              }

              return value;
            }
            
            
            
            double normcuminv_polyhelper ( int n, double a[], double x )
            {
              int i;
              double value;

              value = 0.0;

              for ( i = n-1; 0 <= i; i-- )
              {
                value = value * x + a[i];
              }

              return value;
            }            

            double normcuminv ( double p ) //from http://people.sc.fsu.edu/~jburkardt/c_src/asa241/asa241.html
            //
            //  Purpose:
            //
            //    R8_NORMAL_01_CDF_INVERSE inverts the standard normal CDF.
            //
            //  Discussion:
            //
            //    The result is accurate to about 1 part in 10**16.
            //
            //  Licensing:
            //
            //    This code is distributed under the GNU LGPL license. 
            //
            //  Modified:
            //
            //    19 March 2010
            //
            //  Author:
            //
            //    Original FORTRAN77 version by Michael Wichura.
            //    C++ version by John Burkardt.
            //
            //  Reference:
            //
            //    Michael Wichura,
            //    The Percentage Points of the Normal Distribution,
            //    Algorithm AS 241,
            //    Applied Statistics,
            //    Volume 37, Number 3, pages 477-484, 1988.
            //
            //  Parameters:
            //
            //    Input, double P, the value of the cumulative probability 
            //    densitity function.  0 < P < 1.  If P is outside this range, an "infinite"
            //    value is returned.
            //
            //    Output, double R8_NORMAL_01_CDF_INVERSE, the normal deviate value 
            //    with the property that the probability of a standard normal deviate being 
            //    less than or equal to this value is P.
            //
            {
              static double a[8] = { 
                3.3871328727963666080,     1.3314166789178437745e+2,
                1.9715909503065514427e+3,  1.3731693765509461125e+4,
                4.5921953931549871457e+4,  6.7265770927008700853e+4,
                3.3430575583588128105e+4,  2.5090809287301226727e+3 };
              static double b[8] = {
                1.0,                       4.2313330701600911252e+1,
                6.8718700749205790830e+2,  5.3941960214247511077e+3,
                2.1213794301586595867e+4,  3.9307895800092710610e+4,
                2.8729085735721942674e+4,  5.2264952788528545610e+3 };
              static double c[8] = {
                1.42343711074968357734,     4.63033784615654529590,
                5.76949722146069140550,     3.64784832476320460504,
                1.27045825245236838258,     2.41780725177450611770e-1,
                2.27238449892691845833e-2,  7.74545014278341407640e-4 };
              static double const1 = 0.180625;
              static double const2 = 1.6;
              static double d[8] = {
                1.0,                        2.05319162663775882187,
                1.67638483018380384940,     6.89767334985100004550e-1,
                1.48103976427480074590e-1,  1.51986665636164571966e-2,
                5.47593808499534494600e-4,  1.05075007164441684324e-9 };
              static double e[8] = {
                6.65790464350110377720,     5.46378491116411436990,
                1.78482653991729133580,     2.96560571828504891230e-1,
                2.65321895265761230930e-2,  1.24266094738807843860e-3,
                2.71155556874348757815e-5,  2.01033439929228813265e-7 };
              static double f[8] = {
                1.0,                        5.99832206555887937690e-1,
                1.36929880922735805310e-1,  1.48753612908506148525e-2,
                7.86869131145613259100e-4,  1.84631831751005468180e-5,
                1.42151175831644588870e-7,  2.04426310338993978564e-15 };
              double q;
              double r;
              static double split1 = 0.425;
              static double split2 = 5.0;
              double value;

              if ( p <= 0.0 || 1.0 <= p )
                  gstd::error("invalid input");

              q = p - 0.5;

              if ( Double::abs ( q ) <= split1 )
              {
                r = const1 - q * q;
                value = q * normcuminv_polyhelper ( 8, a, r ) / normcuminv_polyhelper ( 8, b, r );
              }
              else
              {
                if ( q < 0.0 )
                {
                  r = p;
                }
                else
                {
                  r = 1.0 - p;
                }

                r = sqrt ( -log ( r ) );

                if ( r <= split2 )
                {
                  r = r - const2;
                  value = normcuminv_polyhelper ( 8, c, r ) / normcuminv_polyhelper ( 8, d, r ); 
                 }
                 else
                 {
                   r = r - split2;
                   value = normcuminv_polyhelper ( 8, e, r ) / normcuminv_polyhelper ( 8, f, r );
                }

                if ( q < 0.0 )
                {
                  value = -value;
                }

              }

              return value;
            }
            
            double randnorm(int mode)
            {
				double uniformRand = gstd::Rand::d(mode);
                double lowerRand = normcuminv(uniformRand * 0.5);
                int upper = gstd::Rand::i(2);
                if(upper == 0)
                    return lowerRand;
                else
                    return -lowerRand;
            }

            std::vector<double> covarianceMatrix(int dim1, int dim2, std::vector<double>& input)
            {
                gstd::check(dim1*dim2 == (int)input.size(), "received input of incorrect dimension");
                std::vector<double> means(dim2, 0);
                for(int i=0;i<dim2;i++)
                {
                    for(int n=0;n<dim1;n++)
                        means[i] += input[n*dim2+i];
                    means[i] /= (double)dim1;
                }                
                std::vector<double> result(dim2*dim2);
                for(int i=0;i<dim2;i++)
                    for(int j=0;j<dim2;j++)
                    {
                        for(int n=0;n<dim1;n++)
                            result[i*dim2+j] += input[n*dim2+i]*input[n*dim2+j];
                        result[i*dim2+j] /= (double)dim1;
                        result[i*dim2+j] -= means[i]*means[j];
                    }
                return result;            
            } 
            
            std::pair<double,double> meanAndVar(std::vector<double> input)
            {
                std::pair<double, double> res;
                int size = input.size();
                for(int i=0;i<size;i++)
                {
                    res.first += input[i];
                    res.second += input[i]*input[i];
                }
                res.first /= (double)size;
                res.second /= (double) (size-1);
                res.second -= ((double)size) / ((double)size-1) * res.first * res.first;
                return res;
            }

			double meanPval(double targetMean, std::vector<double> vals)
			{
				int size = vals.size();
				double mean, var;
				std::tie(mean, var) = meanAndVar(vals);
				double stdev = sqrt(var / ((double)size));
				double numStds = Double::abs(mean - targetMean) / stdev;
				double p = normcum(-numStds) * 2;
				return p;
			}

			double varPval(double targetVariance, std::vector<double> vals)
			{
				int size = vals.size();
				double dsize = (double)size;
				double mean, var;
				std::tie(mean, var) = meanAndVar(vals);
				(void)var;
				for (int i = 0; i < size; i++)
					vals[i] = (vals[i] - mean)*(vals[i] - mean);
				targetVariance *= (dsize - 1) / dsize;
				return meanPval(targetVariance, vals);
			}

			double combineTestPs(std::vector<double> inputs)
			{
				int size = inputs.size();
				std::sort(inputs.begin(), inputs.end());
				std::vector<double> cdfChords;
				for (int i = 0; i < size; i++)
				{
					double fraction = ((double)i+1) / ((double)size);
					cdfChords.push_back(fraction / inputs[i]);
				}
				double maxChord = gstd::vector::vmax(cdfChords).second;
				double res = 3.996 / maxChord;
				return res;
			}
            
            bool test()
            {
                gstd::Printer::c("This is the test for the gstd::stat group of functions");
                
                //normcum
                {
                    std::vector<double> testpoints = {-5,-4,-3,-2,-1,1,2,3,4};
                    int size = testpoints.size();
                    for(int i=0;i<size;i++)
                    {
                        double deriv = (normcum(testpoints[i]+0.5e-5) - normcum(testpoints[i]-0.5e-5))*1e5;
                        double compare = 1/sqrt(2*M_PI)*exp(-testpoints[i]*testpoints[i]/2);
                        if(!gstd::Double::equals(deriv,compare,1e-7,true)) // gradient is a bit wobbly
                        {
                            gstd::reportFailure("gstd::stat::normcum");
                            return false;
                        }
                    }                    
                }
                
                //normcuminv
                {
                    std::vector<double> testpoints = {1e-50,1e-40,1e-30,1e-20,1e-10,1e-7,1e-4,1e-1,0.3,0.7,1-1e-1,1-1e-4,1-1e-7,1-1e-10};
                    int size = testpoints.size();
                    for(int i=0;i<size;i++)
                    {
                        double output = normcum(normcuminv(testpoints[i]));
                        if(!gstd::Double::equals(testpoints[i], output, 1e-8, true)) //this is not as good as I had hoped (at least is doesn't do worse on the tails)
                        {
                            gstd::reportFailure("gstd::stat::normcuminv");
                            return false;
                        }
                    }
                }
                
                //randnorm
                {
                    double first = 0;
                    double second = 0;
                    double fourth = 0;
                    for(int k=0;k<10000;k++)
                    {
                        double val = randnorm(4);
                        first += val;
                        second += val*val;
                        fourth += val*val*val*val;
                    }
                    first /= 10000;
                    second /= 10000;
                    fourth /= 10000;
                    
                    gstd::Printer::c(first);
                    gstd::Printer::c(second);
                    gstd::Printer::c(fourth/second/second-3);
                    
                    if(!gstd::Double::equals(first, 0, 5.0/100, false)
                            || !gstd::Double::equals(second, 1, 5.0/100.0, false)
                            || !gstd::Double::equals(fourth/second/second-3, 0, 0.2, false)
                        )
                    {
                        gstd::reportFailure("gstd::stat::randnorm");
                        return false;
                    }
                }      
                
                //CovarianceMatrix
                {
                    std::vector<double> datamatrix = {2, 0, 0, 4, 2, 4, 0, 0};
                    if(!gstd::Linalg::mequals(covarianceMatrix(4,2,datamatrix), {1,0,0,4}))
                    {
                        gstd::reportFailure("gstd::stat::covarianceMatrix");
                        return false;
                    }
				}

				//meanAndVar
				{
					std::vector<double> datavector = { 1, 1, 1, 3, 3, 3 };
					std::pair<double, double> meanVar = meanAndVar(datavector);
					if (!gstd::Double::equals(meanVar.first, 2.0)
						|| !gstd::Double::equals(meanVar.second, 6.0/5.0)
						)
					{
						gstd::reportFailure("gstd::stat::meanAndVar");
						return false;
					}
				}

				//meanPval, varPval
				{
					std::vector<int> mbins(10,0);
					std::vector<int> vbins(10,0);
					for (int i = 0; i < 10000;i++)
					{
						std::vector<double> testvec;
						for (int j = 0; j < 20; j++)
							testvec.push_back(gstd::Rand::d(3));
						double mp = meanPval(0.5, testvec);
						double vp = varPval(1.0/12.0, testvec);
						for (int j = 1; j <= 10; j++)
						{
							if (mp*10.0 < (double)j)
							{
								mbins[j-1]++;
								break;
							}								
						}
						for (int j = 1; j <= 10; j++)
						{
							if (vp*10.0 < (double)j)
							{
								vbins[j-1]++;
								break;
							}
						}
					}
					for (int j = 0; j < 10; j++)
					{
						if (mbins[j] < 700 || mbins[j] > 1300 || vbins[j] < 700 || vbins[j] > 1300)
						{
							gstd::reportFailure("gstd::stat::meanPval or gstd::stat::varPval");
							return false;
						}
					}
				}

				//combineTestPs
				{
					std::vector<double> input1 = { 0.01, 0.015, 0.02, 0.02,  0.03, 0.03, 1, 0.015, 0.02, 0.02 };
					std::vector<double> input2 = input1;
					input2[6] = 0.021;
					double res1 = combineTestPs(input1);
					double res2 = combineTestPs(input2);
					double target1 = 3.996 / 0.7*0.02;
					double target2 = 3.996 / 0.8*0.021;
					if (!gstd::Double::equals(res1,target1,1e-12) || !gstd::Double::equals(res2,target2,1e-12))
					{
						gstd::reportFailure("gstd::stat::combineTestPs");
						return false;
					}
				}
                
                return true;
            }
        }  
    }
}

