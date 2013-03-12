#ifndef indextree
#define indextree
#include "indextree.h"

using namespace std;

int fd=-1;
long pnum=-1;
int PAGESIZE=-1;

void bPlusTree:: init_pagesize(int pagesize)
{
	PAGESIZE=pagesize;
}


void bread(void *buffer, long page_number, int fd1)
{
	lseek(fd1,page_number*PAGESIZE,SEEK_SET);
	read(fd1,buffer,PAGESIZE);
}

void bwrite(void *buffer, long page_number, int fd2)
{
	lseek(fd2,page_number*PAGESIZE,SEEK_SET);
	write(fd2,buffer,PAGESIZE);
	
}

int bPlusTree :: bcreateDB(char *dbname, long pages)
{

    int i=0;
	char name[MAXDBNAME];
	//cout<<dbname;
	strcpy(name,dbname);
	cout<<"\n"<<name;
    if( open(strcat(name,".db"), O_RDONLY, 0666) != -1)
    {
    	printf("\n why failure");
    	//return FAILURE;
    }
    fd = open(name, O_CREAT | O_RDWR, 0666);
    //cout<<"created db with fd"<<fd;

    for(i=0;i<PAGESIZE*pages;i++)
    {
    	lseek(fd,i,SEEK_SET);
    	write(fd,"",1);
    }

    close(fd);
    return 1;
}

int  bPlusTree :: bopenDB(char *dbname)
{   
	int i = 0;
    //int fd;
	char name[MAXDBNAME];
	strcpy(name,dbname);
   

    fd = open(strcat(name,".db"),O_RDWR,0666);

    if (fd == -1)
    {
        cout<<"\n cannot open the database";
        return FAILURE;
    }

   
     return SUCESS;
}



bPlusTree::bPlusTree()
{
//cout<<"hi i am anil";
}
bPlusTree::bPlusTree(long rootPageNumber,int numattributes,int fanout,int keyType,int keySize)
{
this->fanout=fanout;
this->rootPageNumber=rootPageNumber;
this->numAttributes=numattributes;
this->keyType=keyType;
this->keySize=keySize;
//cout<<"hello i have done my task";
}
Node* bPlusTree::makeNode()
{
node *newnode=(Node*)malloc(PAGESIZE);
newnode->num_keys=0;
newnode->rightPageNumber=-1;
newnode->leftPageNumber=-1;
newnode->parentPageNumber=-1;
newnode->isleaf=false;
newnode->priority=2;
return newnode;
}


long getPageNumber()
{
pnum++;
return pnum;
}



long bPlusTree::findLeafPage(char *key){
    if(this->rootPageNumber == -1){
        return -1;
    }
    long tempPageNumber = this->rootPageNumber;
    
    Node *newNode = makeNode();
    char *bkey = (char*)malloc(keySize);
    while(tempPageNumber!=-1){
       
        bread(newNode,tempPageNumber,fd);
        if(newNode->isleaf)
            return tempPageNumber;
        int i;
        for(i=0;i<newNode->num_keys;i++){
            memcpy(bkey,KEY(newNode,i),keySize);
            if(keyCompare(key,bkey)<0)
                break;
        }
        tempPageNumber = PTR(newNode,i);
   }
   free(bkey);
   free(newNode);
   }
/*int bPlusTree::searchInNode(Node *searchNode,char *key){
     char *bkey;
     bkey=(char*)malloc(keySize);
     int i;
     for(i=0;i<searchNode->num_keys;i++)
     {
     if(searchNode->isleaf)
     {
     memcpy(bkey,LKEY(searchNode,i),keySize);
     if(keyCompare(bkey,key)==0)
     return 1;
     }
     else
     {
      memcpy(bkey,KEY(searchNode,i),keySize);
     if(keyCompare(bkey,key)==0)
     return 1;
     }
     if(i==(searchNode->num_keys-1))
     return 0;
       
 }
 }*/
 bool bPlusTree::search(char* key,vector<RecordIdentifier> &ridVector)
 {
  if(this->rootPageNumber == -1)
        return false;
    long leafPageNumber = findLeafPage(key);
    int i,diff,j=0;
    Node *leaf = makeNode();
    char bkey[keySize];
    bool flag;
    do{
        flag = false;
        bread(leaf,leafPageNumber,fd);
        for(i=0;i<leaf->num_keys;i++){
          memcpy(bkey,LKEY(leaf,i),keySize);
          diff = keyCompare(key,bkey);
          if(i==0 && diff==0)
                flag = true;
          if(diff == 0)
              ridVector.push_back(RID(leaf,i));
        }
        leafPageNumber = leaf->leftPageNumber;
        if(leafPageNumber == -1) break;
    }while(flag);
    free(leaf);
 
 }
int bPlusTree::keyCompare(char* newKey, char* oldKey){
    int newIntKey,oldIntKey;
    float newFloatKey,oldFloatKey;
    char newCharKey,oldCharKey;
    char *newVarcharKey,*oldVarcharKey;
//    long newLongKey,oldLongKey;
    
    int offset = 0;
    int returnValue;
    for(int i=0;i<numAttributes;i++){
        switch(keyType){
            case TYPE_INTEGER:
                memcpy(&newIntKey,&newKey[offset],keySize);
                memcpy(&oldIntKey,&oldKey[offset],keySize);
                offset += keySize;
                if(newIntKey < oldIntKey)
                    return -1;
                else if(newIntKey > oldIntKey)
                    return 1;
                break;
            case TYPE_FLOAT:
                memcpy(&newFloatKey,&newKey[offset],keySize);
                memcpy(&oldFloatKey,&oldKey[offset],keySize);
                offset += keySize;
                if(newFloatKey < oldFloatKey)
                    return -1;
                else if(newFloatKey > oldFloatKey)
                    return 1;
                break;
            case TYPE_CHARECTER:
                memcpy(&newCharKey,&newKey[offset],keySize);
                memcpy(&oldCharKey,&oldKey[offset],keySize);
                offset += keySize;
                if(newCharKey < oldCharKey)
                    return -1;
                else if(newCharKey > oldCharKey)
                    return 1;
                break;
            case TYPE_VARCHAR:
                newVarcharKey = (char*)malloc(keySize);
                oldVarcharKey = (char*)malloc(keySize);
                memcpy(newVarcharKey,&newKey[offset],keySize);
                memcpy(oldVarcharKey,&oldKey[offset],keySize);
                offset += keySize;
                returnValue = memcmp(newVarcharKey,oldVarcharKey,keySize);
                free(newVarcharKey);
                free(oldVarcharKey);
                if(returnValue < 0 || returnValue > 0)
                    return returnValue;
                break;
//            case TYPE_LONG://Not defined in commons.h
//                memcpy(&newLongKey,&newKey[offset],keySize);
//                memcpy(&oldLongKey,&oldKey[offset],keySize);
//                offset += keySize;
//                if(newLongKey < oldLongKey)
//                    return -1;
//                else if(newLongKey > oldLongKey)
//                    return 1;
//                break;
        }
    }
    return 0;
    
}
int bPlusTree::cut(int x){
    if(x % 2 == 0)
        return x/2;
    return x/2 + 1;
}
bool bPlusTree::insertIntoLeafAfterSplitting(Node* leafNode, char* key, RecordIdentifier rid){
    Node *newLeaf = makeNode();
    long newLeafPageNumber = getPageNumber();

    newLeaf->pageNumber = newLeafPageNumber;
    newLeaf->isleaf = true;
    
    char tempKeys[fanout][keySize];
    
    RecordIdentifier tempRids[fanout];
    
    
    int position = searchInNode(leafNode,key);
    
    int i,j;
    
    for(i=0,j=0;i<leafNode->num_keys;i++,j++){
        if(j == position) j++;
        memcpy(tempKeys[j],LKEY(leafNode,i),keySize);
        tempRids[j] = RID(leafNode,i);
    }
    memcpy(tempKeys[position],key,keySize);
    tempRids[position] = rid;
    
    leafNode->num_keys = 0;
    
    int split = cut(fanout-1);
    
    for(i=0;i<split;i++){
        memcpy(LKEY(leafNode,i),tempKeys[i],keySize);
        RID(leafNode,i) = tempRids[i];
        leafNode->num_keys++;
    }
    
    for(i=split,j=0;i<fanout;i++,j++){
        memcpy(LKEY(newLeaf,j),tempKeys[i],keySize);
        RID(newLeaf,j) = tempRids[i];
        newLeaf->num_keys++;
    }
    newLeaf->parentPageNumber = leafNode->parentPageNumber;

    
    //Adjusting left right page numbers
    
    newLeaf->leftPageNumber = leafNode->pageNumber;
    newLeaf->rightPageNumber = leafNode->rightPageNumber;
    
    
    if(leafNode->rightPageNumber != -1){
        Node *temp = makeNode();
        bread(temp,leafNode->rightPageNumber,fd);
        temp->leftPageNumber = newLeaf->pageNumber;
        temp->isleaf = true;
        bwrite(temp,leafNode->rightPageNumber,fd);
        free(temp);
    }
    leafNode->rightPageNumber = newLeaf->pageNumber;
    
    
    
    bool retVal = false;
    char newKey[keySize];
    
    memcpy(newKey,LKEY(newLeaf,0),keySize);
    
    bwrite(leafNode,leafNode->pageNumber,fd);
    bwrite(newLeaf,newLeaf->pageNumber,fd);
    
    
    retVal = insertIntoParent(leafNode,newKey,newLeaf);
    return retVal;
            
}
int bPlusTree::getLeftIndex(Node* parent, Node* left){
    int leftIndex = 0;
    while(leftIndex <= parent->num_keys && PTR(parent,leftIndex)!= left->pageNumber)
        leftIndex++;
    return leftIndex;
}
bool bPlusTree::insertIntoNewRoot(Node *leftNode,char *key,Node *rightNode){
        Node *parent = makeNode();
        long parentPageNumber = getPageNumber();

        rootPageNumber = parentPageNumber;
        parent->pageNumber = parentPageNumber;
        memcpy(KEY(parent,0),key,keySize);
        PTR(parent,0) = leftNode->pageNumber;
        PTR(parent,1) = rightNode->pageNumber;
        parent->num_keys++;
        leftNode->parentPageNumber = parentPageNumber;
        rightNode->parentPageNumber = parentPageNumber;
        bwrite(leftNode,leftNode->pageNumber,fd);
        bwrite(rightNode,rightNode->pageNumber,fd);
        bwrite(parent,parent->pageNumber,fd);
        free(parent);
        return true;
}


bool bPlusTree::insertIntoParent(Node* leftNode, char* key, Node* rightNode){
    long parentPageNumber = leftNode->parentPageNumber;
    if(parentPageNumber == -1){
        return insertIntoNewRoot(leftNode,key,rightNode);
    }
    Node *parent = makeNode();
    bread(parent,parentPageNumber,fd);
    
    int leftIndex = getLeftIndex(parent,leftNode);
    bool retVal = false;
    if(parent->num_keys < fanout-1){
        retVal = insertIntoNode(parent,leftIndex,key,rightNode);
        bwrite(leftNode,leftNode->pageNumber,fd);
        bwrite(rightNode,rightNode->pageNumber,fd);
        free(parent);
        return retVal;
    }
    
    
    retVal = insertIntoNodeAfterSplitting(parent,leftIndex,key,rightNode);
    
    return retVal;
}
bool bPlusTree::insertIntoNode(Node *parent,int leftIndex,char *key,Node *rightNode){
    for(int i=parent->num_keys;i > leftIndex;i--){
        PTR(parent,(i+1)) = PTR(parent,i);
        memcpy(KEY(parent,i),KEY(parent,(i-1)),keySize);
    }
    PTR(parent,(leftIndex+1)) = rightNode->pageNumber;
    memcpy(KEY(parent,leftIndex),key,keySize);
    parent->num_keys++;
    bwrite(parent,parent->pageNumber,fd);
    return true;
}
void bPlusTree::printPage(long pageNumber){
    Node *tempNode = makeNode();
    char *key = (char*)malloc(keySize);
    bread(tempNode,pageNumber,fd);
    if(tempNode->isleaf){
        for(int i=0;i<tempNode->num_keys;i++){
            memcpy(key,LKEY(tempNode,i),keySize);
            printKey(key);
            cout<<endl;
        }
    }
    else{
        long pointer;
        cout<<"\nPrinting Internal node"<<endl;
        cout<<"Number of keys:"<<tempNode->num_keys<<endl;
        int i;
        for(i=0;i<tempNode->num_keys;i++){
            pointer = PTR(tempNode,i);
            memcpy(key,KEY(tempNode,i),keySize);
            cout<<pointer<<" ";
            printKey(key);
            cout<<" ";
        }
        pointer = PTR(tempNode,i);
        cout<<pointer<<" ";
    }
    free(key);
    cout<<endl;    
}
void bPlusTree::printKey(char* key){
    
    int intKey;
    float floatKey;
    char charKey;
    char *varcharKey;
//    long longKey;
    
    int offset=0;
    cout<<"{";
    for(int i=0;i<numAttributes;i++){
        switch(keyType){
            case TYPE_INTEGER:
                memcpy(&intKey,&key[offset],keySize);
                cout<<intKey<<",";
                offset += keySize;
            break;
            
            case TYPE_FLOAT:
                memcpy(&floatKey,&key[offset],keySize);
                cout<<floatKey<<",";
                offset += keySize;
            break;
            
            case TYPE_CHARECTER:
                memcpy(&charKey,&key[offset],keySize);
                cout<<charKey<<",";
                offset += keySize;
            break;
            
            case TYPE_VARCHAR:
                varcharKey = (char*) malloc(keySize);
                memcpy(varcharKey,&key[offset],keySize);
                cout<<varcharKey<<",";
                free(varcharKey);
                offset += keySize;
            break;
            
//            case LONG://Not defined in commons.h
//                memcpy(&longKey,&key[offset],keySize);
//                cout<<longKey<<",";
//                offset += keySize;
//            break;
        }
    }
    cout<<"}";
}

bool bPlusTree::insertIntoNodeAfterSplitting(Node* oldNode, int leftIndex, char* key, Node* rightNode){
    
    long tempPointers[fanout+1];
    char tempKeys[fanout][keySize];
    
    int i,j;
    
    for(i=0,j=0;i<oldNode->num_keys+1;i++,j++){
        if(j == leftIndex+1) j++;
        tempPointers[j] = PTR(oldNode,i);
    }
    
    for(i=0,j=0;i<oldNode->num_keys;i++,j++){
        if(j == leftIndex) j++;
        memcpy(tempKeys[j],KEY(oldNode,i),keySize);
    }
    tempPointers[leftIndex+1] = rightNode->pageNumber;
    memcpy(tempKeys[leftIndex],key,keySize);
    
    int split = cut(fanout);
    Node *newNode = makeNode();
    newNode->pageNumber = getPageNumber();

    
    oldNode->num_keys = 0;
    for(i=0;i< split-1;i++){
        PTR(oldNode,i) = tempPointers[i];
        memcpy(KEY(oldNode,i),tempKeys[i],keySize);
        oldNode->num_keys++;
    }
    PTR(oldNode,i) = tempPointers[i];
    
    char newKey[keySize];
    memcpy(newKey,tempKeys[split-1],keySize);
    
    for(++i, j=0; i<fanout;i++,j++){
        PTR(newNode,j) = tempPointers[i];
        memcpy(KEY(newNode,j),tempKeys[i],keySize);
        newNode->num_keys++;
    }
    PTR(newNode,j) = tempPointers[i];
    
    //Adjust left right page numbers
    newNode->leftPageNumber = oldNode->pageNumber;
    newNode->rightPageNumber = oldNode->rightPageNumber;
    
    if(oldNode->rightPageNumber != -1){
        Node *temp = makeNode();
        bread(temp,oldNode->rightPageNumber,fd);
        temp->leftPageNumber = oldNode->pageNumber;
        bwrite(temp,oldNode->rightPageNumber,fd);
        free(temp);
    }
    oldNode->rightPageNumber = newNode->pageNumber;
    
    //Copying parent page number
    newNode->parentPageNumber = oldNode->parentPageNumber;
    
    Node *child = makeNode();
    
    for(i=0;i<=newNode->num_keys;i++){
        bread(child,PTR(newNode,i),fd);
        child->parentPageNumber = newNode->pageNumber;
        bwrite(child,child->pageNumber,fd);
    }
    bwrite(oldNode,oldNode->pageNumber,fd);
    bwrite(newNode,newNode->pageNumber,fd);
    bool retVal = false;
    retVal = insertIntoParent(oldNode,newKey,newNode);
    
    free(oldNode);
    free(newNode);
    free(child);
}

int bPlusTree::searchInNode(Node* searchNode, char* key){
    char bkey[keySize];
    int i;
    for(i=0;i < searchNode->num_keys;i++){
        if(searchNode->isleaf)
            memcpy(bkey,LKEY(searchNode,i),keySize);
        else
            memcpy(bkey,KEY(searchNode,i),keySize);
        if(keyCompare(key,bkey)<0)
            break;
    }
    return i;
}

bool bPlusTree::insertIntoLeaf(Node* leafNode,char* key,RecordIdentifier rid)
{
if(leafNode->num_keys < fanout-1){
        int position = searchInNode(leafNode,key);
        int i;
        for(i=leafNode->num_keys;i>position;i--){
            memcpy(LKEY(leafNode,i),LKEY(leafNode,(i-1)),keySize);
            //printKeyAtNode(leafNode,i-1);
            RID(leafNode,i) = RID(leafNode,(i-1));
        }
        memcpy(LKEY(leafNode,position),key,keySize);
        RID(leafNode,position) = rid;
        leafNode->num_keys++;
        bwrite(leafNode,leafNode->pageNumber,fd);
    }
    return true;
}
void bPlusTree::searchLesserThanKey(char *lesserThanKey, vector<RecordIdentifier> &ridVector){
long leafPageNumber = findLeafPage(lesserThanKey);
    Node *leafNode = makeNode();
    RecordIdentifier rid;
    bread(leafNode,leafPageNumber,fd);

    char bkey[keySize];
    for(int i=0;i<leafNode->num_keys;i++){
        memcpy(bkey,LKEY(leafNode,i),keySize);
        if(keyCompare(bkey,lesserThanKey)<0){
            rid = RID(leafNode,i);
            ridVector.push_back(rid);
        }
    }
    while(leafNode->leftPageNumber!=-1){
        bread(leafNode,leafNode->rightPageNumber,fd);
        for(int i=0;i<leafNode->num_keys;i++){
            rid = RID(leafNode,i);
            ridVector.push_back(rid);
        }
    }
    free(leafNode);
}
    void bPlusTree::searchGreaterThanKey(char *greaterThanKey,vector<RecordIdentifier> &ridVector){
     long leafPageNumber = findLeafPage(greaterThanKey);
    Node *leafNode = makeNode();
    RecordIdentifier rid;
    bread(leafNode,leafPageNumber,fd);

    char bkey[keySize];
    for(int i=0;i<leafNode->num_keys;i++){
        memcpy(bkey,LKEY(leafNode,i),keySize);
        if(keyCompare(bkey,greaterThanKey)>0){
            rid = RID(leafNode,i);
            ridVector.push_back(rid);
        }
    }
    while(leafNode->rightPageNumber!=-1){
        bread(leafNode,leafNode->rightPageNumber,fd);
        for(int i=0;i<leafNode->num_keys;i++){
            rid = RID(leafNode,i);
            ridVector.push_back(rid);
        }
    }
    free(leafNode);
    }

bool bPlusTree::insert(char* key, RecordIdentifier rid){
    //printf("In Insert%d\n",*(int *)key);
    fflush(stdout);
	Node *newNode;
    if(this->rootPageNumber == -1){
        newNode = makeNode();
        memcpy(LKEY(newNode,0),key,keySize);
        RID(newNode,0) = rid;
        newNode->isleaf = true;
        newNode->num_keys++;
        this->rootPageNumber = getPageNumber();

        newNode->pageNumber = this->rootPageNumber;
        bwrite(newNode,newNode->pageNumber,fd);
        free(newNode);
        //printf("Out of Insert%d\n",*(int *)key);
        fflush(stdout);
        return true;
    }
    long leafPageNumber = findLeafPage(key);
    newNode = makeNode();
    bread(newNode,leafPageNumber,fd);
    bool retVal = false;
    if(newNode->num_keys < fanout-1){
        retVal = insertIntoLeaf(newNode,key,rid);
        free(newNode);
        //printf("Out of Insert%d\n",*(int *)key);
                fflush(stdout);
        return retVal;
    }
    retVal = insertIntoLeafAfterSplitting(newNode,key,rid);
    free(newNode);
    //printf("Out of Insert%d\n",*(int *)key);
            fflush(stdout);
    return retVal;
}
#endif
