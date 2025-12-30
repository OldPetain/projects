#ifndef STACK_H
#define STACK_H

#include <iostream>

class Stack {
private:
    struct Node {
        int data;
        Node* next;
        Node(int d) : data(d), next(nullptr) {}
    };
    Node* topNode;

public:
    Stack();
    ~Stack();

    void push(int val);
    int pop();
    bool isEmpty() const;
    void display() const;
};

#endif
