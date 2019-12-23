#include "vari-siena.h"
#include <bitset>




unsigned hashFn(unsigned att, unsigned pos){
    return ((att * pos * att + SEED - pos ) >> 1) % HASH_BUCKETS;
}

void SienaInner::insert(IntervalSub sub)
{
    for (int i = 0; i != sub.size; ++i)
        data[sub.constraints[i].att].emplace_back(  sub.constraints[i].lowValue, \
                                                    sub.constraints[i].highValue, \
                                                    number_of_constraints.size()
                                                  );
    number_of_constraints.push_back(sub.size);
}

void SienaInner::match(const Pub &pub, int &matchSubs)
{
    vector<size_t> counter = number_of_constraints;
    for (int i = 0; i < pub.size; i++)
    {
        int att = pub.pairs[i].att, value = pub.pairs[i].value;
        for (int j = 0; j < data[att].size(); j++)
            if (value >= data[att][j].lowValue && value <= data[att][j].highValue){
                --(counter.at(data[att][j].subID));
            }
    }
    for (int i = 0; i < counter.size(); i++)
        if (counter[i] == 0)
            ++matchSubs;
}

void VariSiena::insert(IntervalSub sub)
{
    size_t access_attribute = rand() % (sub.size -1);
    size_t hash_attribute = rand() % (sub.size -1 );
    if (access_attribute == hash_attribute && sub.size != 1){
        while (hash_attribute == access_attribute){
            hash_attribute = rand() % (sub.size -1 );
        }
    }
    subs_by_attribute[access_attribute][hash_attribute].insert(sub);
}

void VariSiena::match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList) {
    std::bitset<HASH_BUCKETS> hash_buckets = {0};
    for (auto i = 0; i != pub.size; ++i) {
        unsigned hash_att = hashFn(pub.pairs[i].att, i);
        hash_buckets[hash_att] = 1;

    }
    for (auto i = 0; i != pub.size; ++i) {
        for (auto j = 0; j != HASH_BUCKETS; ++j)
            if (hash_buckets[j]) {
                subs_by_attribute[pub.pairs[i].att][j].match(pub, matchSubs);
            }
    }
}
    // match per attribute.
//