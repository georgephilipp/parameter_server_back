/* 
 * File:   Reader.h
 * Author: gschoenh
 *
 * Created on November 22, 2013, 6:58 PM
 */

#include "stdafx.h"

#include"standard.h"
#include"ex.h"
#include"Base.h"
#include<fstream>
#include "Parser.h"
#ifndef READER_H
#define	READER_H
namespace msii810161816
{   
    namespace gstd
    {     
        class MSII810161816_GSTD_API Reader : virtual public Base
        {
        public:  
            Reader();
			~Reader();
            
            //inputs
            std::string location;

			//options 
			char delimiter;
            
            //actions
            void open();
            void close();   
			bool fileIsOpen();
			gstd::trial<std::string> line(int index = -1);
                        template<typename type>
                        gstd::trial<std::vector<type> > row(int index = -1)
        {
            gstd::trial<std::vector<type> > res;
            gstd::trial<std::string> l = line(index);
            if(!l.success)
            {
                res.success = false;
                return res;
            }
            res.success = true;
            res.result = gstd::Parser::vector<type>(l.result, delimiter);
            return res;            
        }
            			static int numRows(std::string location);
                        static std::vector<std::string> ls(std::string location);
                        template<typename type>
                        static std::vector<std::vector<type> > rs(std::string location, char delimiter)
		{
			Reader reader;
			reader.location = location;
                        reader.delimiter = delimiter;
			reader.open();
			std::vector<std::vector<type> > res;
			while (1)
			{
				gstd::trial<std::vector<type> > next = reader.row<type>();
				if (!next.success)
					break;
				else
					res.push_back(next.result);
			}
			return res;
		}

        protected:
            std::ifstream file;
            int position;   
		private:
			bool openInner();
            
        //Base Package    
        public:
            virtual gstd::TypeName getTypeName();
            virtual void setInputs();
            virtual bool test();
            virtual std::string toString();
		private:
			std::string getTestFileName(); //needed for the test
        };
    }
}

namespace msii810161816
{
    namespace gstd
    {
        template<>
        struct TypeNameGetter<Reader>
        {
            static TypeName get()
            {
                TypeName t;
                t.name = "gstd::Reader";
                return t;
            }
        };
    }
}

#endif	/* READER_H */

