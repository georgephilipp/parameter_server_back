/* 
 * File:   TypeTree.h
 * Author: gschoenh
 *
 * Created on December 4, 2013, 5:50 PM
 */

#include "stdafx.h"

#ifdef _WIN32
#ifdef MSII810161816_GSTD_EXPORTS
#define MSII810161816_GSTD_API __declspec(dllexport)
#else
#define MSII810161816_GSTD_API __declspec(dllimport)
#endif
#else
#define MSII810161816_GSTD_API
#endif

#ifndef TYPETREE_H
#define	TYPETREE_H
#include"standard.h"
#include<map>
#include"Vector.h"
namespace msii810161816
{   
    namespace gstd
    {                 
        class MSII810161816_GSTD_API TypeTree {
        public:
            TypeTree();
            //TypeTree(const TypeTree& orig);
            virtual ~TypeTree();
            
            std::map<std::string,std::vector<std::string> > typeMap;
            std::vector<TypeTree*> childTypeTrees;
            
            bool contains(std::string query);
            std::map<std::string, bool> get(std::string query, bool cache); 
            bool inherits(std::string super, std::string sub);
                      
        private:
            std::map<std::string,std::map<std::string, bool> > typeMapExpanded;

        };
        
        class MSII810161816_GSTD_API gstdTypeTree
        {
        public:
            static TypeTree makeInst();
            static TypeTree inst;        
        };
        
    }
}

#endif	/* TYPETREE_H */

