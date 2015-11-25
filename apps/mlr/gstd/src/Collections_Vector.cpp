/* 
 * File:   Vector.cpp
 * Author: gschoenh
 * 
 * Created on November 15, 2013, 4:21 PM
 */

#include "stdafx.h"

#include "Vector.h"
#include "Printer.h"
#include "Primitives.h"
#include <set>


namespace msii810161816
{   
    namespace gstd
    {
        namespace vector
        {
            std::vector<int> invertToVector(std::vector<int>& input, int maxkey)
            {
                int size = input.size();
                if(maxkey == 0)
                    for(int i=0;i<size;i++)
                        if(input[i] > maxkey)
                            maxkey = input[i];
                std::vector<int> result(maxkey+1,-1);
                for(int i=0;i<size;i++)
                    if(input[i] >= 0)
                        result[input[i]] = i;
                return result;                
            }

			std::vector<double> stod(std::vector<std::string> input)
			{
				int size = input.size();
				std::vector<double> res(size);
				for (int i = 0; i < size; i++)
					res[i] = std::stod(input[i]);
				return res;
			}

			std::vector<std::string> dtos(std::vector<double> input)
			{
				int size = input.size();
				std::vector<std::string> res(size);
				for (int i = 0; i < size; i++)
					res[i] = gstd::Printer::p(input[i]);
				return res;
			}
			
			std::vector<std::vector<double> > stod2d(std::vector<std::vector<std::string> > input)
			{
				int size = input.size();
				std::vector<std::vector<double> > res(size);
				for (int i = 0; i < size; i++)
					res[i] = gstd::vector::stod(input[i]);
				return res;
			}

			std::vector<std::vector<std::string> > dtos2d(std::vector<std::vector<double> > input)
			{
				int size = input.size();
				std::vector<std::vector<std::string> > res(size);
				for (int i = 0; i < size; i++)
					res[i] = gstd::vector::dtos(input[i]);
				return res;
			}

			std::vector<int> dtoi(std::vector<double> input, std::string mode)
			{
				std::vector<int> res;
				int size = input.size();
				for (int i = 0; i < size; i++)
					res.push_back(Int::create(input[i], mode));
				return res;
			}

			std::vector<double> itod(std::vector<int> input)
			{
				std::vector<double> res;
				int size = input.size();
				for (int i = 0; i < size; i++)
					res.push_back((double)input[i]);
				return res;
			}
            
            bool test()
            {
                gstd::Printer::c("This is the test for the gstd::vector group of functions");
                                
                //create
                {
                    std::map<int, double> map;
                    map[1] = 1.1;
                    map[3] = 2.2;
                    std::vector<double> res = create(map);
                    if((int)res.size() != 4 || res[1] != 1.1 || res[3] != 2.2 )
                    {
                        gstd::reportFailure("gstd::vector::create");
                        return false;
                    }
                }
                
                //getmany
                {
                    std::vector<int> vals = {1,2,3,4,5};
                    std::vector<int> keys = {0,2,4};
                    std::vector<int> target = {1,3,5};
                    if(getmany(vals, keys) != target)
                    {
                        gstd::reportFailure("gstd::vector::getmany");
                        return false;
                    }
                }
                
                //invertToVector
                {
                    std::vector<int> start = {6,3,2,-1,-1,5,-1,0};
                    std::vector<int> inverse = {7,-1,2,1,-1,5,0};
                    std::vector<int> inverted = invertToVector(start);
                    std::vector<int> invertedtwice = invertToVector(inverted);
                    if( inverted != inverse || invertedtwice != start)
                    {
                        gstd::reportFailure("gstd::vector::invertToVector");
                        return false;
                    }                                        
                }
                
                //empty
                {
                    std::vector<int> start = {1,2,3};
                    empty(start);
                    if(start.size() != 0)
                    {
                        gstd::reportFailure("gstd::vector::empty");
                        return false;
                    } 
                }
                
                //unique
                {
                    std::vector<int> start = {1,2,3,4,1,2,3,4,1,2,6,7};
                    std::vector<int> res = unique(start);
                    std::sort(res.begin(),res.end());
                    std::vector<int> target = {1,2,3,4,6,7};
                    if(res != target)
                    {
                        gstd::reportFailure("gstd::vector::unique");
                        return false;
                    }  
                }
                
                //insertIndeces
                {
                    std::vector<int> start = {1,2,3};
                    std::vector<std::pair<int, int>> target;
                    std::pair<int, int> p1;
                    p1.first = 0;
                    p1.second = 1;
                    target.push_back(p1);
                    p1.first = 1;
                    p1.second = 2;
                    target.push_back(p1);
                    p1.first = 2;
                    p1.second = 3;
                    target.push_back(p1);
                    std::vector<std::pair<int, int>> res = insertIndeces(start);
                    if(res != target)
                    {
                        gstd::reportFailure("gstd::vector::insertIndeces");
                        return false;
                    } 
                }
                
                //sort
                {
                    std::vector<int> start = {4,2,70,3,800,5,6,-1};
                    std::vector<int> targetinds = {4,2,6,5,0,3,1,7};
                    std::vector<int> targetvals = {800,70,6,5,4,3,2,-1};
                    std::pair<std::vector<int>,std::vector<int>> res = sort(start, true);
                    if(res.first != targetinds || res.second != targetvals)
                    {
                        gstd::reportFailure("gstd::vector::sort");
                        return false;
                    }    
                }
                
                //max
                {
                    std::vector<int> start = {4,2,3,7,1,5,2,3,5};
                    std::pair<int, int> _max = vmax(start);
                    if(_max.first != 3 || _max.second != 7)
                    {
                        gstd::reportFailure("gstd::vector::max");
                        return false;
                    }  
                }
                
                //min
                {
                    std::vector<int> start = {4,2,3,7,1,5,2,3,5};
                    std::pair<int, int> _min = vmin(start);
                    if(_min.first != 4 || _min.second != 1)
                    {
                        gstd::reportFailure("gstd::vector::min");
                        return false;
                    }  
                }
                
                //sub
                {
                    std::vector<int> start = {4,2,3,7,1,5,2,3,5};
                    std::vector<int> target1 = {7,1,5};
                    std::vector<int> target2 = {7,1,5,2,3,5};    
                    if(sub(start, 3, 3) != target1 || sub(start, 3, 10, true) != target2)
                    {
                        gstd::reportFailure("gstd::vector::sub");
                        return false;
                    }                        
				}

				//rank
				{
					std::vector<double> start = { 1.1, 2.2, 3.3, 2.2, 3.3, 1.1, 4.4, 2.2, 1.1, 2.2 };
					std::vector<double> start2 = { -1.1, -2.2, -3.3, -2.2, -3.3, -1.1, -4.4, -2.2, -1.1, -2.2 };
					std::vector<double> target = { 8, 4.5, 1.5, 4.5, 1.5, 8, 0, 4.5, 8, 4.5 };
					if (rank(start, false) != target || rank(start2, true) != target)
					{
						gstd::reportFailure("gstd::vector::rank");
						return false;
					}
				}

				//rankIndeces
				{
					std::vector<double> start = { 1.1, 2.2, 3.3, 2.2, 3.3, 1.1, 4.4, 2.2, 1.1, 2.2 };
					std::vector<std::vector<int> > target = { { 6 }, { 2, 4 }, { 1, 3, 7, 9 }, { 0, 5, 8 } };
					std::vector<std::vector<int> > res = rankIndeces(start);
					for(int i=0;i<(int)res.size();i++)
						res[i] = sort(res[i]).second;
					if (res != target)
					{
						gstd::reportFailure("gstd::vector::rankIndeces");
						return false;
					}
				}

				//apply
				{
					std::function<int(int)> addone = [](int in){return in + 1; };
					std::vector<int> start = { 1, 2, 3 };
					std::vector<int> target = { 2, 3, 4 };
					if (apply(addone, start) != target)
					{
						gstd::reportFailure("gstd::vector::apply");
						return false;
					}
				}

				//apply2D
				{
					std::function<int(int)> addfive = [](int in){return in + 5; };
					std::vector<std::vector<int> > start = { { 1, 2, 3 }, { 4, 5 } };
					std::vector<std::vector<int> > target = { { 6, 7, 8 }, { 9, 10 } };
					if (apply2d(addfive, start) != target)
					{
						gstd::reportFailure("gstd::vector::apply2d");
						return false;
					}
				}

				//shuffle
				{
					std::vector<int> input = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
					std::set<int> inSet = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
					std::vector<int> output = shuffle(input);
					std::set<int> outSet;
					for (int i = 0; i < (int)output.size(); i++)
						outSet.insert(output[i]);
					if (input == output || inSet != outSet)
					{
						gstd::reportFailure("gstd::vector::shuffle");
						return false;
					}
				}

				//stod, dtos
				{
					std::vector<std::string> stringVec = { "1.1", "2.2", "3.3" };
					std::vector<double> doubleVec = { 1.1, 2.2, 3.3 };
					std::vector<double> doubleTarget = stod(stringVec);
					std::vector<std::string> stringTarget = dtos(doubleVec);
					if (stringVec != stringTarget || doubleVec != doubleTarget)
					{
						gstd::reportFailure("gstd::vector::stod or gstd::vector::dtos");
						return false;
					}
				}

				//stod2d, dtos2d
				{
					std::vector<std::vector<std::string> > start = { { "1.1", "2.2", "3.3" }, { "4", "5", "6" } };
					std::vector<std::vector<double> > target = { { 1.1, 2.2, 3.3 }, { 4, 5, 6 } };
					if (stod2d(start) != target || dtos2d(target) != start)
					{
						gstd::reportFailure("gstd::vector::stod2d or gstd::vector::dtos2d");
						return false;
					}
				}

				//dtoi, itod
				{
					std::vector<int> iVec = { 1, 2, 3 };
					std::vector<double> dVec = { 1, 2, 3 };
					if (iVec != dtoi(dVec, "round") || dVec != itod(iVec))
					{
						gstd::reportFailure("gstd::vector::dtos or gstd::vector::itod");
						return false;
					}
				}
                
                return true;     
            }
        }
    }
}

