
#include "sequences.h"
#include "ex.h"

namespace msii810161816
{   
    namespace gstd
    {
        namespace seq
        {
            std::vector<int> irange(int begin, int numsteps, int returnpoint /*= 0*/)
            {
                std::vector<int> out(numsteps, 0);
                int i,k;
                if(returnpoint > 0)
                    begin = begin % returnpoint;
                k = begin - 1;
                for (i=0;i<numsteps;i++)
                {
                    k++;
                    if (k==returnpoint)
                        k=0;
                    out[i] = k;
                }
                return out;             
            }
            
            std::vector<std::vector<double> > d2range(double begin, int numrows, int numcols, int rowstep /*= 1*/, int colstep /*= 1*/)
            {
                std::vector<std::vector<double> > out;
                out.resize(numrows);
                int i,j;
                double rcursor, ccursor;
                rcursor = begin;
                for(i=0;i<numrows;i++)
                {
                    ccursor = rcursor;
                    out[i].resize(numcols);
                    for(j=0;j<numcols;j++)
                    {
                        out[i][j] = ccursor;
                        ccursor = ccursor + colstep;
                    }                    
                    rcursor = rcursor + rowstep;
                }
                return out;
            }
        }
    }
}



