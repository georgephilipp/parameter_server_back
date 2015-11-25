/* 
 * File:   Pointer.h
 * Author: gschoenh
 *
 * Created on December 12, 2013, 7:23 PM
 */

#include "stdafx.h"

#ifndef POINTER_H
#define	POINTER_H
#include"standard.h"
#include"Base.h"
#include"Printer.h"

namespace msii810161816
{   
    namespace gstd
    {  
        template<typename type>
        class Pointer : virtual public gstd::Base
        {
        public:
            Pointer() 
            {
                isOwnedPointer = false;
                pointer = 0;
            }
            Pointer(const Pointer& orig)
            {
                gstd::error("gstd::Pointer cannot be copied. Use .clone() function instead");
                pointer = orig.pointer;
                /*isOwnedPointer = false;
                //Base Package
                gstd::Base::doc = orig.doc;
                gstd::Base::safe = orig.safe;*/
            }
            Pointer& operator=(const Pointer& other)
            {
                    gstd::error("assigning gstd::Pointer is illegal. Use get and set instead");
                    pointer = other.pointer;
                    return *this;
            }
            virtual ~Pointer() 
            {
                if(isOwnedPointer)
                    delete pointer;            
            }
            
            void set(type* _pointer, bool _isOwnedPointer )
            {
                if( isOwnedPointer )
                    gstd::error("cannot overwrite owned pointer, get and release first");
                
                isOwnedPointer = _isOwnedPointer;                
                pointer = _pointer;                
            }
            bool pointerIsOwned()
            {
                return isOwnedPointer;
            }
            type* get(bool release, bool safe = true) 
            {
                if(safe)
                    gstd::check(pointer != 0, "cannot retrieve null pointer in safe mode");
                if(release)
                {
                    if(!isOwnedPointer)
                        gstd::error("cannot release pointer that is not owned");
                    else
                        isOwnedPointer = false;
                }
                return pointer;
            }   
            bool isNull()
            {
                if(pointer == 0)
                    return true;
                else
                    return false;
            }
			void setnull()
			{
				gstd::check(!isOwnedPointer, "cannot set owned pointer to null");
				pointer = 0;
			}
        protected:
            bool isOwnedPointer;
            type* pointer;
        
        //Base package
        public:
            virtual gstd::TypeName getTypeName()
            {
                return gstd::TypeNameGetter<Pointer<type> >::get();
            }
            bool includeServerInString;
            virtual std::string toString()
            {
                std::stringstream res;
                res << "This is a gstd::pointer" << std::endl;
                res << "isOwnedPointer is: " << isOwnedPointer << std::endl;
                return res.str();
            }
            virtual void setInputs() {}
            virtual bool test()
            {
                Pointer<type> p;
                p.setInputs();  
                try
                {
                    if(!p.isNull())
                        throw std::exception();
                    gstd::Printer::c("Expecting error message...");
                    p.get(false);
                    gstd::Printer::c("gstd::Pointer failed test 1." + p.toString());
                    return false;                    
                }
                catch(std::exception e) {}
                type* dummy = 0;
                p.set(dummy, true);
                try
                {
                    gstd::Printer::c("Expecting error message...");
                    p.set(dummy, true);
                    gstd::Printer::c("gstd::Pointer failed test 2." + p.toString());
                    return false;                    
                }
                catch(std::exception e) {}
				try
				{
					gstd::Printer::c("Expecting error message...");
					p.setnull();
					gstd::Printer::c("gstd::Pointer failed test 3." + p.toString());
					return false;
				}
				catch (std::exception e) {}
				try
				{
					gstd::Printer::c("Expecting error message...");
					Pointer<type> p2(p);
					gstd::Printer::c("gstd::Pointer failed test 4." + p.toString());
					return false;
				}
				catch (std::exception e) {}
				try
				{
					gstd::Printer::c("Expecting error message...");
					Pointer<type> p2;
					p2 = p;
					gstd::Printer::c("gstd::Pointer failed test 5." + p.toString());
					return false;
				}
				catch (std::exception e) {}
                if(!p.pointerIsOwned())
                {
                    gstd::Printer::c("gstd::Pointer failed test 6." + p.toString());
                    return false; 
                }
                p.get(true, false);
                p.set(dummy, false);
                try
                {
                    gstd::Printer::c("Expecting error message...");
                    p.get(true, false);
                    gstd::Printer::c("gstd::Pointer failed test 7." + p.toString());
                    return false;                    
                }
                catch(std::exception e) {}
				p.setnull();
				if (!p.isNull())
				{
					gstd::Printer::c("gstd::Pointer failed test 8." + p.toString());
					return false;
				}
                return true;
            }
        };
        
        template<typename type>
        struct TypeNameGetter<Pointer<type> >
        {
            static TypeName get()
            {
                TypeName t;
                t.name = "gstd::Pointer";
                t.parameters.push_back(TypeNameGetter<type>::get());
                return t;
            }
        };
    }
}

#endif	/* POINTER_H */

