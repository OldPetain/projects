#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>

class Queue {
private:
    struct Node {
        int data;
        Node* next;
        Node(int d) : data(d), next(nullptr) {}
    };
    Node* front;
    Node* rear;

public:
    Queue();
    ~Queue();

    void enqueue(int val);  // 入队
    int dequeue();          // 出队，空时返回 -1 或抛异常
    bool isEmpty() const;
    void display() const;
};

#endif
