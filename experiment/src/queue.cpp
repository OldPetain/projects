#include "queue.h"
#include <stdexcept>

Queue::Queue() : front(nullptr), rear(nullptr) {}

Queue::~Queue() {
    while (!isEmpty()) {
        dequeue();
    }
}

// 向队列中添加元素
void Queue::enqueue(int val) {
    Node* newNode = new Node(val);
    if (!rear) {
        front = rear = newNode;
    } else {
        rear->next = newNode;
        rear = newNode;
    }
}

// 从队列中删除元素并返回其值
int Queue::dequeue() {
    if (isEmpty()) {
        throw std::runtime_error("Queue is empty");
    }
    Node* temp = front;
    int val = temp->data;
    front = front->next;
    if (!front) rear = nullptr;
    delete temp;
    return val;
}

// 判断队列是否为空
bool Queue::isEmpty() const {
    return front == nullptr;
}

// 打印队列中的元素
void Queue::display() const {
    Node* curr = front;
    while (curr) {
        std::cout << curr->data << " ";
        curr = curr->next;
    }
    std::cout << std::endl;
}
