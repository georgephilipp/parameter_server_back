/* 
 * File:   Rand.cpp
 * Author: gschoenh
 * 
 * Created on November 14, 2013, 12:39 PM
 */

#include "stdafx.h"

#include "Rand.h"
#include"standard.h"
#include"ex.h"
#include "Map.h"
#include "Stat.h"
#include "Printer.h"
#include "Primitives.h"
#include "Timer.h"

#include <math.h>
#include <time.h>

#ifdef _WIN32
#pragma warning(disable:4244)
#endif

namespace msii810161816
{   
    namespace gstd
    { 
        Rand::Rand() {
        }

        /*Rand::Rand(const Rand& orig) {
        }*/

        Rand::~Rand() {
        }
        
        //int Rand::defaultTailedMode;
        //int Rand::defaultTailFreeMode;
        bool Rand::staticConstructor()
        {
            srand(0);
            //defaultTailFreeMode = 3;
            //defaultTailedMode = 4;
            return true;
        }      
        bool Rand::staticConstructorDummy = Rand::staticConstructor();

        void Rand::randomize()
        {
            srand(time(NULL));
        }
        
        void Rand::seed(int input)
        {
            gstd::check(input > 0, "cannot use random seed that is not positive");
            srand((unsigned)input);            
        }
        
		int Rand::getRandMax()
		{
#ifdef __APPLE__
			return RAND_MAX - 1;
#else
			return RAND_MAX;
#endif
		}

		int Rand::get()
		{
#ifdef __APPLE__
			return rand() - 1;
#else
			return rand();
#endif
		}

        int Rand::i(int max)
        {
			int randMax = getRandMax();
			if (max > randMax)
			{
				int factor = i(max / randMax + 1);
				int remainder = i(randMax);
				if (factor == max / randMax && remainder >= max % randMax)
					return i(max);
				else
					return factor * randMax + remainder;
			}
			int ratio = randMax / max;
            int draw = get();
            if(draw / max == ratio)
                return i(max);
            return draw % max;            
        }
        
        double Rand::d(int mode)
        {
			int randMax = getRandMax();
            //::rand() returns integers between AND INCLUDING 0 and RAND_MAX EXCEPT on a mac, where it returns between AND INCLUDING 1 and RAND_MAX
            
            //mode 0: 32-bit equispaced between and including 0 and 1 (fastest)
            //mode 1: 64-bit equispaced between and including 0 and 1 (fastest for 64 bit)
            //mode 2: 32-bit equispaced between and excluding 0/1 and symmetric (clipped) tail
            //mode 3: 64-bit equispaced between and excluding 0/1 and symmetric (clipped) tail (should be the default for most calls!)
            //mode 4: 64-bit equispaced between and excluding 0/1 with high-accuracy / asymmetric tail (should be the default for normal random vars!)
            
			if (randMax > 100000000)
			{
				if (mode == 0) // fastest
				{
					return (double)get() / randMax;
				}
				else if (mode == 1) //fastest for double precision
				{
					return ((0.5 + (double)get()) / ((double)randMax + 1) + (double)get()) / ((double)randMax + 1);
				}
				else if (mode == 2) // cannot equal 0 or 1 (single precision)
				{
					int lower = i(2);
					double base = ((double)get() + 0.5) / 2 / ((double)randMax + 1);
					base = gstd::Double::clipPrecision(base);
					if (lower == 0)
						return base;
					else
						return 1 - base;
				}
				else if (mode == 3)
				{
					if (randMax > 100000000)
					{
						int lower = i(2);
						int draw1 = i(25000000);
						int draw2 = i(50000000);
						double base = (((double)(4 * draw1) + 2) * 0.00000001 + (double)draw2)*0.00000001;
						if (lower == 0)
							return base;
						else
							return 1 - base;
					}
					else
						gstd::error("no available implementation for when RAND_MAX is less than 10 to the 8.");
				}
				else if (mode == 4)
				{
					if (randMax > 1000000000)
					{
						double res = 0.5;
						for (int j = 0; j < 10; j++)
							res = ((double)i(1000000000) + res)*0.000000001;
						return res;
					}
					else
						gstd::error("no available implementation for when RAND_MAX is less than 10 to the 8.");
				}
				else
				{
					gstd::error("invalid mode");
					return 0;
				}
			}
			else if (randMax > 10000)
			{
				if (mode == 0) // fastest
				{
					return (double)get() / randMax;
				}
				else if (mode == 1) //fastest for double precision
				{
					return ((((0.5 + (double)get()) / ((double)randMax + 1) + (double)get()) / ((double)randMax + 1) + (double)get()) / ((double)randMax + 1) + (double)get()) / ((double)randMax + 1);
				}
				else if (mode == 2) // cannot equal 0 or 1 (single precision)
				{
					int lower = i(2);
					double base = ((double)get() + 0.5) / 2 / ((double)randMax + 1);
					base = gstd::Double::clipPrecision(base);
					if (lower == 0)
						return base;
					else
						return 1 - base;
				}
				else if (mode == 3)
				{
					int lower = i(2);
					int draw1 = i(2500);
					int draw2 = i(10000);
					int draw3 = i(10000);
					int draw4 = i(5000);
					double base = (((((double)(4 * draw1) + 2) * 0.0001 + (double)draw2)*0.0001 + (double)draw3)*0.0001 + (double)draw4) * 0.0001;
					if (lower == 0)
						return base;
					else
						return 1 - base;
				}
				else if (mode == 4)
				{
					double res = 0.5;
					for (int j = 0; j < 20; j++)
						res = ((double)i(10000) + res)*0.0001;
					return res;
				}
				else
				{
					gstd::error("invalid mode");
					return 0;
				}
			}
			else
				gstd::error("no available implementation for when RAND_MAX is less than 10 to the 4.");
			return 0;
        }
 
        std::vector<int> Rand::perm(int total, int top)
        {
            if(top*2 < total)
            {
                std::map<int, bool> selected;
                while((int)selected.size() < top)
                    selected[i(total)] = true;
                return gstd::map::keys(selected);                
            }
            else
            {
                std::vector<int> cache(total);
                std::vector<int> result;
                for(int j=0;j<total;j++)
                    cache[j] = j;
                for(int j=0;j<top;j++)
                {
                    int nextindex = i(total - j);
                    result.push_back(cache[j+nextindex]);
                    cache[j+nextindex] = cache[j];
                }
                return result;
            }            
        }
        
        //Base Package
        TypeName Rand::getTypeName()
        {
            return gstd::TypeNameGetter<Rand>::get();
        }
        void Rand::setInputs() {}
        bool Rand::test()
        {
            //test i
            std::vector<int> res(4);
            for(int j=0;j<10000;j++)
            {
                int newval = i(4);
                if(newval >= 0 && newval < 4)
                    res[newval]++;
                else
                {
                    reportFailure("1");
                    return false;
                }
            }
            for(int j=0;j<4;j++)
                if(!gstd::Double::equals((double)res[j], 2500, 5*sqrt(0.25*0.75*10000), false))
                {
                    reportFailure("2");
                    return false;
                }
			std::vector<int> scales = { 1000, RAND_MAX - 10, RAND_MAX / 2 + 10, 1000000000 };
			int numrepl = 10;
			std::vector<double> dscales;
			for (int j = 0; j < (int)scales.size(); j++)
				dscales.push_back((double)scales[j]);
			for (int j = 0; j < (int)scales.size(); j++)
			{
				gstd::Printer::c("Staring random integer test " + gstd::Printer::p(j) + " on scale " + gstd::Printer::p(scales[j]));
				double sum = 0;
				double sumsquares = 0;
				bool top = false;
				bool exceed = false;
				bool zero = false;
				for (int k = 0; k < scales[j]; k++)
				{
					if (k % 10000000 == 0 && k > 0)
						gstd::Printer::c(k);
					for (int l = 0; l < numrepl; l++)
					{
						int draw = i(scales[j]);
						if (scales[j] - 1 == draw)
							top = true;
						if (draw == 0)
							zero = true;
						if (draw >= scales[j])
							exceed = true;
						sum += (double)draw;
						sumsquares += ((double)draw)*((double)draw);
					}
				}
				double sumtarget = (dscales[j]-1) / 2 * dscales[j] * numrepl;
				double sumsquaretarget = (dscales[j] - 1) * (2 * dscales[j] - 1) / 6 * dscales[j] * numrepl;
				if (!top)
				{
					reportFailure("top not returned on scale iteration " + gstd::Printer::p(j) + " scale " + gstd::Printer::p(scales[j]));
					return false;
				}
				if (!zero)
				{
					reportFailure("zero not returned on scale iteration " + gstd::Printer::p(j) + " scale " + gstd::Printer::p(scales[j]));
					return false;
				}
				if (exceed)
				{
					reportFailure("top exceeded on scale iteration " + gstd::Printer::p(j) + " scale " + gstd::Printer::p(scales[j]));
					return false;
				}
                                double abssumtargetdiff = sum - sumtarget;
                                if(abssumtargetdiff < 0)
                                    abssumtargetdiff = -abssumtargetdiff;
				if (abssumtargetdiff / sumtarget > 0.1)
				{
					reportFailure("sum not within bound on scale iteration " + gstd::Printer::p(j) + " scale " + gstd::Printer::p(scales[j]));
					return false;
				}
                                double abssumsquaretargetdiff = sumsquares - sumsquaretarget;
                                if(abssumsquaretargetdiff < 0)
                                    abssumsquaretargetdiff = -abssumsquaretargetdiff;
				if (abssumsquaretargetdiff / sumsquaretarget > 0.1)
				{
					reportFailure("sum not within bound on scale iteration " + gstd::Printer::p(j) + " scale " + gstd::Printer::p(scales[j]));
					return false;
				}
			}
            
            //test d
            int nummodes = 5;
            for(int j=0;j<nummodes;j++)
            {
                double first = 0;
                double second = 0;
                double fourth = 0;
				for (int k = 0; k < 10000; k++)
				{
					double val = d(j) - 0.5;
					//gstd::Printer::c(val);
					first += val;
					second += val*val;
					fourth += val*val*val*val;
				}
                first /= 10000;
                second /= 10000;
                fourth /= 10000;             
                if(!gstd::Double::equals(first, 0, 5.0/sqrt(12)/100, false)
                        || !gstd::Double::equals(second, 1.0/12.0, 5.0/sqrt(12)/100.0, false)
                        || !gstd::Double::equals(fourth/second/second-3, -6.0/5.0, 0.1, false)
                    )
                {
					gstd::Printer::c(first);
					gstd::Printer::c(second);
					gstd::Printer::c(fourth / second / second - 3);

                    reportFailure("3");
                    return false;
                }
            }
            for(int j=0;j<1000000;j++)
            {
                double val = d(3);
                char buffer[50];
                sprintf(buffer,"%.20f",val);
                double interestingPart = (double)std::stoi(std::string({buffer[16],buffer[17],buffer[18],buffer[19]}));
                int intval = (int)round(interestingPart * 0.01);                
                if(intval % 4 == 0)
                {
                    reportFailure("4");
                    return false;
                }
            }
            
            //test perm
            res.clear();
            res.resize(4);
            for(int j=0;j<10000;j++)
            {
                std::vector<int> newperm = perm(4, 2);
                if(newperm.size() != 2 || newperm[0] == newperm[1] || newperm[0]  < 0 || newperm[0] >= 4 || newperm[1] < 0 || newperm[1] >= 4)
                {
                    reportFailure("5");
                    return false;
                }
                res[newperm[i(2)]]++;
            }
            for(int j=0;j<4;j++)
                if(!gstd::Double::equals((double)res[j], 2500, 5*sqrt(0.25*0.75*10000), false))
                {
                    reportFailure("6");
                    return false;
                }
            return true;
        }
        std::string Rand::toString()
        {
            return "This is gstd::Rand";
        }

        
    }
}

