/*
 * conversions.h
 *
 *  Created on: Nov 7, 2012
 *      Author: associatedean
 */

#ifndef CONVERSIONS_H_
#define CONVERSIONS_H_
#include <string>
using namespace std;

class conversion
{
public:
	conversion();
	static string int_to_string(int val);
	static int string_to_int(string val);
	static string float_to_string(float val);
	static float string_to_float(string val);
	static string char_to_string(char *val);
};



#endif /* CONVERSIONS_H_ */
