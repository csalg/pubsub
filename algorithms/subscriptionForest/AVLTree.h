#ifndef PUBSUB_AVLTREE_H
#define PUBSUB_AVLTREE_H

#include<algorithm>

template<typename T>
struct AVLTree {
    struct Node {
        Node *left = nullptr, *right = nullptr;
        int height = 0;
        T data;

        Node() = default;
        Node(T data) : data(data) {};
        void print(){
            std::cout << "H: " << height << " [" << data << "] Left: " << left << " Right: " << right << std::endl;
        }
    };

    Node* root = nullptr;

    AVLTree() = default;
    AVLTree(T data) {
        delete root;
        root = new Node(data);
    }

    Node *rotateRight(Node* &root){
        static Node * newRoot = root->left;
        root->left = newRoot->right;
        newRoot->right = root;
        root->height = std::max(height(root->left), height(root->right)) + 1;
        newRoot->height = std::max(height(newRoot->left), height(newRoot->right)) + 1;
        return newRoot;
    }

    Node *rotateLeft(Node* &root) {
        Node *newRoot = root->right;
        root->right = newRoot->left;
        newRoot->left = root;
//        std::cout << newRoot << " " << root << " " << (newRoot->left) << std::endl;

        root->height = std::max(height(root->left), height(root->right)) + 1;
        newRoot->height = std::max(height(newRoot->left), height(newRoot->right)) + 1;
        return newRoot;
    }

    int height(Node *node) {
        return (node == nullptr) ? 0 : node->height;
    }

    Node *insertInner(Node* &root, T data) {
//        std::cout<< "Made it here"<< std::endl;
        if (root == nullptr) {
            auto node = new Node(data);
            return node;
        };

        if (data <= root->data) root->left = insertInner(root->left, data);
        else root->right = insertInner(root->right, data);
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
                std::cout << "This case. Root is: " << root << std::endl;
                return rotateLeft(root);
            } else {
                root->right = rotateRight(root->right);
                return rotateLeft(root);
            }
        }

        root->height = 1 + std::max(height(root->left), height(root->right));
        return root;

    }

    void insert(T data){
        root = insertInner(root, data);
    }


    Node* search(Node *root, T key){

        if (root == nullptr) throw "Not found!";
        root->print();
        if (root->data == key) return root;
        else if (key > root->data) return search(root->right, key);
        else return search(root->left, key);
    }

    T search(T key){
        auto node = search(&root, key);
        std::cout << "here";
        std::cout << (node->data);
        return (node->data);
    }

    void printInner(Node* &root){
        if (root == nullptr) throw "WTF";
        std::cout << root << " ";
        root->print();

        if (root->left != nullptr) printInner(root->left);
        if (root->right != nullptr) printInner(root->right);
    }

    void print(){
        printInner(root);
    }


};

#endif //PUBSUB_AVLTREE_H
