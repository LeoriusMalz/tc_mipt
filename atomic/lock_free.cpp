#include <atomic>
#include <memory>

template <typename T>
class LockFreeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next;

        Node(const T& value) : data(std::make_shared<T>(value)), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    std::atomic<Node*> toDelete;

public:
    LockFreeQueue() : head(nullptr), tail(nullptr), toDelete(nullptr) {}

    ~LockFreeQueue() {
        while (Node* node = head.load()) {
            head.store(node->next);
            delete node;
        }

        // Освобождение памяти удаленных узлов
        while (Node* node = toDelete.load()) {
            toDelete.store(node->next);
            delete node;
        }
    }

    void Push(const T& value) {
        Node* newNode = new Node(value);
        newNode->next = nullptr;

        Node* currTail = tail.exchange(newNode);
        currTail->next = newNode;
    }

    std::shared_ptr<T> Pop() {
        Node* currHead = head.load();

        while (currHead && !head.compare_exchange_weak(currHead, currHead->next)) {
            // Потоку не удалось обновить указатель head, попробуем снова
        }

        if (currHead) {
            // Добавляем удаленный узел в список для освобождения памяти
            currHead->next = toDelete.load();
            while (!toDelete.compare_exchange_weak(currHead->next, currHead))
                ;  // Ожидаем успешного обновления списка
            return currHead->data;
        }

        return nullptr;
    }
};
