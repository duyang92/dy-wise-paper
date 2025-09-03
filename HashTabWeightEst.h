#ifndef HASHTABWEIGHTEST_H
#define HASHTABWEIGHTEST_H
#include<unordered_map>
#include<set>
#include<iostream>
using namespace std;
class HashTabWeightEst {
public:
    unordered_map<uint32_t,uint32_t> frequency;
    unordered_map<uint32_t,set<uint32_t>> inverse_frequency;
    set<uint32_t> allids;
    void insert(uint32_t id, uint32_t element) {
        frequency[id]++;
        frequency[element^id]++;
        inverse_frequency[element].insert(id);
        allids.insert(id);
    }
    double estimate(uint32_t id, uint32_t element)
    {
        if(frequency[element^id] == 0) return 0.0;
        double tf = (double)frequency[element^id] / frequency[id];
        double idf = log2((double)allids.size() / (inverse_frequency[element].size() + 1));
        return max((double)0,tf * idf);
    }
    uint32_t usedmem()
    {
        uint32_t mem = 0;
        mem += frequency.size() * (sizeof(uint32_t) * 2); // id and frequency
        for(const auto& pair : inverse_frequency) {
            mem += pair.second.size() * sizeof(uint32_t); // ids in the set
            mem+= sizeof(uint32_t); // element key
        }
        mem += allids.size() * sizeof(uint32_t); // ids
        return mem;
    }
    void clear()
    {
        frequency.clear();
        inverse_frequency.clear();
        allids.clear();
    }
};
#endif