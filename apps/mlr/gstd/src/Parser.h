/* 
 * File:   Parser.h
 * Author: gschoenh
 *
 * Created on November 15, 2013, 4:08 PM
 */

#include "stdafx.h"

#include"standard.h"
#include"ex.h"
#include "Printer.h"
#include "Vector.h"
#include<sstream>
#include"Base.h"
#ifndef PARSER_H
#define	PARSER_H


namespace msii810161816
{   
    namespace gstd 
    {  
        template<typename type>
        struct ParserHelper;
        
        class MSII810161816_GSTD_API Parser : public virtual Base
        {
        public:  
            Parser();
            
            //inputs
            Parser(std::string in);
            void set(std::string in); //this or the above
            char delimiter;
            
            template<typename type> gstd::trial<type> getnext()
            {
                return ParserHelper<type>::getnext(this);
            }
            
            template<typename type> std::vector<type> getall()
            {
                return ParserHelper<type>::getall(this);
            }
            
            template<typename type> static std::vector<type> vector(std::string input, char delimiter)
            {
                Parser p(input);
                p.delimiter = delimiter;
                return p.getall<type>();                
            }
            
            std::stringstream content;
            
            //Base Package
            virtual TypeName getTypeName();
            virtual void setInputs();
            virtual bool test();
            virtual std::string toString(); 
            
        };     
        
        template<typename type>
        struct ParserHelper
        {
            static gstd::trial<type> getnext(Parser* p)
            {
                gstd::trial<type> res;          
                                
                //return fail if end of stream is reached
                if(p->content.eof())
                {
                    res.success = false;
                    return res;
                }
                
                //this is the fastest I can think of for unspecified type            
                std::string out;
                std::getline(p->content,out,p->delimiter);
                std::stringstream putback(out);
                type gotten;
                putback >> gotten;
                if(putback.fail())
                    gstd::error("conversion failed");
                
                res.result = gotten;
                res.success = true;
                return res;
            }
            
            static std::vector<type> getall(Parser* p)
            {
                std::vector<type> res(0);
                                
                //return fail if end of stream is reached
                while(!p->content.eof())
                {
                    //this is the fastest I can think of for unspecified type            
                    std::string out;
                    std::getline(p->content,out,p->delimiter);
                    std::stringstream putback(out);
                    type gotten;
                    putback >> gotten;
                    if(putback.fail())
                        gstd::error("conversion failed");
                    res.push_back(gotten);
                }
                return res;
            }  
        };
   
		template<>
		struct ParserHelper<std::string>
		{
			static gstd::trial<std::string> getnext(Parser* p)
			{
				gstd::trial<std::string> res;

				//return fail if end of stream is reached
				if (p->content.eof())
				{
					res.success = false;
					return res;
				}

				std::string out;
				std::getline(p->content, res.result, p->delimiter);
				res.success = true;
				return res;
			}

			static std::vector<std::string> getall(Parser* p)
			{
				std::vector<std::string> res(0);

				//return fail if end of stream is reached
				while (!p->content.eof())
				{
					//this is the fastest I can think of for unspecified type            
					std::string out;
					std::getline(p->content, out, p->delimiter);
					res.push_back(out);					
				}
				return res;
			}
		};

        template<>
        struct ParserHelper<double>
        {
            static gstd::trial<double> getnext(Parser* p)
            {
                gstd::trial<double> res;   

                //return fail if end of stream is reached
                if(p->content.eof())
                {
                    res.success = false;
                    return res;
                }

                //this is the fastest I can think of for unspecified type            
                std::string out;
                std::getline(p->content,out,p->delimiter);
                res.result =  std::stod(out);
                res.success = true;
                return res;
            } 
            
            static std::vector<double> getall(Parser* p)
            {
                std::vector<double> res(0);

                while(!p->content.eof())
                {           
                    //this is slightly slower than the below
                    /*double out;
                    content << out;
                    res.push_back(out);*/                

                    //this is 2*10^7 per conversion
                    std::string out;
                    std::getline(p->content,out,p->delimiter);
                    res.push_back(std::stod(out));
                }
                return res;
            }  
        };
        
		template<>
		struct ParserHelper<int>
		{
			static gstd::trial<int> getnext(Parser* p)
			{
				gstd::trial<int> res;

				//return fail if end of stream is reached
				if (p->content.eof())
				{
					res.success = false;
					return res;
				}

				//this is the fastest I can think of for unspecified type            
				std::string out;
				std::getline(p->content, out, p->delimiter);
				res.result = std::stoi(out);
				res.success = true;
				return res;
			}

			static std::vector<int> getall(Parser* p)
			{
				std::vector<int> res(0);

				while (!p->content.eof())
				{
					//this is slightly slower than the below
					/*double out;
					content << out;
					res.push_back(out);*/

					//this is 2*10^7 per conversion
					std::string out;
					std::getline(p->content, out, p->delimiter);
					res.push_back(std::stoi(out));
				}
				return res;
			}
		};

        template<>
        struct TypeNameGetter<Parser>
        {
            static TypeName get()
            {
                TypeName t;
                t.name = "gstd::Parser";
                return t;
            }
        };
    }
}

#endif	/* PARSER_H */

