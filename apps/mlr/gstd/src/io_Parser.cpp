/* 
 * File:   Parser.cpp
 * Author: gschoenh
 * 
 * Created on November 15, 2013, 4:08 PM
 */

#include "stdafx.h"

#include "Parser.h"

namespace msii810161816
{   
    namespace gstd
    {  
        Parser::Parser() : content("")
        {
            delimiter = ',';
        }       
        
        Parser::Parser(std::string in) : content(in)
        {
            delimiter = ',';
        }
        
        void Parser::set(std::string in)
        {
            gstd::check(content.str() == "", "cannot feed into stream that is already being read from ... no idea what that does");
            content << in;
        }
        
        //Base Package
        TypeName Parser::getTypeName()
        {
            return gstd::TypeNameGetter<Parser>::get();
        }
        void Parser::setInputs()
        {
            set("1,2,3,4");
            delimiter = ',';
        }
        bool Parser::test()
        {
            Parser p;
            p.setInputs();
            double n1 = p.getnext<double>().result;
            int n2 = p.getnext<int>().result;
            std::vector<double> n34 = p.getall<double>();
            std::vector<int> n0 = p.getall<int>();
            if( n1 != 1.0 || n2 != 2 || n34 != std::vector<double>({3.0,4.0}) || n0.size() > 0 
				|| Parser::vector<int>("1,2", ',') != std::vector<int>({ 1, 2 }) 
				|| Parser::vector<double>("1.1,2.2", ',') != std::vector<double>({ 1.1, 2.2 }) 
				|| Parser::vector<std::string>("1,2,shamrock shake", ',') != std::vector<std::string>({ "1", "2", "shamrock shake" })
				|| Parser::vector<std::string>("1,2,shamrock shake,", ',') != std::vector<std::string>({ "1", "2", "shamrock shake", "" })
				)
            {
                p.reportFailure();
                return false;
            }
            return true;
        }
        std::string Parser::toString()
        {
            std::stringstream res;
            res << "This is a gstd::Parser" << std::endl;
            res << "delmiter is: " << delimiter << std::endl;
            res << "content is: " << content.str() << std::endl;
            return res.str();
        }
    }
}