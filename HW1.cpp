/********************
Arthur: Chung Han Liang
Student ID: 104409017
********************/

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
        NYTListIndex=256;
    }

    chainNode* leftChild;
    chainNode* rightChild;
    chainNode* parent;
    int NYTListIndex;   //256 means this node is not in NYT list
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

        // initial NYT List index
        curNYTListIndex=0;

        // initial image
        for(int i=0;i<512;i++)
            for(int j=0;j<512;j++)
                img[i][j]=0;
    }

    chainNode* root;
    chainNode* curNYT;
    map<int, list<chainNode*> > sameWeight; //same weight block
    map<int, chainNode* > appedSymbol;  //not first appearance symbol
    int curNYTListIndex;
    int img[512][512];

    int EncodingOneSymbol(int symbol)
    {
        int encode[255];
        int encodeLength=0;
        // initial to 2, separate used and unused array space
        for(int i=0;i<255;i++){encode[i]=2;}

        map<int, chainNode*>::iterator search01=appedSymbol.find(symbol);
        if(search01!=appedSymbol.end())
        {
            // This symbol has already exist in the tree
            // use appedSymbol to get the encode, from leaf to root
            chainNode* tempNode=appedSymbol[symbol];
            while(tempNode!=root)   //left 0, right 1
            {
                if(tempNode->parent->leftChild==tempNode)
                {
                    encode[encodeLength]=0;
                    encodeLength++;
                }
                else if(tempNode->parent->leftChild==tempNode)
                {
                    encode[encodeLength]=1;
                    encodeLength++;
                }

                tempNode=tempNode->parent;
            }

            //then update the tree, this symbol's node's weight will +1
            AddNotNYT(symbol);
        }
        else
        {
            //Get the NYTcode + index in the appedSymbol
            chainNode* tempNode=curNYT;
            while(tempNode!=root)   //left 0, right 1
            {
                if(tempNode->parent->leftChild==tempNode)
                {
                    encode[encodeLength]=0;
                    encodeLength++;
                }
                else if(tempNode->parent->leftChild==tempNode)
                {
                    encode[encodeLength]=1;
                    encodeLength++;
                }

                tempNode=tempNode->parent;
            }
            encode[encodeLength]=curNYTListIndex;
            encodeLength++;
            //This symbol is a NYT
            AddNYT(symbol);
            // adjust the NYT list index
            curNYTListIndex++;
        }
    }

    void AddNotNYT(int symbol)
    {
        chainNode* curSymbolNode=appedSymbol[symbol];
        /*list<chainNode*>::iterator findIter=find(sameWeight[curSymbolNode->weight].begin()
                                                 , sameWeight[curSymbolNode->weight].end()
                                                 , curSymbolNode);
        sameWeight[curSymbolNode->weight].erase(findIter);
        sameWeight[(curSymbolNode->weight)+1].push_back(curSymbolNode);
        curSymbolNode->weight=(curSymbolNode->weight)+1;*/

        // Adjust parents weight
        chainNode* curParent=curSymbolNode;
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

            /*cout<<"curWeight: "<<curWeight<<endl;
            cout<<"curBiggestIndex->weight: "<<curBiggestIndex->weight<<endl;
            cout<<"curBiggestIndex->symbol: "<<curBiggestIndex->symbol<<endl;
            cout<<"curBiggestIndex->Index: "<<curBiggestIndex->Index<<endl;*/

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
                //swap the current node's parent pointer
                {
                    chainNode* tempNode=curParent->parent;
                    curParent->parent=curBiggestIndex->parent;
                    curBiggestIndex->parent=tempNode;
                }

                //swap the index
                int tempIndex=curParent->Index;
                curParent->Index=curBiggestIndex->Index;
                curBiggestIndex->Index=tempIndex;
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
        //root weight adjust
        list<chainNode*>::iterator findIter=find(sameWeight[root->weight].begin()
                                         , sameWeight[root->weight].end(), root);
        sameWeight[root->weight].erase(findIter);
        (root->weight)++;
        sameWeight[root->weight].push_back(root);
        //Adjust Parents weight End
    }

    void AddNYT(int symbol)
    {
        // create new NYT
        curNYT->leftChild=new chainNode((curNYT->Index)-2
                                        , 0, 256, curNYT);
        curNYT->rightChild=new chainNode((curNYT->Index)-1
                                         , 1, symbol, curNYT);
        sameWeight[0].push_back(curNYT->leftChild);
        sameWeight[1].push_back(curNYT->rightChild);
        curNYT->weight=1;
        list<chainNode*>::iterator findIter=find(sameWeight[0].begin()
                                         , sameWeight[0].end(), curNYT);
        sameWeight[0].erase(findIter);
        sameWeight[1].push_back(curNYT);

        // Add new symbol to NYT list and give it an index
        appedSymbol[curNYT->rightChild->symbol]=curNYT->rightChild;
        curNYT->rightChild->NYTListIndex=curNYTListIndex;

        // adjust parent node weight
        chainNode* curParent=curNYT->parent;
        while(curParent!=root&&curParent!=0)
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
                //swap the current node's parent pointer
                {
                    chainNode* tempNode=curParent->parent;
                    curParent->parent=curBiggestIndex->parent;
                    curBiggestIndex->parent=tempNode;
                }
                //swap the index
                int tempIndex=curParent->Index;
                curParent->Index=curBiggestIndex->Index;
                curBiggestIndex->Index=tempIndex;

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
        if(curNYT!=root)
        {
            list<chainNode*>::iterator findIter=find(sameWeight[root->weight].begin()
                                         , sameWeight[root->weight].end(), root);
            sameWeight[root->weight].erase(findIter);
            (root->weight)++;
            sameWeight[root->weight].push_back(root);
        }

        //change curNYT to the right one
        curNYT=curNYT->leftChild;

    }// AddNYT() end

    int readRAW()
    {
        ifstream ifs("Lena.raw", ios::binary);
        unsigned char value;
        char buf[sizeof(unsigned char)];    //unsigned char is 1 byte

        for(int i=0;i<512;i++){
        for(int j=0;j<512;j++){
        ifs.read(buf, sizeof(buf));
        memcpy (&value, buf, sizeof(value));
        //cout<<(int)value<<" ";
        //change char to int
        img[i][j]=(int)value;
        }
        }
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
    tree newTree=tree();
    //readRAW();
    newTree.readRAW();


    //newTree.AddNYT(1);
    newTree.EncodingOneSymbol(1);
    /*cout<<newTree.appedSymbol[1]->Index<<endl;
    cout<<newTree.curNYT->Index<<endl;
    cout<<newTree.root->Index<<endl;*/

    //newTree.AddNYT(2);
    newTree.EncodingOneSymbol(2);
    /*cout<<newTree.appedSymbol[2]->Index<<endl;
    cout<<newTree.appedSymbol[2]->weight<<endl;
    cout<<newTree.curNYT->Index<<endl;
    cout<<newTree.curNYT->weight<<endl;
    cout<<newTree.root->Index<<endl;
    cout<<newTree.root->weight<<endl;
    cout<<newTree.appedSymbol[2]->parent->Index<<endl;
    cout<<newTree.appedSymbol[2]->parent->weight<<endl;*/

    //newTree.AddNotNYT(2);
    newTree.EncodingOneSymbol(2);
    /*cout<<newTree.appedSymbol[2]->Index<<endl;
    cout<<newTree.appedSymbol[2]->weight<<endl;
    cout<<newTree.curNYT->Index<<endl;
    cout<<newTree.curNYT->weight<<endl;
    cout<<newTree.root->Index<<endl;
    cout<<newTree.root->weight<<endl;
    cout<<newTree.appedSymbol[2]->parent->Index<<endl;
    cout<<newTree.appedSymbol[2]->parent->weight<<endl;*/

    //newTree.AddNYT(3);
    newTree.EncodingOneSymbol(3);
    cout<<"newTree.root->Index: "<<newTree.root->Index<<endl;
    cout<<"newTree.root->weight: "<<newTree.root->weight<<endl;
    cout<<"newTree.root->rightChild->Index: "<<newTree.root->rightChild->Index<<endl;
    cout<<"newTree.root->rightChild->weight: "<<newTree.root->rightChild->weight<<endl;
    cout<<"newTree.root->leftChild->Index: "<<newTree.root->leftChild->Index<<endl;
    cout<<"newTree.root->leftChild->weight: "<<newTree.root->leftChild->weight<<endl;
    cout<<"newTree.root->leftChild->rightChild->Index: "<<newTree.root->leftChild->rightChild->Index<<endl;
    cout<<"newTree.root->leftChild->rightChild->weight: "<<newTree.root->leftChild->rightChild->weight<<endl;
    cout<<"newTree.root->leftChild->leftChild->Index: "<<newTree.root->leftChild->leftChild->Index<<endl;
    cout<<"newTree.root->leftChild->leftChild->weight: "<<newTree.root->leftChild->leftChild->weight<<endl;
    cout<<"newTree.root->leftChild->leftChild->rightChild->Index: "<<newTree.root->leftChild->leftChild->rightChild->Index<<endl;
    cout<<"newTree.root->leftChild->leftChild->rightChild->weight: "<<newTree.root->leftChild->leftChild->rightChild->weight<<endl;
    cout<<"newTree.root->leftChild->leftChild->leftChild->Index: "<<newTree.root->leftChild->leftChild->leftChild->Index<<endl;
    cout<<"newTree.root->leftChild->leftChild->leftChild->weight: "<<newTree.root->leftChild->leftChild->leftChild->weight<<endl;
}

