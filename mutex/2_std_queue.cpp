#include <queue>
#include <mutex>
#include <iostream>

template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;

public:
    void Push(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
    }

    bool Pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!queue_.empty()) {
            value = queue_.front();
            queue_.pop();
            return true;
        }
        return false;
    }

    bool IsEmpty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
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
