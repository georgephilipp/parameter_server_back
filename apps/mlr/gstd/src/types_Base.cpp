/* 
 * File:   Base.cpp
 * Author: gschoenh
 * 
 * Created on December 10, 2013, 12:52 PM
 */

#include "stdafx.h"

#include "Base.h"
#include "Printer.h"

namespace msii810161816
{   
    namespace gstd
    {  
        std::string TypeName::toString()
        {
            std::stringstream res;
            res << name;
            
            int size = parameters.size();
            if(size > 0)
            {
                std::vector<std::string> parmstrings;
                for(int i=0;i<size;i++)
                    parmstrings.push_back(parameters[i].toString());
                res << "<" << gstd::Printer::vp(parmstrings,',') << ">";    
            }
            return res.str();
        }
        
        Base::Base() 
        {
            doc = false;
            safe = false;        
        }

        Base::~Base() {}
        
        void Base::reportFailure(std::string extraMessage)
        {
            gstd::Printer::c(getTypeName().toString() + " failed test " + extraMessage + " " + toString());
        }
        
        namespace base
        {
            bool test()
            {
                gstd::Printer::c("This is the test for the Base.h objects");
                //TypeNameGetter and TypeName
                {
                    if(gstd::TypeNameGetter<std::map<std::vector<double>,std::map<bool,int>>>::get().toString() != "std::map<std::vector<double>,std::map<bool,int>>")
                    {
                        gstd::reportFailure("gstd::TypeName or gstd::TypeNameGetter failed");
                        return false;
                    }
                }
                return true;
            }
        }
    }
}
