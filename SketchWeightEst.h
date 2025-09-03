#ifndef SKETCHWEIGHTEST_H
#define SKETCHWEIGHTEST_H
#include<unordered_map>
#include<set>
#include<iostream>
#include"vHLL.h"
#include<vector>
#include<algorithm>
#include"CountMin.h"
using namespace std;
class SketchWeightEst {
public:
    vHLL* vhll;
    CountMinSketch* cms;
    SketchWeightEst(int m,int v,int w,int d)
    {
        vhll = new vHLL(m, v);
        cms = new CountMinSketch(w, d);
    }
    void insert(uint32_t id, uint32_t element) {
        cms->insert(id^element);
        cms->insert(id);
        vhll->update(element, id,0);
        vhll->update(0,id,0);
    }
    double estimate(uint32_t id, uint32_t element)
    {
        double tf = (double)cms->query(element^id) / cms->query(id);
        double idf = std::log2(static_cast<double>(vhll->estimate(0,0)) / (vhll->estimate(element,id) + 1));
        return tf * idf;
    }
    uint32_t usedmem()
    {
        return vhll->usedmem() + cms->usedmem();
    }
    void clear()
    {
        vhll->clear();
        cms->clear();
    }
};
#endif