//
// Created by Work on 2019-08-11.
//

#ifndef PUBSUB_SUBSCRIPTIONCLUSTERTREE_H
#define PUBSUB_SUBSCRIPTIONCLUSTERTREE_H
#include "../params.h"


#include<vector>
#include <random>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include "../common/data_structure.h"
//#include "./subscriptionForest/AVLTree.h"
//#include "./subscriptionForest/IntervalNode.h"
//#include "./subscriptionForest/CenteredIntervalTree.h"

//std::random_device rd;     // only used once to initialise (seed) engine
//std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)

typedef struct Node;
typedef struct NodeList;

struct Node {
    NodeList* child = nullptr;
    int lo[MAX_ATTS] = {0};
    int hi[MAX_ATTS] = {0};
    int center[MAX_ATTS] = {0};
    int width = 0;

    Node() {
        std::fill_n(lo, MAX_ATTS, MAX_CARDINALITY);
    };

    Node(IntervalSub &sub){
//        cout << "Constructing from sub" << endl;
        for (auto constraint : sub.constraints) {
//            cout << constraint.lowValue << endl;
            lo[constraint.att] = constraint.lowValue;
            hi[constraint.att] = constraint.highValue;
            width += constraint.highValue - constraint.lowValue;
            center[constraint.att] = constraint.lowValue+(constraint.highValue - constraint.lowValue)/2;
        }
//        this->print();
    }

    Node(Node &node1, Node &node2);

    void expand(Node &node){
        for (auto i=0; i !=MAX_ATTS; i++){
            if (node.lo[i] < this->lo[i]){
                width += this->lo[i] - node.lo[i];
                this->lo[i] = node.lo[i];
            }
        }

        for (auto i=0; i !=MAX_ATTS; i++){
            if (node.hi[i] > this->hi[i]){
                width += node.hi[i] - this->hi[i];
                this->hi[i] = node.hi[i];
            }
        }

        for (auto i=0; i !=MAX_ATTS; i++){
            this->center[i] = this->lo[i] + (this->hi[i] - this->lo[i])/2;
        }
    }

    bool hasChildren(){
        return !(child == nullptr);
    }

    void print();

    void match(const Pub &pub, int &matchSubs);

};

struct NodeList {
    vector<Node> nodes;

    int size(){
        return nodes.size();
    }

    void push(Node &node) {
        nodes.push_back(node); // Node is copied
    }

    void push(IntervalSub &sub) {
        nodes.emplace_back(sub); // Node is constructed from sub
    }

    void insert(Node &node, unsigned leaveOutRate){
        if (nodes.size() >= MAX_SPAN) {
            this->cluster(leaveOutRate);
        }
        push(node);
    }

    void insert(NodeList &newNodes, unsigned leaveOutRate){
        for (auto node : newNodes.nodes){
            nodes.push_back(node); // Node is copied
        }

        if (nodes.size() >= MAX_SPAN) {
            this->cluster(leaveOutRate);
        }
    }

    void insert(IntervalSub &sub, unsigned leaveOutRate){
        if (nodes.size() >= MAX_SPAN) {
            this->cluster(leaveOutRate);
        }
//        cout << nodes.size() << " ";
        push(sub);
    }

    long double dist(Node &a, Node &b){
        unsigned sum = 0;
        for (unsigned i = 0; i != MAX_ATTS; i++){
            sum += pow((a.center[i] - b.center[i]),2);
        }
        return sqrt(sum);
    }

    void cluster(unsigned leaveOutRate);

    void match(const Pub &pub, int &matchSubs){
//        cout << nodes.size() << endl;
//        for (auto node : nodes){
//            cout << node.lo[0] << " ";
//        }

        for (auto node : nodes) node.match(pub, matchSubs);
    }

    void print(){
        for (auto i=0; i!=nodes.size(); i++){
            nodes.at(i).print();
        }
    }

    void count(int &amount);

    };

void Node::match(const Pub &pub, int &matchSubs) {
//    cout << "Matched inside" << endl;
//    matchSubs++;

    for (auto pair : pub.pairs){
        if (pair.value < lo[pair.att]) return;
    }

    for (auto pair : pub.pairs){
        if (pair.value > hi[pair.att]) return;
    }

    if (hasChildren()){
//        cout << "has children" << endl;
//        this->print();
        child->match(pub,matchSubs);
        return;
    }

//    cout << "Found match" << endl;

    matchSubs++;
}

void Node::print(){
    int children = hasChildren() ? child->size() : 0;
    for (auto i =0; i!=MAX_ATTS; i++){
        cout << "(" << lo[i] << ", " << center[i] << ", " << hi[i] << ")";
    }
    cout << ", Width: " << width
         << ", Children? " << children << endl;
}

Node *merge(Node &node1, Node &node2, unsigned leaveOutRate){
    if ( !node1.hasChildren() && !node2.hasChildren() )
    {
        // Both are simply subscriptions.
//        cout << "Merging two subscriptions" << endl;
//        node1.print();
//        node2.print();
        auto nl = new NodeList;
        nl->push(node1);
        nl->push(node2);
        Node* node = new Node(node1,node2);
//        node->print();
//        node->child = nl;
//        cout << "New node created from two nodes!" << endl;
//        node->print();
//        node->print();
        return node;
    }
    else if (!(node1.hasChildren()))
    { // One of them is a subscription.
//        cout << "Node2 has children and node1 is a sub." << endl;
//        node1.print();
//        node2.print();
        node2.expand(node1);
        node2.child->insert(node1, leaveOutRate);
//        node2.print();
        return &node2;
    }
    else if (!(node2.hasChildren()))
    {
        // The other one is a subscription.
//        cout << "Node2 has children and node1 is a sub." << endl;
//        node1.print();
//        node2.print();
        node1.expand(node2);
        node1.child->insert(node2, leaveOutRate);
//        node2.print();
        return &node1;
    }
    else {
//        cout << "The segfault" << endl;
//        cout << &node1 << " ";
//        cout << &node2;

        auto maxSize = std::max(node1.child->size(), node2.child->size());
        Node &bigNode = node1.child->size() == maxSize ? node1 : node2;
        Node &smallNode = &bigNode==&node1 ? node2 : node1;

//        cout << &node1 << endl;
//        cout << &node2 << endl;
//        cout << &bigNode << " " << &bigNode.child << endl;
//        bigNode.print();
//        cout << &smallNode  << " " << &smallNode.child << endl;
//        smallNode.print();
//        bigNode.expand(smallNode);
//        cout << "bigNode expanded" << endl;
//        bigNode.print();

        for (auto node : smallNode.child->nodes){
            bigNode.expand(node);
        }
//        cout << "bigNode expanded again, let's see" << endl;
//        bigNode.print();

        bigNode.child->insert(*(smallNode.child), leaveOutRate);
//        cout << " doesn't happen here" << endl;

//        delete &smallNode;
        return &bigNode;
    }
}


void NodeList::cluster(unsigned leaveOutRate){
//    this->print();
//    cout << "Clustering" << endl;
    // Sort nodes by width.
    sort(nodes.begin(), nodes.end(), [](Node a, Node b) {return a.width > b.width; });

    const unsigned len = ((100-leaveOutRate)*this->size())/100; // How many nodes are we actually considering?
//    cout << "len: " << len << endl;
    const unsigned k   = (MAX_SPAN*(50 - leaveOutRate))/100; // How many centers do we need?
    vector<bool> centers(len,0); // Bitset to mark selected centers.
    auto distance = new long double[len];
    auto nearest_center = new unsigned[len];


    // Set distances to infinity.
    for (auto i=0; i!=len; i++){
        distance[i] = INFINITY;
    }

    unsigned next_center = rand() % len;
//    cout << "Current center is " << next_center << endl;
    unsigned further_distance = 0;
    centers[next_center] = 1;
    for(auto i=0; i!=len;i++){
        nearest_center[i]=next_center;
    }
    for (auto i = 1; i!= k; i++){
        further_distance = 0;
        unsigned current_center = next_center;
        // calculate distances
//        cout << "Calculating distance." << endl;

        for (auto j = 0; j!=len; j++){
//            for (auto center : centers) cout << center;

            if (!centers[j]){

//            cout << "k is: " << k;
//            cout << " i is: " << i;
//            cout << " current center is: " << next_center;
//            cout << " j is: " << j << endl;
//            cout << "center node: ";
//            nodes.at(next_center).print();
//            cout << "node being calculated: ";
//            nodes.at(j).print();


            auto new_dist = dist(nodes.at(current_center), nodes.at(j));
//            cout << " new dist is: " << new_dist << endl;
//            cout << " old dist is: " << distance[j] << endl;

            if (distance[j] > new_dist){
                distance[j] = new_dist;
                nearest_center[j] = current_center;
            }

            if (distance[j] > further_distance){
//                cout << changing
                further_distance = distance[j];
                next_center = j;
            }
        }
        }
        centers[next_center] = 1;
    }
//    cout << "Centers array" << endl;
//    for (auto center : centers) cout << center;
//    cout << endl;
//
//    cout << "Distances array" << endl;
//    for (int i=0; i != len; i++ ) cout << distance[i] << " ";
//    cout << endl;
//
//    cout << "Nearest center array" << endl;
//    for (int i=0; i != len; i++ ) cout << nearest_center[i] << " ";
//    cout << endl;

    vector<Node> newNodes;

    for (auto i = 0; i!=len; i++){
        if (!centers[i]){
//            cout << "Setting center " << nearest_center[i] << endl;
            nodes.at(nearest_center[i]) = *(merge(nodes[i], nodes[nearest_center[i]], leaveOutRate));
        }
    }

    for (auto i = 0; i!=len; i++) {
        if (centers[i]){
            newNodes.push_back(nodes.at(i));
        }
    }
//    cout << "Here are the new nodes:" << endl;
//    for (auto node : newNodes) node.print();

        nodes.clear();
        nodes.insert(nodes.begin(),newNodes.begin(), newNodes.end());
//        delete newNodes;
//    cout << "Finished successfully. k was " << k << endl;
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

}



Node::Node(Node &node1, Node &node2){
        for (auto i =0; i != MAX_ATTS; i++){
        lo[i] = std::min(node1.lo[i], node2.lo[i]);
        }
        for (auto i =0; i != MAX_ATTS; i++){
        hi[i] = std::max(node1.hi[i], node2.hi[i]);
        }

        width=0;
        for (auto i=0; i !=MAX_ATTS; i++){
        center[i] = lo[i]+(hi[i] - lo[i])/2;
        width =+ hi[i] - lo[i];
        }

        // Both are simply subscriptions.
        this->child = new NodeList;
        child->push(node1);
        child->push(node2);
}

void NodeList::count(int &amount){
    for (auto node : nodes){
        if (node.hasChildren()) node.child->count(amount);
        else amount++;
    }
}

//
//int* mergeLo(Node* &node1, Node*&node2){
//    int newLo[MAX_ATTS];
//    for (auto i =0; i != MAX_ATTS; i++){
//        newLo[i] = std::min(node1->lo[i], node2->lo[i]);
//    }
//
//    return newLo;
//};
//
//int* mergeHi(Node* &node1, Node*&node2){
//    int newHi[MAX_ATTS];
//    for (auto i =0; i != MAX_ATTS; i++){
//        newHi[i] = std::max(node1->hi[i], node2->hi[i]);
//    }
//    return newHi;
//};


class SubscriptionClusterTree : public Broker {
//    unsigned short m;
    std::uniform_int_distribution<unsigned> uni; // guaranteed unbiased
    NodeList *root = new NodeList();
    unsigned leaveOutRate;
public:
    SubscriptionClusterTree(unsigned leaveOutRate) : Broker(), leaveOutRate(leaveOutRate) {
    };

    void insert(IntervalSub sub) {
//        cout << "Inserting" << endl;
        root->insert(sub, leaveOutRate);
    };

    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList){
//        cout<<endl;
//        cout<<endl;
//        print();
//        cout<<endl;
//        cout<<endl;

        root->match(pub,matchSubs);

    }

    void printInner(NodeList* &root){
        queue<NodeList*> q;
        q.push(root);
        q.push(nullptr);

        while (!q.empty()){
            NodeList* node = q.front();
            q.pop();
            if (node == nullptr){
                cout << endl;
                cout << endl;
                if (!q.empty()) q.push(nullptr);
                continue;
            }
            for (auto node_ : node->nodes){
                if (node_.hasChildren()){
                    q.push(node_.child);
                }
            }

            node->print();
        }
    }

    int count(){
        int amount=0;
        root->count(amount);
        return amount;
    }

    void print(){
        cout << "Total subscriptions stored: " << count() << endl;
        printInner(root);
    }


    ;

};
#endif //PUBSUB_SUBSCRIPTIONCLUSTERTREE_H
