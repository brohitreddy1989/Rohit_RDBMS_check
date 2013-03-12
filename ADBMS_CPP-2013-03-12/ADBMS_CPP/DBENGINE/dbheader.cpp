#include<iostream>//For file operations in c++
#include<stdio.h>
//#include<stdlib.h>
#include<time.h>//For calculating the time
#include<ctime>
#include<fcntl.h>
#include<string.h>
#include<string>
//#include<cstring>


#include "indextree.h"

#include <iostream>//For file operations in c++
#include <fstream>//For file operations
#include "dbheader.h"
#include "globalstructures.h"
#include "conversions.h"
#include "schema.h"
#include "exprinfo.h"
#include <algorithm>//used in find operation i.e searching the element from the starting address to ending address
#include <vector>
//#include "datapage.h"
//#include "dbheader.h"
using namespace std;


void initialize_defaultavalues()
{
	PAGE_SIZE=1024*4;
	globalstructures=new globalStructures();
}

//creating the file if it is not there and also setting some variables
int initialize_path(char *dbname)
{


	//printf("%s\n",path);
	fflush(stdout);
	strcpy(path,dbname);
	//printf("%s\n",path);
	fflush(stdout);
	fd_db=open(path,O_RDWR);

	if(fd_db>0)
	{
					printf("File already exist");
					fflush(stdout);
					return 0;
	}

	fd_db=open(path,O_CREAT,0666);

	if(fd_db==-1)
	{
					printf("\n File Cannot Be Created");
					fflush(stdout);
					return 0;
	}

	close(fd_db);
	return 1;

}


int initFreeList(char *dbname)
{
	long counter=3;
	long i=2;
	int check;
	char *buffer=(char*)calloc(1,PAGE_SIZE);

	maindbHeader->nextFreePageNumber=1;//If you need any page you need to come here for the pageno
	//maindbHeader->freeCounter=-1;

	memcpy(&buffer[0],&i,sizeof(i));//writing 2
	//maindbHeader->state=0;


	check=initialize_path(dbname);//creating the database for persistent storage
	if(check==0)
	{
		printf("File Not created successfully");
		free(buffer);
		return -1;
	}

	check=writePage(buffer,1);//writing the buffer into the data page
	if(check==-1)
	{
		printf("Unable to write the page");
		free(buffer);
		return -1;
	}
	persistMainDBPage();//This will make the data persistent writePage(maindbHeader,0);
	free(buffer);
	return 0;
}

int writePage(void *buffer,long pagenumber)
{
	fd_db=open(path,O_WRONLY, 0644);

	if (fd_db < 0)
	    return -1;

	lseek(fd_db,pagenumber*PAGE_SIZE,SEEK_SET);

	if(write(fd_db,buffer,PAGE_SIZE)==-1)
		return false;


	close(fd_db);
	return true;
}

int readPage(void *buffer,long pagenumber)
{

	fd_db=open(path,O_RDONLY,0644);

	if (fd_db < 0)
	    return -1;

	if(lseek(fd_db,pagenumber*PAGE_SIZE,SEEK_SET) == -1)
	    return false;
	if(read(fd_db,buffer,PAGE_SIZE)==-1)
	    return false;


	close(fd_db);
	return true;
}

long getFreePage()
{
	long freePage;
	long temp;

	//char *buffer=(char*)malloc(PAGE_SIZE);

	//if(maindbHeader->state == 0)//currently i am working only on creation while deletion state 1
	//{							//will be useful
		freePage = maindbHeader->nextFreePageNumber;
		maindbHeader->nextFreePageNumber++;

		//printf("Free Page Number--->%d\n",freePage);
		return freePage;
	//}

	//return 0;//just for completion sake i kept this
}


int createDatabase(char *dbname)
{



	maindbHeader = (MainDBHeader*) calloc(1,PAGE_SIZE);

	initFreeList(dbname);//initializing it and creating a file for storing

	//printf("%s %d\n",dbname,PAGE_SIZE);
	fflush(stdout);



	maindbHeader->numberOfColumns = 0;//Initially so 0
	maindbHeader->numberOfIndexes = 0;
	maindbHeader->numberOfTables = 0;

	maindbHeader->pageNumber = 0;//Remember always the page number of the DBHeaderPage will be 0
	maindbHeader->priority = 0;//0 is the highest priority
	maindbHeader->sysTablePageNumber = 1;//Always the page number of the sysTablePageNumber will be 1
	maindbHeader->sysColumnPageNumber = 2;
	maindbHeader->sysIndexPageNumber = 3;
	maindbHeader->pageSize = PAGE_SIZE;
	strcpy(maindbHeader->databaseName,dbname);

	//Initialize Systables,Syscolumns, SysIndex DirectoryPages
	DirectoryPage *directoryPage;
    int i=0;
	for(i=1;i<=3;i++)
	{
		directoryPage = (DirectoryPage*) calloc(1,PAGE_SIZE);
		directoryPage->pageNumber = getFreePage();

		directoryPage->priority = 1;
		directoryPage->currNoOfDE = 0;
		directoryPage->maxNoOfDE = (PAGE_SIZE - sizeof(DirectoryPage)-1)/sizeof(DirectoryEntry);
		directoryPage->maxFreeSpace = PAGE_SIZE - (sizeof(datapage)-1);
		directoryPage->nextDirPageNumber = -1;

		writePage(directoryPage,directoryPage->pageNumber);
		free(directoryPage);

	}

	int offset=0;

	int sysTableRecordSize=0;
	int sysColumnRecordSize=0;

	char *sysTableRecord;//For inserting record into the sysTable we are bringing them into proper format
	char *sysColumnRecord;//For inserting record into the sysColumn we are bringing them into proper format

	// sysTables
	long tableTid;
	char tableName[40];
	long tableHeaderPage;

	// sysColumns
	long columnTableId;
	long columnId;
	char colName[30];
	int fieldType;
	int fieldLength;
	int constraint;
	char defaultValues[30];

	// sysIndex
	long indexId;
	char indexName[30];
	char indexTableName[40];
	char indexColName[40];
	long indexHeaderPage;

	sysTableRecordSize=sizeof(tableTid)+sizeof(tableName)+sizeof(tableHeaderPage);
	
	sysColumnRecordSize=(sizeof(tableTid)+sizeof(columnId)+sizeof(colName)+sizeof(fieldType)+sizeof(fieldLength)+sizeof(constraint)+sizeof(defaultValues));
	

	int i1=0;
	//Add SYS TABLE record into SYSTABLE
	for(i1=0;i1<3;i1++)
	{



		//while creating database we will create 3 tables by default
		//so it will be repeated 3 times
		 sysTableRecord=(char*)calloc(1,sysTableRecordSize);
		 tableTid=maindbHeader->numberOfTables++;


		 if(i1 == 0)
		        strcpy(tableName,"SYS_TAB");
		 else if(i1 == 1)
		        strcpy(tableName,"SYS_COL");
		 else
		        strcpy(tableName,"SYS_IND");

		 strcat(tableName,"\0");

		 tableHeaderPage=i1+1;
		 	 offset=0;

		  	  //copying the Tid,tableName,tableHeaderPage values into sysTableRecord
			  memcpy(&sysTableRecord[offset],&tableTid,sizeof(tableTid));
			  	  offset+=sizeof(tableTid);
			  memcpy(&sysTableRecord[offset],tableName,sizeof(tableName));
			  	  offset+=sizeof(tableName);
			  memcpy(&sysTableRecord[offset],&tableHeaderPage,sizeof(tableHeaderPage));

			   //inserting into Table the SYS TABLE record this table will always have page number 1
			  //0 will be used for the MainDBHeader Page
			  insertIntoTable(sysTableRecord,1,sysTableRecordSize);

	      //From here entering values of the SYS TABLE into the SYS COLUMN...
		  if(i1==0)
		  {

			  int j=0;
			  //since SYS TABLE will contains 3 entries
			  for(j=0;j<3;j++)
			  {
				  sysColumnRecord=(char*)calloc(1,sysColumnRecordSize);
				  columnTableId=i1;//since 3 tables so i value is enough
				  columnId=maindbHeader->numberOfColumns++;

				  if(j==0)
				  {
				       strcpy(colName,"tid");
				       fieldType=0;//since tid is of int  data type so....#defined in dbheader.h file
				       fieldLength=sizeof(tableTid);
				  }

				  else if(j==1)
				  {
				       strcpy(colName,"tableName");
				       fieldType=3;//since here i know perfectly the size of the tableName i.e char tableName[40]
				       fieldLength=sizeof(tableName);
				  }

				  else
				  {
				       strcpy(colName,"tableHeaderPage");
				       fieldType=0;//since this will be an integer entry so 0.
				       fieldLength=sizeof(tableHeaderPage);
				  }

				  constraint = CONSTRAINTNOTNULL;//check this i defined in dbheader.h
				  strcpy(defaultValues,"NULL");

				  offset=0;

				  memcpy(&sysColumnRecord[offset],&columnTableId,sizeof(columnTableId));
				  offset+=sizeof(columnTableId);
				  memcpy(&sysColumnRecord[offset],&columnId,sizeof(columnId));
				  offset+=sizeof(columnId);
				  memcpy(&sysColumnRecord[offset],colName,sizeof(colName));
				  offset+=sizeof(colName);
				  memcpy(&sysColumnRecord[offset],&fieldType,sizeof(fieldType));
				  offset+=sizeof(fieldType);
				  memcpy(&sysColumnRecord[offset],&fieldLength,sizeof(fieldLength));
				  offset+=sizeof(fieldLength);
				  memcpy(&sysColumnRecord[offset],&constraint,sizeof(constraint));
				  offset+=sizeof(constraint);
				  memcpy(&sysColumnRecord[offset],defaultValues,sizeof(defaultValues));

				  //why the page number is 2??? since the SYS COLUMN page number is 2...always
				  insertIntoTable(sysColumnRecord,2,sysColumnRecordSize);
				  free(sysColumnRecord);


			  }//end of inner for loop
		  }//end of it loop

		  //From here entering the SYS COLUMN entries into the SYS COLUMN table
		  if(i1==1)
		  {

			  int j=0;
			  //There will be 7 entries check above
			  for(j=0;j<7;j++)
			  {
				  sysColumnRecord=(char*)calloc(1,sysColumnRecordSize);
				  //here i1 is enough for columnTableId, since 3 table entries only i1 also varies from 0 to 2
				  columnTableId=i1;
				  columnId=maindbHeader->numberOfColumns++;
				  offset=0;

				  if(j==0)
				  {
					  strcpy(colName,"tid");
					  fieldType=0;//since integer values for tid
					  fieldLength=sizeof(tableTid);
				  }

				  else if(j==1)
				  {
				       strcpy(colName,"cid");
				       fieldType=0;//since integer values for tid
				       fieldLength=sizeof(columnId);
				  }

				  else if(j==2)
				  {
				       strcpy(colName,"colName");
				       fieldType=3;//since here i know perfectly the size of the colName i.e char colName[30]
				       fieldLength=sizeof(colName);
				  }

				  else if(j==3)
				  {
				       strcpy(colName,"fieldType");
				       fieldType=0;//since integer values for tid
				       fieldLength=sizeof(fieldType);
				  }

				  else if(j==4)
				  {
				       strcpy(colName,"fieldLength");
				       fieldType=0;//since integer values for tid
				       fieldLength=sizeof(fieldLength);
				  }

				  else if(j==5)
				  {
				       strcpy(colName,"constraints");
				       fieldType=3;//since here i know perfectly the size of the colName i.e char colName[30]
				       fieldLength=sizeof(constraint);
				  }

				  else
				  {
				       strcpy(colName,"defaultValues");
				       fieldType=3;//since here i know perfectly the size of the colName i.e char colName[30]
				       fieldLength=sizeof(defaultValues);
				  }

				  constraint = CONSTRAINTNOTNULL;
				  strcpy(defaultValues,"NULL");
				  offset=0;

				  memcpy(&sysColumnRecord[offset],&columnTableId,sizeof(columnTableId));
				  offset+=sizeof(columnTableId);
				  memcpy(&sysColumnRecord[offset],&columnId,sizeof(columnId));
				  offset+=sizeof(columnId);
				  memcpy(&sysColumnRecord[offset],colName,sizeof(colName));
				  offset+=sizeof(colName);
				  memcpy(&sysColumnRecord[offset],&fieldType,sizeof(fieldType));
				  offset+=sizeof(fieldType);
				  memcpy(&sysColumnRecord[offset],&fieldLength,sizeof(fieldLength));
				  offset+=sizeof(fieldLength);
				  memcpy(&sysColumnRecord[offset],&constraint,sizeof(constraint));
				  offset+=sizeof(constraint);
				  memcpy(&sysColumnRecord[offset],defaultValues,sizeof(defaultValues));

				  insertIntoTable(sysColumnRecord,2,sysColumnRecordSize);

				  free(sysColumnRecord);


			  }//end of inner for loop

		  }//end of if loop

		  //Inserting the SYS INDEX entries into the SYS COLUMN
		  if(i1==2)
		  {
			  int j;
			  //Since there are only 5 entries check above
			  for(j=0;j<5;j++)
			  {
				  sysColumnRecord=(char*)calloc(1,sysColumnRecordSize);
				  columnTableId=i1;//since 3 tables only so i1 is enough
				  columnId=maindbHeader->numberOfColumns++;

				  if(j==0)
				  {
				            strcpy(colName,"indexId");
				            fieldType=0;//Since this will be an int type so 0
				            fieldLength=sizeof(indexId);
				  }

				  else if(j==1)
				  {
				            strcpy(colName,"indexName");
				            fieldType=2;
				            fieldLength=sizeof(indexName);
				  }

				  else if(j==2)
				  {
				             strcpy(colName,"indexTableName");
				             fieldType=2;
				             fieldLength=sizeof(indexTableName);
				  }

				  else if(j==3)
				  {
				             strcpy(colName,"indexColName");
				             fieldType=2;
				             fieldLength=sizeof(indexColName);
				  }

				  else if(j==4)
				  {
				             strcpy(colName,"indexHeaderPage");
				             fieldType=0;
				             fieldLength=sizeof(indexHeaderPage);
				  }

				  constraint = CONSTRAINTNOTNULL;
				  strcpy(defaultValues,"NULL");
				  offset=0;
				  memcpy(&sysColumnRecord[offset],&columnTableId,sizeof(columnTableId));
				  offset+=sizeof(columnTableId);
				  memcpy(&sysColumnRecord[offset],&columnId,sizeof(columnId));
				  offset+=sizeof(columnId);
				  memcpy(&sysColumnRecord[offset],colName,sizeof(colName));
				  offset+=sizeof(colName);
				  memcpy(&sysColumnRecord[offset],&fieldType,sizeof(fieldType));
				  offset+=sizeof(fieldType);
				  memcpy(&sysColumnRecord[offset],&fieldLength,sizeof(fieldLength));
				  offset+=sizeof(fieldLength);
				  memcpy(&sysColumnRecord[offset],&constraint,sizeof(constraint));
				  offset+=sizeof(constraint);
				  memcpy(&sysColumnRecord[offset],defaultValues,sizeof(defaultValues));

				  insertIntoTable(sysColumnRecord,2,sysColumnRecordSize);


				  free(sysColumnRecord);

				}//end of inner for loop

		  }//end of if i==2 loop

		  	  writePage(maindbHeader,0);
		  	 free(sysTableRecord);
	}//end of outer for loop

	return 0;
}

//where the record should be inserted will be decided by the directoryPageNumber and size is recordSize
int insertIntoTable(char* record, long directoryPageNumber,int recordSize)
{

	//printf("\ndirectoryPageNumber---->%d recordSize---->%d",directoryPageNumber,recordSize);

	if(recordSize>PAGE_SIZE)
		return false;

	//Reading the directoryPage specified in the directoryPageNumber
	DirectoryPage *directoryPage = (DirectoryPage*)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;
	readPage(directoryPage,directoryPageNumber);


	int slotSize=sizeof(slotentry);
	long dataPageNumber;
	int flag = 0;

	//checking whether we can fill the record in the particular directoryPage i.e since data is inserted into the datapage via the directory Page
	while((recordSize+slotSize) > directoryPage->maxFreeSpace)
	{
		//Go to next directory page
		directoryPageNumber = directoryPage->nextDirPageNumber;

		//This means this is the last directory page we cannot go to the next directory page
		if(directoryPageNumber < 0)
		{
		        flag = true;
		        break;
		}

		//This means going to the next directory page, i.e checking until the condition in while loop satisfy
		readPage(directoryPage,directoryPageNumber);
	}

	//you need to create a new directory page.since we had searched until the last directory page and no perfect match was found
	if(flag)
	{
		//creating a new directory page
		long tempDirPageNumber=directoryPageNumber;
		directoryPageNumber=getFreePage();
		directoryPage->nextDirPageNumber=directoryPageNumber;
		writePage(directoryPage,directoryPage->pageNumber);//writing it to disk the reason is for updating the nextDirPageNumber
		//This will create a new directory Page with the directory page number and will be stored also
		initDirectoryPage(directoryPageNumber);
		//Now reading the directoryPage since now here we need to insert
		readPage(directoryPage,directoryPageNumber);
	}

	//Upto now we found out the which directory Page now searching with directory entry in the directory page

	int dirEntryNumber=searchDirectoryEntry(record,recordSize,directoryPage);//In searchDirectoryEntry we are finding is there any existing
																			 //Directory Entry which is enough for this data entry

	//creating a new directory entry since <0
	if(dirEntryNumber<0)
	{
		//creating a new datapage for the new directory entry
		dataPageNumber=getFreePage();
		dirEntry.dataPageNumber=dataPageNumber;
		//the -1 is because in the datapage structure we had defined char data[1] which includes data
		dirEntry.totalFreeSpace=PAGE_SIZE-(sizeof(datapage)-1);
		//entering this new data in the directoryPage
		memcpy(getDirectoryEntry(directoryPage,(directoryPage->currNoOfDE+1)),&dirEntry,sizeof(dirEntry));
		directoryPage->currNoOfDE++;
		initDataPage(dataPageNumber,directoryPageNumber);
		insertRecord(record,recordSize,dataPageNumber);

		dirEntryNumber = directoryPage->currNoOfDE;
	}
	else
	{
		memcpy(&dirEntry,getDirectoryEntry(directoryPage,dirEntryNumber),sizeof(dirEntry));
		insertRecord(record,recordSize,dirEntry.dataPageNumber);
	}

	int temp = dirEntry.totalFreeSpace;
	dirEntry.totalFreeSpace -= (recordSize + sizeof(slotentry));
	memcpy(getDirectoryEntry(directoryPage,dirEntryNumber),&dirEntry,sizeof(dirEntry));

	//update MaxFreeSpace in directoryPage
	//This will be useful for faster checking i.e whether there is any chance of inserting data in this directory page or not
	//since if the MaxFreeSpace is not enough than it will immediately goes through the next directory page
	if(directoryPage->currNoOfDE == directoryPage->maxNoOfDE)
		updateMaxFreeSpace(directoryPage);

	writePage(directoryPage,directoryPageNumber);;
	free(directoryPage);

	return 1;
}

void updateMaxFreeSpace(DirectoryPage* directoryPage)
{
	DirectoryEntry dirEntry;

	//Find max of all the DE and update MaxFreeSpace
	int maxFreeSpace = 0,i=0;
	for(i=1;i<=directoryPage->currNoOfDE;i++)
	{
		memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
		if(maxFreeSpace < dirEntry.totalFreeSpace)
		      maxFreeSpace = dirEntry.totalFreeSpace;
	}
	directoryPage->maxFreeSpace = maxFreeSpace;
}





void initDirectoryPage(long directoryPageNumber)
{
	DirectoryPage *directoryPage = (DirectoryPage*) calloc(1,PAGE_SIZE);
	//printf("DirectoryPage-->%d\n",directoryPageNumber);
	directoryPage->pageNumber = directoryPageNumber;
	directoryPage->priority = 1;
	directoryPage->currNoOfDE = 0;
	directoryPage->maxNoOfDE = (PAGE_SIZE - (sizeof(DirectoryPage)-1))/sizeof(DirectoryEntry);
	directoryPage->maxFreeSpace = PAGE_SIZE - (sizeof(datapage)-1);
	directoryPage->nextDirPageNumber = -1;//always remember that -1 shows the end of the list of directory pages
	writePage(directoryPage,directoryPageNumber);
	readPage(directoryPage,directoryPageNumber);
	//printf("directoryPage->priority---->%d\n",directoryPage->priority);
	free(directoryPage);
}

//In searchDirectoryEntry we are finding is there any existing Directory Entry which is enough for this data entry if not return -1 for creating
//new slot entry
int searchDirectoryEntry(char* record, int recordSize,DirectoryPage *directoryPage)
{
	DirectoryEntry dirEntry;
	int i=0;
	datapage *dataPage=(datapage *)calloc(1,PAGE_SIZE);
	//checking one by one slot i.e checking the total free space and then checking the cfs also...since no defragmentation has taken place
	for(i=1;i<=directoryPage->currNoOfDE;i++)
	{
		//copying the Directory Entry i into the dirEntry
		memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
		//checking whether the totalFreeSpace is enough or not if enough then check the cfs also since no defragmentation so...
		if(dirEntry.totalFreeSpace>=(recordSize+sizeof(slotentry)))
		{
			//checking whether in cfs will it works or not since we are not doing defragmentation work so we need to check the cfs also
			readPage(dataPage,dirEntry.dataPageNumber);
			//if cfs also satisfied then only break if not go and check the next dirEntry and slot
			if(dataPage->cfs>=(recordSize+sizeof(slotentry)))
				break;
		}
	}
	//means unable to found out the directoryPage so return -1 means create a new directoryPage
	if(i>directoryPage->currNoOfDE)
	{
		free(dataPage);
		return -1;
	}
	free(dataPage);
	return i;
}

//creating a new dataPage of dataPageNumber and writing them
void initDataPage(long dataPageNumber,long directoryPageNumber)
{
	datapage *dataPage=(datapage *)calloc(1,PAGE_SIZE);////stopped here
	dataPage->cfs=PAGE_SIZE-(sizeof(datapage)-1);
	dataPage->cfsptr=0;
	dataPage->dirPageNo=directoryPageNumber;
	dataPage->pagenumber=dataPageNumber;
	dataPage->priority=2;
	dataPage->slotcount=0;
	writePage(dataPage,dataPageNumber);
	free(dataPage);
}

int insertRecord(char *record,int recordSize,long dataPageNumber)
{
	datapage *dataPage=(datapage *)calloc(1,PAGE_SIZE);
	readPage(dataPage,dataPageNumber);

	slotentry slot;

	//for checking and freeslotnumber storage
	int freeSlotNumber=-1;
	int negativeFlag=0;
	int freeFlag=0;
	int i=0;

	//checking all the slot counts since is there any slot which has been deleted so
	for(i=1;i<dataPage->slotcount;i++)
	{
		//simply here we are copying the slot of Slot(dataPage,i)---->slot
		slot=Slot(dataPage,i);
		//If i found out a slot which has been deleted but its size exactly matches
		if(slot.slotsize*-1 == recordSize)
		{
			negativeFlag=1;
			break;
		}

		//This will be helpful while defragmentation since while defragmenting we won't delete the slot we will just erase the data and
		//makes the slot size as 0 and pointer points to CFS
		if(slot.slotsize == 0)
		{
			freeSlotNumber=i;//Here we are trying to go to the last free slot so that next time no need to search unnecessarily
			freeFlag=1;
		}
	}



	int returnFlag=0;//For checking in the below cases whether we had found out any slot

	//This should be given atmost importance since although we had deleted the slot we got the perfect match
	//that's why this if loop at first
	if(negativeFlag)
	{
		//add your record at slot i which has been identified in the above for loop
		slot=Slot(dataPage,i);
		slot.slotsize=recordSize;
		//since slotsize has been updated so
		Slot(dataPage,i)=slot;
		returnFlag=1;
	}

	//It means after defragmenting we had find out the slot
	else if(freeFlag==1)
	{
		 slot = Slot(dataPage,freeSlotNumber);
		 slot.slotsize = recordSize;
		 slot.slotaddress = dataPage->cfsptr;//since after defragmented slots need to point to the cfsptr..think u will get
		 Slot(dataPage,freeSlotNumber) = slot;//writing back
		 dataPage->cfs -= recordSize;//updating the cfs
		 dataPage->cfsptr += recordSize;//changing the cfsPtr to point to exact location
		 returnFlag=1;
	}

	//It means we had found out one slot for the data i.e exactly at the deleted data slot
	if(returnFlag==1)
	{
		memcpy(&dataPage->data[slot.slotaddress],record,recordSize);
		writePage(dataPage,dataPage->pagenumber);
		free(dataPage);
		return returnFlag;
	}

	//Unable to found out a slot so, create a new slot checking also here no need since in the above i handled this case
	//This case has been given just to check whether we need defragment or not but since defragmentation not implemented
	//so we cannot insert although tfs satisfy so while coming to this data page only i had checked the cfs also
	if(recordSize+sizeof(slotentry)<=dataPage->cfs)
	{
		//printf("%d %d %d %d\n",dataPage->cfs,dataPage->cfsptr,dataPage->dirPageNo,dataPage->pagenumber);
		//fflush(stdout);

		slot.slotsize = recordSize;
		slot.slotaddress = dataPage->cfsptr;
		dataPage->cfs -= (recordSize+sizeof(slotentry));
		dataPage->cfsptr += recordSize;
		dataPage->slotcount++;
		Slot(dataPage,dataPage->slotcount)=slot;

		//printf("%d %d %d %d",dataPage->cfs,dataPage->cfsptr,dataPage->dirPageNo,dataPage->pagenumber);
		//fflush(stdout);
		memcpy(&dataPage->data[slot.slotaddress],record,recordSize);

		//printf("%d %u %u",dataPage->data[0],&dataPage->data[0],&dataPage->data[slot.slotaddress]);
		//fflush(stdout);

		writePage(dataPage,dataPageNumber);
		free(dataPage);
		return 1;
	}

	else
	{
		//defragmenting the page left over
		return 1;//need to change this
	}

	free(dataPage);
}

int createTable()
{
	long tid,theaderpage;//these are used since while storing table name in SYS_TABLE they need
						 //these two parameters also along with table name and while checking
						//whether the table is present or not these are needed.

	//In globalstructures->schema.tableName the table name will be placed by the parser or user and then filling the tid,theaderpage values
	if(searchSysTables(globalstructures->schema.tableName,tid,theaderpage) == true)
	{
	        //globalstructures->errorMsg = "TABLE ALREADY EXISTS";
	        return CREATEFAILEDTABLEALREADYEXISTS;
	}

	//In globalstructures->schema.tableName the table name will be placed by the parser or user
	//Since there is no table having that name so creating that
	char tableName[40];
	strcpy(tableName,globalstructures->schema.tableName.c_str());
	long tableId = maindbHeader->numberOfTables++;
	theaderpage = getFreePage();//This will be the directory page number for the new table
	//printf("theaderpage->%d\n",theaderpage);
	initDirectoryPage(theaderpage);



	//when ever we are creating a new table we need to update that in the SYS TABLE
	//In SYS TABLE the parameters are tid,theaderpage,tableName
	int recordSize = sizeof(tid) + sizeof(theaderpage) + sizeof(tableName);


	//For the sake of SYS TABLE record
	char sysTableRecord[recordSize];
	int offset=0;
	memcpy(&sysTableRecord[offset],&tableId,sizeof(long));
	offset += sizeof(long);
	memcpy(&sysTableRecord[offset],tableName,sizeof(tableName));
	offset += sizeof(tableName);
	memcpy(&sysTableRecord[offset],&theaderpage,sizeof(long));

	insertIntoTable(sysTableRecord,1,recordSize);


	long columnId;
	char colName[30];
	int fieldType;
	int fieldLength;
	int constraint;
	char defaultValues[30];

	recordSize = sizeof(tid) + sizeof(columnId) + sizeof(colName) + sizeof(fieldType) + sizeof(fieldLength) + sizeof(constraint) + sizeof(defaultValues);
	char sysColumnsRecord[recordSize];



	int numColumns = globalstructures->schema.noofcolumns;
	//initDirectoryPage(theaderpage);//This is placed because i am getting an error...for that i.e 09th page number...
								   //check in my google docs the issue...i.e at 1200 index there page number is displaying perfectly
								   //but the priority is getting changed..i.e -1 to some IND...no idea why it is happening
								   //so again setting that so now no issues..(after placing calloc issue is solved)...

	for(int i=0;i<numColumns;i++)
	{
		columnId = maindbHeader->numberOfColumns++;
		strcpy(colName,globalstructures->schema.columnnames.at(i).c_str());
		fieldType = globalstructures->schema.fieldType.at(i);
		fieldLength = globalstructures->schema.field_length.at(i);

			if(strcmp(globalstructures->schema.constraints.at(i).c_str(),"NOT NULL")==0)
			        constraint = CONSTRAINTNOTNULL;
			else if(strcmp(globalstructures->schema.constraints.at(i).c_str(),"PRIMARY KEY")==0)
			            constraint = CONSTRAINTPK;
			else if(strcmp(globalstructures->schema.constraints.at(i).c_str(),"NULL")==0)
			            constraint = CONSTRAINTNULL;
			else if(strcmp(globalstructures->schema.constraints.at(i).c_str(),"DEFAULT")==0)
			            constraint = CONSTRAINTDEFAULT;
		    else
			            constraint = -1;

			strcpy(defaultValues,globalstructures->schema.default_values.at(i).c_str());
			offset = 0;
			memcpy(&sysColumnsRecord[offset],&tableId,sizeof(tableId));
			offset += sizeof(tableId);
			memcpy(&sysColumnsRecord[offset],&columnId,sizeof(columnId));
			offset += sizeof(columnId);
			memcpy(&sysColumnsRecord[offset],colName,sizeof(colName));
			offset += sizeof(colName);
			memcpy(&sysColumnsRecord[offset],&fieldType,sizeof(fieldType));
			offset += sizeof(fieldType);
			memcpy(&sysColumnsRecord[offset],&fieldLength,sizeof(fieldLength));
			offset += sizeof(fieldLength);
			memcpy(&sysColumnsRecord[offset],&constraint,sizeof(constraint));
			offset += sizeof(constraint);
		    memcpy(&sysColumnsRecord[offset],defaultValues,sizeof(defaultValues));
		    insertIntoTable(sysColumnsRecord,2,recordSize);
	}

		writePage(maindbHeader,0);
//		globalstructures->errorMsg ="TABLE CREATED";





		return CREATETABLESUCCESS;
}



//To check whether already there is a table or not....long &tid is a reference to original tid...which is possible in c++
bool searchSysTables(string tableName,long &tid,long &theaderpage)
{
	int i,j;
	long do_while=-1;//For the sake of while condition

	char tName[40];

	//i.e tid,tableName,tableheaderpage
	char *record=(char *)calloc(1,(sizeof(long)+sizeof(tName)+sizeof(long)));

	DirectoryPage *directoryPage = (DirectoryPage*)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;

	slotentry slot;

	datapage *dataPage=(datapage *)calloc(1,PAGE_SIZE);

	//Since SYS TABLE directory page will starts from 1...so
	readPage(directoryPage,1);

	do
	{
		do_while=-1;
		//In the particular directory Page we are checking all directory entries...since the SYS TABLE is a table having the records inserted
		//in the regular table insertion format...so searching one after one directory entries
		for(i=1;i<=directoryPage->currNoOfDE;i++)
		{
			memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
			readPage(dataPage,dirEntry.dataPageNumber);
			for(j=1;j<=dataPage->slotcount;j++)
			{
				slot=Slot(dataPage,j);
				//means either the slot has been deleted or because of defragmentation
				if(slot.slotsize<=0)
					continue;
				//copying the data present in the slot to record
				memcpy(record,&dataPage->data[slot.slotaddress],slot.slotsize);
				//since initially the tid will be the first entry in the record so...reading after that
				memcpy(tName,&record[sizeof(tid)],40);
				//the tablename is present
				if(strcmp(tName,tableName.c_str()) == 0)
				{
					//since already name will be der so no need to copy that
					memcpy(&tid,&record[0],sizeof(tid));
				    //40 is next entry will be the tablename
					memcpy(&theaderpage,&record[sizeof(tid)+40],sizeof(theaderpage));
				    free(record);
				    free(directoryPage);
				    free(dataPage);
				    return true;
				}
			}
		}
		if(directoryPage->nextDirPageNumber!=-1)
					{
						do_while=directoryPage->nextDirPageNumber;
						readPage(directoryPage,directoryPage->nextDirPageNumber);
					}

	}while(do_while!=-1);

	free(record);
	free(directoryPage);
	free(dataPage);

	return false;
}


int insertRoutine()
{
	long tid=-1,theaderpage;
	//checking whether the specified table is there or not....
	searchSysTables(globalstructures->tablename,tid,theaderpage);
	if(tid==-1)
	{
		printf("Table does not exist----Insert Routine\n");
		return INSERTFAILEDNOSUCHTABLE;
	}

	else if(tid < 3 )
	{
	    cout<<"Invalid Operation"+globalstructures->tablename+" cant be inserted";
		return INSERTFAILEDNOSUCHTABLE;
	}


	int numColumns = 0;
	vector<int> typesVector;
	vector<int> constraintsVector;
	vector<int> lengthsVector;
	vector<string> columnNamesVector;
	vector<int> offsetsVector;//This will be used when you are entering only few elements into the table
							  //at that time this offset vector will specify the exact column position i.e whether it is first or second or third.....

	//All the above values are filled and are used
	getInfoFromSysColumns(tid,numColumns,typesVector,constraintsVector,lengthsVector,columnNamesVector,offsetsVector);

	//There are 3 error conditions
	//1.In Insert statement the column names selection and the inserted values won't matches
	//2.If the typesVector won't matches i.e inserted type not matching
	//3.checking for lengths whether acceptable lengths are given or not

	//If in the insert statement the column names selection and the inserted values won't matches
	if(numColumns != globalstructures->insert_values.size())
	{
		printf("numColumns->%d globalstructures->insert_values->%d",numColumns,globalstructures->insert_values.size());
		printf("\n Insert Failed wrong number of arguments %d",tid);
		fflush(stdout);
		return INSERTIONFAILEDWRONGNOOFARGUMENTS;
	}


	//If the typesVector won't matches i.e inserted type not matching
	if(typesVector != globalstructures->insert_type)
	{
	    printf("Insertion Fails because of Type Mismatch");
	    return INSERTIONFAILEDTYPEMISMATCH;
	}

	//checking for lengths whether acceptable lengths are given or not

	int recordSize = 0;//For insertion we need to have the size of the record so....here while checking whether valid size or not for varchar there only filling

	for(int i=0;i<globalstructures->insert_type.size();i++)
	{
		if(globalstructures->insert_type.at(i) == TYPE_VARCHAR)
		{
			if((globalstructures->insert_values.at(i).length()+1) > lengthsVector.at(i))
			{
				printf("\n Insertion Failed because of VARCHAR length mismatch");
				return INSERTIONFAILEDVARCHARLENGTHMISMATCH;
			}

			else
				recordSize += globalstructures->insert_values.at(i).length()+1;
		}

		else
		        recordSize += sizeof(int);//This is for int since we are handling differently for strings and ints sooo....
	}

	recordSize += numColumns*sizeof(short);//This is used for tag offset in prepareRecord.
										   //the size of datatypes will be different in different systems and for varchar..for fast accessing also
										   //Specify the size of each entry for varchar
	char record[recordSize];

	//preparing the record for entry since initially we need to add the size of each entry initially soooo..preparing them into proper format
	prepareRecord(record);

	insertIntoTable(record,theaderpage,recordSize);

}

void prepareRecord(char *record)
{

	//.size() will specify number of entries and initially short will be used for specifying the size of each entry
	int recordOffset = globalstructures->insert_values.size()*sizeof(short);
	int tagOffset = 0;
	int intValue = -1;
	short intSize = sizeof(int);
	short charSize = 0;
	char *charValue = NULL;
	for(int i=0;i<globalstructures->insert_values.size();i++)
	{
		switch(globalstructures->insert_type.at(i))
		{
		case TYPE_INTEGER:
			memcpy(&record[tagOffset],&intSize,sizeof(short));
			tagOffset += sizeof(short);
			intValue=conversion::string_to_int(globalstructures->insert_values.at(i));
			memcpy(&record[recordOffset],&intValue,sizeof(intValue));
			recordOffset += sizeof(int);
			break;
		case TYPE_VARCHAR:
			charSize = globalstructures->insert_values.at(i).length()+1;
			memcpy(&record[tagOffset],&charSize,sizeof(short));
			tagOffset += sizeof(short);
			charValue = (char*)calloc(1,charSize);
			strcpy(charValue,globalstructures->insert_values.at(i).c_str());
			charValue[charSize-1] = '\0';
			memcpy(&record[recordOffset],charValue,charSize);
			recordOffset += charSize;
			//printf("charValue-->%s \n",charValue);
			fflush(stdout);
			free(charValue);
			break;
		}
	}
}




int getInfoFromSysColumns(long tid,int &numColumns,vector<int> &typesVector,vector<int> &constraintsVector,vector<int> &lengthsVector,vector<string> &columnNamesVector,vector<int> &offsetsVector)
{
	int i,j;
	long _tid;
	long cid;
	char colName[30];
	int fieldType;
	int fieldLength;
	int constraint;
	char defaultValues[30];
	int temp_directoryPage_number=-1;//just for do while

	char *record=(char*)calloc(1,(sizeof(_tid)+sizeof(cid)+sizeof(colName)+sizeof(fieldType)+sizeof(fieldLength)+sizeof(constraint)+sizeof(defaultValues)));

	DirectoryPage *directoryPage=(DirectoryPage *)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;
	slotentry slot;
	datapage *dataPage=(datapage *)calloc(1,PAGE_SIZE);
	//Since SYS COLUMN will have pagenumber 2...so
	readPage(directoryPage,2);

	do
	{

		temp_directoryPage_number=-1;
		for(i=1;i<=directoryPage->currNoOfDE;i++)
		{
			memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
			readPage(dataPage,dirEntry.dataPageNumber);

			for(j=1;j<=dataPage->slotcount;j++)
			{
				slot = Slot(dataPage,j);
				if(slot.slotsize <=0)
				     continue;
				memcpy(record,&dataPage->data[slot.slotaddress],slot.slotsize);
				//memcpy(&_tid,&dataPage->data[slot.slotaddress],sizeof(_tid));
				memcpy(&_tid,&record[0],sizeof(_tid));

				if(_tid==tid)
				{
					numColumns++;
					memcpy(colName,&record[2*sizeof(long)],sizeof(colName));//why 2 *sizeof(long) in SYS COLUMNS directorypage the first 2 entries
																			//are of long data type after that only we will get colNames

					columnNamesVector.push_back(conversion::char_to_string(colName));
					memcpy(&fieldType,&record[2*sizeof(long)+sizeof(colName)],sizeof(fieldType));
					typesVector.push_back(fieldType);
					memcpy(&constraint,&record[2*sizeof(long)+sizeof(colName)+2*sizeof(int)],sizeof(constraint));
					constraintsVector.push_back(constraint);
					memcpy(&fieldLength,&record[2*sizeof(long)+sizeof(colName)+sizeof(int)],sizeof(fieldLength));
					lengthsVector.push_back(fieldLength);
				}
			}
		}
		if(directoryPage->nextDirPageNumber!= -1)
		{
			temp_directoryPage_number=directoryPage->nextDirPageNumber;
			readPage(directoryPage,directoryPage->nextDirPageNumber);
		}
		//free(dataPage);
	}while(temp_directoryPage_number!=-1);

	int retValue=0;

	if(globalstructures->allColumns){
	        for(i=0;i<numColumns;i++)
	            offsetsVector.push_back(i);
	}
	else
	{
		vector<string>::iterator myIterator;
		for(i=0;i<globalstructures->resultSetColumnList.size();i++){
			myIterator = find(columnNamesVector.begin(),columnNamesVector.end(),globalstructures->resultSetColumnList.at(i));
			if(myIterator != columnNamesVector.end()){//exists
			       offsetsVector.push_back(int(myIterator-columnNamesVector.begin()));
			       retValue = 2;
			 }
			else{
			       retValue = NOSUCHCOLUMN;
			       globalstructures->errorMsg = "NO SUCH COLUMN:" + globalstructures->resultSetColumnList.at(i);
			       cout<<globalstructures->resultSetColumnList.at(i);
			       break;
			}

		}

	}


	free(record);
	free(directoryPage);
	free(dataPage);
	return retValue;//this will be used while writing else statement... in selectFromTable also....

}


void showTables()
{
	DirectoryPage *directoryPage=(DirectoryPage*)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;
	datapage *dataPage=(datapage*)calloc(1,PAGE_SIZE);

	readPage(directoryPage,1);

	slotentry se;
	char tableName[40];
	long temp_storage_directoryPageNumber;//In do while loop it will be useful

	char *record;
	string tuple;
	long tid,theaderpage;

	globalstructures->resultSetColumnList.push_back("TableName");


	do
	{
		temp_storage_directoryPageNumber=-1;
		for(int i=1;i<=directoryPage->currNoOfDE;i++)
		{
			memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
			readPage(dataPage,dirEntry.dataPageNumber);
			for(int j=1;j<=dataPage->slotcount;j++)
			{
				se=Slot(dataPage,j);
				if(se.slotsize <= 0)
				continue;
				record=(char*)calloc(1,se.slotsize);
				memcpy(record,&dataPage->data[se.slotaddress],se.slotsize);
				memcpy(&tid,&record[0],sizeof(tid));
				memcpy(&tableName,&record[sizeof(long)],sizeof(tableName));
				memcpy(&theaderpage,&record[sizeof(long)+sizeof(tableName)],sizeof(theaderpage));
				tuple=conversion::char_to_string(tableName);
				globalstructures->resultSet.push_back(tuple);
				free(record);
			}
		}
		if(directoryPage->nextDirPageNumber != -1)
		{
			temp_storage_directoryPageNumber=directoryPage->pageNumber;
			readPage(directoryPage,directoryPage->nextDirPageNumber);
		}

	}while(temp_storage_directoryPageNumber!=-1);

	free(dataPage);
	free(directoryPage);
}

int selectFromSysTable()
{
	DirectoryPage *directoryPage=(DirectoryPage*)calloc(1,PAGE_SIZE);
    DirectoryEntry dirEntry;
    datapage *dataPage=(datapage*)calloc(1,PAGE_SIZE);

    readPage(directoryPage,1);

    slotentry se;
    char tableName[40];
    long do_while=-1;

    char *record;
    string tuple;
    long tid,theaderpage;
    globalstructures->resultSetColumnList.push_back("Tid");
    globalstructures->resultSetColumnList.push_back("TableName");
    globalstructures->resultSetColumnList.push_back("DirectoryPage");

    do
    {
       do_while=-1;
       for(int i=1;i<=directoryPage->currNoOfDE;i++)
       {
    	   memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
    	   readPage(dataPage,dirEntry.dataPageNumber);
    	   for(int j=1;j<=dataPage->slotcount;j++)
    	   {
    	                   se=Slot(dataPage,j);
    	                   if(se.slotsize <= 0)
    	                      continue;
    	                   record=(char*)calloc(1,se.slotsize);
    	                   memcpy(record,&dataPage->data[se.slotaddress],se.slotsize);
    	                   memcpy(&tid,&record[0],sizeof(tid));
    	                   memcpy(&tableName,&record[sizeof(long)],sizeof(tableName));
    	                   memcpy(&theaderpage,&record[sizeof(long)+sizeof(tableName)],sizeof(theaderpage));
    	                   tuple = conversion::int_to_string(tid) + "#" + conversion::char_to_string(tableName) + "#" + conversion::int_to_string(theaderpage)+ "#";
    	                   globalstructures->resultSet.push_back(tuple);
    	                   free(record);
    	    }

        }

       if(directoryPage->nextDirPageNumber != -1)
       {
    	     do_while=directoryPage->nextDirPageNumber;
    	     readPage(directoryPage,directoryPage->pageNumber);
       }


    }while(do_while!=-1);



    free(directoryPage);
    free(dataPage);
    return 0;
}


int selectFromSysColumn()
{
	DirectoryPage *directoryPage=(DirectoryPage*)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;
	datapage *dataPage=(datapage*)calloc(1,PAGE_SIZE);
	readPage(directoryPage,2);

	slotentry se;

	long columnTableId;
	long columnId;
	char colName[30];
	int fieldType;
	int fieldLength;

	globalstructures->resultSetColumnList.push_back("Tid");
	globalstructures->resultSetColumnList.push_back("Cid");
	globalstructures->resultSetColumnList.push_back("ColumnName");
	globalstructures->resultSetColumnList.push_back("FieldType");
	globalstructures->resultSetColumnList.push_back("FieldLength");

	char *record;
	string tuple;
	long do_while=-1;
	do
	{
	    do_while=-1;
		for(int i=1;i<=directoryPage->currNoOfDE;i++)
	    {
	    	memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
	    	readPage(dataPage,dirEntry.dataPageNumber);
	    	for(int j=1;j<=dataPage->slotcount;j++)
	    	{
	    		se=Slot(dataPage,j);
	    		if(se.slotsize <= 0)
	    		   continue;

	    		record=(char*)malloc(se.slotsize);
	    		memcpy(record,&dataPage->data[se.slotaddress],se.slotsize);

	    		memcpy(&columnTableId,&record[0],sizeof(columnTableId)); // tid
	    	    memcpy(&columnId,&record[sizeof(columnTableId)],sizeof(columnId));//cid

	    		memcpy(&colName,&record[sizeof(columnTableId)+sizeof(columnId)],sizeof(colName));

	    		memcpy(&fieldType,&record[sizeof(columnTableId)+sizeof(columnId)+sizeof(colName)],sizeof(fieldType));
	    		memcpy(&fieldLength,&record[sizeof(columnTableId)+sizeof(columnId)+sizeof(colName)+sizeof(fieldType)],sizeof(fieldLength));

	    		tuple = conversion::int_to_string(columnTableId)+ "#" +conversion::int_to_string(columnId) + "#" + conversion::char_to_string(colName) + "#" +"#"+conversion::int_to_string(fieldType)+"#"+conversion::int_to_string(fieldLength)+"#";

	    	    globalstructures->resultSet.push_back(tuple);

	    		free(record);
	    	}

	    }
	               if(directoryPage->nextDirPageNumber != -1)
	               {
	                   do_while=directoryPage->nextDirPageNumber;
	            	   readPage(directoryPage,directoryPage->pageNumber);
	               }

    }while(do_while!=-1);

	free(directoryPage);
	free(dataPage);
	return 0;
}


int selectFromSysIndex()
{
	DirectoryPage *directoryPage=(DirectoryPage*)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;
	datapage *dataPage=(datapage*)calloc(1,PAGE_SIZE);

	readPage(directoryPage,maindbHeader->sysIndexPageNumber);

	slotentry se;

	long indexId,indexHeaderPage;
	char indexName[30],tableName[40],colName[30];
	globalstructures->resultSetColumnList.push_back("IndexId");
	globalstructures->resultSetColumnList.push_back("IndexName");
	globalstructures->resultSetColumnList.push_back("TableName");
	globalstructures->resultSetColumnList.push_back("ColumnName");
	globalstructures->resultSetColumnList.push_back("IndexHeaderPage");

	char *record;
	string tuple;
	long do_while=-1;

	do
	{
	    do_while=-1;
		for(int i=1;i<=directoryPage->currNoOfDE;i++)
	    {
	    	memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
	    	readPage(dataPage,dirEntry.dataPageNumber);
	    	for(int j=1;j<=dataPage->slotcount;j++)
	    	{
	    	      se=Slot(dataPage,j);
	    	      if(se.slotsize <= 0)
	    	          continue;
	    	      record=(char*)malloc(se.slotsize);
	    	      memcpy(record,&dataPage->data[se.slotaddress],se.slotsize);

	    	      memcpy(&indexId,&record[0],sizeof(indexId));
	    	      memcpy(&indexName[0],&record[sizeof(long)],sizeof(indexName));
	    	      memcpy(&tableName[0],&record[sizeof(long)+sizeof(indexName)],sizeof(tableName));
	    	      memcpy(&colName[0],&record[sizeof(long)+sizeof(indexName)+sizeof(tableName)],sizeof(colName));
	    	      memcpy(&indexHeaderPage,&record[sizeof(long)+sizeof(indexName)+sizeof(tableName)+sizeof(colName)],sizeof(indexHeaderPage));

	    	      tuple = conversion::int_to_string(indexId) + "#" + conversion::char_to_string(indexName) + "#" + conversion::char_to_string(tableName)+ "#"+conversion::char_to_string(colName)+"#"+conversion::int_to_string(indexHeaderPage);

	    	      globalstructures->resultSet.push_back(tuple);

	    	      free(record);
	    	}
	    }
	   if(directoryPage->nextDirPageNumber != -1)
	   {
	       do_while=directoryPage->nextDirPageNumber;
		   readPage(directoryPage,directoryPage->pageNumber);
	   }

	}while(do_while!=-1);

	free(directoryPage);
	free(dataPage);
	return 0;
}

int selectFromTable()
{
	long tid=-1;
	long theaderpage;
	if(searchSysTables(globalstructures->tablename,tid,theaderpage) == false)
	{
	     globalstructures->errorMsg = "SELECT FAILED NO SUCH TABLE "+globalstructures->tablename;
	     cout<<globalstructures->tablename;
	     return SELECTFAILEDNOSUCHTABLE;
	}

	int numColumns = 0;
	vector<int> typesVector;
	vector<int> constraintsVector;
	vector<int> lengthsVector;
	vector<string> columnNamesVector;
	vector<int> offsetsVector;
	int returnValue = getInfoFromSysColumns(tid,numColumns,typesVector,constraintsVector,lengthsVector,columnNamesVector,offsetsVector);

	if(returnValue == NOSUCHCOLUMN){
	      globalstructures->errorMsg = "NO SUCH COLUMN IN TABLE BEFORE WHERE ";
	      printf("\n NO SUCH COLUMN IN TABLE BEFORE WHERE");
	      return NOSUCHCOLUMN;
	}

	//returnValue = checkTypesAndColumnNamesInWhereList(typesVector,columnNamesVector);

	if(returnValue == NOSUCHCOLUMN || returnValue == SELECTFAILEDTYPEMISMATCH)
	{
	      if(returnValue == NOSUCHCOLUMN )
	      {
	          globalstructures->errorMsg = "NO SUCH COLUMN IN TABLE AFTER WHERE ";

	      }
	      else
	      {
	         // globalstructures->errorMsg = "TYPE MISMATCH IN QUERY AFTER WHERE ";
	      }
	            return returnValue;
    }

	if(globalstructures->whereExprList.size()>0){
		vector<string> indexedColumns;
		vector<long> indexHeaderPages;
		ExprInfo expr;
		string identifierValue;
		vector<string>::iterator myIterator;
		int position = -1;
		getAllIndexesOnTable(globalstructures->tablename,indexedColumns,indexHeaderPages);

		int i=0;
		bool flag = false;
		for(i=0;i<indexedColumns.size();i++){
			flag=true;
			for(int j=0;j<globalstructures->whereExprList.size();j++){
				expr = globalstructures->whereExprList.at(j);
				if(expr.type == ExprInfo::IDENTIFIER){
					identifierValue = expr.identifier_value;
					if(identifierValue!=indexedColumns.at(i)){
						flag = false;//Index doesnot match
						break;
					}//if(identifierValue!=indexedColumns.at(i)){
				}//if(expr.type == ExprInfo::IDENTIFIER){

			}//for(int j=0;j<globalstructures->whereExprList.size();j++){

			if(flag)//An index exists
			  break;
		}//end of for(i=0;i<indexedColumns.size();i++){

		if(flag){
			myIterator = find(columnNamesVector.begin(),columnNamesVector.end(),indexedColumns.at(i));
			position = int(myIterator-columnNamesVector.begin());

			IndexHeaderPage *indexheaderpage = (IndexHeaderPage*)malloc(PAGE_SIZE);
			readPage(indexheaderpage,indexHeaderPages.at(i));

			//---------------------
			bPlusTree bptree(indexheaderpage->rootPageNumber,1,indexheaderpage->fanout,indexheaderpage->keyType,indexheaderpage->keySize);
			//----------------------

			vector<RecordIdentifier> ridVector1;

				char *key;
				int intValue;
				expr = globalstructures->whereExprList.at(2);

				if(typesVector.at(position)==TYPE_INTEGER){
				    intValue = conversion::string_to_int(expr.literal_value);
				    key = (char*)malloc(sizeof(int));
				    memcpy(key,&intValue,sizeof(int));
				}

				else if(typesVector.at(position) == TYPE_VARCHAR){
					key = (char*)malloc(lengthsVector.at(position));
					strcpy(key,expr.literal_value.c_str());
				}

				expr = globalstructures->whereExprList.at(1);
				int opr = mapStringToInteger(expr.operator_value);
				switch(opr){
				                case EQUAL:
				                    //-----------------------
				                	bptree.search(key,ridVector1);
				                	//-------------------------
				                    break;
				                case NOTEQUAL:
				                	//------------------------
				                    //bptree.searchNotEqualKey(key,ridVector1);
				                    //------------------------
				                    break;
				                case LT:
				                	//------------------------
				                    bptree.searchLesserThanKey(key,ridVector1);
				                	//------------------------
				                    break;
				                case GT:
				                	//-----------------------
				                	printf("\n Entered..........");
				                	fflush(stdout);
				                    bptree.searchGreaterThanKey(key,ridVector1);
				                    //-----------------------
				                    break;
				}
				                free(key);
				                getRecordsForRIDS(ridVector1,typesVector);

				       return SUCCESSINDEXUSED;


		}//if(flag){

	}//end of if(globalstructures->whereExprList.size()>0){


	DirectoryPage *directoryPage=(DirectoryPage *)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;
	slotentry slot;
	int intValue;
	int intValue1;
	char *charValue = NULL;
	char *charValue1 = NULL;

	short tagValue;
	short tagValue1;//This will be used while evaluating where
	long recordBaseAddress = 0;//This is the base address i.e from where actual data is going to start...since starting we have some offsets value
	datapage *dataPage = (datapage*)calloc(1,PAGE_SIZE);
	readPage(directoryPage,theaderpage);

	string tuple;
	string tuple1;
	if(directoryPage->currNoOfDE == 0){
	        free(dataPage);
	        free(directoryPage);
	        globalstructures->errorMsg = "EMPTY TABLE";
	        printf("\n No Records are there in that table....It is an empty table");
	        return SELECTEMPTYTABLE;
	}

	int recordOffset=0,tagOffset=0;
	int tagOffset1=0,recordOffset1=0;//This will be used while evaluating where

	int addToOffset,addToOffset1;

	int type;//This will be used while evaluating where just to specify the type

	int toEval = SELECTEVALSUCCESS;

	int oper,eval;//This will be used while evaluating where i.e as operator and return value of evaluate

	if (globalstructures->allColumns == true)
	     globalstructures->resultSetColumnList = columnNamesVector;

	long recordCount = 0;//for...for loop i think and but not used i.e for condition
	int dirPageCount = 0;//for...for loop i think and but not used i.e for condition

	//These three will be used in getInfoRegardingWhere
	vector<string> values;
	vector<int> whereOffset;
	vector<string> operators;

	long do_while=-1;//This will be used while checking i.e at last do_wwhile
	int i=0,j=0;


	//In this we are getting information regarding the constraints after where i.e for checking the
	getInfoRegardingWhere(columnNamesVector,whereOffset,values,operators);


	do{
		do_while=-1;
	   for(i=1;i<=directoryPage->currNoOfDE;i++){
		   memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
		   readPage(dataPage,dirEntry.dataPageNumber);
		   for(j=1;j<=dataPage->slotcount;j++){
			   toEval = SELECTEVALSUCCESS;
			   slot = Slot(dataPage,j);
			   if(slot.slotsize <=0)//It means because of defragmentation the slot won't gets deleted but it will be -1.
			       continue;
			   recordBaseAddress = slot.slotaddress + numColumns*sizeof(short);//It means from here actual data is going to start

			   //if(globalstructures->whereExprList.size() > 0){
			     //     toEval = postfixEval(&dataPage->data[slot.slotaddress],numColumns,columnNamesVector,typesVector);
			   //}


			   //printf("\nwhereOffset.size()--->%d",whereOffset.size());

			   int count=0;//This will be used for OR and AND condition
			   int count1=whereOffset.size();
			   toEval=SELECTEVALSUCCESS;
			   for(int l=0;l<whereOffset.size();l++)
			   {
				   tuple1="";
				   addToOffset1=0;
				   tagOffset1=slot.slotaddress+sizeof(short)*whereOffset.at(l);
				   memcpy(&tagValue1,&dataPage->data[tagOffset1],sizeof(short));
				   short temp1=0;
				   long offset1=slot.slotaddress;
				   int type;
				   for(int m=0;m<whereOffset.at(l);m++)
				   {
					   memcpy(&temp1,&dataPage->data[offset1],sizeof(temp1));
					   addToOffset1+=(int)temp1;
					   offset1+=sizeof(short);
				   }

				   recordOffset1=recordBaseAddress+addToOffset1;

				   switch(typesVector.at(whereOffset.at(l)))
				   {
				   	   case TYPE_INTEGER:
				   		   memcpy(&intValue1,&dataPage->data[recordOffset1],tagValue1);
				   		   tuple1=conversion::int_to_string(intValue1);
				   		   type=TYPE_INTEGER;
				   		   break;

				   	   case TYPE_VARCHAR:
				   		   	charValue1=(char *)calloc(1,tagValue1);
				   		   	memcpy(charValue1,&dataPage->data[recordOffset1],tagValue1);
				   		   	tuple1=conversion::char_to_string(charValue1);
				   		   	type=TYPE_VARCHAR;
				   		   	free(charValue1);
				   		   	break;
				   }//end of switch

				   if(globalstructures->operators=="AND")
				   {
					   oper=mapStringToInteger(operators.at(l));
					   eval=evaluate(type,tuple1,oper,values.at(l));

					   if(eval==1)
					   {
						   count++;;
					   	   //check for the next time
					   }
					   //To Check whether all matches or not
					   if(count==count1)
					   {
						   toEval=SELECTEVALSUCCESS;
						   break;
					   }

					   else if(eval==0)
					   {
						   toEval=NOTMATCH;
						   break;
					   }

				   }

				   else if(globalstructures->operators=="OR")
				   {
					   oper=mapStringToInteger(operators.at(l));
					   eval=evaluate(type,tuple1,oper,values.at(l));

					   if(eval==1)//Since in OR loop after reaching the final point we need to return rite
					   {
					  			//Since OR if one time matches it's enough
					  			toEval=UPDATEEVALSUCCESS;//just to specify
					  			break;
					   }

					   if(eval==0)
					   {
					  			count++;
					  	        //check for the next value
					   }

					   if(count==count1)
					   {
					  	        toEval=NOTMATCH;
					  			break;
					   }

				   }

				   //if only single condition no combination
				   else
				   {
					   oper=mapStringToInteger(operators.at(l));



					   eval=evaluate(type,tuple1,oper,values.at(l));
					   if(eval==1)
				       {
					   		 //Since OR if one time matches it's enough
					   		 toEval=SELECTEVALSUCCESS;//just to specify
					   	     break;
				       }
					   else if(eval==0)
					   {
						   toEval=NOTMATCH;
						   break;
					   }

				   }

			   }



			   if(toEval == SELECTEVALSUCCESS){
			            tuple = "";
			            for(int k=0;k<offsetsVector.size();k++){
			            	tagOffset = slot.slotaddress + sizeof(short)*offsetsVector.at(k);//This will make you to point to the next entry of offset
			            	//remember that offsetsvector will starts from 0 and the elements will contain 0 for the first element
			            	memcpy(&tagValue,&dataPage->data[tagOffset],sizeof(short));
			            	addToOffset = 0;
			            	long offset = slot.slotaddress;
	                        short temp = 0;

	                        //addToOffset will make you to point to the exact entry
	                       for(int m=0;m<offsetsVector.at(k);m++){
	                               memcpy(&temp,&dataPage->data[offset],sizeof(temp));
	                               addToOffset += (int)temp;
	                               offset += sizeof(short);
	                        }

	                        recordOffset = recordBaseAddress + addToOffset;//This will point to the exact entry
	                        switch(typesVector.at(offsetsVector.at(k))){
	                        	case TYPE_INTEGER:
	                        		memcpy(&intValue,&dataPage->data[recordOffset],tagValue);
	                        		tuple += conversion::int_to_string(intValue);
	                        		tuple += "#";
	                        		break;
	                        	case TYPE_VARCHAR:
	                        		charValue=(char *)calloc(1,tagValue);
	                        		memcpy(charValue,&dataPage->data[recordOffset],tagValue);
	                        		tuple += conversion::char_to_string(charValue);
	                        		tuple += "#";
	                        		free(charValue);
	                        		break;
	                        }
			            }
			            globalstructures->resultSet.push_back(tuple);
			            recordCount++;
			   }
		   }
	   }
	   if(directoryPage->nextDirPageNumber!=-1){
		           do_while=directoryPage->nextDirPageNumber;
	               readPage(directoryPage,directoryPage->nextDirPageNumber);
	               dirPageCount++;
	       }
	   }while(do_while!=-1);


	free(dataPage);
	free(directoryPage);
	return 0;
}

void getRecordsForRIDS(vector<RecordIdentifier> &ridVector1,vector<int> &typesVector){
	RecordIdentifier rid;
	datapage *dataPage = (datapage*)malloc(PAGE_SIZE);
	slotentry slot;
	for(int i=0;i<ridVector1.size();i++){
		rid = ridVector1.at(i);
		readPage(dataPage,rid.pageNumber);
		slot = Slot(dataPage,rid.slotNo);
		string s = getRecord(&dataPage->data[slot.slotaddress],typesVector);
		globalstructures->resultSet.push_back(s);
	}//for(int i=0;i<ridVector1.size();i++){

}//getRecordsForRIDS(vector<RecordIdentifier> &ridVector1,vector<int> &typesVector){


string getRecord(char *record,vector<int> &typesVector)
{
	int intValue;
	char *charValue = NULL;
	int tagOffset = 0;
	int recordOffset = typesVector.size()*sizeof(short);
    short tagValue = 0;

    string tuple = "";
    for(int i=0;i<typesVector.size();i++){
            memcpy(&tagValue,&record[tagOffset],sizeof(short));
            tagOffset += sizeof(short);
            switch(typesVector.at(i)){

            case TYPE_INTEGER:
                memcpy(&intValue,&record[recordOffset],tagValue);
                recordOffset += tagValue;
                tuple += conversion::int_to_string(intValue);
                tuple += "#";
                break;
            case TYPE_VARCHAR:
                charValue = (char*)malloc(tagValue);
                memcpy(charValue,&record[recordOffset],tagValue);
                recordOffset += tagValue;
                tuple += conversion::char_to_string(charValue);
                tuple += "#";
                free(charValue);
                break;
            }
        }
        return tuple;

}

void getAllIndexesOnTable(string tableName,vector<string> &indexedColumnNames,vector<long> &indexHeaderPages)
{
	int i,j;
	char tName[40];
	char iName[30];
	char columnName[30];

	long indexHeaderPageNumber = -1;

	DirectoryPage *directoryPage = (DirectoryPage*)malloc(PAGE_SIZE);
	DirectoryEntry dirEntry;
	slotentry slot;
	datapage *dataPage = (datapage*)malloc(PAGE_SIZE);
	readPage(directoryPage,maindbHeader->sysIndexPageNumber);

	long do_while=-1;
	do
	{
		for(i=1;i<=directoryPage->currNoOfDE;i++){
			memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
			readPage(dataPage,dirEntry.dataPageNumber);
			for(j=1;j<=dataPage->slotcount;j++){
				slot = Slot(dataPage,j);
				if(slot.slotsize <=0)
				     continue;
				memcpy(tName,&dataPage->data[slot.slotaddress + sizeof(long) + sizeof(iName)],sizeof(tName));
				if(strcmp(tName,tableName.c_str()) == 0){
					memcpy(columnName,&dataPage->data[slot.slotaddress + sizeof(long) + sizeof(iName) + sizeof(tName)],sizeof(columnName));
					memcpy(&indexHeaderPageNumber,&dataPage->data[slot.slotaddress + sizeof(long) + sizeof(iName) + sizeof(tName) + sizeof(columnName)],sizeof(indexHeaderPageNumber));
					indexedColumnNames.push_back(conversion::char_to_string(columnName));
					indexHeaderPages.push_back(indexHeaderPageNumber);

				}//end of if(strcmp(tName,tableName.c_str()) == 0){
			}//end of for(j=1;j<=dataPage->slotCount;j++){
		}//end of for(i=1;i<=directoryPage->currNoOfDE;i++)
			if(directoryPage->nextDirPageNumber!=-1)
			{
			    do_while=directoryPage->nextDirPageNumber;
				readPage(directoryPage,directoryPage->nextDirPageNumber);
			}


	}while(do_while!=-1);

	free(dataPage);
	free(directoryPage);


}//End of Function


int checkTypesAndColumnNamesInWhereList(vector<int> &typesVector, vector<string> &columnNamesVector)
{
	int i=0;
	ExprInfo expr;
	vector<string>::iterator myIterator;
	string columnName;
	int position,Ltype,Itype,typeCounter = 0;

	while(i < globalstructures->whereExprList.size()){
	        expr = globalstructures->whereExprList.at(i);
	        if(expr.type == ExprInfo::IDENTIFIER){
	        	columnName = expr.identifier_value;
	        	myIterator = find(columnNamesVector.begin(),columnNamesVector.end(),columnName);
	        	if(myIterator != columnNamesVector.end())
	        	    position = int(myIterator-columnNamesVector.begin());
	        	else{
	        	       globalstructures->errorMsg = "NO SUCH COLUMN:" + columnName;
	        	       return NOSUCHCOLUMN;
	        	}
	        	Itype = typesVector.at(position);
	        }
	        else if(expr.type == ExprInfo::LITERAL){
	        	//Ltype = globalstructures->insert_type.at(typeCounter++);
	        	Ltype=globalstructures->insert_type.at(position);
	        	if(Ltype != Itype){
	        	         globalstructures->errorMsg = "TYPE MISMATCH AT:" + expr.literal_value;
	        	         return SELECTFAILEDTYPEMISMATCH;
	        	}
	        }
	        i++;

	}
return 3;
}

void getInfoRegardingWhere(vector<string> columnNamesVector,vector<int> &whereOffset,vector<string> &valueOffset,vector<string> &operators)
{
	vector<string>::iterator myIterator;//while finding the index
	ExprInfo expr;
	string columnName;
	int position;
	string value;//This will be used to store the values of identifier as well as operator

	for(int i=0;i<globalstructures->whereExprList.size();i++)
	{
		expr=globalstructures->whereExprList.at(i);

		if(expr.type==ExprInfo::IDENTIFIER)
		{
			columnName=expr.identifier_value;
			myIterator=find(columnNamesVector.begin(),columnNamesVector.end(),columnName);
			//no need to check whether the columnName is present or not since already this step has been performed
			position=int(myIterator-columnNamesVector.begin());
			whereOffset.push_back(position);
		}

		else if(expr.type==ExprInfo::LITERAL)
		{
			value=expr.literal_value;
			valueOffset.push_back(value);
		}

		else if(expr.type==ExprInfo::OPERATOR)
		{
			value=expr.operator_value;
			operators.push_back(value);
		}


	}


}


int mapStringToInteger(string opr)
{
		if(!opr.compare("="))
	        return EQUAL;
	    else if(!opr.compare("<>"))
	        return NOTEQUAL;
	    else if(!opr.compare(">"))
	        return GT;
	    else if(!opr.compare("<"))
	        return LT;
	    else if(!opr.compare(">="))
	        return GTE;
	    else if(!opr.compare("<="))
	        return LTE;
	    else return -1;
}

int evaluate(int Ltype,string Rvalue,int oper,string Lvalue)
{
	int intLvalue;
	int intRvalue;
	switch(oper)
	{

	case EQUAL:
		switch(Ltype)
		{
		case TYPE_INTEGER:
			intLvalue = conversion::string_to_int(Lvalue);
			intRvalue = conversion::string_to_int(Rvalue);
			if(intLvalue==intRvalue)
				return 1;
			else
				return 0;
			break;
		case TYPE_VARCHAR:
			if(strcmp(Lvalue.c_str(),Rvalue.c_str())==0)
				return 1;
			else
				return 0;
			break;
		default:
			return 0;

		}//end of switch(Ltype)

	 break;

	case NOTEQUAL:
		 switch(Ltype)
		 {
		        case TYPE_INTEGER:
		        	intLvalue = conversion::string_to_int(Lvalue);
		        	intRvalue = conversion::string_to_int(Rvalue);
		        	if(intLvalue!=intRvalue)
		        		return 1;
		        	else
		        		return 0;
		        break;
		        case TYPE_VARCHAR:
		        	if(strcmp(Lvalue.c_str(),Rvalue.c_str())==0)
		        			return 1;
		        	else
		        			return 0;
		        break;

		        default:
		        	return 0;

		 }//end of switch(Ltype)

	break;

	case GT:
		switch(Ltype)
		{
			case TYPE_INTEGER:
				intLvalue = conversion::string_to_int(Lvalue);
				intRvalue = conversion::string_to_int(Rvalue);
				if(intRvalue>intLvalue)
						return 1;
				else
						return 0;
				break;

			default:
				return 0;
		}

	break;

	case LT:
				switch(Ltype)
				{
					case TYPE_INTEGER:
						intLvalue = conversion::string_to_int(Lvalue);
						intRvalue = conversion::string_to_int(Rvalue);
						if(intRvalue<intLvalue)
								return 1;
						else
								return 0;
						break;

					default:
						return 0;
				}

			break;

				case GTE:
						switch(Ltype)
						{
							case TYPE_INTEGER:
								intLvalue = conversion::string_to_int(Lvalue);
								intRvalue = conversion::string_to_int(Rvalue);
								if(intRvalue>=intLvalue)
										return 1;
								else
										return 0;
								break;

							default:
								return 0;
						}

					break;

					case LTE:
								switch(Ltype)
								{
									case TYPE_INTEGER:
										intLvalue = conversion::string_to_int(Lvalue);
										intRvalue = conversion::string_to_int(Rvalue);
										if(intRvalue<=intLvalue)
												return 1;
										else
												return 0;
										break;

									default:
										return 0;
								}

							break;




	}//end of switch(oper)
}

int updateTable()
{
	long tid = -1;
	long theaderpage;

	//From here we will get the tid,theaderpage
	if(searchSysTables(globalstructures->tablename,tid,theaderpage) == false || tid < 3)
	{
	     globalstructures->errorMsg = "UPDATE FAILED NO SUCH TABLE";
	     return UPDATEFAILEDNOSUCHTABLE;
	}


	int numColumns = 0;
	vector<int> typesVector;
	vector<int> constraintsVector;
	vector<int> lengthsVector;
	vector<string> columnNamesVector;
	vector<int> offsetsVector;


	int returnValue = getInfoFromSysColumns(tid,numColumns,typesVector,constraintsVector,lengthsVector,columnNamesVector,offsetsVector);

	vector<int>updateOffsetVector;
	returnValue = checkTypesAndColumnNamesInUpdateList(typesVector,columnNamesVector,updateOffsetVector);

	if(returnValue == NOSUCHCOLUMN || returnValue == UPDATEFAILEDTYPEMISMATCH)
	{
	        if(returnValue == NOSUCHCOLUMN)
	            globalstructures->errorMsg = "NO SUCH COLUMN  IN TABLE BEFORE WHERE";


	        else
	            globalstructures->errorMsg = "UPDATE FAILED TYPE MISMATCH BEFORE WHERE";
	        return returnValue;
	}

	returnValue = checkTypesAndColumnNamesInWhereList(typesVector,columnNamesVector);

	if(returnValue == NOSUCHCOLUMN || returnValue == UPDATEFAILEDTYPEMISMATCH)
	{
	        //cout<<returnValue;

		    if(returnValue == NOSUCHCOLUMN)
	            globalstructures->errorMsg = "NO SUCH COLUMN  IN TABLE AFTER WHERE";


	        else
	            globalstructures->errorMsg = "UPDATE FAILED TYPE MISMATCH AFTER WHERE";

	        return returnValue;
	}

	//Not checked the length......i.e varchar type mismatch

	int recordOffset=0,tagOffset=0;

	int addToOffset;

	int toEval = UPDATEEVALSUCCESS;

	DirectoryPage *directoryPage = (DirectoryPage*)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;
	slotentry slot;
	int intValue;
	char *oldRecord = NULL;
	char *newRecord = NULL;
	short tagValue;
	long recordBaseAddress = 0;

	datapage *dataPage = (datapage*)calloc(1,PAGE_SIZE);
	readPage(directoryPage,theaderpage);

	long recordCount = 0;
	int dirPageCount = 0;
	int updateRecordOffset = 0;
	int temp2;
	int i=0;
	int j=0;
	long do_while=-1;

	//These three will be used in getInfoRegardingWhere
		vector<string> values;
		vector<int> whereOffset;
		vector<string> operators;

	//All these are used in the where evaluation
	string tuple1;
	int tagOffset1=0,recordOffset1=0;//This will be used while evaluating where
	int addToOffset1=0;
	short tagValue1;//This will be used while evaluating where
	int intValue1;
	char *charValue1=NULL;
	int oper,eval;//This will be used while evaluating where i.e as operator and return value of evaluate

	//In this we are getting information regarding the constraints after where i.e for checking the
	getInfoRegardingWhere(columnNamesVector,whereOffset,values,operators);

	globalstructures->allColumns=true;
	globalstructures->resultSetColumnList.clear();
	globalstructures->resultSetColumnList.push_back(columnNamesVector.at(updateOffsetVector.at(0)));
	globalstructures->whereExprList.clear();
	selectFromTable();
	int count2=globalstructures->resultSet.size();
	globalstructures->resultSet.clear();
	globalstructures->resultSetColumnList.clear();
	//printf("\n count------>%d",count2);




	int x=0;
	do{
		do_while=-1;

		for(i=1;i<=directoryPage->currNoOfDE&&x<count2;i++){
			memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));

			readPage(dataPage,dirEntry.dataPageNumber);
			for(j=1;j<=dataPage->slotcount&&x<count2;j++){
				x++;
				toEval = UPDATEEVALSUCCESS;

				slot = Slot(dataPage,j);
				if(slot.slotsize <=0)
				      continue;



				recordBaseAddress = slot.slotaddress + numColumns*sizeof(short);


							   int count=0;//This will be used for OR and AND condition
							   int count1=whereOffset.size();
							   toEval=UPDATEEVALSUCCESS;
							   for(int l=0;l<whereOffset.size();l++)
							   {
								   tuple1="";
								   addToOffset1=0;
								   tagOffset1=slot.slotaddress+sizeof(short)*whereOffset.at(l);
								   memcpy(&tagValue1,&dataPage->data[tagOffset1],sizeof(short));
								   short temp1=0;
								   long offset1=slot.slotaddress;
								   int type;
								   for(int m=0;m<whereOffset.at(l);m++)
								   {
									   memcpy(&temp1,&dataPage->data[offset1],sizeof(temp1));
									   addToOffset1+=(int)temp1;
									   offset1+=sizeof(short);
								   }

								   recordOffset1=recordBaseAddress+addToOffset1;

								   switch(typesVector.at(whereOffset.at(l)))
								   {
								   	   case TYPE_INTEGER:
								   		   memcpy(&intValue1,&dataPage->data[recordOffset1],tagValue1);

								   		   tuple1=conversion::int_to_string(intValue1);
								   		   type=TYPE_INTEGER;
								   		   break;

								   	   case TYPE_VARCHAR:
								   		   	charValue1=(char *)calloc(1,tagValue1);
								   		   	memcpy(charValue1,&dataPage->data[recordOffset1],tagValue1);
								   		   	tuple1=conversion::char_to_string(charValue1);
								   		   	type=TYPE_VARCHAR;
								   		   	free(charValue1);
								   		   	break;
								   }//end of switch

								   if(globalstructures->operators=="AND")
								   {
									   oper=mapStringToInteger(operators.at(l));
									   eval=evaluate(type,tuple1,oper,values.at(l));

									   if(eval==1)
									   {
										   count++;;
									   	   //check for the next time
									   }
									   //To Check whether all matches or not
									   if(count==count1)
									   {
										   toEval=UPDATEEVALSUCCESS;
										   break;
									   }

									   else if(eval==0)
									   {
										   toEval=NOTMATCH;
										   break;
									   }

								   }

								   else if(globalstructures->operators=="OR")
								   {
									   oper=mapStringToInteger(operators.at(l));
									   eval=evaluate(type,tuple1,oper,values.at(l));

								       if(eval==1)//Since in OR loop after reaching the final point we need to return rite
									   {
									   		//Since OR if one time matches it's enough
								    	   toEval=UPDATEEVALSUCCESS;//just to specify
								    	   break;
									   }

								       if(eval==0)
									   {
									   	   count++;
										   //check for the next value
									   }

								       if(count==count1)
								       {
								          toEval=NOTMATCH;
								           break;
								       }

								   }

								   //if only single condition no combination
								   else
								   {
									   oper=mapStringToInteger(operators.at(l));



									   eval=evaluate(type,tuple1,oper,values.at(l));
									   if(eval==1)
								       {
									   		 //Since OR if one time matches it's enough
									   		 toEval=UPDATEEVALSUCCESS;//just to specify
									   	     break;
								       }
									   else if(eval==0)
									   {
										   toEval=NOTMATCH;
										   break;
									   }

								   }

							   }





				if(toEval==UPDATEEVALSUCCESS)
				{
					for(int k=0;k<updateOffsetVector.size();k++)
					{
						tagOffset=slot.slotaddress+sizeof(short)*updateOffsetVector.at(k);
						memcpy(&tagValue,&dataPage->data[tagOffset],sizeof(short));
						addToOffset = 0;
						long offset = slot.slotaddress;
						short temp1 = 0;

						for(int m=0;m<updateOffsetVector.at(k);m++)
						{
						      memcpy(&temp1,&dataPage->data[offset],sizeof(temp1));
						      addToOffset += (int)temp1;
						      offset += sizeof(short);
						}
						recordOffset = recordBaseAddress + addToOffset;

						int newRecordLength = 0;
						short updateTagValue = 0;
						char *copyRecord = NULL;
						int copy = globalstructures->update_values.at(k).length()+1;

						switch(typesVector.at(updateOffsetVector.at(k)))
						{
							case TYPE_INTEGER:
								intValue = conversion::string_to_int(globalstructures->update_values.at(k));
								memcpy(&dataPage->data[recordOffset],&intValue,tagValue);
								writePage(dataPage,dataPage->pagenumber);
								break;

							case TYPE_VARCHAR:
								oldRecord = (char*)calloc(1,slot.slotsize);
								newRecordLength = slot.slotsize;

								if(tagValue < (short)copy)
								   newRecordLength += ((short)copy - tagValue);

								else
								   newRecordLength -= (tagValue - (short)copy);

								newRecord = (char*)calloc(1,newRecordLength);
								updateRecordOffset = (recordOffset - slot.slotaddress);
								updateTagValue = (short)(globalstructures->update_values.at(k).length() +1);
								memcpy(newRecord,&dataPage->data[slot.slotaddress],updateRecordOffset);
								memcpy(&newRecord[ updateOffsetVector.at(k)*(sizeof(short)) ],&updateTagValue,sizeof(short));
								copyRecord = (char*)calloc(1,copy);
								strcpy(copyRecord,globalstructures->update_values.at(k).c_str());
								copyRecord[copy-1] = '\0';
								memcpy(&newRecord[updateRecordOffset],copyRecord,copy);

								//Since no need to do for the last item since there won't be any column value after that so no need
								if(numColumns != updateOffsetVector.at(k)+1)
								{
								      temp2 = ((slot.slotaddress+slot.slotsize)-1) - ( recordOffset + tagValue ) +1;
								      memcpy(&newRecord[updateRecordOffset+copy],&dataPage->data[recordOffset + tagValue],temp2);
								}

								free(oldRecord);

								free(copyRecord);

								dirEntry.totalFreeSpace += sizeof(slot)+slot.slotsize;
								memcpy(getDirectoryEntry(directoryPage,i),&dirEntry,sizeof(dirEntry));

								slot.slotsize *= -1;
								Slot(dataPage,j)= slot;

								//since all the entries are filled we need to update the MaxFreeSpace
								if(directoryPage->currNoOfDE == directoryPage->maxNoOfDE)
								                updateMaxFreeSpace(directoryPage);


								writePage(dataPage,dataPage->pagenumber);
								writePage(directoryPage,directoryPage->pageNumber);
								                        //printRecord(newRecord);

								for(int x=0;x<newRecordLength;x++)
									printf("%d ",*(newRecord+x));

								printf("\n");


								insertIntoTable(newRecord,theaderpage,newRecordLength);
								readPage(dataPage,dataPage->pagenumber);
								readPage(directoryPage,directoryPage->pageNumber);
								free(newRecord);

								break;
						   }
						}

					}
				}
			}
			 if(directoryPage->nextDirPageNumber!=-1){
				 	 	do_while= directoryPage->nextDirPageNumber;
			            readPage(directoryPage,directoryPage->nextDirPageNumber);
			            dirPageCount++;
			        }
			    }while(do_while!=-1 && x<count2);

free(dataPage);
free(directoryPage);
printf("\n x--->%d %d",x,count2);
return 0;
}












int checkTypesAndColumnNamesInUpdateList(vector<int> &typesVector,vector<string> &columnNamesVector,vector<int> &updateOffsetVector)
{
	int retValue;
	vector<string>::iterator myIterator;

	for(int i=0;i<globalstructures->update_fields.size();i++)
	{
		myIterator = find(columnNamesVector.begin(),columnNamesVector.end(),globalstructures->update_fields.at(i));
		if(myIterator != columnNamesVector.end())
			updateOffsetVector.push_back(int(myIterator-columnNamesVector.begin()));//Offset will be used to identify the column

		else
		{
			retValue=NOSUCHCOLUMN;
			globalstructures->errorMsg = "NO SUCH COLUMN:" + globalstructures->update_fields.at(i);
			return retValue;
		}

	}

	for(int i=0;i<globalstructures->update_values.size();i++)
	{
		//cout<<typesVector.at(updateOffsetVector.at(i))+" "+globalstructures->update_type.at(i);

	    if(typesVector.at(updateOffsetVector.at(i)) != globalstructures->update_type.at(i))
	          return UPDATEFAILEDTYPEMISMATCH;
	}
}


int deleteFromTable()
{
	long tid = -1;
	long theaderpage;

	if(searchSysTables(globalstructures->tablename,tid,theaderpage) == false || tid < 3)
	{
	        if(tid < 3 and tid >0)
	            globalstructures->errorMsg = " INVALID OPERATION " +globalstructures->tablename+" CANT BE DELETED ";
	        else
	            globalstructures->errorMsg = " TABLE DOES NOT EXIST";
	        return DELETEFAILEDNOSUCHTABLE;
	}

	int numColumns = 0;
	vector<int> typesVector;
	vector<int> constraintsVector;
	vector<int> lengthsVector;
	vector<string> columnNamesVector;
	vector<int> offsetsVector;
	int returnValue = getInfoFromSysColumns(tid,numColumns,typesVector,constraintsVector,lengthsVector,columnNamesVector,offsetsVector);

	if(returnValue == NOSUCHCOLUMN)
	{
	     globalstructures->errorMsg= " NO SUCH COLUMN ";
	     return NOSUCHCOLUMN;
	}

	returnValue = checkTypesAndColumnNamesInWhereList(typesVector,columnNamesVector);

	if(returnValue == NOSUCHCOLUMN || returnValue == SELECTFAILEDTYPEMISMATCH)
	{

		if(returnValue == NOSUCHCOLUMN)
			globalstructures->errorMsg= " NO SUCH COLUMN AFTER WHERE ";

	    else
	        globalstructures->errorMsg = "SELECT FAILED TYPE MISMATCH AFTER WHERE";

		return returnValue;
	}

	DirectoryPage *directoryPage = (DirectoryPage*)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;
	slotentry slot;
	datapage *dataPage = (datapage*)calloc(1,PAGE_SIZE);
	readPage(directoryPage,theaderpage);

	if(directoryPage->currNoOfDE == 0){
	        free(dataPage);
	        free(directoryPage);
	        globalstructures->errorMsg = "EMPTY TABLE";
	        return DELETEEMPTYTABLE;
	}

	int toEval = DELETEEVALSUCCESS;

	if (globalstructures->allColumns == true)
	      globalstructures->resultSetColumnList = columnNamesVector;

	long recordCount = 0;//just to know how many records are deleted
	int dirPageCount = 0;
	long recordBaseAddress = 0;
	long do_while=-1;

	//These three will be used in getInfoRegardingWhere
			vector<string> values;
			vector<int> whereOffset;
			vector<string> operators;

		//All these are used in the where evaluation
		string tuple1;
		int tagOffset1=0,recordOffset1=0;//This will be used while evaluating where
		int addToOffset1=0;
		short tagValue1;//This will be used while evaluating where
		int intValue1;
		char *charValue1=NULL;
		int oper,eval;//This will be used while evaluating where i.e as operator and return value of evaluate


	//In this we are getting information regarding the constraints after where i.e for checking the
     getInfoRegardingWhere(columnNamesVector,whereOffset,values,operators);

     globalstructures->allColumns=true;
	 globalstructures->resultSetColumnList.clear();
	 //since just we need the count only so no need to bother much on whether 0 or 1 or anything
	 globalstructures->resultSetColumnList.push_back(columnNamesVector.at(0));
	 globalstructures->whereExprList.clear();
	 selectFromTable();
	 int count2=globalstructures->resultSet.size();
	 globalstructures->resultSet.clear();
	 globalstructures->resultSetColumnList.clear();
	 printf("\n count------>%d",count2);

	do
	{
		do_while=-1;
		for(int i=1;i<=directoryPage->currNoOfDE;i++){
			memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
			readPage(dataPage,dirEntry.dataPageNumber);
			for(int j=1;j<=dataPage->slotcount;j++)
			{
				toEval = DELETEEVALSUCCESS;

				slot = Slot(dataPage,j);
				if(slot.slotsize <=0)
				     continue;

				recordBaseAddress = slot.slotaddress + numColumns*sizeof(short);

				int count=0;//This will be used for OR and AND condition
				int count1=whereOffset.size();
				toEval=DELETEEVALSUCCESS;
				for(int l=0;l<whereOffset.size();l++)
				{
					tuple1="";
					addToOffset1=0;
					tagOffset1=slot.slotaddress+sizeof(short)*whereOffset.at(l);
					memcpy(&tagValue1,&dataPage->data[tagOffset1],sizeof(short));
					short temp1=0;
					long offset1=slot.slotaddress;
					int type;

					for(int m=0;m<whereOffset.at(l);m++)
					{
						memcpy(&temp1,&dataPage->data[offset1],sizeof(temp1));
						addToOffset1+=(int)temp1;
						offset1+=sizeof(short);
					}//END OF for(int m=0;m<whereOffset.at(l);m++)

					recordOffset1=recordBaseAddress+addToOffset1;

					switch(typesVector.at(whereOffset.at(l)))
					{
						case TYPE_INTEGER:
							memcpy(&intValue1,&dataPage->data[recordOffset1],tagValue1);
							tuple1=conversion::int_to_string(intValue1);
							type=TYPE_INTEGER;
							break;
						case TYPE_VARCHAR:
							charValue1=(char *)calloc(1,tagValue1);
							memcpy(charValue1,&dataPage->data[recordOffset1],tagValue1);
							tuple1=conversion::char_to_string(charValue1);
							type=TYPE_VARCHAR;
							free(charValue1);
							break;
					}//END OF SWITCH

					if(globalstructures->operators=="AND")
					{
							oper=mapStringToInteger(operators.at(l));
							eval=evaluate(type,tuple1,oper,values.at(l));

							if(eval==1)
							{
									count++;;
									//check for the next time
							}
							//To Check whether all matches or not
							if(count==count1)
							{
									toEval=DELETEEVALSUCCESS;
									break;
							}

							else if(eval==0)
							{
									toEval=NOTMATCH;
									break;
							}

					}

					else if(globalstructures->operators=="OR")
					{
							oper=mapStringToInteger(operators.at(l));
							eval=evaluate(type,tuple1,oper,values.at(l));

							if(eval==1)//Since in OR loop after reaching the final point we need to return rite
							{
									//Since OR if one time matches it's enough
									toEval=DELETEEVALSUCCESS;//just to specify
									break;
							}

							if(eval==0)
							{
									count++;
									//check for the next value
							}

							if(count==count1)
							{
									toEval=NOTMATCH;
									break;
							}

						}

						//if only single condition no combination
						else
						{
							oper=mapStringToInteger(operators.at(l));

							eval=evaluate(type,tuple1,oper,values.at(l));
							if(eval==1)
							{
									//Since OR if one time matches it's enough
									toEval=DELETEEVALSUCCESS;//just to specify
									break;
							}
							else if(eval==0)
							{
									toEval=NOTMATCH;
									break;
							}

						}


				}//END OF for(int l=0;l<whereOffset.size();l++)


				if(toEval == DELETEEVALSUCCESS)
				{
					dirEntry.totalFreeSpace += sizeof(slot)+slot.slotsize;

					slot.slotsize *= -1;

					Slot(dataPage,j)= slot;

					memcpy(getDirectoryEntry(directoryPage,i),&dirEntry,sizeof(dirEntry));

					//if all the DE are filled than we need to update Max Free Space which will be used while filling the data i.e inserting
					//since we will check the free space before we fill the data
					if(directoryPage->currNoOfDE == directoryPage->maxNoOfDE)
					       updateMaxFreeSpace(directoryPage);

					writePage(dataPage,dataPage->pagenumber);

					recordCount++;

				}
				writePage(directoryPage,directoryPage->pageNumber);

			}

		}

		if(directoryPage->nextDirPageNumber!=-1){
					do_while=directoryPage->nextDirPageNumber;
		            readPage(directoryPage,directoryPage->nextDirPageNumber);
		            dirPageCount++;
		}


	}while(do_while!=-1);

	globalstructures->errorMsg = "RECORDS DELETED "+globalstructures->tablename;
	free(dataPage);
	free(directoryPage);

}

int dropTable(string tableName)
{
	long tid=-1,theaderPage=-1;
	long dupTid;

	//Initial Values of MainDBPage
	printf("\n Initial Values of Main DB PAGE are:\n");
	printMainDBPage();

	searchSysTables(tableName,tid,theaderPage);
	if(tid<3)
	{
	   if(tid < 0)
	       globalstructures->errorMsg="Table not found "+tableName;
	   else
	       globalstructures->errorMsg="Invalid Operation "+tableName+" cant be deleted ";
	   return DROPTABLENOTFOUND;
	}

	maindbHeader->numberOfTables--;

	DirectoryPage *directoryPage=(DirectoryPage*)calloc(1,PAGE_SIZE);
	DirectoryEntry dirEntry;
	datapage *dataPage=(datapage*)malloc(PAGE_SIZE);

	readPage(directoryPage,1);


	//First Delete the table from SYS Tables
	slotentry se;
	int i=1;

	int offset=0;

	bool flag = false;
	long do_while=-1;

	do
	{
		do_while=-1;
	    for(i=1;i<=directoryPage->currNoOfDE;i++)
	    {
	         memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
	         readPage(dataPage,dirEntry.dataPageNumber);

	         for(int j=1;j<=dataPage->slotcount;j++)
	         {
	        	 se=Slot(dataPage,j);
	        	 if(se.slotsize<=0)
	        	     continue;

	        	 else
	        	 {
	        		  //In DirectoryPage(1) of SYS Table slot each slot contains 3 entries i.e tableID,name,theaderpage...the tid will be long
	        		 memcpy(&dupTid,&dataPage->data[se.slotaddress],sizeof(dupTid));
	        		 if(dupTid==tid)
	        		 {
	        			 //Since the current entry will be a free entry from now onwards making that as -1 and adding that size to the totalFreeSpace
	        			 dirEntry.totalFreeSpace += sizeof(se)+se.slotsize;
	        			 se.slotsize *= -1;
	        			 Slot(dataPage,j)= se;

	        			 //copying the updated part i.e dirEntry which has been updated because of totalFreeSpace updated...is going to store
	        			 memcpy(getDirectoryEntry(directoryPage,i),&dirEntry,sizeof(dirEntry));

	        			 //If in the directoryPage all entries are filled and if we modify the totalFreeSpace of any directory Entry
	        			 //once again we need to update the entire Max Free Space of the directory Page this will be used while insertion of slot
	        			 if(directoryPage->currNoOfDE == directoryPage->maxNoOfDE)
	        			         updateMaxFreeSpace(directoryPage);

	        			 writePage(directoryPage,directoryPage->pageNumber);
	        			 writePage(dataPage,dataPage->pagenumber);

	        			 flag=true;
	        			 break;
	        		 }//IF(DUPTID==TID)

	        	 }//else
	         }//end of for(int j=1;j<=dataPage->slotCount;j++)

	         //since we found out the table after traversing all the entries corresponding to the directory page.
	         if(flag)
	            break;

	       }//end of for(i=1;i<=directoryPage->currNoOfDE;i++)

	    if(flag)
	    	break;
	    if(directoryPage->nextDirPageNumber!=-1 )
	    {
	    	do_while=directoryPage->nextDirPageNumber;
	    	readPage(directoryPage,directoryPage->nextDirPageNumber);
	    }

	}while(do_while!=-1);//end of do while

	//Deleting data from SYS COLUMNS
	readPage(directoryPage,2);
	do
	{
		do_while=-1;
		for(int i=1;i<=directoryPage->currNoOfDE;i++)
		{
			memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
			readPage(dataPage,dirEntry.dataPageNumber);
			for(int j=1;j<=dataPage->slotcount;j++)
			{
				se = Slot(dataPage,j);
				if(se.slotsize <=0)
				   continue;

				memcpy(&dupTid,&dataPage->data[se.slotaddress],sizeof(dupTid));

				//like SYS TABLE even in SYS COLUMN the first entry is columnTableID checking for the match of that
				if(dupTid == tid)
				{
					dirEntry.totalFreeSpace += sizeof(se)+se.slotsize;

					//updating the dirEntry so that we can write it into the file for presistent which is in mainmemory
					memcpy(getDirectoryEntry(directoryPage,i),&dirEntry,sizeof(dirEntry));
					se.slotsize=se.slotsize*-1;
					Slot(dataPage,j)=se;//For making into persistent storing into the same slot from where it had read

					if(directoryPage->currNoOfDE == directoryPage->maxNoOfDE)
					       updateMaxFreeSpace(directoryPage);

					writePage(directoryPage,directoryPage->pageNumber);

					maindbHeader->numberOfColumns--;

					writePage(dataPage,dataPage->pagenumber);

				}//end of if(dupTid == tid)

			}//end of for(int j=1;j<=dataPage->slotcount;j++)

		}//end of for(int i=1;i<=directoryPage->currNoOfDE;i++)

		if(directoryPage->nextDirPageNumber!=-1)
		{
			do_while=directoryPage->nextDirPageNumber;
			readPage(directoryPage,directoryPage->nextDirPageNumber);
		}

	}while(do_while!=-1);

	//Now deleting the data from the exact table
	readPage(directoryPage,theaderPage);

	do
	{
		do_while=-1;
		for(int i=1;i<=directoryPage->currNoOfDE;i++)
		{
			memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
			deletePageNumber(dirEntry.dataPageNumber);
		}//end of for(int i=1;i<=directoryPage->currNoOfDE;i++)

		//Now deleting the directoryPage
		deletePageNumber(directoryPage->pageNumber);

		if(directoryPage->nextDirPageNumber!=-1)
		{
			do_while=directoryPage->nextDirPageNumber;
			readPage(directoryPage,directoryPage->nextDirPageNumber);
		}

	}while(do_while!=-1);

	persistMainDBPage();
	//globalstructures->errorMsg = "Table Dropped "+tableName;
	 free(directoryPage);
	 free(dataPage);

	 printf("\n After deletion of Table %s the Main DB Header Page values are:\n",globalstructures->tablename.c_str());
	 printMainDBPage();


}//end of DROP TABLE

//This will delete the page content by creating a new empty page and replacing data in it
bool deletePageNumber(long pageNumber)
{
	long freePage=maindbHeader->nextFreePageNumber;
	//creating a new empty page
	char *buffer=(char*)calloc(1,PAGE_SIZE);
	memcpy(&buffer[0],&freePage,sizeof(freePage));

	writePage(buffer,pageNumber);
	maindbHeader->nextFreePageNumber=pageNumber;
	//maindbHeader->state=1;

	//since maindbHeader has been updated
	writePage(maindbHeader,0);

}

void printMainDBPage()
{
		cout<<"Contents of Main DB Header are\n";

	    cout<<"Database Name:"<<maindbHeader->databaseName<<endl;
	    //cout<<"FreeCounter:"<<maindbHeader->freeCounter<<endl;
	    cout<<"NextFreePageNumber:"<<maindbHeader->nextFreePageNumber<<endl;
	    cout<<"Number of Columns:"<<maindbHeader->numberOfColumns<<endl;
	    cout<<"Number of Indexes:"<<maindbHeader->numberOfIndexes<<endl;
	    cout<<"Number of Tables:"<<maindbHeader->numberOfTables<<endl;
	    cout<<"PageNumber:"<<maindbHeader->pageNumber<<endl;
	    cout<<"Priority:"<<maindbHeader->priority<<endl;
	    cout<<"SysColumnPageNumber:"<<maindbHeader->sysColumnPageNumber<<endl;
	    cout<<"SysIndexPageNumber:"<<maindbHeader->sysIndexPageNumber<<endl;
	    cout<<"SysTablePageNumber:"<<maindbHeader->sysTablePageNumber<<endl;
	    cout<<"Page Size:"<<maindbHeader->pageSize<<endl;
}

void persistMainDBPage()
{
	writePage(maindbHeader,0);
}

int createIndex()
{
	long tid,theaderpage;

	if(!searchSysTables(globalstructures->tablename,tid,theaderpage)){
	        globalstructures->errorMsg = "NO SUCH TABLE:" + globalstructures->tablename;
	        return INDEXCREATEFAILEDNOSUCHTABLE;
	}

	int numColumns = 0;
	vector<int> typesVector;
	vector<int> constraintsVector;
	vector<int> lengthsVector;
	vector<string> columnNamesVector;
	vector<int> offsetsVector;

	getInfoFromSysColumns(tid,numColumns,typesVector,constraintsVector,lengthsVector,columnNamesVector,offsetsVector);

	string indexedColumnName = globalstructures->schema.columnnames.at(0);

	int position;

	vector<string>::iterator myIterator;

	myIterator = find(columnNamesVector.begin(),columnNamesVector.end(),indexedColumnName);
	if(myIterator != columnNamesVector.end())
	        position = int(myIterator-columnNamesVector.begin());
	else{
	        globalstructures->errorMsg = "NO SUCH COLUMN:" + indexedColumnName;
	        return INDEXCREATEFAILEDNOSUCHCOLUMN;
	 }

	long indexHeaderPageNumber = -1;
	long indexId = -1;

	if(searchSysIndex(globalstructures->indexName,indexId,indexHeaderPageNumber)){
	        globalstructures->errorMsg = "INDEX ALREADY EXISTS: " + globalstructures->indexName;
	        return INDEXCREATEFAILEDSUPLICATEINDEXNAME;
	 }

	//insert into sysIndex
	//prepare sys index record to insert
	indexId = maindbHeader->numberOfIndexes++;
	char indexName[30];
	strcpy(indexName,globalstructures->indexName.c_str());
	char tableName[40];
	strcpy(tableName,globalstructures->tablename.c_str());
	char columnName[30];
	strcpy(columnName,globalstructures->schema.columnnames.at(0).c_str());
	indexHeaderPageNumber = getFreePage();

	int sysIndexRecordSize = sizeof(indexId) + sizeof(indexName) + sizeof(tableName) + sizeof(columnName) + sizeof(indexHeaderPageNumber);

	char sysIndexRecord[sysIndexRecordSize];

	int offset = 0;

	memcpy(&sysIndexRecord[offset],&indexId,sizeof(indexId));
	offset += sizeof(indexId);
	memcpy(&sysIndexRecord[offset],indexName,sizeof(indexName));
	offset += sizeof(indexName);
	memcpy(&sysIndexRecord[offset],tableName,sizeof(tableName));
	offset += sizeof(tableName);
	memcpy(&sysIndexRecord[offset],columnName,sizeof(columnName));
	offset += sizeof(columnName);
	memcpy(&sysIndexRecord[offset],&indexHeaderPageNumber,sizeof(indexHeaderPageNumber));
	offset += sizeof(indexHeaderPageNumber);

	insertIntoTable(sysIndexRecord,3,sysIndexRecordSize);

	//create an indexheader page;

	IndexHeaderPage *indexheaderpage = (IndexHeaderPage*)malloc(PAGE_SIZE);

	indexheaderpage->pageNumber = indexHeaderPageNumber;
	indexheaderpage->priority = 2;
	indexheaderpage->rootPageNumber = -1;


	//printf("\n verification----------------------%d\n",indexHeaderPageNumber);
	indexheaderpage->keyType = typesVector.at(position);
	indexheaderpage->keySize = lengthsVector.at(position);


	int temp = PAGE_SIZE-sizeof(Node)+1;
	indexheaderpage->fanout = (temp/(sizeof(RecordIdentifier)+indexheaderpage->keySize)) + 1;



	DirectoryPage *directoryPage = (DirectoryPage*)malloc(PAGE_SIZE);
	DirectoryEntry dirEntry;
    slotentry slot;

    char *record = NULL;
    char *fieldValue = NULL;
    short fieldLength;
    RecordIdentifier rid;
    datapage *dataPage = (datapage*)malloc(PAGE_SIZE);
    readPage(directoryPage,theaderpage);

    //printf("\n indexheaderpage->keyType==%d && indexheaderpage->KeySize==%d  %d \n",indexheaderpage->keyType,indexheaderpage->keySize,sizeof(int));

    //-------------------------------
    bPlusTree bptree(indexheaderpage->rootPageNumber,1,indexheaderpage->fanout,indexheaderpage->keyType,indexheaderpage->keySize);
    //-------------------------------

    bptree.bcreateDB("ram",5);
    bptree.bopenDB("ram");

    long do_while=-1;
    do
    {
    	do_while=-1;
    	for(int i=1;i<=directoryPage->currNoOfDE;i++)
    	{
    		memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
    		readPage(dataPage,dirEntry.dataPageNumber);
    		for(int j=1;j<=dataPage->slotcount;j++)
    		{
    			slot = Slot(dataPage,j);
    			if(slot.slotsize <=0)
    			    continue;
    			record = (char*)calloc(1,slot.slotsize);
    			memcpy(record,&dataPage->data[slot.slotaddress],slot.slotsize);
    			memcpy(&fieldLength,&record[sizeof(short)*position],sizeof(short));
    			fieldValue = (char*)malloc(fieldLength);
    			getFieldFromRecord(record,fieldValue,fieldLength,position,numColumns);
    			rid.pageNumber=dataPage->pagenumber;
    			rid.slotNo = j;

    			if(typesVector.at(position) == TYPE_INTEGER){

    				string res=conversion::int_to_string(*(int *)fieldValue);
    				//cout<<"Sending data"<<res<<"\n";

    				if(*(int *)fieldValue==27)
    				{
    					//printf("Rohit");
    				}
    				//----------------------------------------------From bptree---------------
    				bptree.insert(fieldValue,rid);
    				//----------------------------------------------From bptree---------------

    				fflush(stdout);
    			}

    			else if(typesVector.at(position) == TYPE_VARCHAR){

    				string check=conversion::char_to_string(fieldValue);
    				//cout<<check<<"\n";

    				char *varcharKey = (char*)malloc(lengthsVector.at(position));

    				memcpy(varcharKey,fieldValue,fieldLength);

    				string res=conversion::char_to_string(varcharKey);
    				//cout<<res<<"\n";
    				//----------------------------------------------From bptree---------------
    				bptree.insert(varcharKey,rid);
    				//----------------------------------------------From bptree---------------

    				free(varcharKey);
    			}//end of else if(typesVector.at(position) == TYPE_VARCHAR){
    			free(fieldValue);
    			free(record);

    		}//End of for(int j=1;j<=dataPage->slotCount;j++){
    	}//end of for(int i=1;i<=directoryPage->currNoOfDE;i++)

    	if(directoryPage->nextDirPageNumber!=-1)
    	{
    	    do_while=directoryPage->nextDirPageNumber;
    		readPage(directoryPage,directoryPage->nextDirPageNumber);
    	}
    }while(do_while!=-1);

    indexheaderpage->rootPageNumber = bptree.rootPageNumber;
    //printf("\n--------bptree.rootPageNumber --%d",bptree.rootPageNumber);
    writePage(indexheaderpage,indexheaderpage->pageNumber);
    free(indexheaderpage);
}//end of function

void getFieldFromRecord(char *record,char* &field,short &fieldLength,int &offset,int &numColumns)
{
	short temp;
	int offset1 = 0;
	int addToOffset = 0;

    for(int m=0;m<offset;m++){
        memcpy(&temp,&record[offset1],sizeof(temp));
        addToOffset += (int)temp;
        offset1 += sizeof(short);
    }
    int position = numColumns*sizeof(short) + addToOffset;

    memcpy(&field[0],&record[position],fieldLength);
}


bool searchSysIndex(string indexName,long &indexId,long &indexHeaderPageNumber)
{
	int i,j;

	char iName[30];
	char tableName[40];
	char columnName[30];

	DirectoryPage *directoryPage = (DirectoryPage*)malloc(PAGE_SIZE);
	DirectoryEntry dirEntry;
	slotentry slot;
	datapage *dataPage = (datapage*)malloc(PAGE_SIZE);
	readPage(directoryPage,3);

	long do_while=-1;


	do
	{
		do_while=-1;
		for(i=1;i<=directoryPage->currNoOfDE;i++)
		{
			memcpy(&dirEntry,getDirectoryEntry(directoryPage,i),sizeof(DirectoryEntry));
			readPage(dataPage,dirEntry.dataPageNumber);
			for(j=1;j<=dataPage->slotcount;j++)
			{
				slot = Slot(dataPage,j);
				if(slot.slotsize <=0)
				     continue;
				//first will be the indexId of long type
				memcpy(iName,&dataPage->data[slot.slotaddress + sizeof(long)],sizeof(iName));
				if(strcmp(iName,indexName.c_str()) == 0)
				{
					memcpy(&indexId,&dataPage->data[slot.slotaddress],sizeof(indexId));
					memcpy(&indexHeaderPageNumber,&dataPage->data[slot.slotaddress + sizeof(long) + sizeof(iName) + sizeof(tableName) + sizeof(columnName)],sizeof(indexHeaderPageNumber));
					free(dataPage);
					free(directoryPage);
					return true;

				}//end of if(strcmp(iName,indexName.c_str()) == 0)
			}//end of for(j=1;j<=dataPage->slotcount;j++)
		}//end of  for(i=1;i<=directoryPage->currNoOfDE;i++)

		if(directoryPage->nextDirPageNumber!=-1)
		{
			do_while=directoryPage->nextDirPageNumber;
			readPage(directoryPage,directoryPage->nextDirPageNumber);
		}

	}while(do_while!=-1);

	free(dataPage);
	free(directoryPage);
	return false;

}

void usedatabase(char *dbname)
{

	strcpy(path,dbname);
}


int main()
{

	initialize_defaultavalues();//initializing the page size and globalstructures also
	bPlusTree bptree;
	bptree.init_pagesize(PAGE_SIZE);

	clock_t start=clock();

	printf("\nSelect the Option:\n"
			"1.Creation Of Database\n"
			"2.Use Database\n"
			"3.Creation of Table\n"
			"4.Insertion into Table\n"
			"5.Selection From Table\n"
			"6.Creation Of Index\n"
			"7.Updation Of Table\n"
			"8.Dropping Of Table\n"
			"9.Deletion Of Table\n"
			"10.Bulk insertion into Table\n"
			"11.Show Tables\n"
			"12.Exit\n"
			"13.Selection From SysTable\n"
			"14.Selection From SysColumns\n"
			"15.Creation of Table For Bulk insertion\n");

	printf("------------------\nEnter the option:");
	int option;
	scanf("%d",&option);


//Datatypes for switch handling
	string table_name;
	string tabname_4;
	string  colvalue_4;


	string tablename_6;
	string columnname_6;
	string tablename_5;
	string columnname;

	string columnname_7;
	string index_column;
	string updated_value;
	int updated_value_type;


	string str[10]={"Rohit","Ravi","Ramu","Rahul","Reddy","Nirmala","Ramya","Raju","Rani","Priya"};

while(option!=12)
{

switch(option)
{

	case 1:
		char db_name[256];
		printf("\nEnter the Database Name:");
		cin>>db_name;
		createDatabase(db_name);
		printf("\n Database Successfully created:");
		fflush(stdout);
		break;

	case 2:
		char db_name1[256];
		printf("\nEnter the Database Name:");
		cin>>db_name1;
		usedatabase(db_name1);
		printf("\n Database Changed");
		break;

	case 3:
		//Keeping them in comments instead of running once again


		if(globalstructures->resultSet.size()>0)
			globalstructures->resultSet.clear();
		if(globalstructures->resultSetColumnList.size()>0)
			globalstructures->resultSetColumnList.clear();
		if(globalstructures->insert_type.size()>0)
			globalstructures->insert_type.clear();
		if(globalstructures->insert_values.size()>0)
			globalstructures->insert_values.clear();
		if(globalstructures->whereExprList.size()>0)
			globalstructures->whereExprList.clear();
		if(globalstructures->schema.columnnames.size()>0)
			globalstructures->schema.columnnames.clear();
		if(globalstructures->schema.constraints.size()>0)
			globalstructures->schema.constraints.clear();
		if(globalstructures->schema.default_values.size()>0)
			globalstructures->schema.default_values.clear();
		if(globalstructures->schema.fieldType.size()>0)
			globalstructures->schema.fieldType.clear();
		if(globalstructures->schema.field_length.size()>0)
			globalstructures->schema.field_length.clear();

		int numcolumns;
		printf("\nEnter the Table Name:");
		cin>>table_name;
		globalstructures->schema.tableName=table_name;
		printf("\n Enter the number of Columns:");
		cin>>numcolumns;
		globalstructures->schema.noofcolumns=numcolumns;
		for(int i=0;i<numcolumns;i++)
		{
			string columnname;
			int fieldtype;
			int field_length;
			string constraints;
			string default_values;
			printf("\n Enter the column name:");
			cin>>columnname;
			printf("\n Enter the field type:");
			cin>>fieldtype;
			if(fieldtype!=0)
			{
				printf("\n Enter the field length:");
				cin>>field_length;
				globalstructures->schema.field_length.push_back(field_length);
			}
			else
			{
				int integer_length=sizeof(int);
				globalstructures->schema.field_length.push_back(integer_length);
			}
			printf("\n Enter the constraints If nothing is there press 'n':");
			cin>>constraints;
			printf("\n Enter the default values If nothing is there press 'n':");
			cin>>default_values;
			globalstructures->schema.columnnames.push_back(columnname);
			globalstructures->schema.fieldType.push_back(fieldtype);


			if(strcmp(constraints.c_str(),"n")!=0)
				globalstructures->schema.constraints.push_back(constraints);
			else
				globalstructures->schema.constraints.push_back("");

			if(strcmp(default_values.c_str(),"n")!=0)
				globalstructures->schema.default_values.push_back(default_values);

			else
				globalstructures->schema.default_values.push_back("");

		}
		createTable();
		cout<<"\nTable is Successfully Created "<<table_name;

		/*
							globalstructures->schema.tableName="test";//table name
							globalstructures->schema.noofcolumns=6;//number of columns in my table is 2
							globalstructures->schema.columnnames.push_back("id");
							globalstructures->schema.columnnames.push_back("rollnumber");
							globalstructures->schema.columnnames.push_back("name");
							globalstructures->schema.columnnames.push_back("name1");
							globalstructures->schema.columnnames.push_back("name2");
							globalstructures->schema.columnnames.push_back("name3");
							globalstructures->schema.fieldType.push_back(0);
							globalstructures->schema.fieldType.push_back(0);
							globalstructures->schema.fieldType.push_back(2);
							globalstructures->schema.fieldType.push_back(2);
							globalstructures->schema.fieldType.push_back(2);
							globalstructures->schema.fieldType.push_back(2);
							globalstructures->schema.field_length.push_back(4);
							globalstructures->schema.field_length.push_back(4);
							globalstructures->schema.field_length.push_back(30);
							globalstructures->schema.field_length.push_back(30);
							globalstructures->schema.field_length.push_back(30);
							globalstructures->schema.field_length.push_back(30);
							globalstructures->schema.constraints.push_back("");
							globalstructures->schema.constraints.push_back("");
							globalstructures->schema.constraints.push_back("");
							globalstructures->schema.constraints.push_back("");
							globalstructures->schema.constraints.push_back("");
							globalstructures->schema.constraints.push_back("");
							globalstructures->schema.default_values.push_back("");
							globalstructures->schema.default_values.push_back("");
							globalstructures->schema.default_values.push_back("");
							globalstructures->schema.default_values.push_back("");
							globalstructures->schema.default_values.push_back("");
							globalstructures->schema.default_values.push_back("");*/
						    createTable();
						    cout<<"\n"+globalstructures->errorMsg;
						    break;

	case 4:
		//Kept the Comments for testing

		int type;
		int numberofcolumns;
		globalstructures->insert_values.clear();
		globalstructures->insert_type.clear();

		if(globalstructures->resultSet.size()>0)
			globalstructures->resultSet.clear();
		if(globalstructures->resultSetColumnList.size()>0)
			globalstructures->resultSetColumnList.clear();
		if(globalstructures->insert_type.size()>0)
			globalstructures->insert_type.clear();
		if(globalstructures->insert_values.size()>0)
			globalstructures->insert_values.clear();
		if(globalstructures->whereExprList.size()>0)
			globalstructures->whereExprList.clear();

		printf("\n Enter the Table Name:");
		cin>>tabname_4;
		globalstructures->tablename=tabname_4;
		//numberofcolumns=getting_number_of_columns(tabname_4);
		for(int i=0;i<globalstructures->schema.columnnames.size();i++)
		{
			cout<<"\nEnter the values for Column :";
			cin>>colvalue_4;
			//cout<<"Column Value is"<<colvalue_4;
			cout<<"\nEnter the type for Column:";
			cin>>type;
			//cout<<"Column Type is"<<type;
			globalstructures->insert_values.push_back(colvalue_4);
			//cout<<"Column Value is"<<globalstructures->insert_values.at(i);
			globalstructures->insert_type.push_back(type);
			//cout<<"Column Type is"<<globalstructures->insert_type.at(i);
		}
		insertRoutine();
		/*
		if(globalstructures->resultSet.size()>0)
			globalstructures->resultSet.clear();
		if(globalstructures->resultSetColumnList.size()>0)
			globalstructures->resultSetColumnList.clear();
		if(globalstructures->insert_type.size()>0)
			globalstructures->insert_type.clear();
		if(globalstructures->insert_values.size()>0)
			globalstructures->insert_values.clear();

						globalstructures->tablename="test";
						globalstructures->insert_values.push_back("3");
						globalstructures->insert_values.push_back("4");
						globalstructures->insert_values.push_back("rohit");
						globalstructures->insert_values.push_back("rani");
						globalstructures->insert_values.push_back("ravi");
						globalstructures->insert_values.push_back("raju");
						globalstructures->insert_type.push_back(0);
						globalstructures->insert_type.push_back(0);
						globalstructures->insert_type.push_back(2);
						globalstructures->insert_type.push_back(2);
						globalstructures->insert_type.push_back(2);
						globalstructures->insert_type.push_back(2);
						start=clock();
						insertRoutine();*/
		cout<<"\n"+globalstructures->errorMsg;
		cout<<"\n Value Inserted Successfully into"+globalstructures->tablename;
		break;

	case 5:

				if(globalstructures->resultSet.size()>0)
					globalstructures->resultSet.clear();
				if(globalstructures->resultSetColumnList.size()>0)
					globalstructures->resultSetColumnList.clear();
				if(globalstructures->insert_type.size()>0)
					globalstructures->insert_type.clear();
				if(globalstructures->insert_values.size()>0)
					globalstructures->insert_values.clear();
				if(globalstructures->whereExprList.size()>0)
					globalstructures->whereExprList.clear();

				/*globalstructures->resultSet.clear();//Since this was used above we need to clear
				globalstructures->resultSetColumnList.clear();
				globalstructures->whereExprList.clear();
				globalstructures->insert_type.clear();
				globalstructures->insert_values.clear();*/
				bool allcolumns;
				bool whereexpr;
				bool and_or;
				int operator_and_or;
				int numcolumns_5;
				printf("\n Enter the TableName for Selection:");
				cin>>tablename_5;
				globalstructures->tablename=tablename_5;
				cout<<globalstructures->tablename;
				printf("\n Do you want to Select all columns Enter 1(True) or False(0)");
				cin>>allcolumns;
				globalstructures->allColumns=allcolumns;

				if(allcolumns==1)
					globalstructures->allColumns=true;
				else
					globalstructures->allColumns=false;

				if(allcolumns==0)
				{
					printf("\n Enter the number of columns you wanted to Select");
					cin>>numcolumns_5;

				for(int i=0;i<numcolumns_5;i++)
				{
					printf("\n Enter the Column name %d you wanted to select:",i);
					cin>>columnname;
					globalstructures->resultSetColumnList.push_back(columnname);
				}
				}//end of if

				printf("\n Is there Where Expression? Enter 1(Yes) 0(No)");
				cin>>whereexpr;
				int no_of_cond;
				if(whereexpr)
				{
					printf("\n Enter the number of conditions in Where Clause:");
					cin>>no_of_cond;
					for(int j=0;j<no_of_cond;j++)
					{
						printf("\n In %d Condition",j);
						string ident;
						printf("\n Enter the Identifer:");
						cin>>ident;
						ExprInfo expr(ExprInfo::IDENTIFIER,ident);
						cout<<"\nValue is:"<<expr.identifier_value;
						fflush(stdout);
						globalstructures->whereExprList.push_back(expr);
						//expr.~ExprInfo();
						string oper;
						printf("\n Enter the oper:");
						cin>>oper;
						//cout<<"\nOperator---"<<oper;
						ExprInfo expr1(ExprInfo::OPERATOR,oper);
						globalstructures->whereExprList.push_back(expr1);
						//expr1.~ExprInfo();
						string literal_val;
						printf("\n Enter the Value For Literal:");
						cin>>literal_val;
						ExprInfo expr2(ExprInfo::LITERAL,literal_val);
						globalstructures->whereExprList.push_back(expr2);
						int type_literal_5;
						printf("\n Enter the Type of the Literal");
						cin>>type_literal_5;
						globalstructures->insert_type.push_back(type_literal_5);
						//expr2.~ExprInfo();
						string operators1;
						if(j!=(no_of_cond-1))
						{
							printf("\n Enter the operator:");
							cin>>operators1;
							globalstructures->operators=operators1;
						}
					}
				}
				start=clock();
				selectFromTable();
				//printf("\nFrom Select");
				//cout<<"\nglobalstructures size"<<globalstructures->resultSet.size();
					    	   			for(int i=0;i<globalstructures->resultSet.size();i++)
					    	   					    	    {
					    	   					    	   		   	//printf("\nFrom Select");
					    	   					    	   		    cout<<globalstructures->resultSet.at(i)+"\n";
					    	   					    	    }
					    	   			cout<<"\n"+globalstructures->errorMsg;
					    	   			printf("\nTime elapsed for selection Entries: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
		break;

	case 6://Creation Of Index
		printf("\n Enter the Tablename:");
		cin>>table_name;
		globalstructures->tablename=table_name;

		if(globalstructures->schema.columnnames.size()>0)
			globalstructures->schema.columnnames.clear();

		printf("\n Enter the Column on which you wanted to perform Index");
		cin>>index_column;

		globalstructures->schema.columnnames.push_back(index_column);
		globalstructures->indexName="index_id";

		createIndex();
		cout<<"\n"+globalstructures->errorMsg;
		printf("\n Index Created Successfully.");
	break;

	case 7://updation of table
			if(globalstructures->resultSet.size()>0)
				globalstructures->resultSet.clear();//Since this was used above we need to clear
			if(globalstructures->resultSetColumnList.size()>0)
			    globalstructures->resultSetColumnList.clear();
			if(globalstructures->whereExprList.size()>0)
			    globalstructures->whereExprList.clear();

			printf("\n Enter the Table Name");
			cin>>table_name;
			globalstructures->tablename=table_name;

			int no_of_update_columns;

			printf("\n Enter the number of Columns on Which Update need to perform:");
			cin>>no_of_update_columns;


			for(int i=0;i<no_of_update_columns;i++)
			{
				printf("\n Enter the i Column name for update:");
				cin>>columnname_7;
				globalstructures->update_fields.push_back(columnname_7);
				printf("\n Enter the i Column updated value:");
				cin>>updated_value;
				globalstructures->update_values.push_back(updated_value);
				printf("\n Enter the i column updated value type:");
				cin>>updated_value_type;
				globalstructures->update_type.push_back(updated_value_type);
			}

							printf("\n Is there Where Expression? Enter 1(Yes) 0(No)");
							cin>>whereexpr;
							int no_of_cond1;
							if(whereexpr)
							{
								printf("\n Enter the number of conditions in Where Clause:");
								cin>>no_of_cond1;
								for(int j=0;j<no_of_cond1;j++)
								{
									printf("\n In %d Condition",j);
									string ident;
									printf("\n Enter the Identifer:");
									cin>>ident;
									ExprInfo expr(ExprInfo::IDENTIFIER,ident);
									cout<<"\nValue is:"<<expr.identifier_value;
									fflush(stdout);
									globalstructures->whereExprList.push_back(expr);
									//expr.~ExprInfo();
									string oper;
									printf("\n Enter the oper:");
									cin>>oper;
									//cout<<"\nOperator---"<<oper;
									ExprInfo expr1(ExprInfo::OPERATOR,oper);
									globalstructures->whereExprList.push_back(expr1);
									//expr1.~ExprInfo();
									string literal_val;
									printf("\n Enter the Value For Literal:");
									cin>>literal_val;
									ExprInfo expr2(ExprInfo::LITERAL,literal_val);
									globalstructures->whereExprList.push_back(expr2);
									int type_literal_5;
									printf("\n Enter the Type of the Literal");
									cin>>type_literal_5;
									globalstructures->insert_type.push_back(type_literal_5);
									//expr2.~ExprInfo();
									string operators1;
									if(j!=(no_of_cond-1))
									{
										printf("\n Enter the operator:");
										cin>>operators1;
										globalstructures->operators=operators1;
									}
								}
							}
							cout<<"\n"+globalstructures->errorMsg;
			  			    updateTable();

		break;

	case 8://dropping the Table
		printf("\n Enter the Table you wanted to drop:");
		cin>>table_name;
		dropTable(table_name);
		printf("\n Select 'SHOW TABLES' option for seeing all the tables");
		break;

	case 9://Deletion of Table

						if(globalstructures->resultSet.size()>0)
						globalstructures->resultSet.clear();//Since this was used above we need to clear
					if(globalstructures->resultSetColumnList.size()>0)
					    globalstructures->resultSetColumnList.clear();
					if(globalstructures->whereExprList.size()>0)
					    globalstructures->whereExprList.clear();

					printf("\n Enter the Table Name");
					cin>>table_name;
					globalstructures->tablename=table_name;

					printf("\n Is there Where Expression? Enter 1(Yes) 0(No)");
					cin>>whereexpr;
									int no_of_cond2;
									if(whereexpr)
									{
										printf("\n Enter the number of conditions in Where Clause:");
										cin>>no_of_cond2;
										for(int j=0;j<no_of_cond2;j++)
										{
											printf("\n In %d Condition",j);
											string ident;
											printf("\n Enter the Identifer:");
											cin>>ident;
											ExprInfo expr(ExprInfo::IDENTIFIER,ident);
											cout<<"\nValue is:"<<expr.identifier_value;
											fflush(stdout);
											globalstructures->whereExprList.push_back(expr);
											//expr.~ExprInfo();
											string oper;
											printf("\n Enter the oper:");
											cin>>oper;
											//cout<<"\nOperator---"<<oper;
											ExprInfo expr1(ExprInfo::OPERATOR,oper);
											globalstructures->whereExprList.push_back(expr1);
											//expr1.~ExprInfo();
											string literal_val;
											printf("\n Enter the Value For Literal:");
											cin>>literal_val;
											ExprInfo expr2(ExprInfo::LITERAL,literal_val);
											globalstructures->whereExprList.push_back(expr2);
											int type_literal_5;
											printf("\n Enter the Type of the Literal");
											cin>>type_literal_5;
											globalstructures->insert_type.push_back(type_literal_5);
											//expr2.~ExprInfo();
											string operators1;
											if(j!=(no_of_cond2-1))
											{
												printf("\n Enter the operator:");
												cin>>operators1;
												globalstructures->operators=operators1;
											}
										}
									}
									deleteFromTable();
									break;



	case 10:

										if(globalstructures->resultSet.size()>0)
											globalstructures->resultSet.clear();
										if(globalstructures->resultSetColumnList.size()>0)
											globalstructures->resultSetColumnList.clear();
										if(globalstructures->insert_type.size()>0)
											globalstructures->insert_type.clear();
										if(globalstructures->insert_values.size()>0)
											globalstructures->insert_values.clear();
										if(globalstructures->whereExprList.size()>0)
											globalstructures->whereExprList.clear();

		globalstructures->tablename="test";
		int ran_entr;
		printf("\n How Many Random Entries you wanted to insert:");
		scanf("%d",&ran_entr);

		for(int i=5;i<ran_entr;i++)
		{
							globalstructures->insert_values.clear();
							globalstructures->insert_type.clear();
							globalstructures->insert_values.push_back(conversion::int_to_string(i));
						    globalstructures->insert_values.push_back(conversion::int_to_string(i));
						    globalstructures->insert_values.push_back(str[i%10]);
						    globalstructures->insert_values.push_back(str[i%2]);
						    globalstructures->insert_values.push_back(str[i%3]);
						    globalstructures->insert_values.push_back(str[i%5]);
							globalstructures->insert_type.push_back(0);
							globalstructures->insert_type.push_back(0);
							globalstructures->insert_type.push_back(2);
							globalstructures->insert_type.push_back(2);
							globalstructures->insert_type.push_back(2);
							globalstructures->insert_type.push_back(2);
							insertRoutine();
		}
		printf("\n Insertion of %d entries are successful",ran_entr);
		break;

	case 11://Showing the tables
		printf("\n The Tables in the database '");
		cout<<path<<"' are:\n";

		if(globalstructures->resultSet.size()>0)
			globalstructures->resultSet.clear();
		if(globalstructures->resultSetColumnList.size()>0)
			globalstructures->resultSetColumnList.clear();
		if(globalstructures->insert_type.size()>0)
			globalstructures->insert_type.clear();
		if(globalstructures->insert_values.size()>0)
			globalstructures->insert_values.clear();
		if(globalstructures->whereExprList.size()>0)
			globalstructures->whereExprList.clear();

		showTables();
		for(int i=0;i<globalstructures->resultSet.size();i++)
		{
							printf("\n");
							cout<<globalstructures->resultSet.at(i);
		}
		break;

	case 13://Selecting from SYS TABLE
				globalstructures->resultSet.clear();//Since this was used above we need to clear
				globalstructures->resultSetColumnList.clear();
				selectFromSysTable();
				//Selecting from the Sys Table
				cout<<"\n-----------------------";
				cout<<"\nContents of Sys-Table";
				cout<<"\nTableId#TableName#TableHeaderPage";
				cout<<"\n-----------------------";
				for(int i=0;i<globalstructures->resultSet.size();i++)
				{
									printf("\n");
									cout<<globalstructures->resultSet.at(i);
				}

		break;

	case 14://Selecting from SYS COLUMN
				globalstructures->resultSet.clear();//Since this was used above we need to clear
			    globalstructures->resultSetColumnList.clear();
			    selectFromSysColumn();
			    //Selecting from the Sys Column Table
			    cout<<"\n-----------------------";
			    cout<<"\nContents of Sys-Column";
			    cout<<"\nColumnTableId#columnId#colName#fieldType#fieldLength#";
			    cout<<"\n-----------------------";
			    for(int i=0;i<globalstructures->resultSet.size();i++)
			    {
			    							printf("\n");
			    							cout<<globalstructures->resultSet.at(i);
			    }

		break;

	case 15://Creation of Table for million row insertion

								if(globalstructures->resultSet.size()>0)
									globalstructures->resultSet.clear();
								if(globalstructures->resultSetColumnList.size()>0)
									globalstructures->resultSetColumnList.clear();
								if(globalstructures->insert_type.size()>0)
									globalstructures->insert_type.clear();
								if(globalstructures->insert_values.size()>0)
									globalstructures->insert_values.clear();
								if(globalstructures->whereExprList.size()>0)
									globalstructures->whereExprList.clear();
								if(globalstructures->schema.columnnames.size()>0)
									globalstructures->schema.columnnames.clear();
								if(globalstructures->schema.constraints.size()>0)
									globalstructures->schema.constraints.clear();
								if(globalstructures->schema.default_values.size()>0)
									globalstructures->schema.default_values.clear();
								if(globalstructures->schema.fieldType.size()>0)
									globalstructures->schema.fieldType.clear();
								if(globalstructures->schema.field_length.size()>0)
									globalstructures->schema.field_length.clear();



											globalstructures->schema.tableName="test";//table name
											globalstructures->schema.noofcolumns=6;//number of columns in my table is 2
											globalstructures->schema.columnnames.push_back("id");
											globalstructures->schema.columnnames.push_back("rollnumber");
											globalstructures->schema.columnnames.push_back("name");
											globalstructures->schema.columnnames.push_back("name1");
											globalstructures->schema.columnnames.push_back("name2");
											globalstructures->schema.columnnames.push_back("name3");
											globalstructures->schema.fieldType.push_back(0);
											globalstructures->schema.fieldType.push_back(0);
											globalstructures->schema.fieldType.push_back(2);
											globalstructures->schema.fieldType.push_back(2);
											globalstructures->schema.fieldType.push_back(2);
											globalstructures->schema.fieldType.push_back(2);
											globalstructures->schema.field_length.push_back(4);
											globalstructures->schema.field_length.push_back(4);
											globalstructures->schema.field_length.push_back(30);
											globalstructures->schema.field_length.push_back(30);
											globalstructures->schema.field_length.push_back(30);
											globalstructures->schema.field_length.push_back(30);
											globalstructures->schema.constraints.push_back("");
											globalstructures->schema.constraints.push_back("");
											globalstructures->schema.constraints.push_back("");
											globalstructures->schema.constraints.push_back("");
											globalstructures->schema.constraints.push_back("");
											globalstructures->schema.constraints.push_back("");
											globalstructures->schema.default_values.push_back("");
											globalstructures->schema.default_values.push_back("");
											globalstructures->schema.default_values.push_back("");
											globalstructures->schema.default_values.push_back("");
											globalstructures->schema.default_values.push_back("");
											globalstructures->schema.default_values.push_back("");
										    createTable();
										    cout<<"\n"<<globalstructures->errorMsg;
										    printf("\n Table has created successfully");

										    break;


	}//End of Switch
		printf("\n------------------\nEnter the option:");
		scanf("%d",&option);
}




	    free(maindbHeader);
	    printf("\n---------------");
	    printf("\nOver");
	    return 0;
}



