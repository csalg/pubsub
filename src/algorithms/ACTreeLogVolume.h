//
// Created by Work on 2019-08-11.
//

#ifndef PUBSUB_ACTREELOGVOLUME_H
#define PUBSUB_ACTREELOGVOLUME_H
#include "../params.h"


#include<vector>
#include <random>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <math.h>
#include "../common/data_structure.h"
//#include "./subscriptionForest/AVLTree.h"
//#include "./subscriptionForest/IntervalNode.h"
//#include "./subscriptionForest/CenteredIntervalTree.h"

//std::random_device rd;     // only used once to initialise (seed) engine
//std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)

namespace actlv {
    struct Node;
    struct NodeList;

    static long double approxLogArr[LOG_APPROX_SEGMENTS]{0};
    static unsigned short constraints_num = 0;

    long double approxLog(size_t width) {
        return approxLogArr[(size_t) ((((double) width) / MAX_CARDINALITY) * (double) LOG_APPROX_SEGMENTS)];
    }

    long double calculateLogVol(vector<IntervalCnt> &newConstraints) {
        long double newLogVol = 0;
        for (auto constraint : newConstraints) {
//        cout << constraint.lowValue << " " << constraint.highValue << " " << approxLog(constraint.highValue - constraint.lowValue) << endl;
            newLogVol += approxLog(constraint.highValue - constraint.lowValue);
        }
        return newLogVol;
    }


    struct Node {
        NodeList *child = nullptr;
        vector<IntervalCnt> constraints;
        //    struct Length {
        //        size_t att;
        //        double length;
        //    };
        //    vector<Length> logLengths;
        long double logVol = 0;
        //    int lo[MAX_ATTS] = {0};
        //    int hi[MAX_ATTS] = {0};
        //    int center[MAX_ATTS] = {0};
        //    int logVolume = 0;

        Node() = default;

        Node(IntervalSub &sub) {
            constraints = sub.constraints;
            logVol = calculateLogVol(constraints);
//        cout << logVol;
        }

        Node(Node &node1, Node &node2);


        vector<IntervalCnt> expandConstraints(vector<IntervalCnt> &otherConstraints) {
            // It is assumed that the other node's contraints are ordered by att.
            int i = 0, j = 0;

            vector<IntervalCnt> newConstraints;

            while (i < constraints.size() && j < otherConstraints.size()) {
                if (constraints.at(i).att == otherConstraints.at(j).att) {
                    // same attribute found
                    IntervalCnt newConstraint;
                    newConstraint.att = otherConstraints.at(j).att;

                    newConstraint.lowValue = min(otherConstraints.at(j).lowValue,
                                                 constraints.at(i).lowValue);
                    newConstraint.highValue = max(otherConstraints.at(j).highValue,
                                                  constraints.at(i).highValue);

                    newConstraints.emplace_back(newConstraint);

                    ++i;
                    ++j;
                    continue;
                }

                if (constraints.at(i).att < otherConstraints.at(j).att) {
                    //                cout << i << constraints.size();
                    // The previous constraints have a constraint not found in the new sub.
                    newConstraints.push_back(constraints.at(i));
                    ++i;
                    continue;
                }

                if (constraints.at(i).att > otherConstraints.at(j).att) {
                    // New node has a constraint not found in previous.
                    newConstraints.push_back(otherConstraints.at(j));
                    ++j;
                    continue;
                }
            }


            while (i < constraints.size()) {
                newConstraints.push_back(constraints.at(i));
                i++;
            }

            while (j < otherConstraints.size()) {
                newConstraints.push_back(otherConstraints.at(j));
                j++;
            }

            return newConstraints;
        }

        void expand(Node &that) {
            constraints = expandConstraints(that.constraints);
            logVol = calculateLogVol(constraints);
//        if (hasChildren()){
//            cout << "Log volume updated" << endl;
//            print();
//        }
        }

        bool hasChildren() {
            return !(child == nullptr);
        }

        void print();

        bool match(const Pub &pub);

        bool match(IntervalSub &sub);
    };

    struct NodeList {
        ~NodeList() {
            queue<NodeList *> q;
            q.push(this);
            for (auto i = 0; i != nodesWithChildren.size(); ++i) {
                q.push(nodes.at(i).child);
            }
            while (!q.empty()) {
                NodeList *next = q.front();
                next->~NodeList();
                q.pop();
            }
        }

        vector<Node> nodes;
        vector<size_t> nodesWithChildren;

        int size() {
            return nodes.size();
        }

        void push(Node &node) {
            nodes.push_back(node); // Node is copied
        }

        void push(IntervalSub &sub) {
            nodes.emplace_back(sub); // Node is constructed from sub
        }

        void insert(Node &node) {
            if (nodes.size() >= MAX_SPAN) {
                this->cluster();
            }
            push(node);
        }

        void insert(NodeList &newNodes) {
            for (auto node : newNodes.nodes) {
                nodes.push_back(node); // Node is copied
            }
        }

        void insert(IntervalSub &sub) {
            if (nodes.size() <= SPAN_AFTER_CLUSTERING) {
                push(sub);
                return;
            }

            bool found = 0;
            unsigned short nearest_center = 0;
            long double distance = INFINITY;

            for (auto j = 0; j != nodesWithChildren.size(); j++) {
                auto i = nodesWithChildren[j];
                if (nodes.at(i).match(sub)) {
                    return nodes.at(i).child->insert(sub);
                    //                        Old code looked at all possible places to fit the thing.
                    //                    auto new_distance =cost(nodes.at(i), sub);
                    //                    if (new_distance < distance) {
                    //                        distance = new_distance;
                    //                        nearest_center=i;
                    //                        found=1;
                    //                    }
                }
            }

            //        if (found){
            //            nodes.at(nearest_center).child->insert(sub);
            //            return;
            //        }

            if (nodes.size() >= MAX_SPAN) {
                this->cluster();
            }
            //        cout << nodes.size() << " ";
            push(sub);
        }

        long double cost(Node &a, vector<IntervalCnt> &otherConstraints, long double otherLogVol) {
            //        cout << "Calculating cost";
            auto newConstraints = a.expandConstraints(otherConstraints);
            //        for (auto constraint : *newConstraints) cout << constraint.lowValue;
            auto newVol = calculateLogVol(newConstraints);

            return min(newVol - a.logVol, newVol - otherLogVol);
        }

        //    long double dist(Node &a, IntervalSub &sub){
        //        auto newConstraints = a.expandConstraints(sub.constraints);
        //        auto newVol = a.calculateLogVol(*newConstraints);
        //
        //        return min(newVol - a.logVol, newVol - b.logVol);
        //    }

        void cluster();

        void match(const Pub &pub, int &matchSubs) {
            queue<NodeList *> toVisit;
            toVisit.push(this);
            while (!toVisit.empty()) {
                for (auto node : toVisit.front()->nodes) {
                    if (node.match(pub)) {
                        if (node.hasChildren()) {
                            toVisit.push(node.child);
                        } else {
                            matchSubs++;
                        }
                    }
                }
                toVisit.pop();
            }
        };

        void print() {
            for (auto i = 0; i != nodes.size(); i++) {
                nodes.at(i).print();
            }
        }

        void count(int &amount);

    };

    bool Node::match(const Pub &pub) {
        // The assumption is that the pairs and the sub are sorted in ascending order.

//    print();
//    for (auto pair : pub.pairs) cout << pair.value << " ";
//    cout << endl;

        size_t i = 0, j = 0, matched = 0;
        if (hasChildren()) return 1;

        while (j < pub.pairs.size() && i < constraints.size()) {
//            if (matched == constraints_num) return 1;

            if (pub.pairs.at(j).att == constraints.at(i).att) {
                if (pub.pairs.at(j).value < constraints.at(i).lowValue || \
                    pub.pairs.at(j).value > constraints.at(i).highValue) {
//                    if (!hasChildren()) return 0;
//                    else
                    {
                        ++i;
                        ++j;
                    }
                } else {
//                    cout << "constraint matched" << endl;
                    ++i;
                    ++j;
                    ++matched;
                }
                continue;
            }

            if (pub.pairs.at(j).att < constraints.at(i).att) {
                ++j;
                continue;
            }

            if (pub.pairs.at(j).att > constraints.at(i).att) {
                ++i;
            }
        }
        if (matched == constraints_num) {
//            cout << "match" << endl;
            return 1;
        }
//        cout << "no match" << endl;
        return 0;
    }

    bool Node::match(IntervalSub &sub) {

        size_t i = 0, j = 0, matched = 0;

        while (j < sub.constraints.size() && i < constraints.size()) {
            if (matched == constraints_num) return 1;
            if (sub.constraints.at(j).att == constraints.at(i).att) {
                if (sub.constraints.at(j).lowValue < constraints.at(i).lowValue || \
                    sub.constraints.at(j).highValue > constraints.at(i).highValue) {
                    if (!hasChildren()) return 0;
                    else {
                        ++i;
                        ++j;
                    }
                }
                ++i;
                ++j;
                ++matched;
                continue;
            }

            if (sub.constraints.at(j).att > constraints.at(i).att) {
                i++;
                continue;
            }

            if (sub.constraints.at(j).att < constraints.at(i).att) j++;
        }

        if (matched != constraints_num) return 0;

        return 1;
    }

    void Node::print() {
        int children = hasChildren() ? child->size() : 0;
        cout << "Node " << (this) << ": ";
        for (auto constraint: constraints) {
            cout << constraint.att << ": (" << constraint.lowValue << ", " << constraint.highValue << ") / ";
        }
        cout << ", Log volume: " << logVol
             << ", Child " << child
             << ", Nodes in child " << children << endl;

    }

    Node merge(Node &node1, Node &node2) {
        if (!node1.hasChildren() && !node2.hasChildren()) {
            // Both are simply subscriptions.
            //        cout << "Merging two subscriptions" << endl;
            //        node1.print();
            //        node2.print();
//        auto nl = new NodeList;
//        nl->push(node1);
//        nl->push(node2);
//        Node node(node1, node2);
            //        node->print();
            //        node->child = nl;
            //        cout << "New node created from two nodes!" << endl;
            //        node->print();
            //        node->print();
            return Node(node1, node2);
        } else if (!(node1.hasChildren())) { // One of them is a subscription.
            //        cout << "Node2 has children and node1 is a sub." << endl;
            //        node1.print();
            //        node2.print();
            node2.expand(node1);
            node2.child->insert(node1);
            //        node2.print();
            return node2;
        } else if (!(node2.hasChildren())) {
            // The other one is a subscription.
            //        cout << "Node2 has children and node1 is a sub." << endl;
            //        node1.print();
            //        node2.print();
            node1.expand(node2);
            node1.child->insert(node2);
            //        node2.print();
            return node1;
        } else {
            if (node1.child->size() < node2.child->size()) {
                node2.expand(node1);
                node2.child->insert(*node1.child);
                return node2;
            } else {
                node1.expand(node2);
                node1.child->insert(*node2.child);
                return node1;
            }
        }
    }


    void NodeList::cluster() {
        //    this->print();
        //    cout << "Clustering" << endl;

        //     Sort nodes by logVolume.
        //    sort(nodes.begin(), nodes.end(), [](Node a, Node b) {return a.logVol < b.logVol; });
        //    this->print();

        const unsigned len = this->size() - LEAVE_OUT; // How many nodes are we actually considering?
        //    cout << "len: " << len << endl;
        vector<bool> centers(len, 0); // Bitset to mark selected centers.
        auto cost_and_centers = new pair<long double, unsigned short>[len];
        //    auto nearest_center = new unsigned short[len];

        // Set distances to infinity.
        for (auto i = 0; i != len; i++) {
            (cost_and_centers[i]).first = INFINITY;
        }

        unsigned next_center = rand() % len;
        //    cout << "Current center is " << next_center << endl;
        long double maxCost;
        centers[next_center] = 1;

        for (auto i = 0; i != len; i++) {
            cost_and_centers[i].second = next_center;
        }
//        Timer centersStart;

        for (auto i = 1; i != K; i++) {
            maxCost = 0;
            unsigned current_center = next_center;

            //        cout << "Calculating costs." << endl;

            for (auto j = 0; j != len; j++) {
                //            for (auto center : centers) cout << center;

                if (!centers[j]) {
                    //
                    //            cout << "K is: " << K;
                    //            cout << " i is: " << i;
                    //            cout << " next center is: " << next_center;
                    //            cout << " current center is: " << current_center;
                    //            cout << " j is: " << j << endl;
                    //            cout << "center node: ";
                    //            nodes.at(next_center).print();
                    //            cout << "node being calculated: ";
                    //            nodes.at(j).print();
                    //            cout << "WTF " << nodes.at(j).logVol << " WTF";
                    //            assert(0);


                    auto new_cost = cost(nodes.at(current_center), nodes.at(j).constraints, nodes.at(j).logVol);
                    //            cout << " new cost is: " << new_cost << endl;
                    //            cout << " old cost is: " << cost_and_centers[j].first << endl;

                    if (cost_and_centers[j].first > new_cost) {
                        cost_and_centers[j].first = new_cost;
                        cost_and_centers[j].second = current_center;
                    }

                    if (cost_and_centers[j].first > maxCost) {
                        //                cout << changing
                        maxCost = cost_and_centers[j].first;
                        next_center = j;
                    }
                }
            }
            centers[next_center] = 1;
        }
//        cout << "Finding centers took " << centersStart.elapsed_nano()/1000000 << "ms." << endl;

        //    cout << "Centers array" << endl;
        //    for (auto center : centers) cout << center;
        //    cout << endl;
        //
        //    cout << "Distances and centers array" << endl;
        //    for (int i=0; i != len; i++ ) cout << "(" << cost_and_centers[i].first << ", " << cost_and_centers[i].second << "), ";
        //    cout << endl;
        //
        //    sort(cost_and_centers, cost_and_centers+len, [](pair<long double,unsigned short> a, pair<long double,unsigned short> b) {return a.first < b.first; });
        //
        //    cout << "Distances and centers array sorted in ascending order" << endl;
        //    for (int i=0; i != len; i++ ) cout << "(" << cost_and_centers[i].first << ", " << cost_and_centers[i].second << "), ";
        //    cout << endl;

        //    cout << "Nearest center array" << endl;
        //    for (int i=0; i != len; i++ ) cout << nearest_center[i] << " ";
        //    cout << endl;

//        Timer memoryT;

        vector<Node> newNodes;
        newNodes.reserve(MAX_SPAN);
        vector<bool> toAssign; // Bitset to mark selected centers.

        for (auto i = 0; i != len; i++) {
            toAssign.push_back(centers[i]);
        }
        //    cout << "toAssign before" << endl;
        //    for (auto center : toAssign) cout << center;
        //    cout << endl;


        unsigned short toAssignAfter = 0;
        for (auto i = len; i != nodes.size(); ++i) {
            toAssign.push_back(1);
        }
        //    cout << toAssignAfter << endl;

        for (auto i = 0; i != len; i++) {
            if ((!centers[i]) && (toAssignAfter < SPAN_AFTER_CLUSTERING)) {
                //            cout << "i: " << i << ". Setting center: "  << cost_and_centers[i].second << endl;
                nodes.at(cost_and_centers[i].second) = merge(nodes[i], nodes[cost_and_centers[i].second]);
                toAssign.at(i) = 0;
                toAssignAfter++;
                //            cout << toAssignAfter << endl;

                continue;
            }
            toAssign.at(i) = 1;

            //        if (centers[i]) toAssign.at(i) = 1;
        }
        //    cout << "toAssign after" << endl;
        //    for (auto center : toAssign) cout << center;
        //    cout << endl;
        nodesWithChildren.clear();

        for (auto i = 0; i != nodes.size(); i++) {
            if (toAssign[i]) {
                //            cout << "Assigning " << i << endl;
                newNodes.push_back(nodes.at(i));
                if (nodes.at(i).hasChildren()) nodesWithChildren.push_back(newNodes.size() - 1);

            }
        }

        //    cout << "nodesWithChildren: ";
        //    for (auto parent : nodesWithChildren) cout << parent << " ";
        //    cout << endl;

        //    cout << "Here are the new nodes:" << endl;
        //    for (auto node : newNodes) node.print();
        //
        nodes.clear();
        nodes.reserve(MAX_SPAN);
        nodes.insert(nodes.begin(), newNodes.begin(), newNodes.end());
        std::vector<Node>().swap(newNodes);
//    cout << "Memory operations took " << centersStart.elapsed_nano()/1000000 << "ms." << endl;
//    assert(0);
        ////        delete newNodes;
        //    cout << "Finished successfully. K was " << K << endl;
        //    cout << "Total nodes now: " << nodes.size() <<". Here are the new nodes:" << endl;
        //    int amount = 0;
        //    this->count(amount);
        //    cout << "Total subscriptions stored: " << amount <<". Here are the new nodes:" << endl;
        //    for (auto node : nodes) {
        //        node.print();
        //        if (node.hasChildren()){
        //            cout<<endl;
        //            cout<< "Children" << endl;
        //            for (auto node_ : node.child->nodes) {
        //                node_.print();
        //            }
        //            cout<<endl;
        //        }
        //    }
        //    this->print();
        //    assert(0);

    }


    Node::Node(Node &node1, Node &node2) {
        constraints = node1.expandConstraints(node2.constraints);
        logVol = calculateLogVol(constraints);

        // Both are simply subscriptions.
        this->child = new NodeList;
        child->push(node1);
        child->push(node2);
    }

    void NodeList::count(int &amount) {
        for (auto node : nodes) {
            if (node.hasChildren()) node.child->count(amount);
            else amount++;
        }
    }


    class AdaptiveClusterTreeLogVolume : public Broker {
        std::uniform_int_distribution<unsigned> uni; // guaranteed unbiased
        NodeList *root = new NodeList();
        unsigned leaveOutRate;
        vector<unsigned short> attributes;
    public:
        ~AdaptiveClusterTreeLogVolume() {
            delete root;
        }

        AdaptiveClusterTreeLogVolume(unsigned leaveOutRate, unsigned short constraints_)
                : Broker(), leaveOutRate(leaveOutRate) {
            for (short i = 0; i != LOG_APPROX_SEGMENTS; ++i) {
                double toLog = (((double) i) + 0.5) * (MAX_CARDINALITY / LOG_APPROX_SEGMENTS);
                approxLogArr[i] =
                        log(toLog);
//            cout << toLog << "-> " << log(toLog) << ", ";
            }
            constraints_num = constraints_;
//        cout << endl;
//        assert(0);
        };

        void insert(IntervalSub sub) {
            //        cout << "Inserting" << endl;
            std::sort(sub.constraints.begin(), sub.constraints.end(),
                      [](IntervalCnt a, IntervalCnt b) { return a.att < b.att; });
//        for (auto constraint : sub.constraints) cout << constraint.att;bttttttttggjyfnjnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnjbhbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbjhjhbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbjbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbhjbjhbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbjbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjbhbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhbjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjh
//        cout << endl;
            root->insert(sub);
        };

        void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList) {
//                cout<<endl;
//                cout<<endl;
//                print();
//                cout<<endl;
//                cout<<endl;
//
//                for (auto pair : pub.pairs) cout << pair.att << ": " << pair.value << " / ";
//                cout << endl;
            root->match(pub, matchSubs);
            assert(false);

        }

        void printInner(NodeList *&root) {
            queue<NodeList *> q;
            q.push(root);
            q.push(nullptr);

            while (!q.empty()) {
                NodeList *node = q.front();
                cout << node << endl;
                q.pop();
                if (node == nullptr) {
                    cout << endl;
                    cout << endl;
                    if (!q.empty()) q.push(nullptr);
                    continue;
                }
                for (auto node_ : node->nodes) {
                    if (node_.hasChildren()) {
                        q.push(node_.child);
                    }
                }

                node->print();
            }
        }

        int count() {
            int amount = 0;
            root->count(amount);
            return amount;
        }

        void print() {
            cout << "Total subscriptions stored: " << count() << endl;
            printInner(root);
        };

    };
};

#endif //PUBSUB_ACTREELOGVOLUME_H