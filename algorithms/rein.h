//
// Created by Work on 2019-07-26.
//

#ifndef PUBSUB_REIN_H
#define PUBSUB_REIN_H

#include<vector>
#include<bitset>
#include <cstring>
#include "../common/generator.h"
#include "../common/chrono_time.h"
#include "../common/util.h"
#include "../common/data_structure.h"
#include "../params.h"

struct Rein : public Broker {
    int valDom, buckStep, bucks;
    vector<Combo> data[MAX_ATTS][2][MAX_BUCKS];    // 0:left parenthesis, 1:right parenthesis
    explicit Rein(int _valDom) : Broker ()
    {
        valDom = _valDom;
        buckStep = (valDom - 1) / MAX_BUCKS + 1;
        bucks = (valDom - 1) / buckStep + 1;
    }

    void insert(IntervalSub sub) override;
    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList) override;
};

struct VariRein : public Rein {
    vector<bitset<MAX_ATTS>> constraints_set;
    explicit VariRein(int _valDom) : Rein(_valDom) {};
    void insert(IntervalSub sub) override;
    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList) override;
};
#endif //PUBSUB_REIN_H
