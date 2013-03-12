/*
 * exprinfo.cpp
 *
 *  Created on: Nov 28, 2012
 *      Author: associatedean(ROHIT)
 */

#include "exprinfo.h"

ExprInfo::ExprInfo()
{
}

ExprInfo::ExprInfo(int Type, string value)
{
    type = Type;
    if(type == IDENTIFIER){
            identifier_value = value;
    }else if(type == LITERAL){
            literal_value = value;
    }else if(type == OPERATOR){
            operator_value = value;
    }
}

ExprInfo::~ExprInfo()
{

}


