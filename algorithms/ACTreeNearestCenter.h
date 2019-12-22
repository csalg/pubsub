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
    vector<int> lo, hi, center;
    int width = 0;

    Node() {};

    Node(IntervalSub &sub){
        lo.reserve(sub.constraints.size());
        hi.reserve(sub.constraints.size());
        center.reserve(sub.constraints.size());

        for (auto constraint : sub.constraints) lo.push_back(constraint.lowValue);
        for (auto constraint : sub.constraints) hi.push_back(constraint.highValue);
        for (auto constraint : sub.constraints) center.push_back(constraint.lowValue+(constraint.highValue - constraint.lowValue)/2);
        for (auto constraint : sub.constraints) width += constraint.highValue - constraint.lowValue;
    }

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

    Node(Node &node1, Node &node2);

    bool hasChildren(){ return child != nullptr; }

    void print();

    bool match(const Pub &pub);
    bool match(IntervalSub &sub);

    bool match(const vector<int> &pub);
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

    void insert(Node &node, short maxLevel, unsigned short maxSpan, unsigned short spanAfterClustering, unsigned short k, unsigned short leaveOutRate){
        if (nodes.size() >= MAX_SPAN && (level < maxLevel)) {
            this->cluster(maxSpan, spanAfterClustering, k, leaveOutRate);
        }
        push(node);
    }

    void insert(NodeList &newNodes, unsigned short maxSpan, unsigned short spanAfterClustering, unsigned short k, unsigned short leaveOutRate){
        for (auto node : newNodes.nodes){
            nodes.push_back(node); // Node is copied
        }

        if (nodes.size() >= MAX_SPAN) {
            this->cluster(maxSpan, spanAfterClustering, k, leaveOutRate);
        }
    }

    void insert(IntervalSub &sub, short maxLevels, unsigned short maxSpan, unsigned short spanAfterClustering, unsigned short k, unsigned short leaveOutRate){
        if ((nodes.size() <= spanAfterClustering) || (level >= maxLevels)){
            push(sub);
            return;
        }
//
//        bool found = 0;
//        unsigned short nearest_center = 0;
//        long double distance = INFINITY;

        for (auto j=0; j!= nodesWithChildren.size(); ++j) {
            size_t i = nodesWithChildren[j];
            if (nodes.at(i).match(sub)) {
//                cout << "Inserting " << this <<  " into " << &nodes.at(i) <<endl;
                nodes.at(i).child->insert(sub, maxLevels, maxSpan, spanAfterClustering, k, leaveOutRate);
                return;
            }
        }

//                        Old code looked at all possible places to fit the thing.
//                    auto new_distance =dist(nodes.at(i), sub);
//                    if (new_distance < distance) {
//                        distance = new_distance;
//                        nearest_center=i;
//                        found=1;
//                    }
//            }
//        }

//        if (found){
//            nodes.at(nearest_center).child->insert(sub);
//            return;
//        }
//        cout << nodes.size() << " " << maxSpan << " " << (nodes.size() > maxSpan) << endl;
//        for (auto node : nodes) node.print();

        if (nodes.size() > maxSpan) this->cluster(maxSpan, spanAfterClustering, k, leaveOutRate);

        push(sub);
    }

    long double dist(Node &a, Node &b){
        unsigned sum = 0;

        for (unsigned i = 0; i != a.center.size(); i++)
            sum += pow((a.center[i] - b.center[i]),2);

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

    void match(vector<int> pub, int &matchSubs) {
        // A modified BFS
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
//                        currentNode->nodes[i].print();vi
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

    void cluster(unsigned short maxSpan, unsigned short spanAfterClustering, unsigned short k, unsigned short leaveOutRate);
};

bool Node::match(const vector<int> &pub) {
    for (auto i=0; i != lo.size(); ++i) if (pub[i] < lo[i]) return 0;
    for (auto i=0; i != hi.size(); ++i) if (pub[i] > hi[i]) return 0;
    return 1;
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

void merge(Node &center, Node &node, short level){


    if (!center.hasChildren() && !node.hasChildren()){
        // Both are simply subscriptions.
        Node temp = center;
        center.child = new NodeList;
        center.child->push(temp);
        center.child->push(node);
        center.child->level=level+1;
        center.expand(node);
        return;
    } else if (!(node.hasChildren())) {
        // The node is a subscription
        center.child->push(node);
        center.expand(node);
        return;
    } else if (!(center.hasChildren())) {
        // The center is a subscription
        center.child = new NodeList;
        for(auto node_ : node.child->nodes){
            center.child->push(node_);
        }
        center.child->level=level+1;
        center.expand(node);
        delete node.child;
        return;
    } else {
        // Neither are subcriptions: merge both node lists.
        for(auto node_ : node.child->nodes){
            center.child->push(node_);
        }
        center.expand(node);
//        for(auto node_ : node.child->nodes){
//            center.expand(node_);
//        }
        delete node.child;
    }
}

//
//Node *merge(Node &node1, Node &node2, short level, unsigned short maxSpan, unsigned short spanAfterClustering, unsigned short k, unsigned short leaveOutRate){
//    if (!node1.hasChildren() && !node2.hasChildren()){
//        // Both are simply subscriptions.
//        Node* node = new Node(node1,node2);
//        node->child->level=level+1;
//        return node;
//    }
//    else if (!(node1.hasChildren())) {
//        // One of them is a subscription.
//        node2.expand(node1);
//        node2.child->insert(node1, level, maxSpan, spanAfterClustering, k, leaveOutRate);
//
//        return &node2;
//    }
//    else if (!(node2.hasChildren())) {
//        // The other one is a subscription.
//        node1.expand(node2);
//        node1.child->insert(node1, level, maxSpan, spanAfterClustering, k, leaveOutRate);
//
//        assert(node1.lo.size()!=0);
//        return &node1;
//    }
//    else {
//        // Neither are subcriptions: merge both node lists.
//
//        // Let's quickly check that we're not trying to merge a node with itself
//        if (node1.child == node2.child) return &node1;
//
//        // We will push any children in the smaller NodeList into the larger one.
//        auto maxSize = std::max(node1.child->size(), node2.child->size());
//        Node &bigNode = node1.child->size() == maxSize ? node1 : node2;
//        Node &smallNode = &bigNode==&node1 ? node2 : node1;
//
////        cout << &node1 << endl;
////        cout << &node2 << endl;
////        cout << &bigNode << " " << &bigNode.child << endl;
////        bigNode.print();
////        cout << &smallNode  << " " << &smallNode.child << endl;
////        smallNode.print();
//        bigNode.expand(smallNode);
////        cout << "bigNode expanded" << endl;
////        bigNode.print();
////
////        for (auto node : smallNode.child->nodes){
////            bigNode.expand(node);
////        }
////        cout << "bigNode expanded again, let's see" << endl;
////        bigNode.print();
//
//        bigNode.child->insert(node1, level, maxSpan, spanAfterClustering, k, leaveOutRate);
////        cout << " doesn't happen here" << endl;
////        delete &smallNode;
//        return &bigNode;
//    }
//}
//

void NodeList::cluster(unsigned short maxSpan, unsigned short spanAfterClustering, unsigned short k, unsigned short leaveOutRate) {

    // Sort nodes by width.
    sort(nodes.begin(), nodes.end(), [](Node a, Node b) { return a.width < b.width; });
//
//    cout << "Nodes before clustering: " << nodes.size() << endl;
//    for (auto node : nodes) node.print();

    // How many nodes are we actually considering?
    const unsigned len = this->nodes.size() - leaveOutRate;

    // Bitset to mark selected centers.
    vector<bool> selectedCenters(len, 0);

    // An array to hold the distance and nearest center for each node
    struct NodeInfo {
        unsigned short position = 0;
        long double distance = 0;
        unsigned short center = 0;

        NodeInfo(unsigned short position, long double distance, unsigned short center) :
                position(position), distance(distance), center(center) {};
    };

    vector<NodeInfo> distanceAndCenters;

    // Pick a center at random and assign all other nodes to this centre as its closest
    // Set distances to infinity.
    unsigned nextCenter = rand() % len;
    long double furthestDistance;

    for (auto i = 0; i != len; i++) distanceAndCenters.emplace_back(i, INFINITY, nextCenter);

    selectedCenters[nextCenter] = true;
//    Timer centersStart;

    // This is the main loop to calculate distances and centres as per the k-means algorithm
    for (auto i = 1; i != k; ++i) {
        // Loop starts at 1 because we already assigned the first centre at random
        furthestDistance = 0; // Reset furthest distance
        unsigned currentCenter = nextCenter; // Holds whichever centre has been found to be furthest from all the other nodes

        for (auto j = 0; j != len; j++) {
            if (!selectedCenters[j]) {
                // Recalculate all distances for nodes not marked as centers
                auto newDistance = dist(nodes.at(j), nodes.at(currentCenter));

                if (distanceAndCenters[j].distance > newDistance) {
                    // If previous distance to some other center was larger then reassign to this new centre
                    distanceAndCenters[j].distance = newDistance;
                    distanceAndCenters[j].center = currentCenter;
                }

                if (distanceAndCenters[j].distance > furthestDistance) {
                    // If the distance from this node to the nearest centre is larger than our current recorded furthestDistance
                    // then this should be the next selected center
                    furthestDistance = distanceAndCenters[j].distance;
                    nextCenter = j;
                }
            }
        }
        selectedCenters[nextCenter] = true;
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
//
//    cout << "Distances and centers array sorted in ascending order" << endl;
//    for (int i=0; i != len; i++ ) cout << "(" << distance_and_centers[i].first << ", " << distance_and_centers[i].second << "), ";
//    cout << endl;

//    cout << "Nearest center array" << endl;
//    for (int i=0; i != len; i++ ) cout << nearest_center[i] << " ";
//    cout << endl;

//    Timer memoryT;

    // At this point, we need to decide which nodes get merged with their closest center and which don't.
    // This was a bit tricky to get right, hence the detailed comments

    vector<Node> newNodes; // This is the array where the nodes will live after the clustering
    newNodes.reserve(maxSpan);

    vector<bool> nodesToBeReallocated(nodes.size(), 0);    // Marks the nodes which will be copied to the newNodes array

    // Firstly, whichever nodes are too large to get clustered get to go in the new array
    // This is just following the leave-out rule
//    for (auto i = len; i != nodes.size(); ++i) nodesToBeReallocated[i] = true;

    // Likewise, the centers will be in the newNodes vector
    for (auto i = 0; i != len; ++i) nodesToBeReallocated[i] = selectedCenters[i];
//
//    // Now, some of the nodes which are furthest away from the nearest center will not be merged
//    // Calculating how many of those is simple enough. First, we find out how many we actually need
//    // to make that spanAfterClustering number
//    unsigned short numberOfNodesToKeepWhichAreUnclustered = spanAfterClustering - (leaveOutRate + k);
//
//    // Sort on the distances and we also mark the number of nodes to keep which are unclustered.
//    sort(distanceAndCenters.begin(), distanceAndCenters.end(),
//         [](NodeInfo a, NodeInfo b) { return a.distance < b.distance; });
//
//    {
//        int i = 0;
//        while (numberOfNodesToKeepWhichAreUnclustered > 0) {
//            if (!nodesToBeReallocated[distanceAndCenters[i].position]){
//                nodesToBeReallocated[distanceAndCenters[i].position] = true;
//                --numberOfNodesToKeepWhichAreUnclustered;
//            }
//            ++i;
//        }
//    }

    // Now the nodesToBeReallocated bitset is completely marked, so any node that is not marked gets merged with
    // its nearest center
    for (auto i=0; i != len; ++i ){
        if (!nodesToBeReallocated[distanceAndCenters[i].position]) {
            merge(nodes.at(distanceAndCenters[i].center), nodes.at(distanceAndCenters[i].position), this->level);
        }
    }

//    unsigned short numberOfNodesToBeReallocated = 0;
//
//    for (auto i=0; i!= nodesToBeReallocated.size(); ++i) {
//        // The largest nodes are reallocated because of the leave-out rule
//        // The centers are also allocated
//        if ((i >= len) || (selectedCenters[i])) {
//            nodesToBeReallocated[i] = true;
//            ++numberOfNodesToBeReallocated;
//    }
//
////    cout << "nodesToBeReallocated before" << endl;
////    for (auto center : nodesToBeReallocated) cout << center;
////    cout << endl;
//
//    for (auto i = 0; i!=selectedCenters.size(); i++){
//        if (numberOfNodesToBeReallocated == spanAfterClustering) break;
//
//        // Centers are by definition new nodes.
//        if ((!selectedCenters[i]) && (numberOfNodesToBeReallocated < spanAfterClustering)){
//
//            cout << "i: " << i << ". Setting center: "  << distanceAndCenters[i].second << endl;
//            cout << nodes.size() << " " << numberOfNodesToBeReallocated << endl;
//
//            nodes.at(distanceAndCenters[i].second) = *(merge(nodes[i], nodes[distanceAndCenters[i].second], level, maxSpan, spanAfterClustering, k, leaveOutRate));
//            nodesToBeReallocated.at(i) = 0;
//            numberOfNodesToBeReallocated++;
////            cout << numberOfNodesToBeReallocated << endl;
//            continue;
//        }
//        nodesToBeReallocated.at(i) = 1;
//
////        if (centers[i]) nodesToBeReallocated.at(i) = 1;
//    }
//    cout << "nodesToBeReallocated after" << endl;
//    for (auto center : nodesToBeReallocated) cout << center;
//    cout << endl;
    nodesWithChildren.clear();

    for (auto i = 0; i!=nodes.size(); i++) {
        if (nodesToBeReallocated[i]){
//            cout << i << " " << this << " " << nodes.at(i).child << endl;
//            cout << "Assigning " << i << endl;
            if (this != nodes.at(i).child ) {
                newNodes.push_back(nodes.at(i));
                if (nodes.at(i).hasChildren()) {
                    assert(nodes.at(i).child != this);
                    nodesWithChildren.push_back(newNodes.size() - 1);
                }
            }
        }
    }

//    cout << "nodesWithChildren: ";
//    for (auto parent : nodesWithChildren) cout << parent << " ";
//    cout << endl;

//    cout << "Here are the new nodes:" << endl;
//    for (auto node : newNodes) node.print();
//
    nodes.clear();
    nodes.reserve(maxSpan);
    nodes.insert(nodes.begin(),newNodes.begin(), newNodes.end());
    std::vector<Node>().swap(newNodes);
//    cout << "Memory operations took " << centersStart.elapsed_nano()/1000000 << "ms." << endl;
//
//    cout << "Finished successfully. K was " << k << endl;
//    cout << "nodesToBeReallocated: ";
//    for (auto node : nodesToBeReallocated) cout << (node ? 1 : 0);
//    cout << endl;
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
    unsigned short    leaveOutRate;
    unsigned short    maxLevels;
    unsigned short    maxSpan;
    unsigned short    spanAfterClustering;
    unsigned short    k;

    vector<unsigned long> signatures;
    vector<NodeList> roots;

    bool matchEventAndSubscriptionSignatures(unsigned long eventSignature, unsigned long subscriptionSignature){
        /*
         * Returns true if all the attributes in the subscription signature are found in the event signature
         */
        unsigned long masked_event = eventSignature & subscriptionSignature;
        return masked_event == subscriptionSignature;
    }

    void matchEventAndSubscriptionSignaturesTest(){
        /*
         * Test by calling this function from the match method
         */
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
    SubscriptionClusterTree(unsigned leaveOutRate, unsigned short maxLevels, unsigned short maxSpan) : Broker(),
    leaveOutRate(leaveOutRate), maxLevels(maxLevels) {
        this->maxSpan=maxSpan;
        spanAfterClustering = maxSpan/2;
        k = maxSpan/4;
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
                roots.at(i).insert(sub, maxLevels, maxSpan, spanAfterClustering, k, leaveOutRate);
                return;
            }
        }

        // If function hasn't returned then there isn't a tree for this specific set of attributes
        // Create a new one
        signatures.push_back(signature);
        roots.emplace_back();
        roots.back().insert(sub, maxLevels, maxSpan, spanAfterClustering, k, leaveOutRate);
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
        vector<vector<int>> pseudoEvents;
        pseudoEvents.reserve(signatures.size());

        for (auto i = 0; i < signatures.size(); ++i) {
            // This loop creates a pseudo-event to match using the tree by discarding all pairs which are not in the
            // tree's subscriptions. It then matches the pseudo-event.

//            if (matchedAttributes[i]) {

            vector<int> pseudoEvent;
            unsigned long maskedEvent = maskedEvents[i];  // This will get right-shifted
            int j = 0, z = 0;   // The current attribute we are looping on inside
                                // the masked event and inside the received event.
            while (maskedEvent && (z < sortedPub.pairs.size())) {
                if (maskedEvent & 1) {
                    while (j > sortedPub.pairs[z].att && (z < sortedPub.pairs.size())) {
                        ++z;
                    }
                    pseudoEvent.push_back(sortedPub.pairs[z].value);
                }
                ++j;
                maskedEvent >>= 1;
            }
            pseudoEvents.push_back(pseudoEvent);
        }

        for (auto i = 0; i != pseudoEvents.size(); ++i) {
//            cout << "This thing doesn't run" << endl;
//            Pub const& const_pseudoEvent = pseudoEvents.at(i);
//            for (auto pair : const_pseudoEvent.pairs) cout << pair.att << ": " << pair.value << ", ";
//            cout << endl;
            roots[i].match(pseudoEvents.at(i), matchSubs);
        }



//        cout << "Matched: " << matchSubs << endl;
//        cout << "Subscriptions stored: " << count() << endl;
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
        cout << "Total subscriptions stored: " << count() << endl;
    };

};
#endif //PUBSUB_SCTREE_H