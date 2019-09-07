#ifndef PUBSUB_SCTREEPARALLEL_H
#define PUBSUB_SCTREEPARALLEL_H


#include "../params.h"

#include<vector>
#include <random>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <iostream>
//#include <boost/asio/thread_pool.hpp>
//#include <boost/asio/post.hpp>
#include <nlohmann/json.hpp>
#include "../vendor/ctpl_stl.h"  // or <ctpl_stl.h> if ou do not have Boost library

#include "../common/data_structure.h"
//#include "./subscriptionForest/AVLTree.h"
//#include "./subscriptionForest/IntervalNode.h"
//#include "./subscriptionForest/CenteredIntervalTree.h"

//std::random_device rd;     // only used once to initialise (seed) engine
//std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)

using json = nlohmann::json;



namespace sctp {
//    boost::asio::thread_pool pool(2); // 4 threads
    ctpl::thread_pool p(2 /* two threads in the pool */);

    struct Node;
    struct NodeList;

    struct Node {
        NodeList *child = nullptr;
        int lo[MAX_ATTS] = {0};
        int hi[MAX_ATTS] = {0};
        int center[MAX_ATTS] = {0};
        int width = 0;

        Node() {
            std::fill_n(lo, MAX_ATTS, MAX_CARDINALITY);
        };

        Node(IntervalSub &sub) {
//        cout << "Constructing from sub" << endl;
            for (auto constraint : sub.constraints) {
//            cout << constraint.lowValue << endl;
                lo[constraint.att] = constraint.lowValue;
                hi[constraint.att] = constraint.highValue;
                width += constraint.highValue - constraint.lowValue;
                center[constraint.att] = constraint.lowValue + (constraint.highValue - constraint.lowValue) / 2;
            }
//        this->print();
        }

        Node(Node &node1, Node &node2);

        void expand(Node &node) {
            for (auto i = 0; i != MAX_ATTS; i++) {
                if (node.lo[i] < this->lo[i]) {
                    width += this->lo[i] - node.lo[i];
                    this->lo[i] = node.lo[i];
                }
            }

            for (auto i = 0; i != MAX_ATTS; i++) {
                if (node.hi[i] > this->hi[i]) {
                    width += node.hi[i] - this->hi[i];
                    this->hi[i] = node.hi[i];
                }
            }

            for (auto i = 0; i != MAX_ATTS; i++) {
                this->center[i] = this->lo[i] + (this->hi[i] - this->lo[i]) / 2;
            }
        }

        bool hasChildren() {
            return !(child == nullptr);
        }

        void print();

        void match(const Pub &pub, int &matchSubs);

        bool match(IntervalSub &sub);

        void match(const Pub &pub, int j, vector<bool> &matches);
    };

    struct NodeList {
        vector<Node> nodes;

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

            if (nodes.size() >= MAX_SPAN) {
                this->cluster();
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


            for (auto i = 0; i != size(); i++) {
                if (nodes.at(i).hasChildren()) {
                    if (nodes.at(i).match(sub)) {
                        nodes.at(i).child->insert(sub);
                        return;

//                        Old code looked at all possible places to fit the thing.
//                    auto new_distance =cost(nodes.at(i), sub);
//                    if (new_distance < distance) {
//                        distance = new_distance;
//                        nearest_center=i;
//                        found=1;
//                    }
                    }
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

        long double dist(Node &a, Node &b) {
            unsigned sum = 0;
            for (unsigned i = 0; i != MAX_ATTS; i++) {
                sum += pow((a.center[i] - b.center[i]), 2);
            }
            return sqrt(sum);
        }

        long double dist(Node &a, IntervalSub &sub) {

            int sub_centre[MAX_ATTS] = {0};
            for (auto interval : sub.constraints) {
                sub_centre[interval.att] = interval.lowValue + (interval.lowValue + interval.highValue) / 2;
            }
            unsigned sum = 0;
            for (unsigned i = 0; i != MAX_ATTS; i++) {
                sum += pow((a.center[i] - sub_centre[i]), 2);
            }
            return sqrt(sum);
        }

        void cluster();

        void match(const Pub &pub, int &matchSubs) {
//        cout << nodes.size() << endl;
//        for (auto node : nodes){
//            cout << node.lo[0] << " ";
//        }
            std::vector<std::future<void>> results;
            auto matches = new vector<bool>(nodes.size(),0);
//            for (auto match : *matches) cout << match;
//            cout << endl;

//            for (auto node : nodes) node.match(pub, matchSubs);
            for (auto j = 0; j != nodes.size(); j++)
                results.push_back(p.push([&, j](int id) { nodes[j].match(pub, j, *matches); }));
            for (int j = 0; j != results.size(); ++j) {
                results[j].get();
            }


            for (auto i = 0; i != matches->size(); i++) {
                if (matches->at(i)) {
                    matchSubs++;
                    if (nodes[i].hasChildren()) nodes[i].child->match(pub, matchSubs);
                }
            }
        };

        void print() {
            for (auto i = 0; i != nodes.size(); i++) {
                nodes.at(i).print();
            }
        }

        void count(int &amount);

    };

    void Node::match(const Pub &pub, int &matchSubs) {
//    cout << "Matched inside" << endl;
//    matchSubs++;

        for (auto pair : pub.pairs) {
            if (pair.value < lo[pair.att]) return;
        }

        for (auto pair : pub.pairs) {
            if (pair.value > hi[pair.att]) return;
        }

        if (hasChildren()) {
//        cout << "has children" << endl;
//        this->print();
            child->match(pub, matchSubs);
            return;
        }

//    cout << "Found match" << endl;

        matchSubs++;
    };
        void Node::match(const Pub &pub, int j, vector<bool> &matches){
//    cout << "Matched inside" << endl;
//    matchSubs++;

            for (auto pair : pub.pairs) {
                if (pair.value < lo[pair.att]) {
                    return;
                }
            }

            for (auto pair : pub.pairs) {
                if (pair.value > hi[pair.att]) return;
            }

            matches[j] = 1;
        };


    bool Node::match(IntervalSub &sub) {

        for (auto cons : sub.constraints) {
            if (cons.lowValue < lo[cons.att]) return 0;
        }

        for (auto cons : sub.constraints) {
            if (cons.highValue > hi[cons.att]) return 0;
        }
        return 1;
    }

    void Node::print() {
        int children = hasChildren() ? child->size() : 0;
        for (auto i = 0; i != MAX_ATTS; i++) {
            cout << "(" << lo[i] << ", " << center[i] << ", " << hi[i] << ")";
        }
        cout << ", Width: " << width
             << ", Children? " << children << endl;
    }

    Node *merge(Node &node1, Node &node2) {
        if (!node1.hasChildren() && !node2.hasChildren()) {
            // Both are simply subscriptions.
//        cout << "Merging two subscriptions" << endl;
//        node1.print();
//        node2.print();
            auto nl = new NodeList;
            nl->push(node1);
            nl->push(node2);
            Node *node = new Node(node1, node2);
//        node->print();
//        node->child = nl;
//        cout << "New node created from two nodes!" << endl;
//        node->print();
//        node->print();
            return node;
        } else if (!(node1.hasChildren())) { // One of them is a subscription.
//        cout << "Node2 has children and node1 is a sub." << endl;
//        node1.print();
//        node2.print();
            node2.expand(node1);
            node2.child->insert(node1);
//        node2.print();
            return &node2;
        } else if (!(node2.hasChildren())) {
            // The other one is a subscription.
//        cout << "Node2 has children and node1 is a sub." << endl;
//        node1.print();
//        node2.print();
            node1.expand(node2);
            node1.child->insert(node2);
//        node2.print();
            return &node1;
        } else {
//        cout << "The segfault" << endl;
//        cout << &node1 << " ";
//        cout << &node2;

            auto maxSize = std::max(node1.child->size(), node2.child->size());
            Node &bigNode = node1.child->size() == maxSize ? node1 : node2;
            Node &smallNode = &bigNode == &node1 ? node2 : node1;

//        cout << &node1 << endl;
//        cout << &node2 << endl;
//        cout << &bigNode << " " << &bigNode.child << endl;
//        bigNode.print();
//        cout << &smallNode  << " " << &smallNode.child << endl;
//        smallNode.print();
//        bigNode.expand(smallNode);
//        cout << "bigNode expanded" << endl;
//        bigNode.print();

            for (auto node : smallNode.child->nodes) {
                bigNode.expand(node);
            }
//        cout << "bigNode expanded again, let's see" << endl;
//        bigNode.print();

            bigNode.child->insert(*(smallNode.child));
//        cout << " doesn't happen here" << endl;

//        delete &smallNode;
            return &bigNode;
        }
    }


    void NodeList::cluster() {
//    this->print();
//    cout << "Clustering" << endl;

//     Sort nodes by logVolume.
//    sort(nodes.begin(), nodes.end(), [](Node a, Node b) {return a.logVolume < b.logVolume; });
//    this->print();

        const unsigned len = this->size();
//    const unsigned len = this->size() - LEAVE_OUT; // How many nodes are we actually considering?
//    cout << "len: " << len << endl;
        vector<bool> centers(len, 0); // Bitset to mark selected centers.
        auto distance_and_centers = new pair<long double, size_t>[len];
//    auto nearest_center = new unsigned short[len];

        // Set distances to infinity.
        for (auto i = 0; i != len; i++) {
            (distance_and_centers[i]).first = INFINITY;
        }

        unsigned next_center = rand() % len;
//    cout << "Current center is " << next_center << endl;
        long double further_distance = 0;
        centers[next_center] = 1;

        for (auto i = 0; i != len; i++) {
            distance_and_centers[i].second = next_center;
        }

//        auto updateNearestCenter = [=](
//                pair<long double, unsigned short> *distance_and_centers,
//                int j,
//                unsigned &current_center,
//                unsigned &next_center,
//                long double &further_distance
//        ) {
//            auto new_dist = cost(nodes.at(current_center), nodes.at(j));
//
////            cout << " new dist is: " << new_dist << endl;
////            cout << " old dist is: " << distance[j] << endl;
//
//            if (distance_and_centers[j].first > new_dist) {
//                distance_and_centers[j].first = new_dist;
//                distance_and_centers[j].second = current_center;
//            }
//
////            std::lock_guard<std::mutex> lock(m);
//            if (distance_and_centers[j].first > further_distance) {
////                cout << changing
//                further_distance = distance_and_centers[j].first;
//                next_center = j;
//            }
//        };
        Timer centersStart;

        for (auto i = 1; i != K; i++) {
            further_distance = 0;
            unsigned current_center = next_center;
            // calculate distances
//        cout << "Calculating distance." << endl;
            std::vector<std::future<void>> results;
            for (auto j = 0; j != len; j++) {
//            for (auto center : centers) cout << center;

                if (!centers[j]) {
//            cout << "K is: " << K;
//            cout << " i is: " << i;
//            cout << " current center is: " << next_center;
//            cout << " j is: " << j << endl;
//            cout << "center node: ";
//            nodes.at(next_center).print();
//            cout << "node being calculated: ";
//            nodes.at(j).print();
//                boost::asio::post(pool,updateNearestCenter(
//                        distance_and_centers,
//                        j,
//                        current_center,
//                        next_center,
//                        further_distance));
                    results.push_back(p.push([&, j](int id) {

//                    boost::asio::post(pool, [&, j]() {
//                    cout << j << " " << current_center << " " << nodes.size() <<  endl;

                        auto new_dist = dist(nodes.at(current_center), nodes.at(j));

//            cout << " new cost is: " << new_dist << endl;
//            cout << " old cost is: " << distance[j] << endl;

                        if (distance_and_centers[j].first > new_dist) {
                            distance_and_centers[j] = make_pair(new_dist, current_center);
                        }
                    }));
//                        results.push_back(temp);


//            auto new_dist = cost(nodes.at(current_center), nodes.at(j));
////            cout << " new dist is: " << new_dist << endl;
////            cout << " old dist is: " << distance[j] << endl;
//
//
//            if (distance_and_centers[j].first > new_dist){
//                distance_and_centers[j].first = new_dist;
//                distance_and_centers[j].second = current_center;
//            }
//
//            if (distance_and_centers[j].first > further_distance) {
////                cout << changing
//                further_distance = distance_and_centers[j].first;
//                next_center = j;
//            }

                }
            }

            for (int j = 0; j != results.size(); ++j) {
                results[j].get();
            }
            for (auto j = 0; j != len; j++) {
                if (distance_and_centers[j].first > further_distance) {
//                cout << changing
                    further_distance = distance_and_centers[j].first;
                    next_center = j;
                }
            }
                centers[next_center] = 1;
//        cout << "Made it here" << endl;

        }
//        cout << "Finding centers took " << centersStart.elapsed_nano()/1000000 << "ms." << endl;

//    cout << "Centers array" << endl;
//    for (auto center : centers) cout << center;
//    cout << endl;
//
//    cout << "Distances and centers array" << endl;
//    for (int i=0; i != len; i++ ) cout << "(" << distance_and_centers[i].first << ", " << distance_and_centers[i].second << "), ";
//    cout << endl;
//
//    sort(distance_and_centers, distance_and_centers+len, [](pair<long double,unsigned short> a, pair<long double,unsigned short> b) {return a.first < b.first; });
//
//    cout << "Distances and centers array sorted in ascending order" << endl;
//    for (int i=0; i != len; i++ ) cout << "(" << distance_and_centers[i].first << ", " << distance_and_centers[i].second << "), ";
//    cout << endl;

//    cout << "Nearest center array" << endl;
//    for (int i=0; i != len; i++ ) cout << nearest_center[i] << " ";
//    cout << endl;

        Timer memoryT;
//        cout << "Moving stuff" << endl;

        vector<Node> newNodes;
        newNodes.reserve(MAX_SPAN);
        vector<bool> toAssign; // Bitset to mark selected centers.

        for (auto i = 0; i != nodes.size(); i++) {
            toAssign.push_back(centers[i]);
        }
//    cout << "toAssign before" << endl;
//    for (auto center : toAssign) cout << center;
//    cout << endl;

        unsigned short toAssignAfter = 0;
//    cout << toAssignAfter << endl;

        for (auto i = 0; i != nodes.size(); i++) {
            if ((!centers[i]) && (toAssignAfter < SPAN_AFTER_CLUSTERING)) {
//            cout << "i: " << i << ". Setting center: "  << distance_and_centers[i].second << endl;
                nodes.at(distance_and_centers[i].second) = *(merge(nodes[i], nodes[distance_and_centers[i].second]));
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

        for (auto i = 0; i != nodes.size(); i++) {
            if (toAssign[i]) {
//            cout << "Assigning " << i << endl;
                newNodes.push_back(nodes.at(i));
            }
        }
//    cout << "Here are the new nodes:" << endl;
//    for (auto node : newNodes) node.print();
//
        nodes.clear();
        nodes.reserve(MAX_SPAN);
        nodes.insert(nodes.begin(), newNodes.begin(), newNodes.end());
        std::vector<Node>().swap(newNodes);
//        cout << "Memory operations took " << centersStart.elapsed_nano()/1000000 << "ms." << endl;

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
        for (auto i = 0; i != MAX_ATTS; i++) {
            lo[i] = std::min(node1.lo[i], node2.lo[i]);
        }
        for (auto i = 0; i != MAX_ATTS; i++) {
            hi[i] = std::max(node1.hi[i], node2.hi[i]);
        }

        width = 0;
        for (auto i = 0; i != MAX_ATTS; i++) {
            center[i] = lo[i] + (hi[i] - lo[i]) / 2;
            width = +hi[i] - lo[i];
        }

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


    class SubscriptionClusterTree : public Broker {
        std::uniform_int_distribution<unsigned> uni; // guaranteed unbiased
        NodeList *root = new NodeList();
        unsigned leaveOutRate;
//        boost::asio::thread_pool *pool;

    public:
        SubscriptionClusterTree(unsigned leaveOutRate) : Broker(), leaveOutRate(leaveOutRate) {
        };

        void insert(IntervalSub sub) {
//        cout << "Inserting" << endl;
            root->insert(sub);
        };

        void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList) {
//        cout<<endl;
//        cout<<endl;
//        print();
//        cout<<endl;
//        cout<<endl;
            root->match(pub, matchSubs);

        }

        void printInner(NodeList *&root) {
            queue<NodeList *> q;
            q.push(root);
            q.push(nullptr);

            while (!q.empty()) {
                NodeList *node = q.front();
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
}

#endif //PUBSUB_SCTREEPARALLEL_H
