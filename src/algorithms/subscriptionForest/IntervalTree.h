//
// Created by Work on 2019-08-02.
//


#ifndef PUBSUB_INTERVALTREE_H
#define PUBSUB_INTERVALTREE_H

#include<iostream>
#include <vector>
#include "IntervalNode.h"

template<typename T>
struct CenteredIntervalTree {
    IntervalNode<T> *root = nullptr;
    int size = 0;

    CenteredIntervalTree() = default;

    CenteredIntervalTree(int lo, int hi, T new_element) {
        delete root;
        root = new IntervalNode<T>(lo, hi, new_element);
        size++;
    }

    IntervalNode<T> *rotateRight(IntervalNode<T>* &root){
        IntervalNode<T> * newRoot = root->left;
        root->left = newRoot->right;
        newRoot->right = root;
        root->height = std::max(height(root->left), height(root->right)) + 1;
        newRoot->height = std::max(height(newRoot->left), height(newRoot->right)) + 1;


        newRoot->max = std::max(newRoot->hi, std::max(safeMax(newRoot->left), safeMax(newRoot->right)));
        root->max = std::max(root->hi, std::max(safeMax(root->left), safeMax(root->right)));

        return newRoot;
    }

    IntervalNode<T> *rotateLeft(IntervalNode<T>* &root) {
        IntervalNode<T> *newRoot = root->right;
        root->right = newRoot->left;
        newRoot->left = root;
//        std::cout << newRoot << " " << root << " " << (newRoot->left) << std::endl;

        root->height = std::max(height(root->left), height(root->right)) + 1;
        newRoot->height = std::max(height(newRoot->left), height(newRoot->right)) + 1;

        newRoot->max = std::max(newRoot->hi, std::max(safeMax(newRoot->left), safeMax(newRoot->right)));
        root->max = std::max(root->hi, std::max(safeMax(root->left), safeMax(root->right)));
        return newRoot;
    }

    int height(IntervalNode<T> *node) {
        return (node == nullptr) ? 0 : node->height;
    }

    int safeMax(IntervalNode<T> *node) {
        return (node == nullptr) ? 0 : node->max;
    }

    IntervalNode<T> *insertInner(IntervalNode<T>* &root, int lo,int hi, T* &new_element) {
//        std::cout<< "Made it here"<< std::endl;

        auto node = new IntervalNode<T>(lo,hi,*new_element);
        if (root == nullptr) return node;

        root->max = std::max(hi, root->max);

        int comparison = root->compareTo(node);

        switch(comparison) {
            case 0:
                root->insert(*new_element);
                break;
            case 1:
                root->left = insertInner(root->left, lo, hi, new_element);
                break;
            case -1:
                root->right = insertInner(root->right, lo, hi, new_element);
                break;
        }

        int balance = height(root->left) - height(root->right);

        if (balance > 1) {
            if (height((root->left)->left) >= height((root->left)->right)) {
                return rotateRight(root);
            } else {
                root->left = rotateLeft(root->left);
                return rotateRight(root);

            };
        }
        if (balance < -1) {

            if (height((root->right)->right) >= height((root->right)->left)) {
                return rotateLeft(root);
            } else {
                root->right = rotateRight(root->right);
                return rotateLeft(root);
            }
        }

        root->height = 1 + std::max(height(root->left), height(root->right));
        return root;

    }

    void insert(int lo, int hi, T* &data){
//        cout << "inserting" << endl;
        (this->size)++;
        root = insertInner(root,lo,hi,data);
    }

    void intersectInner(IntervalNode<T> *node, int val, std::vector<T> *intersections){
        if (node == nullptr) return;
        if (node->max < val) return;

        if ((node->lo <= val) && (node->hi >= val)){
            for (auto elem : node->data){
//                std::cout << "Added: " << std::endl;
//                node->print();
                intersections->push_back(elem);
            }
        }

        if (node->left == nullptr && node->right == nullptr) return;
        if (node->right == nullptr){
            intersectInner(node->left,val,intersections);
        } else if (node->left == nullptr){
            intersectInner(node->right,val,intersections);
        } else {
            intersectInner(node->left,val,intersections);
            intersectInner(node->right,val,intersections);
        }

    }

    void intersect(int val, std::vector<T> *intersections){
        intersectInner(root, val, intersections);
    }


    bool matchAPubWithASub(const Pub &pub,  IntervalSub &sub){
//        cout << "Possible match! " << endl;
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

    void matchInner(IntervalNode<T> *node, int val, const Pub &pub, int &matchingSubscriptions){
        if (node == nullptr) return;
        if (node->max < val) return;

        if ((node->lo <= val) && (node->hi >= val)){
            for (auto sub : node->data){
                if (matchAPubWithASub(pub, sub) ) matchingSubscriptions++;
//                matchingSubscriptions++;
            }
        }
            if (node->left == nullptr && node->right == nullptr) return;
        if (node->right == nullptr){
            matchInner(node->left,val,pub,matchingSubscriptions);
        } else if (node->left == nullptr){
            matchInner(node->right,val,pub,matchingSubscriptions);
        } else {
            matchInner(node->left,val,pub,matchingSubscriptions);
            if (val >= node->lo) matchInner(node->right,val,pub,matchingSubscriptions);
        }

    }

    void match(int val, const Pub &pub, int &matchingSubscriptions){
        matchInner(root, val, pub, matchingSubscriptions);
    }


    void printInner(IntervalNode<T>* &root){
//        if (root == nullptr) throw "WTF";
        std::cout << root << " ";
//        root->print();
//
//        if (root->left != nullptr) printInner(root->left);
//        if (root->right != nullptr) printInner(root->right);
//

    }

    void print(){
        printInner(root);
//        root->print();
//        std::cout << (root->left)->height ;
//        (root->right)->print();
    }


};

#endif //PUBSUB_INTERVALTREE_H
