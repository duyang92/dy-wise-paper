#include<string>
#include<iostream>
#include"murmurhash2.cpp"
#include "MurmurHash3.cpp"
#include<unordered_map>
#include<cmath>
#include <climits>
#include"vHLL.h"
#include<vector>
#include<algorithm>
#include"CountMin.h"
#include"HashTabWeightEst.h"
#include"SketchWeightEst.h"
#ifdef SWE
typedef SketchWeightEst WE;
#elif defined(HTWE)
typedef HashTabWeightEst WE;
#endif
using namespace std;
struct bucket
{
    uint32_t id;
    uint32_t element;
    #ifdef LAZY
    double weight;
    #endif
};
class Dss2 {
    
    bucket* buckets;
    WE *we;
    int size;
    vector<uint32_t> seeds;
    int k;
public:
    #ifdef SWE
    Dss2(int m ,int v, int size,int k,int w,int d)
    {
        we= new SketchWeightEst(m,v,w,d);
        buckets = new bucket[size];
        srand(NULL);
        for(int i=0;i<k;i++)
        {
            uint32_t seed = rand();
            seeds.push_back(seed);
        }
        this->size=size;
        this->k=k;
        for(int i=0;i<size;i++)
        {
            buckets[i].id = 0;
            buckets[i].element = 0;
            buckets[i].weight=0;
        }
        cout<<"DSS initialized with m="<<m<<" v="<<v<<" size="<<size<<" k="<<k<<" fixed="<<fixed<<endl;
    }
    #elif defined(HTWE)
    Dss2(int size, int k)
    {
        we = new HashTabWeightEst();
        buckets = new bucket[size];
        srand(NULL);
        for(int i=0;i<k;i++)
        {
            uint32_t seed = rand();
            seeds.push_back(seed);
        }
        this->size=size;
        this->k=k;
        for(int i=0;i<size;i++)
        {
            buckets[i].id = 0;
            buckets[i].element = 0;
        }
        cout<<"DSS initialized with size="<<size<<" k="<<k<<" fixed="<<fixed<<endl;
    }
    #endif
    void clear()
    {
        we->clear();
        for(int i=0;i<k;i++)
        {
            buckets[i].id = 0;
            buckets[i].element = 0;
            #ifdef LAZY
            buckets[i].weight=0;
            #endif
        }
    }
    double weight_est(uint32_t bucket_idx) {
        return we->estimate(buckets[bucket_idx].id, buckets[bucket_idx].element);
    }
    double weight_est(int id, uint32_t element) {
        return we->estimate(id, element);
    }
    int usedmem()
    {
        return size*(sizeof(bucket)) + we->usedmem();
    }
    void insert(int id, string element) {
        uint32_t hash_val=0;
        MurmurHash3_x86_32(element.c_str(), element.size(), 1234, &hash_val);
        we->insert(id, hash_val);
        uint32_t bucket_idx =0;
        int empty_idx=-1;
        int done=0;
        int min_weight_idx=-1;
        double est_w=weight_est(id, hash_val);
        double min_weight=0;
        int sum=0;
        for(int i=0;i<k;i++)
        {
            MurmurHash3_x86_32(&id, sizeof(id), seeds[i], &bucket_idx);
            bucket_idx=bucket_idx%size;
            if(buckets[bucket_idx].id==0)
            {
                empty_idx=bucket_idx;
                continue;
            }
            if(buckets[bucket_idx].id==id)
            {
                if(buckets[bucket_idx].element==hash_val)
                {
                    done=1;
                    break;
                }
                else
                {
                    #ifdef LAZY
                    double weight=buckets[bucket_idx].weight;
                    #else
                    double weight= weight_est(bucket_idx);
                    #endif
                    if(min_weight_idx==-1 || weight < min_weight)
                    {
                        min_weight_idx=bucket_idx;
                        min_weight=weight;
                    }
                    
                }
            }
            else
            {
                #ifdef LAZY
                double weight=buckets[bucket_idx].weight;
                #else
                double weight= weight_est(bucket_idx);
                #endif
                if(min_weight_idx==-1 || weight < min_weight)
                {
                    min_weight_idx=bucket_idx;
                    min_weight=weight;
                }
                
            }
        }
        // cout<<empty_idx<<" "<<min_weight_idx<<" "<<done<<endl;
        if(!done)
        {
            if(empty_idx!=-1)
            {
                buckets[empty_idx].id=id;
                buckets[empty_idx].element=hash_val;
                #ifdef LAZY
                buckets[empty_idx].weight=est_w;
                #endif
            }
            else if(min_weight_idx!=-1&&min_weight<est_w)
            {
                buckets[min_weight_idx].id=id;
                buckets[min_weight_idx].element=hash_val;
                #ifdef LAZY
                buckets[empty_idx].weight=est_w;
                #endif
            }
        }
    }
    double estimate(int id1, int id2)
    {
        vector<uint32_t> eles1;
        vector<double> weights1;
        vector<uint32_t> eles2;
        vector<double> weights2;
        double min_weight_sum=0;
        double max_weight_sum=0;
        for(int i=0;i<k;i++)
        {
            uint32_t bucket_idx=0;
            MurmurHash3_x86_32(&id1, sizeof(id1), seeds[i], &bucket_idx);
            bucket_idx=bucket_idx%size;
            if(buckets[bucket_idx].id==id1)
            {
                eles1.push_back(buckets[bucket_idx].element);
                double weight1 = weight_est(bucket_idx);
                weights1.push_back(weight1);
            }
            MurmurHash3_x86_32(&id2, sizeof(id2), seeds[i], &bucket_idx);
            bucket_idx=bucket_idx%size;
            if(buckets[bucket_idx].id==id2)
            {
                eles2.push_back(buckets[bucket_idx].element);
                double weight2 = weight_est(bucket_idx);
                weights2.push_back(weight2);
            }
        }
        for(int i=0;i<eles1.size();i++)
        {
            uint32_t ele1 = eles1[i];
            double weight1 = weights1[i];
            double weight2=0;
            if(find(eles2.begin(), eles2.end(), ele1) != eles2.end())
            {
                int idx = find(eles2.begin(), eles2.end(), ele1) - eles2.begin();
                weight2 = weights2[idx];
            }
            min_weight_sum += min(weight1, weight2);
            max_weight_sum += max(weight1, weight2);
        }
        for(int i=0;i<eles2.size();i++)
        {
            uint32_t ele2 = eles2[i];
            if(find(eles1.begin(), eles1.end(), ele2) == eles1.end())
            {
                double weight2 = weights2[i];
                max_weight_sum += weight2;
            }
        }
        if(max_weight_sum == 0) return 0.0;
        return min_weight_sum / max_weight_sum;
    }
};