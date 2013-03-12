/*
 * exprinfo.h
 *
 *  Created on: Nov 28, 2012
 *      Author: associatedean
 */

#ifndef EXPRINFO_H

#define EXPRINFO_H
#include <string>

using namespace std;

#pragma pack(1)//Aligns structure members on 1-byte boundaries, or on their natural alignment boundary, whichever is less.

class ExprInfo
{
public:
    ExprInfo();
   ~ExprInfo();
   ExprInfo(int Type, string value);
   int type;
   static const int LITERAL = 0;
   static const int IDENTIFIER = 1;
   static const int OPERATOR = 2;
   static const int RESULT_TYPE = 3;
   string literal_value;
   string identifier_value;
   string operator_value;
   bool result;
};

#endif // EXPRINFO_H


