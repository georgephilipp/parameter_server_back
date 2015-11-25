/* 
 * File:   Printer.h
 * Author: gschoenh
 *
 * Created on November 14, 2013, 6:21 PM
 */

#include "stdafx.h"

#include"standard.h"
#include"ex.h"
#include "Base.h"
#include<map>
#include<sstream>
#include<iostream>
#include<tuple>
#ifndef PRINTER_H
#define	PRINTER_H

namespace msii810161816
{   
    namespace gstd
    {         
        class MSII810161816_GSTD_API Printer : public virtual Base
        {
        public:
            Printer();
            virtual ~Printer();
            
            //options
            static std::map<std::string, bool> mute;

            //actions
            template<typename type> void add(type input)
            {
                content << input;
                lineend = false;
            }            
            
            template<typename type> void line(type input)
            {
                content << input << std::endl;
                lineend = true;
            } 
            
            void newline();
            
            template<typename type> void append(type input, char delimiter = ' ')
            {
                if (!lineend)
                    content << delimiter;
                content << input;
                lineend = false;
            }
            
            template<typename type> void vector(std::vector<type> input, char delimiter = ' ')
            {
                int size = input.size();
                if(!lineend)
                    content << delimiter;
                for(int i=0;i<size-1;i++)
                    content << input[i] << delimiter;
                if( size > 0 )
                    content << input[size-1];
                lineend = false;
            }
            
            template<typename type> void matrix(int dim1, int dim2, std::vector<type> input, char delimiter = ' ')
            {
                if((int)input.size() != dim1*dim2)
                    gstd::error("size mismatch");
                for(int i=0;i<dim1;i++)
                {
                    std::vector<type> temp = std::vector<type>(dim2);
                    for(int j=0;j<dim2;j++)
                        temp[j] = input[i*dim2+j];
                    vector<type>(temp, delimiter);
                    if(i < dim1-1)
                        newline();                    
                }                   
                lineend = false;
            }
            
            template<typename keytype, typename valtype> 
            void map(std::map<keytype, valtype> input, char delimiter = ' ', std::string keyValDelimiter = ":")
            {
                typedef typename std::map<keytype, valtype>::iterator Iterator;
                if(!lineend)
                    content << delimiter;
                Iterator i=input.begin();
                while(i != input.end())
                {
                    content<<i->first<<keyValDelimiter<<i->second;
                    i++;
                    if(i != input.end())
                        content << delimiter;
                }           
                lineend = false;
            }            
            
            std::string get();
                        
            template<typename type> static std::string p(type input)
            {
                gstd::Printer P;
                P.add(input);
                return P.get();
            }       
                        
            template<typename type> static void c(type input, int newline = 1) //we use int here because std::string cannot overload bool
            {
                std::cout << p(input);
                if(newline != 0)
                    std::cout << std::endl;
                std::cout << std::flush;
            }
            
            template<typename type> static void c(type input, std::string channel, bool newline = true) 
            {
                if(mute.count(channel) == 1 && !mute[channel])
                    c(input, newline ? 1 : 0);
            }
            
            template<typename type> static std::string vp(std::vector<type> input, char delimiter = ' ')
            {
                gstd::Printer P;
                P.vector(input, delimiter);
                return P.get();
            }       
                        
            template<typename type> static void vc(type input, char delimiter = ' ', bool newline = true)
            {
                std::cout << vp(input, delimiter);
                if(newline)
                    std::cout << std::endl;
                std::cout << std::flush;
            }
            
            template<typename type> static void vc(type input, std::string channel, char delimiter = ' ', bool newline = true)
            {
                if(mute.count(channel) == 1 && !mute[channel])
                    vc(input, delimiter, newline);
            }   
            
            template<typename type> static std::string mp(int dim1, int dim2, std::vector<type> input, char delimiter = ' ')
            {
                gstd::Printer P;
                P.matrix(dim1, dim2, input, delimiter);
                return P.get();
            }   
            
            template<typename type> static std::string mp(std::tuple<int, int, std::vector<type> > input, char delimiter = ' ')
            {
                return mp(std::get<0>(input), std::get<1>(input), std::get<2>(input), delimiter);
            }  
            
            template<typename type> static void mc(int dim1, int dim2, type input, char delimiter = ' ', bool newline = true)
            {
                std::cout << mp(dim1, dim2, input, delimiter);
                if(newline)
                    std::cout << std::endl;
                std::cout << std::flush;
            }
            
            template<typename type> static void mc(int dim1, int dim2, type input, std::string channel, char delimiter = ' ', bool newline = true)
            {
                if(mute.count(channel) == 1 && !mute[channel])
                    mc(dim1, dim2, input, delimiter, newline);
            }     
            
            template<typename keytype, typename valtype> 
            static std::string mapp(std::map<keytype, valtype> input, char delimiter = ' ', std::string keyValDelimiter = ":")
            {
                gstd::Printer P;
                P.map(input, delimiter, keyValDelimiter);
                return P.get();
            }       
                        
            template<typename keytype, typename valtype>
            static void mapc(std::map<keytype, valtype> input, char delimiter = ' ', std::string keyValDelimiter = ":", bool newline = true)
            {
                std::cout << mapp(input, delimiter, keyValDelimiter);
                if(newline)
                    std::cout << std::endl;
                std::cout << std::flush;
            }
            
            template<typename keytype, typename valtype> 
            static void mapc(std::map<keytype, valtype> input, std::string channel, char delimiter = ' ', std::string keyValDelimiter = ":", bool newline = true)
            {
                if(mute.count(channel) == 1 && !mute[channel])
                    mapc(input, delimiter, keyValDelimiter, newline);
            }   

            template<typename type> static std::string numprint(type in, int padto, char padwith = '0')
            {
                std::stringstream outstream;
                outstream << in;
                std::string out = outstream.str();

                int i;
                int size = out.size();
                if(size > padto)
                {
                    out = out.substr(size - padto, padto);
                }
                else
                    for(i=0;i<padto-size;i++)
                        out = padwith + out;
                return out;
            }
            
        private:
            std::stringstream content;
            bool lineend;
            
        //Base package
        public:
            virtual TypeName getTypeName();
            virtual void setInputs();
            virtual bool test();
            virtual std::string toString();

        };
        
        
        template<>
        struct TypeNameGetter<Printer>
        {
            static TypeName get()
            {
                TypeName t;
                t.name = "gstd::Printer";
                return t;
            }
        };
    }
}

#endif	/* PRINTER_H */

