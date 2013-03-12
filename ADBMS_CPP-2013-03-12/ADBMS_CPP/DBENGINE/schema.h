/*
 * schema.h
 *
 *  Created on: Nov 7, 2012
 *      Author: associatedean
 */

#ifndef SCHEMA_H_
#define SCHEMA_H_
#include <string>
#include <vector>
using namespace std;

//This will be used for create table statement...before calling create table() parser will fill these entries
//These entries will be filled via globalstructures class..
class Schema
{
public:
		Schema();
	    ~Schema();
		string tableName;
		int noofcolumns;
		//multiple column names will be there while creating table,even while index creation also useful
		vector <string> columnnames;
		//Each column name will have the fieldType all those are #defined in dbheader.h
		vector <int> fieldType;
		//For varchar this will be useful since while creating table we will fill that
		vector <int> field_length;
		//For constraints if any
		vector <string> constraints;
		//For default values if any
		vector <string> default_values;
};



#endif /* SCHEMA_H_ */
