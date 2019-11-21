#ifndef PUBSUB_SCTREE_H
#define PUBSUB_SCTREE_H
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

struct Node;
struct NodeList;

struct Node {
    NodeList* child = nullptr;
//    int lo[MAX_ATTS] = {0};
//    int hi[MAX_ATTS] = {0};
//    int center[MAX_ATTS] = {0};
    vector<int> lo, hi, center;
    int width = 0;

    Node() {
    };

    Node(IntervalSub &sub){
//        cout << "Constructing from sub" << endl;
        lo.reserve(sub.constraints.size());
        hi.reserve(sub.constraints.size());
        center.reserve(sub.constraints.size());

        for (auto constraint : sub.constraints) {
//            cout << constraint.lowValue << endl;
            lo.push_back(constraint.lowValue);
            hi.push_back(constraint.highValue);
            center.push_back(constraint.lowValue+(constraint.highValue - constraint.lowValue)/2);
            width += constraint.highValue - constraint.lowValue;
        }
//        this->print();
    }

    Node(Node &node1, Node &node2);

    void expand(Node &node){
        for (auto i=0; i !=lo.size(); i++){
            if (node.lo[i] < this->lo[i]){
                width += this->lo[i] - node.lo[i];
                this->lo[i] = node.lo[i];
            }
        }

        for (auto i=0; i !=hi.size(); i++){
            if (node.hi[i] > this->hi[i]){
                width += node.hi[i] - this->hi[i];
                this->hi[i] = node.hi[i];
            }
        }

        for (auto i=0; i !=center.size(); i++){
            this->center[i] = this->lo[i] + (this->hi[i] - this->lo[i])/2;
        }
    }

    bool hasChildren(){
        return !(child == nullptr);
    }

    void print();

    bool match(const Pub &pub);
    bool match(IntervalSub &sub);
};

struct NodeList {
    vector<Node> nodes;
    vector<size_t> nodesWithChildren;
    short level = 0;

    int size(){
        return nodes.size();
    }

    void push(Node &node) {
        nodes.push_back(node); // Node is copied
    }

    void push(IntervalSub &sub) {
        nodes.emplace_back(sub); // Node is constructed from sub
    }

    void insert(Node &node, short maxLevel){
        if (nodes.size() >= MAX_SPAN && (level < maxLevel)) {
            this->cluster();
        }
        push(node);
    }

    void insert(NodeList &newNodes){
        for (auto node : newNodes.nodes){
            nodes.push_back(node); // Node is copied
        }

        if (nodes.size() >= MAX_SPAN) {
            this->cluster();
        }
    }

    void insert(IntervalSub &sub, short maxLevels){
        if (nodes.size() <= SPAN_AFTER_CLUSTERING || (level >= maxLevels)){
            push(sub);
            return;
        }

        bool found = 0;
        unsigned short nearest_center = 0;
        long double distance = INFINITY;

        for (auto j=0; j!= nodesWithChildren.size(); j++){
            auto i = nodesWithChildren[j];
            if (nodes.at(i).match(sub)){
                nodes.at(i).child->insert(sub, maxLevels);
                return;

//                        Old code looked at all possible places to fit the thing.
//                    auto new_distance =dist(nodes.at(i), sub);
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

    long double dist(Node &a, Node &b){
        unsigned sum = 0;
        for (unsigned i = 0; i != a.center.size(); i++){
            sum += pow((a.center[i] - b.center[i]),2);
        }
        return sqrt(sum);
    }

    long double dist(Node &a, IntervalSub &sub){

        vector<int> newCenters;
        for (auto interval : sub.constraints){
            newCenters.push_back(interval.lowValue +  (interval.lowValue +  interval.highValue)/2);
        }
        unsigned sum = 0;
        for (auto i = 0; i != sub.constraints.size(); ++i){
            sum += pow((a.center[i] - newCenters[i]),2);
        }
        return sqrt(sum);
    }

    void cluster();

    void match(const Pub &pub, int &matchSubs) {
//        cout << nodes.size() << endl;
//        for (auto node : nodes){
//            cout << node.lo[0] << " ";
//        }
//        cout << "Matching event: " << endl;
//        for (auto i=0; i!=pub.pairs.size(); ++i) cout << pub.pairs[i].att << ": " << pub.pairs[i].value << ", ";

        queue<NodeList *> q;
        q.push(this);
        while (!q.empty()) {
            NodeList* currentNode = q.front();
            q.pop();

            for (auto i = 0; i != currentNode->nodes.size(); ++i) {
                if (currentNode->nodes[i].match(pub)) {
                    if      (currentNode->nodes[i].hasChildren())    {
                        q.push(currentNode->nodes[i].child);
//                        cout << "Will be visited" << endl;
//                        currentNode->nodes[i].print();


                    }
                    else {
                        ++matchSubs;
//                        cout << "Event matched! " << currentNode->nodes[i].hasChildren() ;
//                        currentNode->nodes[i].print();
                    }
                }
            }
        }
    }

    void print(){
        for (auto i=0; i!=nodes.size(); i++){
            nodes.at(i).print();
        }
    }

    void count(int &amount);

};

bool Node::match(const Pub &pub) {
//    cout << "Matched inside" << endl;
//    matchSubs++;

    for (auto i=0; i != lo.size(); ++i){
//        cout << pub.pairs[i].value << " < " << lo[i] << endl;
        if (pub.pairs[i].value < lo[i]) return 0;
    }

    for (auto i=0; i != hi.size(); ++i){
//        cout << pub.pairs[i].value << " > " << hi[i] << endl;
        if (pub.pairs[i].value > hi[i]) return 0;
    }
    return 1;

//    if (hasChildren()){
//        cout << "has children" << endl;
//        this->print();
//        child->match(pub,matchSubs);
//        return;
//    }
//
//    cout << "Found match" << endl;
//    cout << "Event:" << endl;
//    for (auto pair : pub.pairs) cout << pair.value << ", ";
//    cout << endl;
//    print();
//
//    ++matchSubs;

}

bool Node::match(IntervalSub &sub) {

    for (auto i=0; i != sub.constraints.size(); ++i){
        if (sub.constraints[i].lowValue < lo[i]) return 0;
    }

    for (auto i=0; i != sub.constraints.size(); ++i){
        if (sub.constraints[i].highValue > hi[i]) return 0;
    }
    return 1;
}

void Node::print(){
    int children = hasChildren() ? child->size() : 0;
    for (auto i =0; i!=lo.size(); i++){
        cout << "(" << lo[i] << ", " << center[i] << ", " << hi[i] << ")";
    }
    cout << ", Width: " << width
         << ", Children? " << children << endl;
}

Node *merge(Node &node1, Node &node2, short level){
    if ( !node1.hasChildren() && !node2.hasChildren() )
    {
        // Both are simply subscriptions.
//        cout << "Merging two subscriptions" << endl;
//        node1.print();
//        node2.print();
        Node* node = new Node(node1,node2);
        node->child->level=level+1;
//        node->print();
//        node->child = nl;
//        cout << "New node created from two nodes!" << endl;
//        node->print();
//        node->print();
        assert(node->lo.size()!=0);
        return node;
    }
    else if (!(node1.hasChildren()))
    { // One of them is a subscription.
//        cout << "Node2 has children and node1 is a sub." << endl;
//        node1.print();
//        node2.print();
        node2.expand(node1);
        node2.child->insert(node1, level);
//        node2.print();
        assert(node2.lo.size()!=0);
        return &node2;
    }
    else if (!(node2.hasChildren()))
    {
        // The other one is a subscription.
//        cout << "Node2 has children and node1 is a sub." << endl;
//        node1.print();
//        node2.print();
        node1.expand(node2);
        node1.child->insert(node2, level);
//        node2.print();
        assert(node1.lo.size()!=0);
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
//
        for (auto node : smallNode.child->nodes){
            bigNode.expand(node);
        }
//        cout << "bigNode expanded again, let's see" << endl;
//        bigNode.print();

        bigNode.child->insert(*(smallNode.child));
//        cout << " doesn't happen here" << endl;

//        delete &smallNode;
        assert(bigNode.lo.size()!=0);
        return &bigNode;
    }
}


void NodeList::cluster(){
//    this->print();
//    cout << "Clustering" << endl;

//     Sort nodes by width.
    sort(nodes.begin(), nodes.end(), [](Node a, Node b) {return a.width < b.width; });
//    this->print();

    const unsigned len = this->size() - LEAVE_OUT; // How many nodes are we actually considering?
//    cout << "len: " << len << endl;
    vector<bool> centers(len,0); // Bitset to mark selected centers.
    auto distance_and_centers = new pair<long double,unsigned short>[len];
//    auto nearest_center = new unsigned short[len];

    // Set distances to infinity.
    for (auto i=0; i!=len; i++){
        (distance_and_centers[i]).first = INFINITY;
    }

    unsigned next_center = rand() % len;
//    cout << "Current center is " << next_center << endl;
    long double further_distance = 0;
    centers[next_center] = 1;

    for(auto i=0; i!=len;i++){
        distance_and_centers[i].second =next_center;
    }
    Timer centersStart;

    for (auto i = 1; i!= K; i++){
        further_distance = 0;
        unsigned current_center = next_center;
        // calculate distances
//        cout << "Calculating distance." << endl;

        for (auto j = 0; j!=len; j++){
//            for (auto center : centers) cout << center;

            if (!centers[j]){

//            cout << "K is: " << K;
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

                if (distance_and_centers[j].first > new_dist){
                    distance_and_centers[j].first = new_dist;
                    distance_and_centers[j].second = current_center;
                }

                if (distance_and_centers[j].first > further_distance){
//                cout << changing
                    further_distance = distance_and_centers[j].first;
                    next_center = j;
                }
            }
        }
        centers[next_center] = 1;
    }
//    cout << "Finding centers took " << centersStart.elapsed_nano()/1000000 << "ms." << endl;

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

    vector<Node> newNodes;
    newNodes.reserve(MAX_SPAN);
    vector<bool> toAssign; // Bitset to mark selected centers.

    for (auto i =0; i!= nodes.size(); i++){
        toAssign.push_back(centers[i]);
    }
//    cout << "toAssign before" << endl;
//    for (auto center : toAssign) cout << center;
//    cout << endl;


    unsigned short toAssignAfter = 0;
//    cout << toAssignAfter << endl;

    for (auto i = 0; i!=nodes.size(); i++){
        if ((!centers[i]) && (toAssignAfter < SPAN_AFTER_CLUSTERING)){
//            cout << "i: " << i << ". Setting center: "  << distance_and_centers[i].second << endl;
            nodes.at(distance_and_centers[i].second) = *(merge(nodes[i], nodes[distance_and_centers[i].second], level));
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

    for (auto i = 0; i!=nodes.size(); i++) {
        if (toAssign[i]){
//            cout << "Assigning " << i << endl;
            newNodes.push_back(nodes.at(i));
            if (nodes.at(i).hasChildren()) nodesWithChildren.push_back(newNodes.size()-1);

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
    nodes.insert(nodes.begin(),newNodes.begin(), newNodes.end());
    std::vector<Node>().swap(newNodes);
//    cout << "Memory operations took " << centersStart.elapsed_nano()/1000000 << "ms." << endl;

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



Node::Node(Node &node1, Node &node2)
{
    for (auto i =0; i != node1.lo.size(); i++){
        lo.push_back(std::min(node1.lo[i], node2.lo[i]));
    }
    for (auto i =0; i != node1.hi.size(); i++){
        hi.push_back(std::max(node1.hi[i], node2.hi[i]));
    }

    width=0;
    for (auto i=0; i !=node1.center.size(); i++){
        center.push_back(lo[i]+(hi[i] - lo[i])/2);
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


class SubscriptionClusterTree : public Broker {
    std::uniform_int_distribution<unsigned> uni; // guaranteed unbiased
    NodeList *root = new NodeList();
    unsigned leaveOutRate;
    short maxLevels;

    vector<unsigned long> signatures;
    vector<NodeList> roots;

//    struct Root {
//        vector<unsigned short> attributes;
//        unsigned long signature;
//        NodeList nodelist = NodeList();
//    };
//    vector<Root> roots;

    bool matchEventAndSubscriptionSignatures(unsigned long eventSignature, unsigned long subscriptionSignature){
        /*
         * Returns true if all the attributes in the subscription signature are found in the event signature
         */
        unsigned long masked_event = eventSignature & subscriptionSignature;
        return masked_event == subscriptionSignature;
    }

    void matchEventAndSubscriptionSignaturesTest(){
        // Some true comparisons
        if (!matchEventAndSubscriptionSignatures(8,74)) cout << "matchEventAndSubscriptionSignatures failed";
        if (!matchEventAndSubscriptionSignatures(16,20)) cout << "matchEventAndSubscriptionSignatures failed";

        // Some false ones
        if (!matchEventAndSubscriptionSignatures(74,8)) cout << "matchEventAndSubscriptionSignatures failed";
        if (!matchEventAndSubscriptionSignatures(0,74)) cout << "matchEventAndSubscriptionSignatures failed";
    };


    unsigned long makeSignature(vector<int> &attributes){
        std::bitset<64> attributes_bs = 0;
        for (int att : attributes) attributes_bs[att] = 1;
        return attributes_bs.to_ullong();
    }


    void makeSignatureTest(){
        vector<int> test7{0,1,2};
        vector<int> test8{3};
        vector<int> test16{4};
        vector<int> test17{0,4};

        if (makeSignature(test7) != 7)  cout << "makeSignature failed! " << makeSignature(test7) << "!= 7" << endl;
        if (makeSignature(test8) != 8)  cout << "makeSignature failed! " << makeSignature(test8) << "!= 8" << endl;
        if (makeSignature(test16) != 16)cout << "makeSignature failed! " << makeSignature(test16) << "!= 16" << endl;
        if (makeSignature(test17) != 17)cout << "makeSignature failed! " << makeSignature(test17) << "!= 17" << endl;
    }

public:
    SubscriptionClusterTree(unsigned leaveOutRate, short levels) : Broker(), leaveOutRate(leaveOutRate), maxLevels(levels) {
    };

    void insert(IntervalSub sub) {
        /*
         * Most of the logic here is to create a signature and match it with existing roots.
         * If no roots match a new tree is created
         */

        sort(sub.constraints.begin(), sub.constraints.end(), [](IntervalCnt a, IntervalCnt b) {return a.att < b.att; });

        vector<int> attributes;
        for (auto constraint : sub.constraints) attributes.push_back(constraint.att);
        unsigned long signature = makeSignature(attributes);

        // Check to see if the structure already has subscriptions with this signature
        for (int i=0; i< signatures.size(); ++i){
            if (signatures[i] == signature){
//                cout << "Signature matched!" << endl;
//                roots.emplace_back();
                roots.at(i).insert(sub, maxLevels);
                return;
            }
        }

        // If function hasn't returned then there isn't a tree for this specific set of attributes
        // Create a new one
        signatures.push_back(signature);
        roots.emplace_back();
        roots.back().insert(sub, maxLevels);
    };

    void matchTest(){
        /*
         * This is a series of tests and verifications which can be called from the match function
         */
        makeSignatureTest();
        cout << "Stored subscriptions:" << count() << endl;
    }

    void match(const Pub &pub, int &matchSubs, const vector<IntervalSub> &subList) {
        /*
         * First find out which roots match (i.e. all attributes in the root are in the event).
         * Then convert the attributes so that only the relevant pairs are matched.
         */
//        matchTest();
//        print();

        Pub sortedPub = pub;
        sort(sortedPub.pairs.begin(), sortedPub.pairs.end(), [](Pair a, Pair b) {return a.att < b.att; });

        unsigned long eventSignature;
        {
            vector<int> eventAttributes;
            for (auto pair : sortedPub.pairs) eventAttributes.push_back(pair.att);
            eventSignature = makeSignature(eventAttributes);
//            bitset<64> eventBitset{eventSignature};
//            cout << "Event attributes: " << eventBitset.to_string() << endl;
        }


        vector<bool> matchedAttributes;
        matchedAttributes.reserve(signatures.size());
        vector<unsigned long> maskedEvents;
        maskedEvents.reserve(signatures.size());

        for (auto i = 0; i < signatures.size(); ++i) {
            matchedAttributes[i] = 0;
            maskedEvents[i] = 0;
        }

        for (auto i = 0; i < signatures.size(); ++i) {
            if (matchEventAndSubscriptionSignatures(eventSignature, signatures[i])) {
                matchedAttributes[i] = 1;
                maskedEvents[i] = (eventSignature & signatures[i]);
//                bitset<64> attributesBitset{maskedEvents[i]};
//                cout << "Attributes matched: " << attributesBitset.to_string() << endl;
            }
        }

        for (auto i = 0; i < signatures.size(); ++i) {
            // This loop creates a pseudo-event to match using the tree by discarding all pairs which are not in the
            // tree's subscriptions. It then matches the pseudo-event.

//            for (auto root : roots){
//                root.print();
//            }
//            cout << "Stored subscriptions:" << count() << endl;

            if (matchedAttributes[i]) {

                Pub             pseudoEvent;
                pseudoEvent.size = 0;
                unsigned long   maskedEvent = maskedEvents[i];  // This will get right-shifted
                int             j = 0, z = 0;                   // The current attribute we are looping on inside
                                                                // the masked event and inside the received event.
                while (maskedEvent && (z < sortedPub.pairs.size())){
                    if (maskedEvent & 1){
                        while(j > sortedPub.pairs[z].att && (z < sortedPub.pairs.size())){
                            ++z;
                        }
                        pseudoEvent.pairs.push_back(sortedPub.pairs[z]);
                        ++pseudoEvent.size;
                    }
                    ++j;
                    maskedEvent >>= 1;
                }
                Pub const& const_pseudoEvent = pseudoEvent;
//                cout << "Size of pseudo-event is: " << pseudoEvent.size << endl;
                roots[i].match(const_pseudoEvent, matchSubs);
            }
            }
//        print();
        }

    void printInner(NodeList* root){
        /*
         * In case it is not clear, this is basically a textbook BFS.
         */
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
            int nodesWithChildren = 0;
            for (auto node_ : node->nodes){
                if (node_.hasChildren()){
                    ++nodesWithChildren;
                    q.push(node_.child);
//                    if (!q.empty()) q.push(nullptr);
                }
            }
            cout << "Level: " << node->level << ". Nodes with children: " << nodesWithChildren << endl;
            node->print();

        }
        cout << endl;
        cout << endl;
    }

    int count(){
        int amount=0;
        for (auto root: roots){
            root.count(amount);
        }
        return amount;
    }

    void print(){
        cout << "Total subscriptions stored: " << count() << endl;
        for (auto root: roots) printInner(&root);
    }


    ;

};
#endif //PUBSUB_SCTREE_H