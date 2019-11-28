#include "siena.h"
#include <cassert>



void Siena::insert(IntervalSub sub)
{
    for (int i = 0; i != sub.size; ++i)
    {
        IntervalCombo combo;
        combo.highValue = sub.constraints[i].highValue;
        combo.lowValue = sub.constraints[i].lowValue;
        combo.subID = sub.id;

        data[sub.constraints[i].att].push_back(combo);
    }
}

void Siena::match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList)
{
    for(auto i=0; i!=MAX_ATTS; ++i){
//        cout << data[i].size() << endl;
        if (data[i].size() > MAX_SUBS) {
            print();
            assert(false);
        }
    }
    for (int i = 0; i < subList.size(); i++)
        counter[i] = subList[i].size;
    for (int i = 0; i < pub.size; i++) {
        int att = pub.pairs[i].att, value = pub.pairs[i].value;
        for (int j = 0; j < data[att].size(); ++j) {
            if (data[att].size() > MAX_SUBS) {
                cout << data[att].size() << endl;
                cout << att << endl;
            }
            if (value >= data[att][j].lowValue && value <= data[att][j].highValue)
                --counter[data[att][j].subID];
        }
    }
    for (int i = 0; i < subList.size(); i++)
        if (counter[i] == 0)
            ++matchSubs;
}

void Siena::print(){
    for (auto i=0; i!=MAX_ATTS; ++i){
        for (auto j=0;j!=data[i].size(); ++j) cout << data[i][j].subID << ": (" << data[i][j].lowValue << ", " << data[i][j].highValue << ") , " << endl;
    }
    cout << endl;
}
