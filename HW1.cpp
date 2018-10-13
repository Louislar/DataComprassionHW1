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
    int Index;  //Node number
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

    int EncodingOneSymbol(int symbol)
    {
        map<int, chainNode*>::iterator search01=appedSymbol.find(symbol);
        if(search01!=appedSymbol.end())
        {
            // This symbol has already exist in the tree
            // use appedSymbol to get the encode, from leaf to root


            //then update the tree, this symbol's node's weight will +1

        }
        else
        {
            //This symbol is a NYT
            AddNYT(symbol);

        }
    }

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
        while(curParent!=root)
        {
            int curWeight=curParent->weight;
            chainNode* curBiggestIndex=curParent;
            for(list<chainNode*>::iterator iter=sameWeight[curWeight].begin()
                ;iter!=sameWeight[curWeight].end();iter++)
            {
                if( (*iter)->Index > curBiggestIndex->Index
                   && *iter != curParent->parent)
                    curBiggestIndex=*iter;
            }

            //swap the biggest Index node with curParent node
            //how to know he is his parent's left child or right child ?
            //A: just compare to its parents left/right child pointer
            {
                if(curParent->parent->leftChild == curParent)   //is left child
                {
                    if(curBiggestIndex->parent->leftChild==curBiggestIndex)
                    {
                        curParent->parent->leftChild=curBiggestIndex;
                        curBiggestIndex->parent->leftChild=curParent;
                    }
                    else if(curBiggestIndex->parent->rightChild==curBiggestIndex)
                    {
                        curParent->parent->leftChild=curBiggestIndex;
                        curBiggestIndex->parent->rightChild=curParent;
                    }
                }
                else if(curParent->parent->rightChild == curParent)
                {
                    if(curBiggestIndex->parent->leftChild==curBiggestIndex)
                    {
                        curParent->parent->rightChild=curBiggestIndex;
                        curBiggestIndex->parent->leftChild=curParent;
                    }
                    else if(curBiggestIndex->parent->rightChild==curBiggestIndex)
                    {
                        curParent->parent->rightChild=curBiggestIndex;
                        curBiggestIndex->parent->rightChild=curParent;
                    }
                }
            }

            // current parent's weight +1
            list<chainNode*>::iterator findIter=find(sameWeight[curParent->weight].begin()
                                         , sameWeight[curParent->weight].end(), curParent);
            sameWeight[curParent->weight].erase(findIter);
            curParent->weight=curParent->weight+1;
            sameWeight[curParent->weight].push_back(curParent);
            //parent's parent
            curParent=curParent->parent;
        }

        //change curNYT to the right one
        curNYT=curNYT->leftChild;

        //add in "not fist appearance symbol "list
        appedSymbol[symbol]=curNYT;

    }// AddNYT() end


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

