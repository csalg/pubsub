
#ifndef PUBSUB_HTREE_H
#define PUBSUB_HTREE_H

#include<vector>
#include <cstring>
#include "../common/generator.h"
#include "../common/chrono_time.h"
#include "../common/util.h"
#include "../common/data_structure.h"
#include "../params.h"


using namespace std;

//const int MAX_SUBS = 200000;

class Htree : public Broker  {
    int cells, atts, mLevel, valDis, valDom;
    double cellStep;
    vector<int> *buckets;
    bool matched[MAX_SUBS];
    void insertToBucket(const vector<vector<int> > &mark, int bucketID, int level, int subID);
    void matchInBucket(const vector<vector<int> > &mark, int bucketID, int level, int &matchSubs,
                       const vector<IntervalSub> &subList, Pub pub);
    //bool match(const IntervalSub &sub, const Pub &pub);
public:
    Htree(int atts_, int level_, int cells_, int valDis_, int valDom_) :Broker ()
    {
        atts=atts_;
        mLevel=level_;
        cells=cells_;
        valDis=valDis_;
        valDom=valDom_;
        cellStep = 1.0 / cells;
        buckets = new vector<int>[int(pow(cells, level_) + 0.5)];
    }

    void insert(IntervalSub sub) override;

    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList) override;

};

#endif //PUBSUB_HTREE_H
