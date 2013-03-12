//#include "directorypage.h"
#include "globalstructures.h"
#include "schema.h"
#pragma pack(1)

#define CONSTRAINTNOTNULL 0
#define CONSTRAINTPK 1
#define CONSTRAINTNULL 2
#define CONSTRAINTDEFAULT 3

#define CREATEFAILEDTABLEALREADYEXISTS 4
#define CREATETABLESUCCESS 5


#define UPDATEFAILEDNOSUCHTABLE 0
#define UPDATEFAILEDTYPEMISMATCH 2
#define UPDATEEVALSUCCESS 3


#define INDEXCREATEFAILEDNOSUCHTABLE 0
#define INDEXCREATEFAILEDNOSUCHCOLUMN 1
#define INDEXCREATEFAILEDSUPLICATEINDEXNAME 2


#define INSERTFAILEDNOSUCHTABLE 0
#define INSERTIONFAILEDWRONGNOOFARGUMENTS 1
#define INSERTIONFAILEDTYPEMISMATCH 2
#define INSERTIONFAILEDVARCHARLENGTHMISMATCH 3

#define SELECTFAILEDNOSUCHTABLE 0
#define SELECTEMPTYTABLE 1
#define SELECTFAILEDTYPEMISMATCH 2
#define SELECTEVALSUCCESS 3

#define DELETEFAILEDNOSUCHTABLE 0
#define DELETEEMPTYTABLE 1
#define DELETEFAILEDTYPEMISMATCH 2
#define DELETEEVALSUCCESS 3

#define SUCCESSINDEXUSED 1

#define NOSUCHCOLUMN -1

#define DROPTABLENOTFOUND -1

//These operators are useful in Where condition
#define EQUAL 0
#define GT 1
#define LT 2
#define GTE 3
#define LTE 4
#define NOTEQUAL 5
//#define O_AND 6
//#define O_OR 7

//THIS WILL BE USED WHEN UNMATCH OF WHERE CLAUSE HAPPENS
#define NOTMATCH 100


#define getDirectoryEntry(p,i) ((char *)p + sizeof(DirectoryPage)-1 + (i-1)*sizeof(DirectoryEntry))
//since starting from 1 so no problem...so directly considering i...since we are inserting from last so we are moving to last and from there insertion is going on
#define Slot(p,i) *((slotentry *)((char *)p + PAGE_SIZE - (sizeof(slotentry)*i)))


typedef struct
{
	int pagenumber;
	int priority;
	long dirPageNo;
	long cfs;
	int cfsptr;
	int slotcount;
	char data[1];
}datapage;


typedef struct
{
	int slotaddress;
	int slotsize;

}slotentry;





typedef struct
{
	int pageNumber;//8 bytes
	int priority;//8 bytes
	long nextDirPageNumber;//8 bytes
	//This will be used when we are adding a new directory entry this entry number will be helpful
	int currNoOfDE;//4 bytes
	int maxNoOfDE;//4 bytes
	int maxFreeSpace;//4 bytes
	char pageData[1];//4 bytes
}DirectoryPage;

typedef struct
{
	int dataPageNumber;
	long totalFreeSpace;
}DirectoryEntry;




typedef struct {
    int pageNumber;//pagenumber of the dbheader page
    int priority;
    //int state;//this will be used while deleting the page just to say that there is a free page
    long sysTablePageNumber;
    long sysColumnPageNumber;
    long sysIndexPageNumber;
    long nextFreePageNumber;//From here only we will get the what is the nextfreepage
    //While adding every table i.e into SYS TABLE this entry will be used as tid
    long numberOfTables;
    //While adding every column i.e into SYS COLUMN this entry will be used as cid
    long numberOfColumns;
    long numberOfIndexes;
    int pageSize;
    char databaseName[40];
    //long freeCounter;
    char pageData[1];
}MainDBHeader;

MainDBHeader *maindbHeader;


//Index related structures

typedef struct{
    int pageNumber;
    int priority;
    long rootPageNumber;
    int fanout;
    int keyType;
    int keySize;
    char pageData[1];
}IndexHeaderPage;

//Indexing Nodes but taken for temporary purpose

/*
typedef struct node
{
long pageNumber;
long leftPageNumber;
long rightPageNumber;
long parentPageNumber;
int priority;
int num_keys;
bool isleaf;
char pageData[1];
}Node;
*/

/*
 *
 *
 typedef struct RecordIdentifier{
    int pageNumber;
    int slotNo;
}RecordIdentifier;*/






void initialize_defaultavalues();//Initializing some default values i.e pagesize and creating object for globalStructures
int createdatabase(char *dbname);
int initFreeList(char *dbname);
long getFreePage();
int persistence_initially(char *dbname,int priority,void *buffer);
//after alligning the data i.e into record inserting into table which is having directoryPageNumber i.e calling from insertRoutine or create Database i.e for creating tables..
//From here after identifying the exact data page calling the insertRecord
int insertIntoTable(char* record, long directoryPageNumber,int recordSize);
int readPage(void *buffer,long pagenumber);
int writePage(void *buffer,long pagenumber);
//We are creating the new directoryPage with page number directoryPageNumber
void initDirectoryPage(long directoryPageNumber);
//In searchDirectoryEntry we are finding is there any existing Directory Entry which is enough for this data entry if not return -1 for creating
//new slot entry
int searchDirectoryEntry(char* record, int recordSize,DirectoryPage *directoryPage);
//We are creating the new dataPage with page number dataPageNumber
void initDataPage(long dataPageNumber,long directoryPageNumber);
//For inserting entry into table this will be called by parser
int insertRoutine();
int insertRecord(char *record,int recordSize,long dataPageNumber);
void updateMaxFreeSpace(DirectoryPage* directoryPage);
void showTables();
int createTable();
//For checking whether tableName is there or not..if so then filling the tid,theaderpage values
bool searchSysTables(string tableName,long &tid,long &theaderpage);
int updateTable();
int getInfoFromSysColumns(long tid,int &numColumns,vector<int> &typesVector,vector<int> &constraintsVector,vector<int> &lengthsVector,vector<string> &columnNamesVector,vector<int> &offsetsVector);
void printRecord();
void prepareRecord(char *record);
void usedatabase(char *dbname);
int selectFromSysTable();
int selectFromSysColumn();
int selectFromSysIndex();
int selectFromTable();
int checkTypesAndColumnNamesInWhereList(vector<int> &typesVector,vector<string> &columnNamesVector);
//int postfixEval(char *record,int &numColumns,vector<string> &columnNamesVector,vector<int> &typesVector);
//int updateTable();
int checkTypesAndColumnNamesInUpdateList(vector<int> &typesVector,vector<string> &columnNamesVector,vector<int> &updateOffsetVector);
//columnNamesVector will be used for generating the offset
//whereOffset will specify in where clause the column names offset
//values will specify in where clause the values of each column name
void getInfoRegardingWhere(vector<string> columnNamesVector,vector<int> &whereOffset,vector<string> &values,vector<string> &operators);
//For mapping operator
int mapStringToInteger(string opr);
//For Evaluation this will be useful
int evaluate(int Ltype,string Rvalue,int oper,string Lvalue);
int deleteFromTable();
int dropTable(string tableName);
bool deletePageNumber(long pageNumber);
//just to display the contents of the page
void printMainDBPage();
void persistMainDBPage();
//creation of index
int createIndex();
bool searchSysIndex(string indexName,long &indexId,long &indexHeaderPageNumber);
void getFieldFromRecord(char *record,char* &field,short &fieldLength,int &offset,int &numColumns);
void getAllIndexesOnTable(string tableName,vector<string> &indexedColumnNames,vector<long> &indexHeaderPages);
void getRecordsForRIDS(vector<RecordIdentifier> &ridVector,vector<int> &typesVector);
string getRecord(char *record,vector<int> &typesVector);
int getting_number_of_columns();

int PAGE_SIZE;//just for comfort
int pagesize;
int fd_db;
char path[256];
globalStructures *globalstructures;
int number_of_columns;



