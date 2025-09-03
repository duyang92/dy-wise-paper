#include<iostream>
#include"datareader.cpp"
#include<vector>

using namespace std;
#ifdef KEYWORD_SKETCH
#include"keyword_sketch.hpp"
typedef Keyword_sketch Sketch_t;
#endif
#ifdef DSS
#include"DSS.hpp"
typedef Dss Sketch_t;
#endif
#ifdef ICWS_SKETCH
#include"icws.hpp"
typedef ICWS_t Sketch_t;
#endif
#ifdef FAST_ICWS_SKETCH
#include"icws.hpp"
typedef FastICWS_t Sketch_t;
#endif
#ifdef BAGMINHASH_SKETCH
#include"bagminhash_wrappers.hpp"
typedef BagMinHash1 Sketch_t;
#endif
#ifdef DARTMINHASH_SKETCH
#include"dartminhash.hpp"
typedef DartMinHash Sketch_t;
#endif
#ifdef DSS2
#include"DSS2.hpp"
typedef Dss2 Sketch_t;
#endif
int main(int argc,char **argv)
{
    #ifdef ICWS_SKETCH 
    if(argc!=3) {
        cout<<"Usage: "<<argv[0]<<"t input_size"<<endl;
        return 1;
    }
    mt19937_64 rng(1);
    int t=stoi(argv[1]);
    int input_size=stoi(argv[2]);
    Sketch_t* sketch= new ICWS_t(rng,t);
    #endif
    #ifdef FAST_ICWS_SKETCH
    if(argc!=3) {
        cout<<"Usage: "<<argv[0]<<"t input_size"<<endl;
        return 1;
    }
    mt19937_64 rng(1);
    int t=stoi(argv[1]);
    int input_size=stoi(argv[2]);
    Sketch_t* sketch= new FastICWS_t(rng,t);
    #endif
    #ifdef BAGMINHASH_SKETCH
    if(argc!=3) {
        cout<<"Usage: "<<argv[0]<<"t input_size"<<endl;
        return 1;
    }
    int t=stoi(argv[1]);
    int input_size=stoi(argv[2]);
    Sketch_t* sketch= new BagMinHash1(t);
    #endif
    #ifdef DARTMINHASH_SKETCH
    #ifdef HTWE
    if(argc!=3) {
        cout<<"Usage: "<<argv[0]<<"t input_size"<<endl;
        return 1;
    }
    mt19937_64 rng(1);
    int k=stoi(argv[1]);
    int input_size=stoi(argv[2]);
    Sketch_t* sketch= new DartMinHash(rng,k);
    #endif
    #ifdef SWE
    if(argc!=7) {
        cout<<"Usage: "<<argv[0]<<"t input_size m v w d"<<endl;
        return 1;
    }
    mt19937_64 rng(1);
    int k=stoi(argv[1]);
    int input_size=stoi(argv[2]);
    int m=stoi(argv[3]);
    int v=stoi(argv[4]);
    int w=stoi(argv[5]);
    int d=stoi(argv[6]);
    Sketch_t* sketch= new DartMinHash(rng,k,m,v,w,d);
    #endif
    #endif
    #ifdef DSS2
    #ifdef SWE
    if(argc!=8) {
        cout<<"Usage: "<<argv[0]<<" m v size k w d input_size"<<endl;
        return 1;
    }
    int m=stoi(argv[1]);
    int v=stoi(argv[2]);
    int size=stoi(argv[3]);
    int k=stoi(argv[4]);
    int w=stoi(argv[5]);
    int d=stoi(argv[6]);
    int input_size=stoi(argv[7]);
    Sketch_t* sketch= new Dss2(m,v,size,k,w,d);
    #endif
    #ifdef HTWE
    if(argc!=4) {
        cout<<"Usage: "<<argv[0]<<" size k input_size"<<endl;
        return 1;
    }
    int size=stoi(argv[1]);
    int k=stoi(argv[2]);
    int input_size=stoi(argv[3]);
    Sketch_t* sketch= new Dss2(size,k);
    #endif
    #endif
    DataReader<Sketch_t> dr(sketch,input_size);
//     dr.load_actual_sim("ip");
//     dr.test_ip(60);
//     dr.clear();
    cout<<"Testing text dataset"<<endl;
    dr.load_actual_sim("text");
    
    dr.test_text_or_recommendation("text");
    dr.clear();
    cout<<"Testing recommendation dataset"<<endl;
    dr.load_actual_sim("rec");
    dr.test_text_or_recommendation("rec");
    dr.clear();
    cout<<"testing quora"<<endl;
    dr.test_quora(0.5);
    dr.test_CORE(0.5);
    dr.test_restaurant(0.5);
    return 0;
}