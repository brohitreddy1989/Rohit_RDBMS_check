/*
 * globalstructures.h
 *
 *  Created on: Nov 6, 2012
 *      Author: associatedean(ROHIT)
 */

#ifndef GLOBALSTRUCTURES_H_
#define GLOBALSTRUCTURES_H_
#include <vector>
#include <string>
#include "schema.h"
#include "exprinfo.h"
using namespace std;

class globalStructures
{
public:
	globalStructures();
	~globalStructures();
	string errorMsg;
	vector <string> resultSetColumnList;//This is filled while parsing to specify what are the colmns to fill up
	vector <string> resultSet;//This will be used to fill the values
	vector <string> insert_values;//This will be used while inserting i.e values that are going to be inserted
	vector <int> insert_type;//This will be used to specify the type of the inserted values...parser will fill this...used to check whether valid type or not
	//Where clause filling...
	//where expression usually contains 3 entries i.e for example where id=110; i.e 'id' is identifier,'=' is operator,'110' is literal
	vector <ExprInfo> whereExprList;
	vector <string> update_fields;//This is used in updating for specifying fields
	vector <int> update_type;//This will be used to specify the data types while updating
	vector <string> update_values;

	//char QueryString[1024];
	//Schema will be used while creating table parser will add entries using schema
	Schema schema;
	string tablename;
	string indexName;
	string operators;//This will be used in where clause to specify.....what is the operator i.e AND , OR
	bool allColumns;

};




#endif /* GLOBALSTRUCTURES_H_ */
