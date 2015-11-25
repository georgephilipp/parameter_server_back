/* 
 * File:   Timer.h
 * Author: gschoenh
 *
 * Created on November 15, 2013, 4:13 PM
 */

#include "stdafx.h"

#include"standard.h"
#include "Base.h"
#include<chrono>
#ifndef TIMER_H
#define	TIMER_H

namespace msii810161816
{   
    namespace gstd
    {  
        class MSII810161816_GSTD_API Timer : public virtual gstd::Base
        {
        public:
            Timer();
            
            //options 
            std::string name;
            std::string channel;
            
            //actions
            static Timer inst;
            double t( bool document = true );
            void reset();
            static void sleep(double numsec);          
            double benchmarkSystem();
            std::pair<double, double> benchmarkTimer();
        private:
            static Timer instConstructor();
            //double starttime;
            //std::chrono::time_point<std::chrono::steady_clock> starttime;
            
            std::chrono::steady_clock::time_point starttime;
            
            //static double now();
            //static std::chrono::time_point<std::chrono::steady_clock> now();
            
            std::chrono::steady_clock::time_point now();
            
        //Base package
        public:
            virtual TypeName getTypeName();
            virtual void setInputs();
            virtual bool test();
            virtual std::string toString();
        };
        
        template<>
        struct TypeNameGetter<Timer>
        {
            static TypeName get()
            {
                TypeName t;
                t.name = "gstd::Timer";
                return t;
            }
        };
    }
}

#endif	/* TIMER_H */

