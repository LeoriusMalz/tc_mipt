#include <mutex>
#include <iostream>

template <typename T>
class ThreadSafeQueue {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
    };

    Node* head_;
    Node* tail_;
    std::mutex head_mutex_;
    std::mutex tail_mutex_;

public:
    ThreadSafeQueue() : head_(nullptr), tail_(nullptr) {}

    ~ThreadSafeQueue() {
        while (head_) {
            Node* temp = head_;
            head_ = head_->next;
            delete temp;
        }
    }

    void Push(const T& value) {
        Node* newNode = new Node(value);
        std::lock_guard<std::mutex> tailLock(tail_mutex_);
        if (tail_) {
            tail_->next = newNode;
            tail_ = newNode;
        } else {
            std::lock_guard<std::mutex> headLock(head_mutex_);
            head_ = tail_ = newNode;
        }
    }

    bool Pop(T& value) {
        std::lock_guard<std::mutex> headLock(head_mutex_);
        if (head_) {
            value = head_->data;
            Node* temp = head_;
            head_ = head_->next;
            delete temp;
            return true;
        }
        return false;
    }

    bool IsEmpty() {
        std::lock_guard<std::mutex> headLock(head_mutex_);
        return (head_ == nullptr);
    }
};

int main() {
    ThreadSafeQueue<int> queue;

    // Пример использования
    queue.Push(1);
    queue.Push(2);
    queue.Push(3);

    int value;
    while (queue.Pop(value)) {
        std::cout << "Popped value: " << value << std::endl;
    }

    return 0;
}
