//
// Created by work on 9/11/19.
//

#ifndef PUBSUB_VARI_SIENA_H
#define PUBSUB_VARI_SIENA_H


#include<vector>
#include <cstring>
#include <random>
#include "../common/generator.h"
#include "../common/chrono_time.h"
#include "../common/util.h"
#include "../common/data_structure.h"
#include "../params.h"

using namespace std;


struct SienaInner {
//    int counter[MAX_SUBS];
    vector<IntervalCombo> data[MAX_ATTS];
    vector<size_t> number_of_constraints;
    SienaInner() {}
    void insert(IntervalSub sub);
    void match(const Pub &pub, int &matchSubs);
};

struct VariSiena : public Broker {
    VariSiena() : Broker() {};
    SienaInner subs_by_attribute[MAX_ATTS][HASH_BUCKETS];
    void insert(IntervalSub sub);
    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList);
};


#endif //PUBSUB_VARI_SIENA_H
