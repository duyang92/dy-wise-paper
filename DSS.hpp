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
using namespace std;
struct bucket
{
    uint32_t id;
    uint32_t element;
    int sum;
    int count;
    int lock=0;
};
class Dss {
    vHLL* vhll;
    bucket* buckets;
    int fixed;
    int size;
    vector<uint32_t> seeds;
    set<int> allids;
    int k;
public:
    Dss(int m ,int v, int size,int k,int fixed)
    {
        vhll = new vHLL(m, v);
        buckets = new bucket[size];
        srand(NULL);
        for(int i=0;i<k;i++)
        {
            uint32_t seed = rand();
            seeds.push_back(seed);
        }
        this->size=size;
        this->k=k;
        this->fixed=fixed;
        for(int i=0;i<size;i++)
        {
            buckets[i].id = 0;
            buckets[i].element = 0;
            buckets[i].sum = 0;
            buckets[i].count = 0;
        }
        cout<<"DSS initialized with m="<<m<<" v="<<v<<" size="<<size<<" k="<<k<<" fixed="<<fixed<<endl;
    }
    void clear()
    {
        vhll->clear();
        for(int i=0;i<k;i++)
        {
            buckets[i].id = 0;
            buckets[i].element = 0;
            buckets[i].sum = 0;
            buckets[i].count = 0;
        }
        allids.clear();
    }
    double weight_est(uint32_t bucket_idx) {
        double tf = (double)buckets[bucket_idx].count / buckets[bucket_idx].sum;
        double idf = std::log2(static_cast<double>(allids.size()) / (vhll->estimate(buckets[bucket_idx].element,buckets[bucket_idx].id) + 1));
        return tf * idf;
    }
    void insert(int id, string element) {
        allids.insert(id);
        uint32_t hash_val=0;
        MurmurHash3_x86_32(element.c_str(), element.size(), 1234, &hash_val);
        vhll->update(hash_val, id,0);
        uint32_t bucket_idx =0;
        int empty_idx=-1;
        int done=0;
        int min_weight_idx=-1;
        double min_weight=0;
        int sum=0;
        cout<<k<<endl;
        for(int i=0;i<k;i++)
        {
            cout<<i<<endl;
            MurmurHash3_x86_32(&id, sizeof(id), seeds[i], &bucket_idx);
            bucket_idx=bucket_idx%size;
            if(buckets[bucket_idx].id==0)
            {
                empty_idx=bucket_idx;
                continue;
            }
            if(buckets[bucket_idx].id==id)
            {
                sum=buckets[bucket_idx].sum;
                buckets[bucket_idx].sum++;
                if(buckets[bucket_idx].element==hash_val)
                {
                    buckets[bucket_idx].count++;
                    done=1;
                }
                else
                {
                    double weight= weight_est(bucket_idx);
                    if(min_weight_idx==-1 || weight < min_weight)
                    {
                        min_weight_idx=bucket_idx;
                        min_weight=weight;
                    }
                }
            }
            else if(buckets[bucket_idx].lock==0)
            {
                double weight= weight_est(bucket_idx);
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
                buckets[empty_idx].count=1;
                buckets[empty_idx].sum=sum+1;
            }
            else
            {
                buckets[min_weight_idx].id=id;
                buckets[min_weight_idx].element=hash_val;
                buckets[min_weight_idx].count=1;
                buckets[min_weight_idx].sum=sum+1;
            }
        }
        cout<<2<<endl;
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