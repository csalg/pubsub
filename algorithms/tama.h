#ifndef PUBSUB_TAMA_H
#define PUBSUB_TAMA_H

#include<vector>
#include <cstring>
#include "../common/generator.h"
#include "../common/chrono_time.h"
#include "../common/util.h"
#include "../common/data_structure.h"
#include "../params.h"

using namespace std;



class Tama : public Broker  {
    int mLevel, atts, valDom;
    int nodeCounter;
    int *lchild, *rchild, *mid;
    vector<int> *data[MAX_ATTS];
    int counter[MAX_SUBS];

    void initiate(int p, int l, int r, int level);

    int median(int l, int r);

    void insert(int p, int att, int subID, int l, int r, int low, int high, int level);

    void match(int p, int att, int l, int r, int value, int level);

public:
    Tama(int atts, int valDom, int level) : atts(atts), valDom(valDom), mLevel(level)
    {
        for (int i = 0; i < atts; i++)
            data[i] = new vector<int>[1 << level];
        lchild = new int[1 << level];
        rchild = new int[1 << level];
        mid = new int[1 << level];
        nodeCounter = 0;
        initiate(0, 0, valDom - 1, 1);
    }

    void insert(IntervalSub sub);

    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList);
};

#endif //PUBSUB_TAMA_H
