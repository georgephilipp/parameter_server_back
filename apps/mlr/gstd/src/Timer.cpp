/* 
 * File:   Timer.cpp
 * Author: gschoenh
 * 
 * Created on November 15, 2013, 4:13 PM
 */

#include "stdafx.h"

#include<time.h>
#include "Timer.h"
#include "Printer.h"
#include "Primitives.h"
#include "Rand.h"
#ifndef _WIN32
    #include<unistd.h>
#endif
#ifdef __APPLE__
    #pragma GCC diagnostic ignored "-Wself-assign"
#endif



namespace msii810161816
{   
    namespace gstd
    {  
        Timer::Timer() 
        {
            reset();
        }
        
        /*double Timer::now()
        {
            //Linux: nanoseconds            
            timespec timer;
            clock_gettime(CLOCK_MONOTONIC, &timer);
            return timer.tv_sec + ((double)timer.tv_nsec)/1000000000;
            
            //Linux and OSX: microseconds (needs to include sys/time)
            timeval a;
            gettimeofday(&a, NULL);
            double a1 = a.tv_sec + a.tv_usec*0.000001;

            return 0;
        }*/
        
        //std::chrono::time_point<std::chrono::steady_clock> Timer::now()
        
        std::chrono::steady_clock::time_point Timer::now()
        {
            return std::chrono::steady_clock::now();
        }
                
        double Timer::t(bool document /*= true*/)
        {
            //double time = now();
            //double diff = time - starttime;
            std::chrono::steady_clock::time_point time = now();
            //std::chrono::duration<double> chrdiff = time - starttime;
            std::chrono::steady_clock::duration chrdiff = time - starttime;
            double diff = (double)chrdiff.count();
#ifdef _WIN32
			diff = diff*1e-7;
#else
			diff = diff*1e-9;
#endif

            if(document)
                gstd::Printer::c("The number of seconds passed in timer " + name + " is " + gstd::Printer::p(diff), channel);
            return diff;
        }
        
        void Timer::reset()
        {
            starttime = now();
        }
        
        Timer Timer::instConstructor()
        {
            Timer t;
            t.name = "gstd::Timer::global";
            t.channel = "gstd::Timer::global";
            return t;
        }
        
        Timer Timer::inst = instConstructor();
        
        void Timer::sleep(double numsec)
        {
			gstd::check(numsec > 0.001, "cannot guarantee that sleeping less than 0.001 seconds works. Also this wrapper probably takes longer than that anyway.");

            #ifdef _WIN32
                Sleep(gstd::Int::create(numsec*1000,"round"));
            #else
			std::pair<int, double> split = Int::splitReal(numsec);
			struct timespec tim;
			tim.tv_sec = split.first;
			tim.tv_nsec = Int::create(split.second*1e9,"round");
			nanosleep(&tim, 0);
            #endif
        }
        
        double Timer::benchmarkSystem()
        {
            double res;
            int d = 1000;
            std::vector<double> a(d*d, 0);
            std::vector<double> b(d*d, 0);
            std::vector<double> c(d*d, 0);
            for(int i=0;i<d*d;i++)
            {
                a[i] = gstd::Rand::d(0);
                b[i] = gstd::Rand::d(0);
            }
            
            printf("System Benchmarking begins\n");
            
            Timer t;
            
            for(int j=0;j<d;j++)
                for(int l=0;l<d;l++)
                    for(int k=0;k<d;k++)
                        c[j*d+l] += a[j*d+k]*b[k*d+l];
           
            res = t.t(false);
            
            gstd::Printer::c("This is a dummy output to protect against undue compiler optimization of benchmark");
            gstd::Printer::c(c[gstd::Rand::i(d*d)]);
            
            printf("System Benchmarking took %f seconds \n", res);
            return res;
        }
        
        std::pair<double, double> Timer::benchmarkTimer()
        {
            std::pair<double, double> res;
            Timer t;
            printf("Timer Benchmarking begins\n");
            
            printf("Timer Benchmarking for time measurement only ...\n");
            double t1 = t.t(false);
            double t2 = t.t(false);
            res.first = t2 - t1;
            printf("Timer Benchmarking including printing ...");
            t.t(true);
            t.t(true);
            t1 = t.t(true);
            t2 = t.t(true);
            res.second = t2 - t1;
            printf("Time it took for measurment only : %.8f\n",res.first);
            printf("Time it took with printing : %.8f\n",res.second);     
            return res;
        }
        
        //Base package
        TypeName Timer::getTypeName()
        {
            return gstd::TypeNameGetter<Timer>::get();
        }
        void Timer::setInputs()
        {
            name = "gstd::Timer::default";
            channel = "gstd::Timer::default";
        }
        bool Timer::test()
        {
            //sleep
            {
                Timer t;
                t.sleep(1.5);
                double elapsed = t.t(false);
                if(!gstd::Double::equals(elapsed,1.5,0.1,false))
                {
                    t.reportFailure("Function was sleep");
                    return false;
                }
            }
            
            //reset
            {
                Timer t;
                t.sleep(1);
                double e1 = t.t(false);
                t.reset();
                double e2 = t.t(false);
                if(e1 < e2)
                {
                    t.reportFailure("Function was reset");
                    return false;
                }
            }
            
            //BenchmarkSystem : not test
            
            //BenchmarkTimer
            {
                Timer t;
                std::pair<double, double> res = t.benchmarkTimer();
                if( res.first > 1e-6 || res.second > 1e-1 )
                {
                    t.reportFailure("Timer in unduly slow");
                    return false;
                }
            }
            
            return true; 
        }
        std::string Timer::toString()
        {
            std::stringstream res;
            res << "This is a gstd::Timer" << std::endl;
            res << "name is: " << name << std::endl;
            res << "channel is: " << channel << std::endl;
            res << "time elapsed is: " << t(false) << std::endl;
            return res.str();
        }
        
    }
}


