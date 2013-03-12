/* errors to be resolved
	1. INT not accepting single digit nubers
	2. calling  .y && .l files from a c file
	3. handling integer attributes in insert query
*/
#ifndef globalvariables_h
#define globalvariables_h
#include <vector>
#include <string>

using namespace std;

//for select query
vector<string> attributes;
vector<string> tables;
int attrCount;

//for create query && drop query
char *dbname;
char *tabname;
vector<string> coloumns;
vector<string> datatype;
vector<int> fieldlength;
int No_Of_Coloumns;


//for insert query
vector<string> coloumnList;
vector<string> insertAttributes;
vector<int> inserttype;
#endif
