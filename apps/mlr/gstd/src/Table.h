/* 
 * File:   Table.h
 * Author: gschoenh
 *
 * Created on December 6, 2013, 1:37 PM
 */

#include "stdafx.h"

#ifndef TABLE_H
#define	TABLE_H
#include"standard.h"

namespace msii810161816
{   
    namespace gstd
    {
        template<typename datatypename>
        class Table 
        {
        public:
            Table() {}
            //Table(const Table& orig);
            virtual ~Table() {}
            
            std::vector<std::vector<datatypename> > content;
            
            
            Table(std::vector<datatypename> col)
            {
                int size = col.size();
                content.resize(size);
                for(int i=0;i<size;i++)
                    content[i].push_back(col[i]);
            }
                        
            
        private:

        };
    }
}

#endif	/* TABLE_H */

