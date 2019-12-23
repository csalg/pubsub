#ifndef PUBSUB_OPINDEX_H
#define PUBSUB_OPINDEX_H
#include<vector>
#include <string>
#include <cstring>
#include "../common/generator.h"
#include "../common/chrono_time.h"
#include "../common/util.h"
#include "../common/data_structure.h"
#include "../params.h"


class opIndex : public Broker {
    vector<ConElement> data[MAX_ATTS][3][SEGMENTS][MAX_SIGNATURE];
    bool sig[MAX_ATTS][3][SEGMENTS][MAX_SIGNATURE];
    int counter[MAX_SUBS];
    bool isPivot[MAX_ATTS];
    int fre[MAX_ATTS];

    void initCounter(const vector<Sub> &subList);

    void initCounter(const vector<IntervalSub> &subList);

    int getMinFre(Sub x);

    int getMinFre(IntervalSub x);

    int signatureHash1(int att, int val);       // for == operation
    int signatureHash2(int att);                // for <= and >= operation
public:
    opIndex()
    {
        memset(isPivot, 0, sizeof(isPivot));
        memset(sig, 0, sizeof(sig));
    }

    void calcFrequency(const vector<Sub> &subList);

    void calcFrequency(const vector<IntervalSub> &subList);

    void insert(Sub x);

    void insert(IntervalSub x) override;

    void match(const Pub &pub, int &matchSubs, const vector<Sub> &subList);

    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList) override;
};

#endif //PUBSUB_OPINDEX_H
