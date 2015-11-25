/* 
 * File:   Vector.h
 * Author: gschoenh
 *
 * Created on November 15, 2013, 4:21 PM
 */

#include "stdafx.h"

#include "standard.h"
#include "ex.h"
#include<map>
#include<iostream>
#include <functional>
#include <algorithm>
#include"Printer.h"
#include"Rand.h"
#ifndef VECTOR_H
#define	VECTOR_H

namespace msii810161816
{   
    namespace gstd
    {
        namespace vector
        {
            template<typename type> std::vector<type> create(std::map<int, type>& input, int maxkey = -1)
            {
                typedef typename std::map<int, type>::iterator Iterator;
                std::vector<type> res(0);
                if(maxkey == -1)
                    for(Iterator i=input.begin();i!=input.end();i++)
                        if(maxkey < i->first)
                            maxkey = i->first;
                res.resize(maxkey+1);
                for(Iterator i=input.begin();i!=input.end();i++)
                    res[i->first] = i->second;
                return res;
            }
                       
            template<typename type> std::vector<type> getmany(std::vector<type>& vals, std::vector<int>& keys)
            {
                std::vector<type> res;
                int size = keys.size();
                for(int i=0;i<size;i++)
                    res.push_back(vals[keys[i]]);
                return res;
            }
            
            std::vector<int> MSII810161816_GSTD_API invertToVector(std::vector<int>& input, int maxkey = 0);
            
            template<typename type> void empty(std::vector<type>& input)
            {
                std::vector<type> emptyVector;
                input.swap(emptyVector);
            }            
            
            template<typename type> std::vector<type> unique(std::vector<type> vec)
            {
                std::sort(vec.begin(), vec.end());
                vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
                return vec;
            }
            
            template<typename type> std::vector<std::pair<int,type> > insertIndeces(std::vector<type>& input)
            {
                std::vector<std::pair<int,type> > result;
                int size = input.size();
                for(int i=0;i<size;i++)
                    result.push_back(std::make_pair(i,input[i]));
                return result;                
            }
            
            template<typename type> std::pair<std::vector<int>,std::vector<type> > sort(
                    std::vector<type>& input, 
                    std::function<bool(std::pair<int,type>, std::pair<int,type>)> comparator
            )
            {
                int size = input.size();
                std::vector<std::pair<int,type> > withIndeces = insertIndeces(input);              
                std::sort(withIndeces.begin(),withIndeces.end(),comparator);
                std::vector<int> resultindeces;
                std::vector<type> resultvalues;
                for(int i=0;i<size;i++)
                {
                    resultindeces.push_back(withIndeces[i].first);
                    resultvalues.push_back(withIndeces[i].second);
                }
                return std::make_pair(resultindeces, resultvalues);
            }
                       
            template<typename type> std::pair<std::vector<int>,std::vector<type> > sort(std::vector<type>& input, int reverse = 0)
            {    
                bool boolReverse = false;
                if(reverse != 0)
                    boolReverse = true;
                
                auto compareWithIndeces = [=](std::pair<int,type> a, std::pair<int,type> b) -> bool
                {
                    if(boolReverse)
                        return (a.second > b.second);
                    else
                        return (a.second < b.second);
                };
                
                return sort<type>(input, compareWithIndeces);                
            }   
            
            template<typename type> std::pair<int, type> vmax(std::vector<type>& input)
            {
                int size = input.size();
                gstd::check(size > 0, "cannot get max of empty vector");
                
                std::pair<int, type> res;
                res.second = input[0];
                res.first = 0;
                for(int i=0;i<size;i++)
                    if(res.second < input[i])
                    {
                        res.second = input[i];
                        res.first = i;
                    }
                return res;
            } 
            
            template<typename type> std::pair<int, type> vmin(std::vector<type>& input)
            {
                int size = input.size();
                gstd::check(size > 0, "cannot get max of empty vector");
                
                std::pair<int, type> res;
                res.second = input[0];
                res.first = 0;
                for(int i=0;i<size;i++)
                    if(res.second > input[i])
                    {
                        res.second = input[i];
                        res.first = i;
                    }
                return res;
            }    
            
            template<typename type> std::vector<type> sub(std::vector<type> input, int start, int length, bool allowSmall = false)
            {
                int size = input.size(); 
				if (length > size - start)
				{
					if (allowSmall)
						length = size - start;
					else
						gstd::check(length < size - start, "cannot extract subvector longer than original vector");
				}
                std::vector<type> res;
                for(int i=start;i<start+length;i++)
                    res.push_back(input[i]);
                return res;
            }
            
            template<typename type> std::vector<double> rank(std::vector<type> input, bool rankSmallestValueFirst)
            {                
                int size = input.size();
                if(size == 0)
                    return std::vector<double>();
                
                //sort
				int sortOrder = 1;
				if (rankSmallestValueFirst)
					sortOrder = 0;
                std::pair<std::vector<int>,std::vector<type> > sorted = sort(input, sortOrder);
                
                //find chunks
                std::vector<int> chunks;
                type oldval = sorted.second[0];
                int count = 0;
                for(int i=0;i<size;i++)
                {
                    if(oldval == sorted.second[i])
                        count++;
                    else
                    {
                        chunks.push_back(count);
                        oldval = sorted.second[i];
                        count = 1;
                    }
                }
                chunks.push_back(count);
                
                //generate ranks
                std::vector<double> ranks;
                count = 0;
                double rval = 0;
                int numchunks = chunks.size();
                for(int i=0;i<numchunks;i++)
                {
                    rval = 0.5*((double)(2*count+chunks[i]-1));
                    for(int j=0;j<chunks[i];j++)
                        ranks.push_back(rval);
                    count += chunks[i];
                }
                
                //assign ranks
                std::vector<int> inv = invertToVector(sorted.first);
                return getmany(ranks,inv);                
            }

			template<typename type> std::vector< std::vector<int> > rankIndeces(std::vector<type> input)
			{
				int size = input.size();
				if (size == 0)
					return std::vector< std::vector<int> >();

				std::vector< std::vector<int> > res;

				//sort
				std::pair<std::vector<int>, std::vector<type> > sorted = sort(input, 1);

				//find chunks
				std::vector<int> chunk;
				type oldval = sorted.second[0];
				for (int i = 0; i<size; i++)
				{
					if (oldval == sorted.second[i])
						chunk.push_back(sorted.first[i]);
					else
					{
						res.push_back(chunk);
						empty(chunk);
						chunk.push_back(sorted.first[i]);
						oldval = sorted.second[i];
					}
				}
				res.push_back(chunk);

				return res;
			}

			template<typename intype, typename outtype> 
			std::vector<outtype> apply(std::function<outtype(intype)> fun, std::vector<intype> input)
			{
				int size = input.size();
				std::vector<outtype> res(size);
				for (int i = 0; i < size; i++)
					res[i] = fun(input[i]);
				return res;
			}

			template<typename intype, typename outtype>
			std::vector<std::vector<outtype> > apply2d(std::function<outtype(intype)> fun, std::vector<std::vector<intype> > input)
			{
				int size1 = input.size();
				std::vector<std::vector<outtype> > res(size1);
				for (int i = 0; i < size1; i++)
				{
					int size2 = input[i].size();
					res[i].resize(size2);
					for (int j = 0; j < size2; j++)
					{
						res[i][j] = fun(input[i][j]);
					}
				}
				return res;
			}

			template<typename type>
			std::vector<type> shuffle(std::vector<type> input)
			{
				int size = (int)input.size();
				std::vector<int> perm = gstd::Rand::perm(size, size);
				std::vector<type> res;
				for (int i = 0; i < size; i++)
					res.push_back(input[perm[i]]);
				return res;
			}

			std::vector<double> MSII810161816_GSTD_API stod(std::vector<std::string> input);
			std::vector<std::string> MSII810161816_GSTD_API dtos(std::vector<double> input);
			std::vector<std::vector<double> > MSII810161816_GSTD_API stod2d(std::vector<std::vector<std::string> > input);
			std::vector<std::vector<std::string> > MSII810161816_GSTD_API dtos2d(std::vector<std::vector<double> > input);
			std::vector<int> MSII810161816_GSTD_API dtoi(std::vector<double> input, std::string mode);
			std::vector<double> MSII810161816_GSTD_API itod(std::vector<int> input);
   
            bool MSII810161816_GSTD_API test();
        }
                 
        /*template<typename type> class Vector : public std::vector<type>
        {
        public:
            Vector() {}
            ~Vector() {}
            
        //multi-dimensional package
        public:
            std::vector<int> getdims() {return dims;}
            
            void init2d( int dim1, int dim2 )
            {
                dims.resize(2);
                dims[0] = dim1;
                dims[1] = dim2;
                
                ind2 = dim1;
                int totalsize = ind2*dim2;
                (*this).resize(totalsize);
            }
            
            void init3d( int dim1, int dim2, int dim3 )
            {
                dims.resize(3);
                dims[0] = dim1;
                dims[1] = dim2;
                dims[2] = dim3;
                
                ind2 = dim1;
                ind3 = ind2*dim2;
                int totalsize = ind3*dim3;
                (*this).resize(totalsize);
            }
            
            void init4d( int dim1, int dim2, int dim3, int dim4 )
            {
                dims.resize(4);
                dims[0] = dim1;
                dims[1] = dim2;
                dims[2] = dim3;
                dims[3] = dim4;
                
                ind2 = dim1;
                ind3 = ind2*dim2;
                ind4 = ind3*dim3;
                int totalsize = ind4*dim4;
                (*this).resize(totalsize);
            }
                       
            void initnd( std::vector<int> _dims )
            {                
                dims = _dims;
                int numdims = dims.size();
                if(numdims < 5)
                {
                    std::cout << "You are initializing a gstd::vector of dimension less than 5 with initnd. Do you want to use init2d/3d/4d instead?" <<std::endl<<std::flush;
                }
                int totalsize = 1;
                int i;
                
                //resize
                for(i=0;i<numdims;i++)
                    totalsize = totalsize*dims[i];           
                (*this).resize(totalsize);
                
                //set up indices
                indn.resize(numdims);
                indn[0] = 1;
                for(i=1;i<numdims;i++)
                    indn[i] = indn[i-1]*dims[i-1];
            }
            
            type get2d(int _dim1, int _dim2)
            {
                return((*this)[_dim1 + ind2*_dim2]);
            }
            
            void set2d(int _dim1, int _dim2, type in)
            {
                (*this)[_dim1 + ind2*_dim2] = in;
            }
            
            type get3d(int _dim1, int _dim2, int _dim3)
            {
                return((*this)[_dim1 + ind2*_dim2 + ind3*_dim3]);
            }
            
            void set3d(int _dim1, int _dim2, int _dim3, type in)
            {
                (*this)[_dim1 + ind2*_dim2 + ind3*_dim3] = in;
            }
            
            type get4d(int _dim1, int _dim2, int _dim3, int _dim4)
            {
                return((*this)[_dim1 + ind2*_dim2 + ind3*_dim3 + ind4*_dim4]);
            }
            
            void set4d(int _dim1, int _dim2, int _dim3, int _dim4, type in)
            {
                (*this)[_dim1 + ind2*_dim2 + ind3*_dim3 + ind4*_dim4] = in;
            }
            
            type getnd(std::vector<int> _dims)
            {
                int index = 0;
                int numdims = _dims.size();
                int i;
                for(i=0;i<numdims;i++)
                    index = index + _dims[i]*indn[i];
                
                return((*this)[index]);
            }
            
            void setnd(std::vector<int> _dims, type in)
            {
                int index = 0;
                int numdims = _dims.size();
                int i;
                for(i=0;i<numdims;i++)
                    index = index + _dims[i]*indn[i];
                
                (*this)[index] = in;
            }
            
        private:
            std::vector<int> dims;   
            int ind2;
            int ind3;
            int ind4;
            std::vector<int> indn;
            
        //buffering package
        public:
            int buffer;
            int bsize;
            
            void initbuf( int _buffer )
            {
                assignments = std::vector<bool>(this->size(), true);
                buffer = _buffer;
                bsize = this->size();
            }
            
            bool isassigned( int index )
            {
                if (index < bsize)
                    return assignments[index];
                else
                    return false;
            }
            
            type getb( int index )
            {
                if (isassigned(index))
                    return (*this)[index];
                else
                {
                    std::cout << "Asking for unassigned element" << std::endl << std::flush;
                    throw std::exception();
                }
                    
            }
            
            void bresize(int newsize)
            {
                this->resize(newsize);
                assignments.resize(newsize);
                if(bsize > newsize)
                    bsize = newsize;
            }
            
            void setb(int index, type val)
            {
                if (index >= (int)this->size())
                    bresize(index+1+buffer);
                
                if (index >= bsize)
                    bsize = index+1;
                
                (*this)[index] = val;
                assignments[index] = true;
            }
            
        private:
            std::vector<bool> assignments;
        };*/
    }
}
          

#endif	/* VECTOR_H */

