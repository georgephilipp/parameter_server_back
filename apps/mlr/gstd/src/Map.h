/* 
 * File:   Map.h
 * Author: gschoenh
 *
 * Created on December 6, 2013, 3:32 PM
 */

#include "stdafx.h"

#ifndef MAP_H
#define	MAP_H
#include "standard.h"
#include "ex.h"
#include<map>


namespace msii810161816
{   
    namespace gstd
    {
        namespace map
        {
            template<typename keytype, typename valtype> std::vector<keytype> keys(std::map<keytype,valtype> input)
            {
                typedef typename std::map<keytype, valtype>::iterator Iterator;
                std::vector<keytype> res;
                for(Iterator i=input.begin();i!=input.end();i++)
                    res.push_back(i->first);
                return res;                
            }
            
            template<typename keytype, typename valtype> std::vector<valtype> values(std::map<keytype,valtype> input)
            {
                typedef typename std::map<keytype, valtype>::iterator Iterator;
                std::vector<valtype> res;
                for(Iterator i=input.begin();i!=input.end();i++)
                    res.push_back(i->second);
                return res;                
            }  
                        
            template<typename type> std::map<type, bool> create(std::vector<type> input)
            {
                std::map<type, bool> res;
                int size = input.size();
                for(int i=0;i<size;i++)
                    res[input[i]] = true;
                return res;
            } 

			template<typename keytype, typename valtype> std::map<keytype, valtype> create(std::vector<keytype> input, std::vector<valtype> output)
			{
				gstd::check(input.size() == output.size(), "cannot create map from unequally sized vector");
				int size = input.size();
				std::map<keytype, valtype> res;
				for (int i = 0; i < size; i++)
					res[input[i]] = output[i];
				return res;
			}
            
            template<typename type> std::map<type, int> invertVector(std::vector<type> input)
            {
                std::map<type, int> res;
                int size = input.size();
                for(int i=0;i<size;i++)
                    res[input[i]] = i;
                return res;
            } 
            
            template<typename keytype, typename valtype> std::map<keytype, valtype> submap(std::map<keytype, valtype>& input, int size, bool allowSmall = false)
            {
                typedef typename std::map<keytype, valtype>::iterator Iterator;
                std::map<keytype, valtype> res;
                int cursor = 0;
                for(Iterator i=input.begin();i!=input.end();i++)
                {
                    res[i->first] = i->second;
                    cursor++;
                    if(cursor == size)
                        break;
                }
                if(!allowSmall && cursor != size)
                    gstd::error("cannot extract submap smaller than original map");
                return res;   
            }
            
            template<typename type> std::map<type, int> getCounts(std::vector<type> input)
            {
                std::map<type, int> res;
                int size = input.size();
                for(int i=0;i<size;i++)
                    res[input[i]] = 0;
                for(int i=0;i<size;i++)
                    res[input[i]]++;
                return res;
            }

			template<typename type> std::map<type, int> tick(std::map<type, int> input, type element)
			{
				if (input.count(element) == 1)
					input[element] = input[element] + 1;
				else
					input[element] = 1;
				return input;
			}
            
            bool MSII810161816_GSTD_API test();
        }
    }
}

#endif	/* MAP_H */

