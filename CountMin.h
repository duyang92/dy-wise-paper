#ifndef COUNTMIN_H
#define COUNTMIN_H
#include<string>
#include<iostream>
using namespace std;
class CountMinSketch {
public:
    int w;
    int d;
    int **sketch;
    int *seeds;
    // This is a constructor.
    CountMinSketch(int w, int d) : w(w), d(d) {
        sketch = new int*[d];
        seeds = new int[d];
        for (int i = 0; i < d; i++) {
            seeds[i] = rand() % 100000; 
            sketch[i] = new int[w];
            for (int j = 0; j < w; j++) {
                sketch[i][j] = 0;
            }
        }
        
    }
    // This is a destructor.
    ~CountMinSketch() = default;
    // This is a member function.
    void insert(int id, string word){
        for (int i = 0; i < d; i++) {
            unsigned int  hash = 0;
            MurmurHash3_x86_32(word.c_str(), word.size(),seeds[i],&hash);
            hash=hash%w;
            if(hash<0)
            {
                hash+=w;
            }
//             if(word=="cars") cout<<i<<" "<<hash<<endl;
            sketch[i][hash]++;
        }
    }
    void insert(uint32_t num)
    {
        for(int i=0;i<d;i++)
        {
            unsigned int hash=0;
            MurmurHash3_x86_32(&num,4,seeds[i],&hash);
            hash=hash%w;
            sketch[i][hash]++;
        }
    }
    int query(string word){
        int min = INT_MAX;
        for (int i = 0; i < d; i++) {
            unsigned int hash = 0;
            MurmurHash3_x86_32(word.c_str(), word.size(),seeds[i],&hash);
            hash=hash%w;
            if(hash<0)
            {
                hash+=w;
            }
            min = std::min(min, sketch[i][hash]);
        }
        return min;
    }
    int query(uint32_t str_hash)
    {
        int min=INT_MAX;
        for(int i=0;i<d;i++)
        {
            unsigned int hash = 0;
            MurmurHash3_x86_32(&str_hash,4,seeds[i],&hash);
            hash=hash%w;
            min=std::min(min,sketch[i][hash]);
        }
        return min;
    }
    int usedmem()
    {
        return w*d*sizeof(uint32_t);
    }
    void clear()
    {
        for(int i=0;i<d;i++)
        {
            for(int j=0;j<w;j++)
            {
                sketch[i][j]=0;
            }
        }
    }
};
#endif 