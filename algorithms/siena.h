//
// Created by Work on 2019-07-26.
//

#ifndef PUBSUB_SIENA_H
#define PUBSUB_SIENA_H

#include<vector>
#include <cstring>
#include "../common/generator.h"
#include "../common/chrono_time.h"
#include "../common/util.h"
#include "../common/data_structure.h"
#include "../params.h"

using namespace std;


class Siena : public Broker {
    int counter[MAX_SUBS];
    vector<IntervalCombo> data[MAX_ATTS];
public:
    Siena() : Broker () {}
    void insert(IntervalSub sub);
    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList);

};

#endif //PUBSUB_SIENA_H
