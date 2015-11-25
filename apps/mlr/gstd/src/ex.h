/* 
 * File:   ex.h
 * Author: gschoenh
 *
 * Created on March 8, 2013, 4:25 PM
 */

#include "stdafx.h"

#ifndef msii810161816_gstd_EX_H
#define	msii810161816_gstd_EX_H
#include"standard.h"

namespace msii810161816
{   
    namespace gstd
    {                   
        void MSII810161816_GSTD_API hello();
        void MSII810161816_GSTD_API error(std::string message);
        void MSII810161816_GSTD_API check(bool condition, std::string message);
		void MSII810161816_GSTD_API checkMany(std::vector<bool> conditions, std::string message);
		void MSII810161816_GSTD_API checkNone(std::vector<bool> conditions, std::string message);
	void MSII810161816_GSTD_API reportFailure(std::string message);
        std::string MSII810161816_GSTD_API numtovar(int num, int varsize);
        
        bool MSII810161816_GSTD_API test();
        
        //Templates     
               
        template<typename type> struct trial
        {
            trial() {}
            trial(bool _success)
            {
                success = _success;
            }
            
            bool success;
            type result;
        };
              
       
    }
}

#endif	/* msii810161816_gstd_EX_H */

