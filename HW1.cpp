#include<iostream>
#include<fstream>
#include<stdio.h>
#include<string.h>
#include<map>
#include<list>
#include<algorithm>

using namespace std;

class chainNode
{
public:
    chainNode(){}
    chainNode(int i, int w, int s, chainNode* p)
    {
        Index=i;
        weight=w;
        symbol=s;
        leftChild=0;
        rightChild=0;
        parent=p;
    }

    chainNode* leftChild;
    chainNode* rightChild;
    chainNode* parent;
    int Index;
    int weight;
    int symbol; //256 means this is not a symbol node
};              //(this node don't have a symbol)

class tree
{
public:
    tree(){
        root=new chainNode(511, 0, 256, 0);
        curNYT=root;

        //sameWeight[0]=list<chainNode*>();
        sameWeight[0].push_back(root);
    }

    chainNode* root;
    chainNode* curNYT;
    map<int, list<chainNode*> > sameWeight;
    map<int, chainNode* > appedSymbol;  //not first appearance symbol
    void AddNYT(int symbol)
    {
        // create new NYT
        curNYT->leftChild=new chainNode(curNYT->Index-2
                                        , 0, 256, curNYT);
        curNYT->rightChild=new chainNode(curNYT->Index-1
                                         , 1, symbol, curNYT);
        sameWeight[0].push_back(curNYT->leftChild);
        sameWeight[1].push_back(curNYT->rightChild);
        curNYT->weight=1;
        list<chainNode*>::iterator findIter=find(sameWeight[0].begin()
                                         , sameWeight[0].end(), curNYT);
        sameWeight[0].erase(findIter);


        // adjust parent node weight
        chainNode* curParent=curNYT->parent;
        while(curParent!=0)
        {
            int curWeight=curParent->weight;
            chainNode* curBiggestIndex=curParent;
            for(list<chainNode*>::iterator iter=sameWeight[curWeight].begin()
                ;iter!=sameWeight[curWeight].end();iter++)
            {
                if( (*iter)->Index > curBiggestIndex->Index
                   || *iter != curParent->parent)
                    curBiggestIndex=*iter;
            }

            //swap the biggest Index node with curParent node
            {
                chainNode* tempNode;

            }

            curParent=curParent->parent;
        }


        curNYT=curNYT->leftChild;

        //add in "not fist appearance symbol "list
        appedSymbol[symbol]=curNYT;

    }
};


int readRAW()
{
    // https://stackoverflow.com/questions/37781876/reading-raw-file-in-c
    ifstream ifs("Lena.raw", ios::binary);
    unsigned char value;
    char buf[sizeof(unsigned char)];    //unsigned char is 1 byte

    int img[500];
    for(int i=0;i<256;i++) {img[i]=0;}

    for(int i=0;i<512;i++){
    for(int j=0;j<512;j++){
    ifs.read(buf, sizeof(buf));
    memcpy (&value, buf, sizeof(value));
    //cout<<(int)value<<" ";
    //change char to int
    img[(int)value]=img[(int)value]+1;
    }
    }

    //the number will between 0~255

    /*for(int i=0;i<256;i++)
    {
        cout<<i<<" ";
    }
    cout<<endl;
    int sum=0;
    for(int i=0;i<256;i++)
    {
        //cout<<summarize[i]<<" ";
        sum=sum+img[i];
    }
    cout<<512*512<<" "<<sum;*/
}


int main()
{
    readRAW();
}

