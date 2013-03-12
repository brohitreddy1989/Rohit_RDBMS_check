/*
 * conversions.cpp
 *
 *  Created on: Nov 7, 2012
 *      Author: associatedean
 */
#include "conversions.h"
#include <iostream>
#include <sstream>

using namespace std;


conversion::conversion()
{
}

string conversion::int_to_string(int val)
{
	ostringstream mystr;
	mystr<<val;
	return mystr.str();
}

int conversion ::string_to_int(string val)
{
        int res = 0;
        istringstream mystr(val);
        mystr >> res;
        return res;
}

string conversion ::char_to_string(char *val)
{
    string res(val);
    return res;
}


