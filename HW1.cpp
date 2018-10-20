/********************
Arthur: Chung Han Liang
Student ID: 104409017
********************/

#include<iostream>
#include<fstream>
#include<stdio.h>
#include<string.h>
#include<string>
#include<map>
#include<list>
#include<algorithm>
#include<bitset>
#include<sstream>

using namespace std;

int NYTcount=0;

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
    list<bool> allEncodes;   //store all encodes from Result.raw
    list<unsigned char> allDecodes; //store all the pixel out from decoder

    list<unsigned char> Decoder(list<bool> encode)
    {
        list<unsigned char> ans;
        list<bool>::iterator curEncodeBit=encode.begin();
        chainNode* curPointing=root;

        while(curPointing!=0 && curEncodeBit!=encode.end())
        {
            if(curPointing==curNYT) // Its a NYTnode
            {
                string fixedLengthCodeStr="";
                for(int i=0;i<8;i++)  //8 bits fixed length code
                {
                    curEncodeBit++;
                    if(*curEncodeBit==1)
                        fixedLengthCodeStr.push_back('1');
                    else if(*curEncodeBit==0)
                        fixedLengthCodeStr.push_back('0');
                }
                bitset<8> fixedLengthCode(fixedLengthCodeStr);
                unsigned long tempUlong=fixedLengthCode.to_ulong();
                unsigned char tempUC=tempUlong;
                ans.push_back(tempUC);
                allDecodes.push_back(tempUC);
                curPointing=root;
                continue;
            }
            //Its a external node == Its a leaf node
            if(curPointing->leftChild==0
               && curPointing->rightChild==0)
            {
                ans.push_back(curPointing->symbol);
                allDecodes.push_back(curPointing->symbol);
                curPointing=root;
                continue;
            }

            if(*curEncodeBit==1)
            {
                curPointing=curPointing->rightChild;
                curEncodeBit++;
                continue;
            }
            else if(*curEncodeBit==0)
            {
                curPointing=curPointing->leftChild;
                curEncodeBit++;
                continue;
            }
        }
        return ans;
    }

    //this function should be change to returning boolean array
    list<bool> EncodingOneSymbol(int symbol)
    {
        list<bool> encodeList;
        string encodeStr;
        //int encode[255];
        //int encodeLength=0;
        // initial to 2, separate used and unused array space
        //for(int i=0;i<255;i++){encode[i]=2;}

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
                    //encode[encodeLength]=0;
                    //encodeLength++;
                    encodeStr.insert(0, "0");
                    encodeList.push_back(0);
                }
                else if(tempNode->parent->rightChild==tempNode)
                {
                    //encode[encodeLength]=1;
                    //encodeLength++;
                    encodeStr.insert(0, "1");
                    encodeList.push_back(1);
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
                    //encode[encodeLength]=0;
                    //encodeLength++;
                    encodeStr.insert(0, "0");
                    encodeList.push_back(0);
                }
                else if(tempNode->parent->rightChild==tempNode)
                {
                    //encode[encodeLength]=1;
                    //encodeLength++;
                    encodeStr.insert(0, "1");
                    encodeList.push_back(1);
                }

                tempNode=tempNode->parent;
            }
            /*send fixed length code for NYTlist*/
            /*the fixed length code will be 8 bits*/
            //encode[encodeLength]=curNYTListIndex;
            //encodeLength++;
            bitset<8> tempBitSet(symbol);   //the NYT list index are equivalent to symbol its  self
            string tempBitSetStr=tempBitSet.to_string();
            for(int i=0;i<tempBitSetStr.length();i++)
            {
                if(tempBitSetStr[i]=='1')
                    encodeList.push_back(1);
                else if(tempBitSetStr[i]=='0')
                    encodeList.push_back(0);
            }
            /*tempBitSetStr.insert(0, " ");
            tempBitSetStr.push_back(' ');*/
            encodeStr.insert(encodeStr.length(), tempBitSetStr);

            //This symbol is a NYT
            AddNYT(symbol);
            // adjust the NYT list index
            curNYTListIndex++;
            //For testing how many NYT be read
            NYTcount++;
        }
        return encodeList;
    }// encodeOneSymbol() end

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
        ifs.close();
    }

    list<bool> readEncode()
    {
        list<bool> ans; //Will be fed to Decoder()
        ifstream ifs("Result.raw", ios::binary);
        unsigned char value;
        char buf[sizeof(unsigned char)];    //unsigned char is 1 byte


        while(1){
        ifs.read(buf, sizeof(buf));
        if(!ifs) break;
        memcpy (&value, buf, sizeof(value));
        //cout<<(int)value<<" ";
        //change char to int
        bitset<8> tempBitset(value);
        string tempStr=tempBitset.to_string();
        for(int i=0;i<tempStr.length();i++)
        {
            if(tempStr[i]=='1')
            {
                ans.push_back(1);
                allEncodes.push_back(1);
            }
            else if(tempStr[i]=='0')
            {
                ans.push_back(0);
                allEncodes.push_back(0);
            }
        }
        }
        ifs.close();
        return ans;
    }

    int OutputToAfterDecode()
    {
        ofstream ofs;
        ofs.open("AfterDecode.raw", ofstream::out | ofstream::app);

        for(list<unsigned char>::iterator it=allDecodes.begin();
            it!=allDecodes.end();it++)
        {
            ofs<<*it;
        }
        ofs.close();
    }

    int OutputToResult(string bitString)
    {
        ofstream ofs;
        ofs.open("Result.raw", ofstream::out | ofstream::app);

        bitset<256> abc(bitString);
        unsigned long longInt=abc.to_ulong();
        unsigned char uc=longInt;
        ofs<<uc;
        ofs.close();
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
    tree decodeTree=tree();
    decodeTree.readEncode();
    //test readEncode()
    /*list<bool>::iterator encodeIt=decodeTree.allEncodes.begin();
    for(int i=0;i<10;i++)
    {
        cout<<*encodeIt<<endl;
        encodeIt++;
    }

    bitset<8> tempBitset(string("10100010"));
    unsigned char tempUC=tempBitset.to_ulong();
    int tempInt=tempBitset.to_ulong();
    cout<<tempInt<<endl;
    cout<<tempUC<<endl;*/

    decodeTree.Decoder(decodeTree.allEncodes);
    list<unsigned char>::iterator encodeIt=decodeTree.allDecodes.begin();
    for(int i=0;i<10;i++)
    {
        cout<<(int) (*encodeIt)<<endl;
        encodeIt++;
    }
    cout<<decodeTree.allDecodes.size()<<endl;

    decodeTree.OutputToAfterDecode();
}

//encoding needs 10s up
int main02()
{
    //don't need to output by byte, it don't need to be a image!!
    //https://stackoverflow.com/questions/27589460/how-to-write-single-bits-to-a-file-in-c
    //cpp can't write single bit to file
    //, so bits need to be batched until it is a byte, then can output
    ofstream ofs;
    ofs.open("Result.raw", ofstream::out | ofstream::app);
    int compareToFixedLenCode=0;
    int TotalBit=0;
    tree encodeTree=tree();
    encodeTree.readRAW();
    list<bool> TotalOut;
    TotalOut.clear();
    for(int i=0;i<512;i++)
    {
        for(int j=0;j<512;j++)
        {
            //string temp=encodeTree.EncodingOneSymbol(encodeTree.img[i][j]);
            //compareToFixedLenCode+=8-temp.length();
            //bitset<256> abc(temp);
            //unsigned long longInt=abc.to_ulong();
            //unsigned char uc=longInt;
            //ofs<<uc;

            list<bool> temp=encodeTree.EncodingOneSymbol(encodeTree.img[i][j]);
            compareToFixedLenCode+=(temp.size()-8);
            TotalBit+=8;
            //TotalOut.merge(temp);
            for(list<bool>::iterator it=temp.begin()
                ;it!=temp.end();it++)
            {
                TotalOut.push_back(*it);
            }
        }
    }

    int outputByte=0;
    //output to Result.raw
    int count8=0;
    bool outBoolArray[8]={0};
    for(list<bool>::iterator it=TotalOut.begin()
        ;;it++)
    {
        if(it==TotalOut.end())
        {
            outputByte++;
            if(count8!=0)           //fill last unfilled bits with 0
            {
                for(int i=count8;i<8;i++)
                {
                    outBoolArray[i]=0;
                }

            }
            unsigned char tempout=0;
            int tempcount2=128;
            for(int i=0;i<8;i++)
            {
                tempout+= outBoolArray[i]*tempcount2;
                tempcount2/=2;
            }
            ofs<<tempout;
            break;
        }
        outBoolArray[count8]=*it;
        count8++;
        if(count8>=8)
        {
            outputByte++;
            //start output
            unsigned char tempout=0;
            int tempcount2=128;
            for(int i=0;i<8;i++)
            {
                tempout+= outBoolArray[i]*tempcount2;
                tempcount2/=2;
            }
            ofs<<tempout;
            count8=0;
            for(int i=0;i<8;i++) outBoolArray[i]=0;
        }
    }

    ofs.close();
    cout<<"compareToFixedLenCode: "<<compareToFixedLenCode<<endl;
    cout<<"NYTcount: "<<NYTcount<<endl;
    cout<<"outputByte: "<<outputByte<<endl;
    cout<<"TotalOut.size(): "<<TotalOut.size()<<endl;
    cout<<"TotalBit: "<<TotalBit<<endl; //if use 8bit fixed length to store
}

int main01()
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
    /*string tempOutput=*/newTree.EncodingOneSymbol(2);
    /*cout<<tempOutput<<endl;*/
    /*cout<<newTree.appedSymbol[2]->Index<<endl;
    cout<<newTree.appedSymbol[2]->weight<<endl;
    cout<<newTree.curNYT->Index<<endl;
    cout<<newTree.curNYT->weight<<endl;
    cout<<newTree.root->Index<<endl;
    cout<<newTree.root->weight<<endl;
    cout<<newTree.appedSymbol[2]->parent->Index<<endl;
    cout<<newTree.appedSymbol[2]->parent->weight<<endl;*/

    //newTree.AddNYT(3);
    /*string tempOutput02=*/newTree.EncodingOneSymbol(3);
    /*cout<<tempOutput02<<endl;*/
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


    //output Test
    //output success
    /*int findInt=tempOutput02.find(" ");
    tempOutput02.erase(findInt, 1);
    ofstream ofs;
    ofs.open("Result.raw", ofstream::out);

    bitset<10> abc(tempOutput02);
    unsigned long longInt=abc.to_ulong();
    unsigned char uc=longInt;
    ofs<<uc;*/
}

