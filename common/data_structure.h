#ifndef PUBSUB_DATA_STRUCTURE_H
#define PUBSUB_DATA_STRUCTURE_H

#include<string>
#include<vector>
#include <iostream>

using namespace std;

struct Cnt{
    int att;
    int value;
    int op;     //  op==0 -> "="  op==1 -> ">="  op==2 -> "<="
};

struct IntervalCnt{
    int att;
    int lowValue, highValue;
};

struct Sub {
    int id;
    int size; 								//number of predicates
    vector<Cnt> constraints;				//list of constraints
};

struct IntervalSub{
    int id;
    int size;
    vector<IntervalCnt> constraints;
};

struct ConElement {
    int att;
    int val;
    int subID;
};

struct Combo{
    int val;
    int subID;
};

struct IntervalCombo{
    int lowValue, highValue;
    int subID;

    IntervalCombo() = default;
    IntervalCombo(int lowValue, int highValue, int subID) : lowValue(lowValue), highValue(highValue), subID(subID) {};
};



struct Pair{
    int att;
    int value;
};

struct Pub{
    int size;
    vector<Pair> pairs;
};

struct attAndCount{
    int att, count;
    bool operator < (const attAndCount &b) const{
        return  count < b.count;
    }
};


struct Broker{
    Broker () {};
    virtual void insert(IntervalSub sub) {
    };

    virtual void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList) {};
};


#endif //PUBSUB_DATA_STRUCTURE_H
