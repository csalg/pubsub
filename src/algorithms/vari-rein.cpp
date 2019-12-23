//
// Created by work on 9/10/19.
//

#include "rein.h"



void VariRein::insert(IntervalSub sub)
{
    bitset<MAX_ATTS> cons_set = {0};
    for (int i = 0; i < sub.size; i++)
    {
        cons_set[sub.constraints[i].att] = 1;
        IntervalCnt cnt = sub.constraints[i];
        Combo c;
        c.val = cnt.lowValue;
        c.subID = sub.id;
        data[cnt.att][0][c.val / buckStep].push_back(c);
        c.val = cnt.highValue;
        data[cnt.att][1][c.val / buckStep].push_back(c);
    }
    constraints_set.push_back(cons_set);
}

void VariRein::match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList)
{
    vector<bool> bits (subList.size(), false);
    vector<size_t> pub_atts_complement;

    // Setting bits for subscriptions with constraints not in event

    bitset<MAX_ATTS> set_of_pub_atts{0};

    for (auto i =0; i != pub.size; ++i){
        set_of_pub_atts[ pub.pairs[i].att] =1;
    }

    for (auto i=0; i!=MAX_ATTS; ++i){
        if (!set_of_pub_atts[i]) {
            pub_atts_complement.push_back(i);
        }
    }

//    for (auto i =0; i != constraints_set.size(); ++i){
//        for (auto j =0; j != pub_atts_complement.size(); ++j){
//            if (constraints_set[i][pub_atts_complement[j]]){
//                bits[i] = 1;
//                break;
//            }
//        }
//    }



    for (int i = 0; i < pub.size; i++)
    {
        int value = pub.pairs[i].value, att = pub.pairs[i].att, buck = value / buckStep;
        for (int k = 0; k < data[att][0][buck].size(); k++)
            if (!bits[data[att][0][buck][k].subID]){
                if (data[att][0][buck][k].val > value)
                    bits[data[att][0][buck][k].subID] = true;
            }
        for (int j = buck + 1; j < bucks; j++)
            for (int k = 0; k < data[att][0][j].size(); k++)
                bits[data[att][0][j][k].subID] = true;

        for (int k = 0; k < data[att][1][buck].size(); k++)
            if (!bits[data[att][1][buck][k].subID]) {
                if (data[att][1][buck][k].val < value)
                    bits[data[att][1][buck][k].subID] = true;
            }
        for (int j = buck - 1; j >= 0; j--)
            for (int k = 0; k < data[att][1][j].size(); k++)
                bits[data[att][1][j][k].subID] = true;
    }

    for (int i = 0; i < subList.size(); i++)
        if (!bits[i])
            ++ matchSubs;
}