/* 
 * File:   Map.cpp
 * Author: gschoenh
 * 
 * Created on December 6, 2013, 3:32 PM
 */

#include "stdafx.h"

#include "Map.h"
#include "Printer.h"
#include "Vector.h"

namespace msii810161816
{   
    namespace gstd
    {
        namespace map
        {
            bool test()
            {
                gstd::Printer::c("This is the test for the gstd::map group of functions");
                
                //keys
                {
                    {
                        std::map<int, double> input;
                        input[0] = 0.0;
                        input[1] = 1.1;
                        input[2] = 2.2;
                        std::vector<int> target = {0,1,2};
                        if(keys(input) != target)
                        {
                            gstd::reportFailure("gstd::map::keys");
                            return false;
                        }
                    }
                }
                
                //values
                {
                    {
                        std::map<int, double> input;
                        input[0] = 0.0;
                        input[1] = 1.1;
                        input[2] = 2.2;
                        std::vector<double> target = {0.0,1.1,2.2};
                        if(values(input) != target)
                        {
                            gstd::reportFailure("gstd::map::values");
                            return false;
                        }
                    }
				}

				//create (single vector)
				{
					{
						std::map<double, bool> target;
						target[0.0] = true;
						target[1.1] = true;
						target[2.2] = true;
						std::vector<double> input = { 0.0, 1.1, 2.2 };
						if (create(input) != target)
						{
							gstd::reportFailure("gstd::map::create (single vector)");
							return false;
						}
					}
				}

				//craete (two vectors)
				{
					std::map<int, double> target;
					target[0] = 0;
					target[1] = 1.1;
					target[2] = 2.2;
					target[3] = 3.3;
					std::vector<int> input = { 2, 1, 3, 0 };
					std::vector<double> output = { 2.2, 1.1, 3.3, 0 };
					if (create(input, output) != target)
					{
						gstd::reportFailure("gstd::map::create (two vectors)");
						return false;
					}				
				}

                
                //invertVector
                {
                    {
                        std::map<double, int> target;
                        target[0.0] = 0;
                        target[1.1] = 1;
                        target[2.2] = 2;
                        std::vector<double> input = {0.0,1.1,2.2};
                        if(invertVector(input) != target)
                        {
                            gstd::reportFailure("gstd::map::invertVector");
                            return false;
                        }
                    }
                }
                
                //submap
                {
                    {
                        std::map<double, int> input;
                        input[0.0] = 0;
                        input[1.1] = 1;
                        input[2.2] = 2;
                        input[3.3] = 3;
                        input[4.4] = 4;
                        input[5.5] = 5;
                        std::map<double, int> res = submap(input, 3);
                        std::vector<int> vals = values(res);
                        std::vector<int> unique = gstd::vector::unique(vals);
                        if((int)unique.size() != 3)
                        {
                           gstd::reportFailure("gstd::map::submap, test 1");
                            return false;
                        }
                        typedef std::map<double, int>::iterator Iterator;
                        for(Iterator i=res.begin();i!=res.end();i++)
                            if(input[i->first] != i->second)
                            {
                                gstd::reportFailure("gstd::map::submap, test 2");
                                return false;
                            }
                    }
				}

				//getCounts
				{
					std::vector<int> input = { 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6, 1, 2, 3 };
					std::map<int, int> target;
					target[1] = 3;
					target[2] = 3;
					target[3] = 3;
					target[4] = 2;
					target[5] = 2;
					target[6] = 2;
					std::map<int, int> res = getCounts(input);
					if (res != target)
					{
						gstd::reportFailure("gstd::map::getCounts");
						return false;
					}
				}

				//tick
				{
					std::map<std::string, int> input, target;
					target["a"] = 1;
					input = tick<std::string>(input, "a");
					if (input != target)
					{
						gstd::reportFailure("gstd::map::tick");
						return false;
					}
					target["a"] = 2;
					input = tick<std::string>(input, "a");
					if (input != target)
					{
						gstd::reportFailure("gstd::map::tick");
						return false;
					}
					target["b"] = 1;
					input = tick<std::string>(input, "b");
					if (input != target)
					{
						gstd::reportFailure("gstd::map::tick");
						return false;
					}
				}
                
                return true;            
            }           
        }
    }
}
