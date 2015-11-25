/* 
 * File:   Base.h
 * Author: gschoenh
 *
 * Created on December 10, 2013, 12:52 PM
 */


#include "stdafx.h"

#ifndef BASE_H
#define	BASE_H
#include "standard.h"
#include <sstream>
#include "ex.h"
#include <map>

namespace msii810161816
{   
    namespace gstd
    {  
        struct MSII810161816_GSTD_API TypeName
        {
            std::string name;
            std::vector<TypeName> parameters;
            
            std::string toString();
        };
        
        template<typename type>
        struct TypeNameGetter
        {
            static gstd::TypeName get()
            {
                gstd::error("not implemented");
                return gstd::TypeName();
            }
        };

        class MSII810161816_GSTD_API Base {
        public:
            Base();
            //Base(const Base& orig);
            virtual ~Base();
                       
            //options
            bool doc;
            bool safe;
                        
            //actions
            virtual TypeName getTypeName() = 0;
            virtual void setInputs() = 0;
            virtual bool test() = 0;
            virtual std::string toString() = 0;
            void reportFailure(std::string extraMessage = "");            
        };
        
        template<>
        struct TypeNameGetter<double>
        {
            static gstd::TypeName get()
            {
                TypeName t;
                t.name = "double";
                return t;
            }
        };
        
        template<>
        struct TypeNameGetter<int>
        {
            static gstd::TypeName get()
            {
                TypeName t;
                t.name = "int";
                return t;
            }
        };
        
        template<>
        struct TypeNameGetter<bool>
        {
            static gstd::TypeName get()
            {
                TypeName t;
                t.name = "bool";
                return t;
            }
        };
        
        template<>
        struct TypeNameGetter<std::string>
        {
            static gstd::TypeName get()
            {
                TypeName t;
                t.name = "std::string";
                return t;
            }
        };
        
        template<typename type>
        struct TypeNameGetter<std::vector<type> >
        {
            static gstd::TypeName get()
            {
                gstd::TypeName t;
                t.name = "std::vector";
                t.parameters.push_back(TypeNameGetter<type>::get());
                return t;
            }
        };
        
        template<typename keytype, typename valtype>
        struct TypeNameGetter<std::map<keytype, valtype> >
        {
            static gstd::TypeName get()
            {
                gstd::TypeName t;
                t.name = "std::map";
                t.parameters.push_back(TypeNameGetter<keytype>::get());
                t.parameters.push_back(TypeNameGetter<valtype>::get());
                return t;
            }
        };
        
        namespace base
        {
            bool MSII810161816_GSTD_API test();
        }
        
    }
}

//Base useage example
 
/*namespace msii810161816
{
    namespace somenamespace
    {
        template<typename type>
        class BaseUseageExample : public gstd::Base
        {
        public:
            type somemember;
            int a;
            int aTarget;
            MyTestClass() {}
            int somefunc()
            {
                return 2*a;
            }

            //actions
            virtual gstd::TypeName getTypeName()
            {
                return gstd::TypeNameGetter<MyTestClass>::get();
            }

            virtual void setInputs()
            {
                a = 1;
                aTarget = 2;
            }

            virtual std::string toString()
            {
                std::stringstream res;
                res << "TypeName: " + getTypeName().toString();
                res << "a: " << a << std::endl;
                return res.str();
            }

            virtual bool test()
            {
                return somefunc() == aTarget;
            }  
        }; 
    }
}

namespace msii810161816
{
    namespace gstd
    {
        template<typename type>
        struct TypeNameGetter<somenamespace::BaseUseageExample<type> >
        {
            static TypeName get()
            {
                TypeName t;
                t.name = "gstd::BaseUseageExample";
                t.parameters.push_back(TypeNameGetter<type>::get());
                return t;
            }
        };
    }
}*/




#endif	/* BASE_H */

