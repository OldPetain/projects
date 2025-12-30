#include "link.h"

//定义LinkList类
LinkList::LinkList() : head(nullptr), tail(nullptr) {}

//析构函数
LinkList::~LinkList() {
    Node* curr = head;
    while (curr) {
        Node* temp = curr;
        curr = curr->next;
        delete temp;
    }
}

//插入一个节点
void LinkList::insert(int val) {
    //创建新节点
    Node* newNode = new Node(val);
    //如果链表为空
    if (!head) head = tail = newNode;
    else {
        tail->next = newNode;//将新节点链接到尾节点
        tail = newNode;//更新尾节点
    }
}

//将other链表追加到当前链表后面
void LinkList::appendList(const LinkList& other) {
    //如果链表为空
    Node* curr = other.head;
    //遍历链表
    while (curr) {
        insert(curr->data);
        curr = curr->next;
    }
}

//显示链表
void LinkList::display() const {
    Node* curr = head;
    while (curr) {
        std::cout << curr->data << " ";
        curr = curr->next;
    }
    std::cout << std::endl;
}
