/* 
 * File:   ErasureBase.h
 * Author: heathermceveety
 *
 * Created on December 4, 2013, 9:05 PM
 */

#ifndef ERASUREBASE_H
#define	ERASUREBASE_H
#include"standard.h"
#include "TypeTree.h"
#include "ex.h"
namespace msii810161816
{   
    namespace gstd
    {  
        class ErasureBaseInterface 
        { 
        public:
            virtual ~ErasureBaseInterface(); 
        };

        class ErasureBase {
        public:
            ErasureBase();
            //ErasureBase(const ErasureBase& orig);
            virtual ~ErasureBase();
            
            template<typename type>
            type* cast(std::string typeName)
            {
                if(!typeTree->inherits(typeName, thisTypeName))
                    gstd::error("attempted to cast " + thisTypeName + " to " + typeName + ". This is not possible.");
                
                return dynamic_cast<type*>(castTarget);
            }
            
            std::string thisTypeName;
            TypeTree* typeTree;
            ErasureBaseInterface* castTarget;

        };
    }
}

#endif	/* ERASUREBASE_H */

