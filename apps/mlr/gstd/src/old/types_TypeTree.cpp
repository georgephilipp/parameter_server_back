/* 
 * File:   TypeTree.cpp
 * Author: gschoenh
 * 
 * Created on December 4, 2013, 5:50 PM
 */

#include "stdafx.h"

#include "TypeTree.h"
#include "ex.h"
namespace msii810161816
{   
    namespace gstd
    {  
        TypeTree::TypeTree() 
        {
        }

        //TypeTree::TypeTree(const TypeTree& orig) {        }

        TypeTree::~TypeTree() {
        }
        
        bool TypeTree::contains(std::string query)
        {
            if(typeMap.count(query) == 0)
                return false;
            else
                return true;
        }
        
        std::map<std::string, bool> TypeTree::get(std::string query, bool cache)
        {
            if(typeMapExpanded.count(query) == 1)
                return typeMapExpanded[query];

            std::map<std::string, bool> res;
            
            if(typeMap.count(query) == 1)
            {
                std::vector<std::string> children = typeMap[query];
                int size = children.size();
                for(int i=0;i<size;i++)
                {
                    res[children[i]] = true;
                    std::map<std::string, bool> grandchildren = get(children[i], false);
                    res.insert(grandchildren.begin(), grandchildren.end());
                }              
            }
            else
            {
                int numchildren = childTypeTrees.size();
                for(int i=0;i<numchildren;i++)
                {
                    if(childTypeTrees[i]->contains(query))
                    {
                        res = childTypeTrees[i]->get(query, false);
                        break;
                    }
                    gstd::error("cannot find queries type");
                }
            }
            
            if(cache)
                typeMapExpanded[query] = res;

            return res;          
        } 
        
        bool TypeTree::inherits(std::string super, std::string sub)
        {
            std::map<std::string, bool> map = get(sub, true);
            if(super == sub || map.count(super) == 1)
                return true;
            else
                return false;
        }
        
        TypeTree gstdTypeTree::makeInst()
        {
            TypeTree tree;
            
            std::map<std::string,std::vector<std::string> > res;
            res["gstd::ErasureBaseInterface"] = {};
            
            tree.typeMap = res;  
            
            return tree;
        }
         
        TypeTree gstdTypeTree::inst = gstdTypeTree::makeInst();
    }
}
