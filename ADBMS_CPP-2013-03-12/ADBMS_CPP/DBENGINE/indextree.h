#ifndef rohit
#define rohit
#include<iostream>
#include <fcntl.h> 
#include <string.h>
#include <malloc.h>
#include<stdlib.h>
#include <fcntl.h> 
#include<vector>
#define MAXDBNAME 20
#define FAILURE 0
#define SUCESS 1
//#define PAGESIZE 4096

//int fd=-1;

#define TYPE_INTEGER 0
#define TYPE_FLOAT 1
#define TYPE_VARCHAR 2
#define TYPE_CHARECTER 3
#define TYPE_DATE 4


//long pnum=-1;

using namespace std;
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

typedef struct RecordIdentifier{
    long pageNumber;
    int slotNo;
}RecordIdentifier;


#define KEY(node,i) node->pageData + (i+1)*sizeof(long) + i*keySize
#define LKEY(node,i) node->pageData + i*(sizeof(RecordIdentifier)+keySize)
#define PTR(node,i) *((long*)(node->pageData + i*(sizeof(long)+keySize)))
#define RID(node,i) *((RecordIdentifier*)(node->pageData + (i+1)*keySize + i*sizeof(RecordIdentifier)))
//vector<RecordIdentifier> ridVector;
class bPlusTree
{
private:
int fanout;
int keyType;
int keySize;
int numAttributes;
public:
long rootPageNumber;
bPlusTree();
bPlusTree(long rootPageNumber,int numattributes,int fanout,int keyType,int keySize);
//insert
bool insert(char *key,RecordIdentifier rid);
long findLeafPage(char *key);
bool insertIntoLeaf(Node *newNode,char* key,RecordIdentifier rid);
node* makeNode();
int keyCompare(char* newKey, char* oldKey);
bool insertIntoParent(Node* leftNode, char* key, Node* rightNode);
bool insertIntoLeafAfterSplitting(Node* leafNode, char* key, RecordIdentifier rid);
int cut(int x);
bool insertIntoNewRoot(Node *leftNode,char *key,Node *rightNode);
int getLeftIndex(Node* parent, Node* left);
bool insertIntoNodeAfterSplitting(Node* oldNode, int leftIndex, char* key, Node* rightNode);
bool insertIntoNode(Node *parent,int leftIndex,char *key,Node *rightNode);
void printKey(char* key);   
void printPage(long pageNumber); 
int bcreateDB(char *dbname, long pages);
int bopenDB(char *dbname);
void init_pagesize(int pagesize);
//search
int searchInNode(Node *searchNode,char *key);
bool search(char* key,vector<RecordIdentifier> &rid);
    void searchGreaterThanKey(char *greaterThanKey,vector<RecordIdentifier> &ridVector);
    void searchLesserThanKey(char *lesserThanKey, vector<RecordIdentifier> &ridVector);
};
#endif
