#include "stdafx.h"

#include"ex.h"
#include"standard.h"
#include<iostream>
#include<map>


namespace msii810161816
{   
    namespace gstd
    {                   
        void hello()
        {
            std::cout << "Hello" << std::endl;
        }  
        
        void error(std::string message)
        {
            std::cout << message << std::endl;
            throw std::exception();
        }    
        
        void check(bool condition, std::string message)
        {
            if(!condition)
                gstd::error(message);
        }

		void checkMany(std::vector<bool> conditions, std::string message)
		{
			int size = conditions.size();
			for (int i = 0; i < size; i++)
				gstd::check(conditions[i], message);
		}
        
		void checkNone(std::vector<bool> conditions, std::string message)
		{
			int size = conditions.size();
			for (int i = 0; i < size; i++)
				gstd::check(!conditions[i], message);
		}

	void reportFailure(std::string message)
	{
		std::cout << "Function test failed : " << message << std::endl;
	}

        std::map<std::string,std::string> makenumtovarexceptionmap()
        {
            std::map<std::string,std::string> exceptions;
            exceptions["do"] = "AA";
            exceptions["if"] = "AB";
            exceptions["or"] = "AC";
            exceptions["and"] = "AAA";
            exceptions["asm"] = "AAB";
            exceptions["for"] = "AAC";
            exceptions["int"] = "AAD";
            exceptions["new"] = "AAE";
            exceptions["not"] = "AAF";
            exceptions["try"] = "AAG";
            exceptions["xor"] = "AAH";
            exceptions["auto"] = "AAAA";
            exceptions["bool"] = "AAAB";
            exceptions["case"] = "AAAC";
            exceptions["char"] = "AAAD";
            exceptions["else"] = "AAAE";
            exceptions["enum"] = "AAAF";
            exceptions["goto"] = "AAAG";
            exceptions["long"] = "AAAH";
            exceptions["this"] = "AAAI";
            exceptions["true"] = "AAAJ";
            exceptions["void"] = "AAAK";
            exceptions["bitor"] = "AAAAA";
            exceptions["break"] = "AAAAB";
            exceptions["catch"] = "AAAAC";
            exceptions["class"] = "AAAAD";
            exceptions["compl"] = "AAAAE";
            exceptions["const"] = "AAAAF";
            exceptions["false"] = "AAAAG";
            exceptions["float"] = "AAAAH";
            exceptions["or_eq"] = "AAAAI";
            exceptions["short"] = "AAAAJ";
            exceptions["union"] = "AAAAK";
            exceptions["using"] = "AAAAL";
            exceptions["throw"] = "AAAAM";
            exceptions["while"] = "AAAAN";
            exceptions["and_eq"] = "AAAAAA";
            exceptions["bitand"] = "AAAAAB";
            exceptions["delete"] = "AAAAAC";
            exceptions["double"] = "AAAAAD";
            exceptions["export"] = "AAAAAE";
            exceptions["extern"] = "AAAAAF";
            exceptions["friend"] = "AAAAAG";
            exceptions["inline"] = "AAAAAH";
            exceptions["not_eq"] = "AAAAAI";
            exceptions["public"] = "AAAAAJ";
            exceptions["return"] = "AAAAAK";
            exceptions["signed"] = "AAAAAL";
            exceptions["sizeof"] = "AAAAAM";
            exceptions["static"] = "AAAAAN";
            exceptions["struct"] = "AAAAAO";
            exceptions["switch"] = "AAAAAP";
            exceptions["typeid"] = "AAAAAQ";
            exceptions["xor_eq"] = "AAAAAR";
            exceptions["default"] = "AAAAAAA";
            exceptions["mutablt"] = "AAAAAAB";
            exceptions["private"] = "AAAAAAC";
            exceptions["typedef"] = "AAAAAAD";
            exceptions["virtual"] = "AAAAAAE";
            exceptions["wchar_t"] = "AAAAAAF";
            exceptions["continue"] = "AAAAAAAA";
            exceptions["explicit"] = "AAAAAAAB";
            exceptions["operator"] = "AAAAAAAC";
            exceptions["register"] = "AAAAAAAD";
            exceptions["template"] = "AAAAAAAE";
            exceptions["typename"] = "AAAAAAAF";
            exceptions["unsigned"] = "AAAAAAAG";
            exceptions["volatile"] = "AAAAAAAH";
            exceptions["namespace"] = "AAAAAAAAA";
            exceptions["protected"] = "AAAAAAAAB";
            exceptions["const_cast"] = "AAAAAAAAAA";
            exceptions["static_cast"] = "AAAAAAAAAAA";
            exceptions["dynamic_cast"] = "AAAAAAAAAAAA";
            exceptions["reinterpret_cast"] = "AAAAAAAAAAAAAAAA";
            
            return exceptions;
        }
        
        std::map<std::string,std::string> numtovarexceptionmap = makenumtovarexceptionmap();
        
        std::vector<std::string> makenumtovarchar1()
        {
            std::vector<std::string> char1;
            int i;
            char c;
            for(i=66;i<=90;i++)
            {
                c = (char)i;
                std::string s(1, c);
                char1.push_back(s);
            }
            for(i=97;i<=122;i++)
            {
                c = (char)i;
                std::string s(1, c);
                char1.push_back(s);
            }
            
            return char1;
        }
               
        std::vector<std::string> numtovarchar1 = makenumtovarchar1();
        
        std::vector<std::string> makenumtovarcharn()
        {
            std::vector<std::string> charn;
            int i;
            char c;
            for(i=65;i<=90;i++)
            {
                c = (char)i;
                std::string s(1, c);
                charn.push_back(s);
            }
            for(i=97;i<=122;i++)
            {
                c = (char)i;
                std::string s(1, c);
                charn.push_back(s);
            }
            for(i=48;i<=57;i++)
            {
                c = (char)i;
                std::string s(1, c);
                charn.push_back(s);
            }
            
            return charn;
        }
               
        std::vector<std::string> numtovarcharn = makenumtovarcharn();
        
        std::string numtovar(int num, int varsize)
        {            
            std::string out;
            int rem = num / 51;
            int res = num - rem*51;
            int remnew;
            out = numtovarchar1[res];            
            int i;
            for(i=1;i<varsize;i++)
            {
                remnew = rem / 62;
                res = rem - 62*remnew;
                rem = remnew;
                out += numtovarcharn[res];
            }
            if(rem > 0)
                gstd::error("numtovar received oversized input");
            if(numtovarexceptionmap.count(out) == 1)
                return numtovarexceptionmap[out];
  	    return out;
        }     
        
        bool test()
        {
			std::cout << "This is a test for the gstd core functions" << std::endl;

            //hello : trivial
            //error : trivial
            //check : trivial
            //numtovar : currently not used
            
			//checkMany & checkNone
			{
				std::vector<bool> input = { false, true };
				try
				{
					std::cout << "Expecting error message ...\n";
					gstd::checkMany(input, "This is the error message!");
					std::cout << "Failed test 1 on functions checkMany / checkNone\n";
					return false;
				}
				catch (std::exception e){}
				try
				{
					std::cout << "Expecting error message ...\n";
					gstd::checkNone(input, "This is the error message!");
					std::cout << "Failed test 1 on functions checkMany / checkNone\n";
					return false;
				}
				catch (std::exception e){}
				input = { false, false };
				try
				{
					std::cout << "Expecting error message ...\n";
					gstd::checkMany(input, "This is the error message!");
					std::cout << "Failed test 1 on functions checkMany / checkNone\n";
					return false;
				}
				catch (std::exception e){}
				try
				{
					gstd::checkNone(input, "This is the error message!");
				}
				catch (std::exception e)
				{
					std::cout << "Failed test 1 on functions checkMany / checkNone\n";
					return false;
				}
				input = { true, true };
				try
				{
					gstd::checkMany(input, "This is the error message!");
				}
				catch (std::exception e)
				{
					std::cout << "Failed test 1 on functions checkMany / checkNone\n";
					return false;
				}
				try
				{
					std::cout << "Expecting error message ...\n";
					gstd::checkNone(input, "This is the error message!");
					std::cout << "Failed test 1 on functions checkMany / checkNone\n";
					return false;
				}
				catch (std::exception e){}
			}

            return true;
        }
    }
        
    
}
