#ifndef LINK_H
#define LINK_H

#include <iostream>

class LinkList {
private:
    struct Node {
        int data;
        Node* next;
        Node(int d) : data(d), next(nullptr) {}
    };
    Node* head;
    Node* tail;

public:
    LinkList();
    ~LinkList();

    void insert(int val);                   // 尾插
    void appendList(const LinkList& other);  // 把另一个链表的元素追加到尾部
    void display() const;
};

#endif
