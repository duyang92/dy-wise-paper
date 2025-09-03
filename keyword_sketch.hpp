#include<string>
#include<iostream>
#include"murmurhash2.cpp"
#include "MurmurHash3.cpp"
#include<unordered_map>
#include<cmath>
#include <climits>
#include"vHLL.h"
using namespace std;
struct cell
{   
    uint32_t str_hash;
    string word;
    int count=0;
};

struct bucket
{
    int sum=0;
    cell *cells;
};
class CountMinSketch {
public:
    int w;
    int d;
    int **sketch;
    int seeds[4]={1234,561,76,9565};
    // This is a constructor.
    CountMinSketch(int w, int d) : w(w), d(d) {
        sketch = new int*[d];
        for (int i = 0; i < d; i++) {
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
    void insert(int id,uint32_t str_hash)
    {
        for(int i=0;i<d;i++)
        {
            unsigned int hash=0;
            MurmurHash3_x86_32(&str_hash,4,seeds[i],&hash);
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
class BloomFilter {
public:
    int w;
    int d;
    int *sketch;
    int seeds[4]={1234,561,76,9565};
    // This is a constructor.
    BloomFilter(int w, int d) : w(w), d(d) {
        sketch=new int[w];
    }
    // This is a destructor.
    ~BloomFilter() = default;
    void clear()
    {
        for(int i=0;i<w;i++)
        {
            sketch[i]=0;
        }
    }
    // This is a member function.
    void insert(int id, string word)
    {
        for (int i = 0; i < d; i++) {
            unsigned int hash = 0;
            MurmurHash3_x86_32(word.c_str(), word.size(),seeds[i],&hash);
            hash=hash%w;
            if(hash<0)
            {
                hash+=w;
            }
//             if(word=="cars") cout<<i<<" "<<hash<<endl;
            sketch[hash]=1;
        }
    }
    void insert(int id,uint32_t str_hash)
    {
        for(int i=0;i<d;i++)
        {
            unsigned int hash = 0;
            unsigned long long tmp=(uint64_t)str_hash<<32+id;
            MurmurHash3_x86_32(&tmp,8,seeds[i],&hash);
            hash=hash%w;
            sketch[hash]=1;
        }
    }
    int query(int id,uint32_t str_hash)
    {
        for(int i=0;i<d;i++)
        {
            unsigned int hash = 0;
            unsigned long long tmp=(uint64_t)str_hash<<32+id;
            MurmurHash3_x86_32(&tmp,8,seeds[i],&hash);
            hash=hash%w;
            if(sketch[hash]==0)
            {
                return 0;
            }
        }
        return 1;
    }
    int query(string word)
    {
        for (int i = 0; i < d; i++) {
            unsigned int hash = 0;
            MurmurHash3_x86_32(word.c_str(), word.size(),seeds[i],&hash);
            hash=hash%w;
            if(hash<0)
            {
                hash+=w;
            }
            if(sketch[hash]==0)
            {
                return 0;
            }
        }
        return 1;
    }
};
class Keyword_sketch { 
public:
    CountMinSketch *cms;
    BloomFilter *bf;
    vHLL *vhll;
    int size;
    bucket *buckets;
    int k;
    int tmp=0;
    // This is a constructor.
    Keyword_sketch(int w1,int d1,int w2,int d2,int size,int k) {
        cms=new CountMinSketch(w1,d1);
        bf=new BloomFilter(w2,d2);
        this->size=size;
        this->k=k;
        buckets=new bucket[size];
        for(int i=0;i<size;i++)
        {
            buckets[i].cells=new cell[k];
            for(int j=0;j<k;j++)
            {
                buckets[i].cells[j].count=0;
                buckets[i].cells[j].str_hash=-1;
                buckets[i].cells[j].word="";
            }
        }
    }
    // This is a destructor.
    ~Keyword_sketch() = default;
    // This is a member function.
    void clear()
    {
        // cout<<"clearing"<<endl;
        for(int i=0;i<size;i++)
        {
            for(int j=0;j<k;j++)
            {
                buckets[i].cells[j].str_hash=-1;
                buckets[i].cells[j].count=0;
                buckets[i].cells[j].word="";
            }
        }
        cms->clear();
        bf->clear();

    }
    void insert(int id,string word)
    {
        unsigned int str_hash=0;

        MurmurHash3_x86_32(word.c_str(), word.size(),1234,&str_hash);
//         uint32_t str_hash=MurmurHash2(word.c_str(), word.size(), 1234);
        if(bf->query(id,str_hash)==0)
        {
            bf->insert(id,str_hash);
//             if(word=="cars") cout<<bf->query(word)<<endl;
            cms->insert(id,str_hash);
        }
//         if(word=="cars")
//         {
//             tmp++;
//             cout<<id<<" "<<word<<" "<<bf->query(word)<<" "<<tmp<<endl;
//         }
        int hash_val=id;
        int bucket_idx=hash_val%size;
        buckets[bucket_idx].sum++;
        int min_val=-1;
        int min_idx=-1;
        int empty_idx=-1;
        
        for(int i=0;i<k;i++){
            if(buckets[bucket_idx].cells[i].str_hash==str_hash)
            {
                buckets[bucket_idx].cells[i].count++;
                return;
            }
            else if(buckets[bucket_idx].cells[i].str_hash==0)
            {
                empty_idx=i;
            }
            else if(min_val==-1 || buckets[bucket_idx].cells[i].count<min_val)
            {
                min_val=buckets[bucket_idx].cells[i].count;
                min_idx=i;
            }
        }
        if (empty_idx!=-1){
            buckets[bucket_idx].cells[empty_idx].str_hash=str_hash;
            buckets[bucket_idx].cells[empty_idx].count=1;
            return;
        }
        buckets[bucket_idx].cells[min_idx].str_hash=str_hash;
        buckets[bucket_idx].cells[min_idx].count+=1;
    }
    double estimate(int id1,int id2)
    {
        std::unordered_map<uint32_t, double> word_weight1, word_weight2;
        double sum_cnt1 = 0.0, sum_cnt2 = 0.0;
        double min_weight_sum = 0.0, max_weight_sum = 0.0;

        // Calculate term frequencies for doc1
        for (int i = 0; i < k; i++) {
            sum_cnt1 += buckets[id1].cells[i].count;
            word_weight1[buckets[id1].cells[i].str_hash] = buckets[id1].cells[i].count;
        }

        // Calculate term frequencies for doc2
        for (int i = 0; i < k; i++) {
            sum_cnt2 += buckets[id2].cells[i].count;
            word_weight2[buckets[id2].cells[i].str_hash] = buckets[id2].cells[i].count;
        }
        // Calculate weighted sums
        for (const auto& [word, count1] : word_weight1) {
            if(count1==0) continue;
            double idf = std::log2(static_cast<double>(size) / (cms->query(word) + 1));
//             idf=std::max(idf,(double)0);
            double tf1 = count1 /buckets[id1].sum;
            double w1 = idf * tf1;
            double tf2 = word_weight2[word] / buckets[id2].sum;
            double w2 = idf * tf2;
            min_weight_sum += std::min(w1, w2);
            max_weight_sum += std::max(w1, w2);
        }

        // Add weights for words unique to word_weight2
        for (const auto& [word, count2] : word_weight2) {
            if (word_weight1.find(word) != word_weight1.end()) continue;
            if(count2==0) continue;
            double idf = std::log2(static_cast<double>(size) / (cms->query(word) + 1));
//             idf=std::max(idf,(double)0);
            double tf1 = word_weight1[word] / buckets[id1].sum;
            double w1 = idf * tf1;
            double tf2 = count2 / buckets[id2].sum;
            double w2 = idf * tf2;
            min_weight_sum += std::min(w1, w2);
            max_weight_sum += std::max(w1, w2);
        }
        if(max_weight_sum==0) return 0;
        return min_weight_sum / max_weight_sum;
    }
};
