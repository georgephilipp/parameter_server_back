/* 
 * File:   NestedVectors.h
 * Author: gschoenh
 *
 * Created on November 15, 2013, 4:06 PM
 */

#ifndef NESTEDVECTORS_H
#define	NESTEDVECTORS_H

namespace msii810161816
{   
    namespace gstd
    {   
        template<typename type> void mprint(std::vector<std::vector<type> > input, int id = -1)
        {
            if(id == -1 || id >= (int)mute.size() || !mute[id])
            {
                int size = input.size();
                int i = 0;
                for(i=0;i<size;i++)
                    vprint<type>(input[i]);
            }
        }

        
        template<typename type> std::vector<std::vector<type> > array2d(int dim1, int dim2)
        {
            std::vector<std::vector<type> > out;
            out.resize(dim1);
            int i;
            for(i=0;i<dim1;i++)
                out[i].resize(dim2);
            return out;
        }
        
        template<typename type> std::vector<std::vector<std::vector<type> > > array3d(int dim1, int dim2, int dim3)
        {
            std::vector<std::vector<std::vector<type> > > out;
            out.resize(dim1);
            int i,j;
            for(i=0;i<dim1;i++)
            {
                out[i].resize(dim2);
                for(j=0;j<dim2;j++)
                    out[i][j].resize(dim3);
            }
            return out;
        }
        
        template<typename type> std::vector<std::vector<std::vector<std::vector<type> > > > array4d(int dim1, int dim2, int dim3, int dim4)
        {
            std::vector<std::vector<std::vector<std::vector<type> > > > out;
            out.resize(dim1);
            int i,j,k;
            for(i=0;i<dim1;i++)
            {
                out[i].resize(dim2);
                for(j=0;j<dim2;j++)
                {
                    out[i][j].resize(dim3);
                    for(k=0;k<dim3;k++)
                        out[i][j][k].resize(dim4);
                }
            }
            return out;
        }
    }
}

#endif	/* NESTEDVECTORS_H */

