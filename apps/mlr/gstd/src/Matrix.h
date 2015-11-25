/* 
 * File:   Matrix.h
 * Author: gschoenh
 *
 * Created on February 5, 2014, 9:08 PM
 */

#include "stdafx.h"

#pragma once
#ifndef MATRIX_H
#define	MATRIX_H

#include "ex.h"     

namespace msii810161816
{   
    namespace gstd
    {   
        namespace matrix
        {
            template<typename type>
            std::vector<type> create(std::vector<std::vector<type> > input)
            {
                std::vector<type> res;   
                int dim1 = input.size();
                if(dim1 == 0)
                    return res;
                int dim2 = input[0].size();
                for(int i=0;i<dim1;i++)
                {
                    gstd::check((int)input[i].size() == dim2, "input is not of matrix form");
                    for(int j=0;j<dim2;j++)
                        res.push_back(input[i][j]);
                }
                return res;               
            }
            
            template<typename type>
            std::vector<std::vector<type> > to2d(int dim1, int dim2, std::vector<type> input)
            {
                gstd::check((int)input.size() == dim1*dim2, "input has incorrect size");                
                std::vector<std::vector<type> > res(dim1);
                for(int i=0;i<dim1;i++)
                {
                    for(int j=0;j<dim2;j++)
                        res[i].push_back(input[i*dim2 + j]);
                }
                return res;               
            }

			template<typename type>
			bool isMatrix(std::vector<std::vector<type> > input)
			{
				if ((int)input.size() == 0)
					return true;

				int numCols = input[0].size();

				for (int i = 0; i < (int)input.size(); i++)
				{
					if ((int)input[i].size() != numCols)
						return false;
				}

				return true;
			}
            
            bool MSII810161816_GSTD_API test();
        }
    }
}

#endif	/* MATRIX_H */

