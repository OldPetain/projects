#include "stack.h"
#include <stdexcept>

Stack::Stack() : topNode(nullptr) {}

Stack::~Stack() {
    while (!isEmpty()) {
        pop();
    }
}

//向栈中压入一个值
void Stack::push(int val) {
    Node* newNode = new Node(val);
    newNode->next = topNode;
    topNode = newNode;
}
//从栈中弹出一个值
int Stack::pop() {
    if (isEmpty()) {
        throw std::runtime_error("Stack is empty");
    }
    Node* temp = topNode;
    int val = temp->data;
    topNode = topNode->next;
    delete temp;
    return val;
}
//判断栈是否为空
bool Stack::isEmpty() const {
    return topNode == nullptr;
}
//打印栈中的所有元素
void Stack::display() const {
    Node* curr = topNode;
    while (curr) {
        std::cout << curr->data << " ";
        curr = curr->next;
    }
    std::cout << std::endl;
}
