
// BUILD COMMAND: g++ -g -std=c++11 infonTest.cpp -o it && ./it

// Current goal:
//   0.  Make sure validate() really works (see comments below)
//   1.  Write innerAdd(), outerAdd() and innerAddTest() and outerAddTest()
//   2. Use innerAdd and outerAdd to finish Join(A,B)
//   -> The main program calls joinTest() which will exercise the Join(A,B) operation.

#include <stdio.h>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <list>

using namespace std;


#include <cstring>
#include <memory>
#include <cstdarg>
std::string sFmt(const std::string fmt_str, ...) {
    int final_n, n = fmt_str.size() * 2; /* reserve 2 times as much as the length of the fmt_str */
    std::string str;
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while(1) {
        formatted.reset(new char[n]); /* wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}


struct inf;      // Forward
struct NQ_pair;  // Forward
void harvestZeros(inf* item, inf* ZEROS);   // Forward
int SingletsPos(inf* parent, inf* singlet); // Forward

struct coset {
    int mod, val, idx;
    coset(int Mod, int Val, int Idx):mod(Mod), val(Val), idx(Idx){};
};

struct inf{
    int mod;
    vector<inf*> singlets;
    inf* ZERO;

    inf(int size=1){mod=size; ZERO=0;};

    void FromInt(int size, int value, inf* Z);
    int ToInt();
    string ToIStr();
    bool isEqual(inf* A, inf* B);
    void asZero(int size);
    int size(){return singlets.size();};

    inf waveForm();
    inf convoluteForm();

    void dump(NQ_pair* NQ=0);  // Print this out. If NQ is passed in it colors the norm and quotient sub-groups.

    bool validate(){  // Verify that this infon is in a valid form. (Does this work? Or does some of the code below produce invalid results?)
        int size=singlets.size();
        if(size != ZERO->singlets.size()){printf("Size Error.\n"); exit(2);}

        int pos=0, coset=0, count=0, value=ToInt();

        for(int v=0; v < size; ++v){
            if(pos!=SingletsPos(ZERO, singlets[v])) {printf("Malformed inf at %i:",value); dump(); printf("\n"); exit(2);}
            //-----
            pos+=value; pos %= size;
            if(pos==coset) {
                coset++; pos=coset;
                count++;
            }
        }
    //    if(mod!= size / count){printf("   \tSize Mismatch."); }//exit(2);}
        return true;
    };

    shared_ptr< list<coset> > fetchCosets(){
        shared_ptr< list<coset> > cosets = make_shared< list<coset> >( );
        int size=singlets.size();
        int val= ToInt();
        for(int cosIdx=0; cosIdx < size/val; ++cosIdx){
            coset c(size, val, cosIdx);
            cosets->push_back(c);
        //    cout <<
        }
        return cosets;
    }

    inf(int interval, inf* parent){  // constructor for normal sub-infons.
        int pos=0; int size=parent->singlets.size();
        ZERO=new inf;
        do{
            singlets.push_back(parent->singlets[pos]);
            pos+=interval; pos %= size;
        }while(pos!=0);
        mod=singlets.size();
        harvestZeros(this, parent->ZERO);
        validate();
    };

    inf(inf* parent, int interval){  // constructor for quotient sub-infons.
        int pos=0; int size=parent->singlets.size();
        ZERO=new inf;
        do{
            singlets.push_back(parent->singlets[pos]);
            pos+=1;
        }while(pos!=interval);
        mod=interval;
        harvestZeros(this, parent->ZERO);
        validate();
    };
};

struct NQ_pair{   // Stores a normal-sub-group-infon and it's associated quotient-sub-group-infon
    inf* normal_subgroup;   // Normal subgroup in parent
    inf* quotient_subgroup; // <normal_subgroup, quotient_subgroup> should = parent.  Where <A,B> means group inner product.
    inf* parent;

    NQ_pair(inf* Parent, inf* N, inf* Q):normal_subgroup(N), quotient_subgroup(Q), parent(Parent){}
};

int SingletsPos(inf* parent, inf* singlet){ // How many positions to the right is this singlet in parent?
    int size=parent->singlets.size();
    for(int Z=0; Z<size; ++Z){
        if(singlet == parent->singlets[Z]) {
            return Z;
        }
    }
    printf("\nSingletsPos(): Couldn't find singlet in infon. size=%d\n\n",size);
    exit(2);
    return -1;
}

int ZerosPosInSinglets(inf* parent, inf* Zsinglet){ // How many positions to the right is this zero singlet in parent?
    int size=parent->singlets.size();
    for(int Z=0; Z<size; ++Z){
       // printf("Zsinglet: %p, Zero[Z]: %p\n", Zsinglet, parent->ZERO->singlets[Z]);
        if(Zsinglet == parent->ZERO->singlets[Z]) {
            return Z;
        }
    }
    printf("\nZerosPosInSinglets(): Couldn't find singlet in Infon. size=%d\n\n",size);
    exit(2);
    return -1;
}

void inf::dump(NQ_pair* NQ){   // Print this infon out. If NQ is passed in it colors the norm and quotient sub-groups.
    printf("(%i) \t", mod);
    for(int i=0; i<singlets.size(); ++i){
        if(NQ){
            bool inN=(find(NQ->normal_subgroup->singlets.begin(), NQ->normal_subgroup->singlets.end(), singlets[i])!=NQ->normal_subgroup->singlets.end());
            bool inQ=(find(NQ->quotient_subgroup->singlets.begin(), NQ->quotient_subgroup->singlets.end(), singlets[i])!=NQ->quotient_subgroup->singlets.end());
            if(inN && inQ){ printf("\x1b[33m");}
            else if(inN){printf("\x1b[32m");}
            else if(inQ){printf("\x1b[31m");}
            else{printf("\x1b[0m");}
        }
        printf("%i \t",SingletsPos(ZERO, singlets[i]));

    }

    if(NQ){
        printf("\x1b[m  ");  // color=normal
        string norm=NQ->normal_subgroup->ToIStr();
        string quot=NQ->quotient_subgroup->ToIStr();
        printf("  \t(%s \tx \t%s) \t= %s",norm.data(), quot.data(), ToIStr().data());
    }
    printf("\n");
};


typedef vector<NQ_pair> divisionList;

void harvestZeros(inf* item, inf* ZEROS){
    int size=ZEROS->singlets.size();
    for(int Z=0; Z<size; ++Z){
        if(SingletsPos(item, ZEROS->singlets[Z]) != -1){
            item->ZERO->singlets.push_back(ZEROS->singlets[Z]);
        }
    }
    if(item->singlets.size() != item->ZERO->singlets.size()){
        printf("Size Mismatch while harvesting zero singlets\n\n");
    }
}

void doDivision(inf* parent, int factor, inf* N, inf* Q){
    int size=parent->singlets.size();
    int mod=0;
    int S=0;
    int coset=0;
    bool pushingToN=true;
    for(int value=0; value < size; ++value){
        if(pushingToN){N->singlets.push_back(parent->singlets[S]);}// printf("<%i>, ", S);}// N->ZERO->singlets.push_back(parent->singlets[S]);}
        /////
        {S+=(factor); S %= size;}
        if(S==coset) {// printf("+\n");
                Q->singlets.push_back(parent->singlets[coset]); // Q->ZERO->singlets.push_back(parent->singlets[coset]); // Push to Q
             //   printf("[%i]", SingletsPos(parent, parent->singlets[coset]));
            coset++; S=coset;
            if(mod==0) {mod=N->singlets.size();}
            /////
            pushingToN=false; // Stop pushing to N

        }
    }
    N->mod=mod;
    Q->mod=size / mod;
    harvestZeros(N, parent->ZERO);
    harvestZeros(Q, parent->ZERO);
    N->validate();
    Q->validate();
}

void populateDivisionList(inf* parent, divisionList* divisions){
    int size=parent->singlets.size();
    for(int factor=0; factor < size; ++factor){ printf("\n");
        inf* N=new inf; N->ZERO=new inf; inf* Q=new inf; Q->ZERO=new inf;
        doDivision(parent, factor, N, Q);
//        printf("\nNORM: \n"); N->dump();
//        printf("\nQUOT: \n"); Q->dump();
//        printf("\n\n");

        divisions->push_back(NQ_pair(parent, N, Q));
    }
}


void inf::FromInt(int size, int value, inf* Z=0){ // initialize an inf with a size and value.
    mod=0;
    singlets.clear();
    if(Z) {ZERO=Z;}
    else {ZERO=new inf(size); ZERO->asZero(size);}
    int S=0;
    int coset=0;
    for(int i=0; i<size; ++i){
        // printf("%i,",S);
        singlets.push_back(ZERO->singlets[S]);
        {S+=(value); S %= size;}
        if(S==coset) {
            coset++; S=coset;
            if(mod==0) {mod=singlets.size();}
        }
    }
}

int inf::ToInt(){   // Returns this infon's value. It's size is this->singlets.size().
    if(singlets.size()==0) return -1;
    if(ZERO->singlets.size()==1){
        if(singlets[0]==ZERO->singlets[0]) return 0;
        else {
            printf("Anomoly: singlets[0]=%p    ZERO->singlets[0]=%p\n", singlets[0], ZERO->singlets[0]);
            return -2;
        }
    } else return SingletsPos(ZERO, singlets[1]);
}

string inf::ToIStr(){  // return a string like "*8 +3 "
    return sFmt("*%i +%i ", singlets.size(), ToInt());
}

bool inf::isEqual(inf* A, inf* B){  // return true if A and B have matching size and ToInt() (not very useful since only identity matters)
    if(A->singlets.size() != B->singlets.size()) return false;
    if(A->ToInt() != B->ToInt()) return false;
    return true;
}

void inf::asZero(int size){
    mod=1; ZERO=0;
    for(int i=0; i<size; ++i){
        singlets.push_back(new inf);
    }
}

// These can be done later. (After I figure out how)
inf inf::waveForm(){}           // Fourier Transform
inf inf::convoluteForm(){}      // Convolution


void divisionTest(){
    int size=12;
    for(size=1; size<=12; ++size){
        printf("\n--------------------------------------------------------- Infon of size %i.\n",size);
        divisionList DivList; // =new divisionList;
        inf Z(size); Z.asZero(size);
       // populateDivisionList(&Z, &DivList); printf("\n");

        for(int i=0; i<size; ++i){
            inf A; A.FromInt(size, i, &Z);
            A.dump();

            for(int subG=1; subG<=size; ++subG){
                inf* N=new inf; N->ZERO=new inf; inf* Q=new inf; Q->ZERO=new inf;
                doDivision(&A, subG, N, Q);
                NQ_pair NQ(&A, N, Q);
                A.dump(&NQ);
            //    printf("\t\t%s %s = %s\n", N.ToIStr().data(), Q.ToIStr().data(), A.ToIStr().data());
            } printf("\n");
        }

    }
}

inf innerAdd(inf* i, int x){  // This does endomorphism add. Result's size is the same as i's size.
    int iSize=i->singlets.size();
    inf ret;
    ret.FromInt(iSize, (i->ToInt()+x) % iSize, i->ZERO);
    return ret;
}

inf* innerAdd_inf(inf* i, inf* j){
    int size=i->size();
    if(i->ZERO != j->ZERO) {printf("ZERO's don't match in inf+inf\n\n"); exit(2);}
    if(i->size() != j->size()){printf("Sizes don't match in inf+inf\n\n"); exit(2);}
    inf* ret=new inf(size);
    ret->ZERO=i->ZERO;
    //for(auto &I_atom: i->singlets){
    printf("\n");
    for(int I_atom=0; I_atom<size; ++I_atom){
        printf("%*i+%*i=%*i", 3, ZerosPosInSinglets(i, i->singlets[I_atom]), 2, ZerosPosInSinglets(j, j->singlets[I_atom]), 2, (ZerosPosInSinglets(i, i->singlets[I_atom]) + ZerosPosInSinglets(j, j->singlets[I_atom])) % size);
        ret->singlets.push_back(i->ZERO->singlets[(ZerosPosInSinglets(i, i->singlets[I_atom]) + ZerosPosInSinglets(j, j->singlets[I_atom])) % size]);

    }
    ret->mod=size;
    //harvestZeros(ret, i->ZERO);
    //ret->validate();
    return ret;
}

inf outerAdd1(inf* i, bool carry){   // Pulls in extra states. So the result size is greater than i size. It should be i's size + 1
    int iSize=i->singlets.size();
    inf ret;
    if(iSize==0){ret.FromInt(1,0);}
    else {ret.FromInt(iSize+1, i->ToInt()+((carry)?1:0));}
    return ret;
}

inf outerAddMulti(inf* i, int m){  // Accumulates and returns m copies of i.
    int iSize=i->singlets.size();
    int mx=m;
    int iVal=i->ToInt();
    inf acc;
    if(mx==0) {acc.FromInt(1,0); return acc;}
    else {acc.FromInt(0,0);}
    for(int M=0; M<mx; ++M){
        for(int ix=iSize-1; ix>=0; --ix){
            if(ix<iVal){acc=outerAdd1(&acc, 1);}
            else{acc=outerAdd1(&acc, 0);}
        }
    }
    return acc;
}

inf Join(inf* A, inf* B){   // Join means combining two infons like *10+5 *16+7 = *160 + 87.
    inf result;
    // using outerAdds, join A to result, B.size times.
    int BSize=B->singlets.size();
    result=outerAddMulti(A, BSize);  // If A is *10+5 and B is *16+7, then after this loop, result should be *160+80.
    result=innerAdd(&result, B->ToInt());  // Add the 7 to result.
    return result;  // Final result should be *160 + 87
}

void innerAddTest(int maxMod=12){  //Print combinations of innerAdds to verify that it works and see how it works.
    for(int S1=1; S1<=maxMod; ++S1){
        inf Z1(S1); Z1.asZero(S1);
        inf I1; I1.FromInt(S1, 0, &Z1);
        printf("Test InnerAdd1 for %s\n", I1.ToIStr().data());
        for(int V1=0; V1<=S1*2; ++V1){
            I1=innerAdd(&I1, 1);
            printf("     %s\n", I1.ToIStr().data());
        }
    }
}


void outerAddTest(int maxMod=12){  // Print combinations of outerAdds to verify that it works and see how it works.
    for(int S1=1; S1<=maxMod; ++S1){
        inf Z1(S1); Z1.asZero(S1);
        inf I1; I1.FromInt(S1, 0, &Z1);
        printf("Test outterAdd1 for %s\n", I1.ToIStr().data());
        for(int V1=0; V1<=S1*2; ++V1){
            I1=outerAdd1(&I1, 1);
            printf("     %s\n", I1.ToIStr().data());
            I1.validate();
        }
    }
}


void outerAddMultiTest(int maxMod=12){  // Print combinations of outerAddMulti to verify that it works and see how it works.
    for(int S1=1; S1<=maxMod; ++S1){
        for(int V1=0; V1<S1; ++V1){
            inf Z1(S1); Z1.asZero(S1);
            inf I1; I1.FromInt(S1, V1, &Z1);
            for(int S2=1; S2<=maxMod; ++S2){
                printf("Test outerAddMulti for %s\n", I1.ToIStr().data());
                for(int V2=0; V2<=S2; ++V2){
                    inf result=outerAddMulti(&I1, V2);
                    printf("\t\t%s * %i = %s\n", I1.ToIStr().data(), V2, result.ToIStr().data());
                    result.validate();
                }
            }
        }
    }
}

map<int, string> joinTable;

void JoinTest(int maxMod=12){      // Test all combinations of *S1 + V1  * S2 + V2
    int count=0;
    for(int S1=0; S1<=maxMod; ++S1){ if(S1==0){printf("S1==0:\n");}
        for(int V1=0; V1<=S1; ++V1){
            inf Z1(S1); Z1.asZero(S1);
            inf I1; I1.FromInt(S1, V1, &Z1);
            for(int S2=0; S2<=maxMod; ++S2){
                for(int V2=0; V2<S2; ++V2){
                    inf Z2(S2); Z2.asZero(S2);
                    inf I2; I2.FromInt(S2, V2, &Z2);
                    inf result=Join(&I1, &I2);
                    printf("\t\t%s %s = %s\n", I1.ToIStr().data(), I2.ToIStr().data(), result.ToIStr().data());
                    result.validate();
                    int index=result.singlets.size()*maxMod*maxMod+result.ToInt();
                    auto itr=joinTable.find(index);
                    if(itr==joinTable.end()) {joinTable[index]=result.ToIStr()+";"; ++count;}
                    joinTable[index] += sFmt(" \t[%s %s]", I1.ToIStr().data(), I2.ToIStr().data());
                }
            }
        }
    }

    int lineCnt=0;
    for(auto itr:joinTable){
        printf("%i: %s\n", lineCnt++, itr.second.data());
    }
    printf("Total Count:%i\n", count);
}


void infPlusInfTest(int maxMod=12){      // Test all combinations of *S1 + V1  * S2 + V2
    int count=0;
    for(int S1=1; S1<=maxMod; ++S1){ if(S1==1){printf("S1==1:\n");}
        for(int V1=0; V1<S1; ++V1){
            inf Z1(S1); Z1.asZero(S1);
            inf I1; I1.FromInt(S1, V1, &Z1);
            for(int V2=0; V2<S1; ++V2){
                //inf Z2(S2); Z2.asZero(S2);
                inf I2; I2.FromInt(S1, V2, &Z1);
                //inf result=Join(&I1, &I2);
                inf* result=innerAdd_inf(&I1, &I2);
                printf("\t\t%s %s = %s\n", I1.ToIStr().data(), I2.ToIStr().data(), result->ToIStr().data());
                result->validate();
                int index=result->singlets.size()*maxMod*maxMod+result->ToInt();
                auto itr=joinTable.find(index);
                if(itr==joinTable.end()) {joinTable[index]=result->ToIStr()+";"; ++count;}
                joinTable[index] += sFmt(" \t[%s %s]", I1.ToIStr().data(), I2.ToIStr().data());
            }

        }
    }

    int lineCnt=0;
    for(auto itr:joinTable){
        printf("%i: %s\n", lineCnt++, itr.second.data());
    }
    printf("Total Count:%i\n", count);
}

void drawTables(int maxMod=12){      // Test all combinations of *S1 + V1  * S2 + V2
    int colWidth=10;
    string colSep=""; for(int x=0; x<colWidth; x++){colSep+="-";}
    int count=0;
    for(int S1=1; S1<=maxMod; ++S1){ if(S1==1){printf("S1==1:\n");}
        printf("\n\n###############################  Table for mod %i  ###############################\n", S1);
        printf("          |");
        string seperatorStr="----------|";
        for(int Col=0; Col<S1; ++Col){
            printf("%*d", colWidth, Col);
            seperatorStr+=colSep;
        }
        printf("\n%s\n", seperatorStr.data());
        for(int V1=0; V1<S1; ++V1){
            inf Z1(S1); Z1.asZero(S1);
            inf I1; I1.FromInt(S1, V1, &Z1);
            I1.validate();
            printf("%*s| ", colWidth, I1.ToIStr().data());
            for(int V2=0; V2<S1; ++V2){
                inf* result=0;
                inf I2; I2.FromInt(S1, V2, &Z1);
                I2.validate();
                string resultStr="";
                //////////////////////////////////////////////
                result=innerAdd_inf(&I1, &I2);  resultStr=result->ToIStr();
                //inf resultVar=innerAdd(&I1, I2.ToInt());  result=&resultVar;  resultStr=result->ToIStr();

                //////////////////////////////////////////////
////                printf("%*s", 10, resultStr.data());
     //           result->validate();
        /*        int index=result->singlets.size()*maxMod*maxMod+result->ToInt();
                auto itr=joinTable.find(index);
                if(itr==joinTable.end()) {joinTable[index]=result->ToIStr()+";"; ++count;}
                joinTable[index] += sFmt(" \t[%s %s]", I1.ToIStr().data(), I2.ToIStr().data());  */
            }
            printf("\n");
        }
    }
}

void printEndomorphismsFromGenerator(int maxMod=12){
    for(int S1=1; S1<=maxMod; ++S1){
        printf("\n--------------------------------------------------------- Infon of size %i.\n",S1);
        for(int V1=1; V1<=S1; ++V1){
            int N=V1;
            for(int x=0; x<S1; ++x){
                printf("\t%i", N);
                N+=V1;
                N %= S1;

            }
            printf("\n");
        }
    }

}

void printEndomorphismsFromZero(int maxMod=12){
    for(int S1=1; S1<=maxMod; ++S1){
        printf("\n--------------------------------------------------------- Infon of size %i.\n",S1);
        for(int V1=0; V1<S1; ++V1){
            inf A;
            A.FromInt(S1, V1);
            A.dump();
        }
    }

}

int main(int argc, char **argv){
    printEndomorphismsFromZero();
    printEndomorphismsFromGenerator();
 // divisionTest();  // Uncomment this to print endomorphisms and highlight some sub-groupings..
//  innerAddTest();
 // outerAddTest();
 //   outerAddMultiTest();
 //   infPlusInfTest(12);  // Test Inf + Inf
 //   JoinTest(12);      // These are the tests I am currently working on.
 //drawTables(12);
}

/*
void fillAutomorphs(grp* G){
    for(int i=0; i<G->order; ++i){
        int S=i;
        int coset=0;
        for(int j=0; j<G->order; ++j){
            // printf("%i,",S);
            G->stateMap[i].rowConfig[j]=S;
            if(S==coset) {coset++; S=i+coset;}
            else{S+=(i); S %= G->order;}
        }
    }
}

void fillEndomorphism(grp* G){
    for(int k=1; k<=G->order; ++k){
        unsigned long long int val=k;
        for(int n=1; n<=G->order; ++n){
            val= (n-1)*k; //pow(n-1,k);
            G->stateMap[k-1].rowConfig[n-1]=val % G->order;
            val=(val+val);
        }
    }
}

*/
