/* 
 * File:   Double.cpp
 * Author: gschoenh
 * 
 * Created on November 15, 2013, 3:10 PM
 */

#include "stdafx.h"

#include "Primitives.h"
#include "Printer.h"
#include "Rand.h"
#include <math.h>
#include <cmath>
#include <iomanip>
namespace msii810161816
{   
    namespace gstd
    {   
        namespace Double
        {
            double abs(double x)
            {
                if(x >= 0)
                    return x;
                else
                    return -x;
            }
            
            bool equals(double x, double y, double margin /*=1e-15*/, bool relative /*=true*/, bool doc)
            {
                if(!relative)
                {
                    if( abs(x-y) > margin )
                        return false;
                    else
                        return true;
                }
                
                double xabs = abs(x);
                double yabs = abs(y);
                double max = xabs;
                if(yabs > xabs)
                    max = yabs;
                
                if(max < 1e-100)
                    return true;
                double diff = abs(x-y);
                
                if(doc)
                {
                    gstd::Printer::vc(std::vector<double>({x,y,max,diff}));
                }
                
                if( diff / max > margin )
                    return false;
                else
                    return true;
            }
            
            double clipPrecision(double x)
            {
                return round(x*1e15)*1e-15;
            }
            
            bool test()
            {
                gstd::Printer::c("This is the gstd::Doubles group of functions test");
                
                //abs
                {
                    double x = -3.0;
                    if(abs(x) != 3.0)
                    {
                        gstd::reportFailure("gstd::Double::abs");
                        return false;
                    }
                }
                
                //equals
                {
                    double x = 10.0;
                    double y = 10.0000000002;
                    double z = 1e-10;
                    double w = 1e-200;
                    if(equals(x,y)
                            ||equals(x,y,1e-11)
                            ||!equals(x,y,1e-10)
                            ||equals(x,y,1e-10, false)
                            ||!equals(x,y,1e-9, false)
                            ||equals(z,0)
                            ||!equals(w,0)
                            )
                    {
                        gstd::reportFailure("gstd::Double::equals");
                        return false;
                    }                  
                }
                
                //clipPrecision
                {
                    int numtests = 1000;
                    for(int i=0;i<numtests;i++)
                    {
                        double x = gstd::Rand::d(4)*1e-4;
                        double clipped = clipPrecision(x);
                        if(abs(clipped - x) > 1e-15 || x == clipped || clipPrecision(1e-16) != 0)
                        {
                            gstd::reportFailure("gstd::Double::clipPrecision");
                            return false;
                        }
                    }
                }
                
                return true;
            }
        }
        
		namespace Int
		{
			int create(double x, std::string mode)
			{
				double safety = 0.1;
				if (x < 0)
					safety = -0.1;
				if (mode == "round")
					return (int)(std::round(x) + safety);
				else if (mode == "down")
					return (int)(std::floor(x) + safety);
				else if (mode == "up")
					return (int)(std::ceil(x) + safety);
				else
					gstd::error("unknown mode");
				return 0;
			}

			std::pair<int, double> splitReal(double x)
			{
				double bottom = std::floor(x);
				double rest = x - bottom;
				gstd::check(0 <= rest && rest < 1, "splitting of double failed");
				int intBottom = Int::create(bottom, "round");
				return std::pair<int, double>(intBottom, rest);
			}

			bool test()
			{
				gstd::Printer::c("This is a test for gstd::Int group of functions");

				//create
				{
					if (create(3, "down") != 3 || create(3.9, "down") != 3 || create(3, "up") != 3 || create(2.1, "up") != 3 || create(2.9, "round") != 3 || create(3.1, "round") != 3)
					{
						gstd::reportFailure("gstd::Int::create, test 1");
						return false;
					}
					if (create(-3, "down") != -3 || create(-2.1, "down") != -3 || create(-3, "up") != -3 || create(-3.9, "up") != -3 || create(-2.9, "round") != -3 || create(-3.1, "round") != -3)
					{
						gstd::reportFailure("gstd::Int::create, test 2");
						return false;
					}
				}
				
				//splitReal
				{
					std::pair<int, double> res1 = splitReal(-2.1);
					std::pair<int, double> res2 = splitReal(3);
					if (res1.first != -3 || !Double::equals(res1.second, 0.9) || res2.first != 3 || !Double::equals(res2.second, 0) || res2.second < 0)
					{
						gstd::reportFailure("gstd::Int::splitReal");
						return false;
					}
				}


				return true;
			}

		}
    }
}
