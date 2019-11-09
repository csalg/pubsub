//
// Created by Work on 2019-08-07.
//

#ifndef PUBSUB_INTERVALNODE_H
#define PUBSUB_INTERVALNODE_H
#include <iostream>
#include "../../common/data_structure.h"


template<typename T>
struct IntervalNode {
    int lo, hi, max;
    int height = 0;

    IntervalNode *left = nullptr, *right = nullptr;
    std::vector<T> data;
    IntervalNode() = default;
    IntervalNode(int lo, int hi, T new_element) : lo(lo), hi(hi), max(hi) {
        data.push_back(new_element);
    };
    int compareTo(IntervalNode *that){
        if      (this->lo < that->lo) return -1;
        else if (this->lo > that->lo) return +1;
        else if (this->hi < that->hi) return -1;
        else if (this->hi > that->hi) return +1;
        else                          return  0;
    }
    void print(){
        std::cout << "[" << lo << ", " << hi << ", " << max << "] (";
        for (auto elem : this->data){
            std::cout << elem << " ";
        }
        std::cout << ")" << std::endl;
    }
    void insert(T new_element){
        data.push_back(new_element);
    }
};

#endif //PUBSUB_INTERVALNODE_H
