//
// Created by Work on 2019-08-08.
//

#ifndef PUBSUB_SUBSCRIPTIONFOREST_H
#define PUBSUB_SUBSCRIPTIONFOREST_H
#include<vector>
#include <random>
#include <cstring>
#include "../common/data_structure.h"
#include "./subscriptionForest/AVLTree.h"
//#include "./subscriptionForest/IntervalNode.h"
#include "./subscriptionForest/CenteredIntervalTree.h"

std::random_device rd;     // only used once to initialise (seed) engine
std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)

class SubscriptionForest : public Broker {
    unsigned m;
    unsigned dimensions;
    std::vector<IntervalSub> subsStore;
    std::uniform_int_distribution<unsigned> uni; // guaranteed unbiased
    std::vector<CenteredIntervalTree<int>> subs;
public:
    SubscriptionForest(unsigned dimensions, unsigned m) : m(m), dimensions(dimensions), Broker() {
        uni = std::uniform_int_distribution<unsigned>(0,m-1);
        for (int i = 0; i<dimensions; i++){
            static CenteredIntervalTree<int> itree;
            subs.push_back(itree);
        }
    };

//    void insert(IntervalSub sub) {
//        cout << "Crap" << endl;
//    };

    void insert(IntervalSub sub) {

        subsStore.push_back(sub);

        auto randomConstraint = uni(rng);
//        cout << sub.size << " " << randomConstraint << endl;
        auto randomAttribute = sub.constraints[randomConstraint].att;

        int lo=(sub.constraints[randomConstraint]).lowValue, hi = (sub.constraints[randomConstraint]).highValue;
//        cout << "subs.size() is " << subs.size() << ". Inserted subscription. Attribute: " << randomAttribute << ", lo: " << lo << " hi: " << hi << endl;

        (subs.at(randomAttribute)).insert(lo,hi,subsStore.size()-1);
    };

    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList){
//        cout << "Looking inside pub" << endl;
//        for (auto pair : pub.pairs){
//            cout << "att " << pair.att << " ";
//        }
//
//        cout << endl;
//
//        cout << "Looking inside first sub" << endl;
//
//        for (auto constraint : subList[0].constraints){
//            cout << "att " << constraint.att << " ";
//        }
//
//        cout << endl;


//        cout << "Matching" << endl;

        for (int i = 0; i<pub.size; i++){
            auto tree = subs.at(  ((pub.pairs).at(i)).att );
//            cout << "Looking in tree: " << ((pub.pairs).at(i)).att << ". It's size is: " << tree.size << endl;
//            tree.print();
            tree.match( ((pub.pairs).at(i)).value, pub, matchSubs, subsStore);
//            cout << "Intersections on attribute " <<  ((pub.pairs).at(i)).att << ": " << matchingSubs.size() << endl;
        }

    };

};

#endif //PUBSUB_SUBSCRIPTIONFOREST_H
//
//struct IntervalCombo{
//    int lowValue, highValue;
//    int subID;
//};