/* 
 * File:   Rand.h
 * Author: gschoenh
 *
 * Created on November 14, 2013, 12:39 PM
 */

#include "stdafx.h"

#ifndef RAND_H
#define	RAND_H
#include "standard.h"
#include "Base.h"
namespace msii810161816
{   
    namespace gstd
    { 
        class MSII810161816_GSTD_API Rand : public virtual gstd::Base
        {
        public:
            Rand();
            /*Rand(const Rand& orig);*/
            virtual ~Rand();
               
            //static int defaultTailFreeMode;
            //static int defaultTailedMode;
            static void randomize();
            static void seed(int input);
			static int getRandMax();
			static int get();
            static int i(int max);
            static double d(int mode);
            static std::vector<int> perm(int total, int top);
            
        private:
            //If someone has a better suggestion on how to do this, let me know.
            static bool staticConstructorDummy;
            static bool staticConstructor();
            
        //Base Package
        public:
            virtual TypeName getTypeName();
            virtual void setInputs();
            virtual bool test();
            virtual std::string toString();
        };
        
        template<>
        struct TypeNameGetter<Rand>
        {
            static TypeName get()
            {
                TypeName t;
                t.name = "gstd::Rand";
                return t;
            }
        };
    }
}

#endif	/* RAND_H */

