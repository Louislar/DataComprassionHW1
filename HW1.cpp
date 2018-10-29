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
#include<math.h>

using namespace std;

int NYTcount=0;

class DPCMarray
{
public:
    DPCMarray(){
        for(int i=0;i<512;i++)
            for(int j=0;j<512;j++)
                DPCMimg[i][j]=0;
    }
    int DPCMimg[512][512];
};

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
        for(int i=0;i<512;i++){
            for(int j=0;j<512;j++)
            {
                img[i][j]=0;
            }
        }

        //initial DPCM
        DPCM=new DPCMarray();
    }

    chainNode* root;
    chainNode* curNYT;
    map<int, list<chainNode*> > sameWeight; //same weight block
    map<int, chainNode* > appedSymbol;  //not first appearance symbol
    int curNYTListIndex;
    int img[512][512];
    DPCMarray* DPCM;
    list<bool> afterEncode;  // store all bits after encoding the original image
    list<bool> allEncodes;   //store all encodes from Result.raw
    list<unsigned char> allDecodes; //store all the pixel out from decoder

    list<unsigned char> Decoder(list<bool> encode)
    {
        list<unsigned char> ans;
        list<bool>::iterator curEncodeBit=encode.begin();
        chainNode* curPointing=root;
        int decodeCount=0;

        while(curPointing!=0 && curEncodeBit!=encode.end())
        {
            if(curPointing==curNYT) // Its a NYTnode
            {
                string fixedLengthCodeStr="";
                for(int i=0;i<8;i++)  //8 bits fixed length code
                {
                    if(*curEncodeBit==1)
                        fixedLengthCodeStr.push_back('1');
                    else if(*curEncodeBit==0)
                        fixedLengthCodeStr.push_back('0');
                    curEncodeBit++;
                }
                bitset<8> fixedLengthCode(fixedLengthCodeStr);
                unsigned long tempUlong=fixedLengthCode.to_ulong();
                unsigned char tempUC=tempUlong;
                ans.push_back(tempUC);
                allDecodes.push_back(tempUC);
                EncodingOneSymbol(tempUC);
                curPointing=root;
                decodeCount++;
                if(decodeCount>=512*512) break;
                continue;
            }
            //Its a external node == Its a leaf node
            else if(curPointing->leftChild==0
               && curPointing->rightChild==0)
            {
                ans.push_back(curPointing->symbol);
                allDecodes.push_back(curPointing->symbol);
                EncodingOneSymbol(curPointing->symbol);
                curPointing=root;
                decodeCount++;
                if(decodeCount>=512*512) break;
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
                    encodeStr.insert(0, "0");
                    encodeList.push_front(0);
                }
                else if(tempNode->parent->rightChild==tempNode)
                {
                    encodeStr.insert(0, "1");
                    encodeList.push_front(1);
                }

                tempNode=tempNode->parent;
            }
            for(list<bool>::iterator it=encodeList.begin();
                it!=encodeList.end();it++)
                {
                    afterEncode.push_back(*it);
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
                    encodeStr.insert(0, "0");
                    encodeList.push_front(0);
                }
                else if(tempNode->parent->rightChild==tempNode)
                {
                    encodeStr.insert(0, "1");
                    encodeList.push_front(1);
                }

                tempNode=tempNode->parent;
            }
            /*send fixed length code for NYTlist*/
            /*the fixed length code will be 8 bits*/
            bitset<8> tempBitSet(symbol);   //the NYT list index are equivalent to symbol its  self
            string tempBitSetStr=tempBitSet.to_string();
            for(int i=0;i<tempBitSetStr.length();i++)
            {
                if(tempBitSetStr[i]=='1'){
                    encodeList.push_back(1);
                }
                else if(tempBitSetStr[i]=='0'){
                    encodeList.push_back(0);
                }
            }
            for(list<bool>::iterator it=encodeList.begin();
                it!=encodeList.end();it++)
                {
                    afterEncode.push_back(*it);
                }
            /*tempBitSetStr.insert(0, " ");
            tempBitSetStr.push_back(' ');*/
            encodeStr.insert(encodeStr.length(), tempBitSetStr);

            //This symbol is a NYT
            AddNYT(symbol);
            // adjust the NYT list index
            curNYTListIndex++;
            //For testing how many NYT have been read
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

    int readRAW(string filename)
    {
        // https://stackoverflow.com/questions/37781876/reading-raw-file-in-c
        ifstream ifs(filename.c_str(), ios::binary);
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

    list<bool> readEncode(string filename)
    {
        list<bool> ans; //Will be fed to Decoder()
        ifstream ifs(filename.c_str(), ios::binary);
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
            else
            {
                cout<<"\nERROR\n";
            }
        }

        }
        ifs.close();
        cout<<"\nallEncodes.size(): "<<allEncodes.size()<<endl;

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

        //test
        cout<<"allDecodes.size()_bytes: "<<allDecodes.size()<<endl;
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

    int createDPCM()
    {
        //first row is top minus down
        for(int i=0;i<511;i++)
        {
            DPCM->DPCMimg[i][0]=img[i][0]-img[i+1][0];
        }
        DPCM->DPCMimg[512][0]=img[512][0];

        //except first row, right row minus left row
        for(int i=0;i<512;i++)
        {
            for(int j=1;j<512;j++)
            {
                DPCM->DPCMimg[i][j]=img[i][j]-img[i][j-1];
            }
        }
    }
};

int main03()
{
    tree decodeTree=tree();
    decodeTree.readEncode("Result.raw");
    //test readEncode()
    list<bool>::iterator encodeIt=decodeTree.allEncodes.begin();
    for(int i=0;i<80;i++)
    {
        cout<<*encodeIt<<" ";
        encodeIt++;
        if((i+1)%20==0&&i!=0)
        cout<<endl;
    }

    bitset<8> tempBitset(string("10100010"));//162
    int tempInt=tempBitset.to_ulong();
    cout<<"\n"<<tempInt<<endl;

    decodeTree.Decoder(decodeTree.allEncodes);
    //test decoder
    list<unsigned char>::iterator decodeIt=decodeTree.allDecodes.begin();
    for(int i=0;i<50;i++)
    {
        if(i>=40)
            cout<<(int) (*decodeIt)<<" ";
        decodeIt++;
    }
    cout<<endl;

    decodeTree.OutputToAfterDecode();
    cout<<"decodeTree.root->weight: "<<decodeTree.root->weight<<endl;
    decodeTree.readRAW("Lena.raw");
    //output original image pixel
    cout<<"Original pixel: \n";
    for(int i=0;i<50;i++)
    {
        cout<<decodeTree.img[0][i]<<" ";
        if((i+1)%10==0&&i!=0) cout<<endl;
    }
    cout<<endl;


    //test for allDecodes
    {
    int IndexCount=0;
    int ImageTemp=0;
    list<unsigned char>::iterator tempUCListIt=decodeTree.allDecodes.begin();
    for(int i=0;i<512;i++)
    {
        bool flag=false;
        for(int j=0;j<512;j++)
        {
            if(decodeTree.img[i][j]!=*tempUCListIt)
            {
                ImageTemp=decodeTree.img[i][j];
                flag=true;
                break;
            }
            tempUCListIt++;
            IndexCount++;
        }
        if(flag)
            break;
    }
    cout<<"IndexCount: "<<IndexCount<<endl;
    cout<<"ImageTemp: "<<ImageTemp<<endl;
    cout<<"*tempUCListIt: "<<(int)*tempUCListIt<<endl;
    cout<<"NYTcount: "<<NYTcount<<endl;
    }
}

float entropyCal(int symbols[], int alphabetSize=256)    //alphabet size is 256
{                                //symbol set size is 512*512
    float sum=0;
    for(int i=0;i<alphabetSize;i++)
    {
        sum+=symbols[i];
    }
    if(sum!=512*512) cout<<"entropyCal() get a symbol set which size is not 512*512\n";

    float symbolsProbability[alphabetSize];
    for(int i=0;i<alphabetSize;i++)
    {
        symbolsProbability[i]=symbols[i]/sum;
    }

    float entropyH=0;
    for(int i=0;i<alphabetSize;i++)
    {
        if(symbolsProbability[i]!=0)
            entropyH+=symbolsProbability[i] * (log10(symbolsProbability[i])/log10(2));
    }

    return entropyH*(-1);
}

int main02()      //print out the entropy result
{
    tree encodeTree=tree();
    encodeTree.readRAW("Baboon.raw");
    int symbolCount[256]={0};
    for(int i=0;i<512;i++)
        for(int j=0;j<512;j++)
            symbolCount[encodeTree.img[i][j]]++;
    float ans=entropyCal(symbolCount);
    cout<<"Original img entropy: "<<ans<<endl;
    ans=0;
    for(int i=0;i<512;i++)
        for(int j=0;j<512;j++)
            encodeTree.EncodingOneSymbol(encodeTree.img[i][j]);

    int compressedSymbolCount[2]={0};
    for(list<bool>::iterator it=encodeTree.afterEncode.begin()
        ;it!=encodeTree.afterEncode.end();it++)
    {
        if(*it==1)
            compressedSymbolCount[1]++;
        else
            compressedSymbolCount[0]++;
    }

    ans=entropyCal(compressedSymbolCount, 2);
    cout<<"After compress entropy: "<<ans<<endl;
}

//encoding needs 10s up
int main()        //This will do encode, decode and DPCM
{
    //Input file name
    string fileName;
    int Userchoose1;
    cout<<"Input file name: ";
    cin>>fileName;
    cout<<"[1] Encode+Decode+DPCM [2] Decode"<<endl;
    cin>>Userchoose1;
    if(Userchoose1==2)
    {
        tree* decodeTree=new tree();
        decodeTree->readEncode(fileName);
        decodeTree->Decoder(decodeTree->allEncodes);
        decodeTree->OutputToAfterDecode();
        return 0;
    }


    //don't need to output by byte, it don't need to be a image!!
    //https://stackoverflow.com/questions/27589460/how-to-write-single-bits-to-a-file-in-c
    //cpp can't write single bit to file
    //, so bits need to be batched until it is a byte, then can output
    ofstream ofs("Result.raw", ios::binary);
    //ofs.open("Result.raw", ofstream::out);
    int compareToFixedLenCode=0;
    int TotalBit=0;
    tree encodeTree=tree();
    encodeTree.readRAW(fileName);
    list<bool> TotalOut;    //all the bits will be output to Result.raw
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
            uint8_t tempout=0;
            int tempcount2=128;
            for(int i=0;i<8;i++)
            {
                tempout+= outBoolArray[i]*tempcount2;
                tempcount2/=2;
            }
            //ofs<<tempout;
            /**********start here*************/
            char buf[sizeof(uint8_t)];
            memcpy (buf, &tempout, sizeof(tempout));
            ofs.write(buf, sizeof(uint8_t));
            break;
        }
        outBoolArray[count8]=*it;
        count8++;
        if(count8>=8)
        {
            outputByte++;
            //start output
            uint8_t tempout=0;
            int tempcount2=128;
            for(int i=0;i<8;i++)
            {
                tempout+= outBoolArray[i]*tempcount2;
                tempcount2/=2;
            }
            //ofs<<tempout;
            char buf[sizeof(uint8_t)];
            memcpy (buf, &tempout, sizeof(tempout));
            ofs.write(buf, sizeof(uint8_t));

            count8=0;
            for(int i=0;i<8;i++) outBoolArray[i]=0;
        }
    }

    ofs.close();
    /*cout<<"compareToFixedLenCode: "<<compareToFixedLenCode<<endl;
    cout<<"NYTcount: "<<NYTcount<<endl;
    cout<<"outputByte: "<<outputByte<<endl;
    cout<<"TotalOut.size(): "<<TotalOut.size()<<endl;
    cout<<"encodeTree.afterEncode.size(): "<<encodeTree.afterEncode.size()<<endl;
    cout<<"TotalBit: "<<TotalBit<<endl;*/ //if use 8bit fixed length to store
    /*cout<<"\n OriginalImageBits: "<<endl;
    for(int i=0;i<10;i++)
    {
        cout<<encodeTree.img[0][i]<<" ";
    }
    cout<<"\n TotalOut: "<<endl;
    list<bool>::iterator tempIt=TotalOut.begin();
    for(int i=0;i<10;i++)
    {
        cout<<*tempIt<<" ";
        tempIt++;
    }*/
    cout<<"Result.raw size: "<<outputByte<<" Bytes."<<endl;


    //decode & output
    tree* decodeTree=new tree();
    decodeTree->readEncode("Result.raw");
    decodeTree->Decoder(decodeTree->allEncodes);
    decodeTree->OutputToAfterDecode();

    //compare totalout and allencodes
    /*int compareIndex=0;
    for(list<bool>::iterator it01=TotalOut.begin()
        , it02=decodeTree->allEncodes.begin();it01!=TotalOut.end()
        ;it01++, it02++)
    {
        if(*it01 != *it02)
            break;
        compareIndex++;
    }
    cout<<"compareIndex: "<<compareIndex<<endl;*/

    //start doing DPCM stuff...
    tree* DPCMTree=new tree();
    encodeTree.createDPCM();
    for(int i=0;i<512;i++)
        for(int j=0;j<512;j++)
            DPCMTree->img[i][j]=encodeTree.DPCM->DPCMimg[i][j];
    for(int i=0;i<512;i++)
        for(int j=0;j<512;j++)
            DPCMTree->EncodingOneSymbol(DPCMTree->img[i][j]);
    ofstream ofs2("ResultDPCM.raw", ios::binary);
    for(list<bool>::iterator it=DPCMTree->afterEncode.begin()
        ;;it++)
    {
        if(it==DPCMTree->afterEncode.end())
        {
            outputByte++;
            if(count8!=0)           //fill last unfilled bits with 0
            {
                for(int i=count8;i<8;i++)
                {
                    outBoolArray[i]=0;
                }

            }
            uint8_t tempout=0;
            int tempcount2=128;
            for(int i=0;i<8;i++)
            {
                tempout+= outBoolArray[i]*tempcount2;
                tempcount2/=2;
            }
            char buf[sizeof(uint8_t)];
            memcpy (buf, &tempout, sizeof(tempout));
            ofs2.write(buf, sizeof(uint8_t));
            break;
        }
        outBoolArray[count8]=*it;
        count8++;
        if(count8>=8)
        {
            outputByte++;
            //start output
            uint8_t tempout=0;
            int tempcount2=128;
            for(int i=0;i<8;i++)
            {
                tempout+= outBoolArray[i]*tempcount2;
                tempcount2/=2;
            }
            char buf[sizeof(uint8_t)];
            memcpy (buf, &tempout, sizeof(tempout));
            ofs2.write(buf, sizeof(uint8_t));
            count8=0;
            for(int i=0;i<8;i++) outBoolArray[i]=0;
        }
    }
    cout<<"DPCMTree->afterEncode: "<<DPCMTree->afterEncode.size()<<" bits"<<endl;
    ofs2.close();

}
