//
// Created by Work on 2019-08-09.
//

#ifndef BST_CENTEREDINTERVALTREE_H
#define BST_CENTEREDINTERVALTREE_H

#include <iostream>
//#include <map>
#include <iterator>
#include <queue>
#include <map>
#include "../../common/data_structure.h"

template<typename T>
struct CenteredIntervalTree {
    struct Node {
        Node *left = nullptr, *right = nullptr, *parent = nullptr;
        int height = 0;
        int center;
        multimap<int,std::pair<int, T>> loMap;
        multimap<int,std::pair<int, T>> hiMap;

        Node() = default;

        Node(int lo, int hi, T data){
            center = (lo+hi)/2;
            insert(lo,hi,data);
        };

        void insert(int lo, int hi, T data){
//            static auto loPair = make_pair(lo, data);
//            static auto hiPair = make_pair(hi, data);
            if (hi<lo) hi=lo;
            loMap.insert(make_pair(lo, make_pair(hi, data)));
            hiMap.insert(make_pair(hi, make_pair(lo, data)));
        };


        void print() {
            std::cout << this << " Parent: " << this->parent <<" C: " << center << ", H: " << height << ", #Data: " << loMap.size() << ", Elements: lo:";
            for (auto it=loMap.begin(); it!=loMap.end();it++){
                cout << it->first << " => ("<< it->second.first << ", " << &(it->second.second )<< ") ";
            }
            cout << " hi: ";
            for (auto it=hiMap.begin(); it!=hiMap.end();it++){
                cout << "("<< it->second.first << ", " << &(it->second.second) << ") ";
            }
            cout << "  |  ";
        }
    };

    Node* root = nullptr;
    int size = 0;

    CenteredIntervalTree() = default;
    CenteredIntervalTree(int lo, int hi, T data) {
        root = new Node(lo,hi,data);
    }


//    Node *rotateRight(Node* &root){
//        static Node * newRoot = root->left;
//        root->left = newRoot->right;
//        root->height = std::max(height(root->left), height(root->right)) + 1;
//        newRoot->height = std::max(height(newRoot->left), height(newRoot->right)) + 1;
//        return newRoot;
//    }

    Node *rotateLeft(Node* &root) {
//        std::cout << "L" << std::endl;
        Node *newRoot = root->right;

        root->right = newRoot->left;
        newRoot->left = root;

        // We also need to set the parents.
        newRoot->parent = root->parent;
        root->parent = newRoot;

//        std::cout << newRoot << " " << root << " " << (newRoot->left) << std::endl;

        root->height = std::max(height(root->left), height(root->right)) + 1;
        newRoot->height = std::max(height(newRoot->left), height(newRoot->right)) + 1;



        // Anything that is in root and whose lo is lower than or eq to newRoot's should be moved to root.
        typename std::multimap<int,pair<int,T>>::iterator it,itlow,itup;

        itlow = root->hiMap.lower_bound(newRoot->center);
        itup = root->hiMap.end();

        vector<int> futureDel;

        for ( it=itlow; it!=itup; ++it) {
//            std::cout << it->first << " => " << (it->second).second << '\n';
            newRoot->insert(it->second.first, it->first, it->second.second );
            futureDel.push_back(it->second.first);
        }
        root->hiMap.erase(itlow,itup);
        for (int loNum : futureDel) {
//            cout << "Deleting: " << loNum << endl;
            root->loMap.erase(loNum);
        }


        return newRoot;
    }



    Node *rotateRight(Node* &root) {
//        std::cout << "R" << std::endl;
        Node *newRoot = root->left;
        root->left = newRoot->right;
        newRoot->right = root;

        // We also need to set the parents.
        newRoot->parent = root->parent;
        root->parent = newRoot;

        root->height = std::max(height(root->left), height(root->right)) + 1;
        newRoot->height = std::max(height(newRoot->left), height(newRoot->right)) + 1;




        // anything that is in root and whose hi is greater than or eq to newRoot's should be moved to root.
        typename std::multimap<int,pair<int,T>>::iterator it,itlow,itup;

        itlow = root->loMap.begin();
        itup = root->loMap.lower_bound(newRoot->center);

//        if (itlow != itup){
//            if (itlow->first > newRoot->center) itlow--;
//        }

        vector<int> futureDel;

        for ( it=itlow; it!=itup; ++it) {
            if (it->first < newRoot->center) {
//                std::cout << "Moving to:";
//                newRoot->print();
//                cout << endl;
//                std::cout << "From:";
//                root->print();
//                cout << endl;

//                std::cout << it->first << " => " << it->second.first << ", " << it->second.second << endl;
                newRoot->insert(it->second.first, it->first, it->second.second);
                futureDel.push_back(it->second.first);
            }
        }
        root->loMap.erase(itlow,itup);

        for (int hiNum : futureDel) {
//            cout << "Deleting: " << loNum << endl;
            root->hiMap.erase(hiNum);
        }

        return newRoot;
    }

    int height(Node *node) {
        return (node == nullptr) ? 0 : node->height;
    }

    Node *insertInner(Node* &parent, Node* &node, int lo, int hi, T &data) {
        // Base case.
        if (node == nullptr) {
//            then we create a new node because no previous node overlapped.
            auto newNode = new Node(lo,hi,data);
//            cout << node << " " << parent << endl;
            newNode->parent = parent;
            Node* previousParent = parent;
            while (previousParent != nullptr) {
                (previousParent->height)++;
                previousParent = previousParent->parent;
                if (previousParent == root){
                    break;
                }
            } ;
            return newNode;
        };

        // The interval contains the center of the node. Then we just insert the interval into this node.
        if( lo <= node->center && hi >= node->center){
            node->insert(lo,hi,data);
            return node;
        }

        // Otherwise go to left or right.
        if (hi < node->center) node->left = insertInner(node,node->left,lo,hi, data);
        else node->right = insertInner(node,node->right,lo,hi, data);

//        Now we balance like a normal AVL tree.
        int balance = height(node->left) - height(node->right);
//        cout << "balance: " << balance << endl;
        if (balance > 1) {
//            cout << "rotating";
            if (height((node->left)->left) >= height((node->left)->right)) {
                return rotateRight(node);
            } else {
                node->left = rotateLeft(node->left);
                return rotateRight(node);

            };
        }
        if (balance < -1) {
//            cout << "rotating";
            if (height((node->right)->right) >= height((node->right)->left)) {
                return rotateLeft(node);
            } else {
                node->right = rotateRight(node->right);
                return rotateLeft(node);
            }
        }

        node->height = 1 + std::max(height(node->left), height(node->right));
        return node;

    }

    void insert(int lo, int hi,T &data){
        size++;
        root = insertInner(root,root, lo, hi, data);
    }


    void printInner(Node* &root){
        queue<Node*> q;
        q.push(root);
        q.push(nullptr);

        while (!q.empty()){
            Node* node = q.front();
            q.pop();
            if (node == nullptr){
                cout << endl;
                if (!q.empty()) q.push(nullptr);
                continue;
            }
            if (node->left != nullptr) q.push(node->left);
            if (node->right !=nullptr) q.push(node->right);
            node->print();
        }
    }

    void print(){
        printInner(root);
    }



    bool matchAPubWithASub(const Pub &pub,  IntervalSub &sub){
//        cout << "Possible match! " << endl;
//        cout << "Val:  " << pub.pairs[0].value << ". lo: " << sub.constraints[0].lowValue << ". hi: " << sub.constraints[0].highValue << endl;
        for (auto constraint : sub.constraints){
//            cout << "Val:  " << pub.pairs[constraint.att].value << ". lo: " << constraint.lowValue << ". hi: " << constraint.highValue << endl;
            if (   pub.pairs[constraint.att].value < constraint.lowValue \
                || pub.pairs[constraint.att].value > constraint.highValue)
                return false;
        }
//        cout<< "Found match!" << endl;
//        cout << "Looking pub" << endl;
//        for (auto pair : pub.pairs){
//            cout << "att: " << pair.att << ", value " << pair.value << " / ";
//        }
//
//        cout << endl;
//
//        cout << "Looking inside sub " << &sub << endl;
//
//        for (auto constraint : sub.constraints){
//            cout << "att " << constraint.att << ", lo: " << constraint.lowValue << ", hi: " << constraint.highValue << " / ";
//        }
//
//        cout << endl;
//

        return true;
    }
    void matchInner(Node* &root,int val, const Pub &pub, int &matchSubs) {
//        cout << "DebuggingInner: " ;
//        root->print();

        if (root == nullptr) return;
//        root->print();
//        cout << endl;

        if (root->center == val) {
            // all intervals are a match and no need to continue traversing.

            for (auto it=root->hiMap.begin(); it!=root->hiMap.end(); ++it) {
//                subsStore[it->second.second];

                if (matchAPubWithASub(pub,it->second.second)) matchSubs++;
//                matchSubs++;
            }

        } else if (root->center > val){
            // look in the loMap for anything that starts before val.
            // recurse to root->left

//            auto itlow = (root->loMap.lower_bound(root->center));
//            if (itlow->first > val) itlow--;
            for (auto it=root->loMap.begin(); it!=root->loMap.lower_bound(root->center); ++it) {
//                subsStore[it->second.second];
//                cout << "Looking in lo" << endl;
//                cout << (it->first) << " " << (it->second.first) << " ";
//                cout << it->second.second.id << endl;
//                cout << (it->second.second.constraints[0]).lowValue << endl;
//                cout << (it->second.second.constraints[0]).highValue << endl;
                if (matchAPubWithASub(pub,it->second.second)) matchSubs++;
//                matchSubs++;
            }

            return matchInner(root->left,val,pub, matchSubs);

        } else {
            // look in the hiMap for anything that ends after val
            // recurse to root->right
            for (auto it=root->hiMap.lower_bound(root->center); it!=root->hiMap.end(); ++it) {
//                subsStore[it->second.second];
                if (matchAPubWithASub(pub,it->second.second)) matchSubs++;
//                matchSubs++;
            }
            return matchInner(root->right,val,pub, matchSubs);

        }
    }

    void match(int val, const Pub  &pub, int &matchSubs) {
//        cout << "Debugging: " ;
        return matchInner(root, val,pub, matchSubs);
    };


    };




#endif //BST_CENTEREDINTERVALTREE_H