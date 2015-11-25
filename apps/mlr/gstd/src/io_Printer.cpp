/* 
 * File:   Printer.cpp
 * Author: gschoenh
 * 
 * Created on November 14, 2013, 6:21 PM
 */

#include "stdafx.h"

#include "Printer.h"
namespace msii810161816
{   
    namespace gstd
    {           
        Printer::Printer() 
        {
            lineend = true;
        }
        
        Printer::~Printer() {}
        
        std::map<std::string, bool> initMute()
        {
            std::map<std::string, bool> res;
            res["gstd::Timer::global"] = false;
            return res;            
        }
        
        std::map<std::string, bool> Printer::mute = initMute();
        
        void Printer::newline()
        {
            content << std::endl;
            lineend = true;
        }
        
        std::string Printer::get()
        {
            return content.str();
        }
        
        //Base Package
        TypeName Printer::getTypeName()
        {
            return gstd::TypeNameGetter<Printer>::get();
        }
        void Printer::setInputs() {}
        bool Printer::test()
        {
            //add,line,newline,append,get
            {
                Printer r;
                r.add(1.3);
                r.line("bling");
                r.newline();
                r.append(1, ':');
                r.append(1, ':');
                std::string res = r.get();
                if(res != "1.3bling\n\n1:1")
                {
                    r.reportFailure("Failed test is the first test");
                    return false;
                }
            }
            
            //p
            {
                double input = 1.234;
                std::string res = p(input);
                if(res != "1.234")
                {
                    gstd::Printer::c("Failed on static function p");
                    return false;
                }    
            }
            
            //vector / vp
            {
                std::vector<int> input = {1,2,3,4};
                std::string res = vp(input,',');
                if(res != "1,2,3,4")
                {
                    gstd::Printer::c("Failed on static function vp");
                    return false;
                }  
            }
            
            //matrix / mp
            {
                std::vector<double> input = {1.1,1.2,2.1,2.2,3.1,3.2,4.1,4.2};
                std::string res = mp(4,2,input,',');
                if(res != "1.1,1.2\n2.1,2.2\n3.1,3.2\n4.1,4.2")
                {
                    gstd::Printer::c("Failed on static function mp");
                    return false;
                }  
            }
            
            //map / mapp
            {
                std::map<int, double> input;
                input[1] = 1.1;
                input[2] = 2.2;
                input[3] = 3.3;
                std::string res = mapp(input,','," : ");
                if(res != "1 : 1.1,2 : 2.2,3 : 3.3")
                {
                    gstd::Printer::c("Failed on static function mapp");
                    return false;
                }  
            }
            
            return true;
        }
        std::string Printer::toString()
        {
            std::stringstream res;
            res << "This is a gstd::Printer" << std::endl;
            res << "mute is: " << mapp(mute) << std::endl;
            res << "lineend is: " << lineend << std::endl;
            res << "content is: " << content.str() << std::endl;
            return res.str();
        }
        
    }
}

