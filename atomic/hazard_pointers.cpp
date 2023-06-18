#include <atomic>
#include <memory>
#include <thread>

template <typename T>
class HazardPointers {
private:
    struct HazardPointer {
        std::atomic<std::thread::id> id;
        std::atomic<void*> pointer;

        HazardPointer() : id(std::thread::id()), pointer(nullptr) {}
    };

    static constexpr int maxThreads = 100;
    static constexpr int maxHazards = 2;

    HazardPointer hazards[maxThreads * maxHazards];

public:
    void* Get() {
        const std::thread::id id = std::this_thread::get_id();
        for (int i = 0; i < maxHazards; ++i) {
            if (hazards[i].id == id) {
                return hazards[i].pointer.load();
            }
        }
        return nullptr;
    }

    void Set(void* pointer) {
        const std::thread::id id = std::this_thread::get_id();
        for (int i = 0; i < maxHazards; ++i) {
            if (hazards[i].id == id) {
                hazards[i].pointer.store(pointer);
                return;
            }
        }

        for (int i = 0; i < maxHazards; ++i) {
            std::thread::id oldId;
            if (hazards[i].id.compare_exchange_strong(oldId, id)) {
                hazards[i].pointer.store(pointer);
                return;
            }
        }

        // Достигнуто максимальное количество указателей опасности
        // Нужно либо увеличить maxHazards, либо обработать эту ситуацию
        throw std::runtime_error("Exceeded maximum number of hazard pointers per thread");
    }

    void Clear() {
        const std::thread::id id = std::this_thread::get_id();
        for (int i = 0; i < maxHazards; ++i) {
            if (hazards[i].id == id) {
                hazards[i].pointer.store(nullptr);
                hazards[i].id.store(std::thread::id());
                return;
            }
        }
    }
};

template <typename T>
class LockFreeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next;

        Node(const T& value) : data(std::make_shared<T>(value)), next(nullptr) {}
    };

    struct RetiredNode {
        Node* node;
        HazardPointers<Node>& hazardPointers;

        RetiredNode(Node* n, HazardPointers<Node>& hp) : node(n), hazardPointers(hp) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    HazardPointers<Node> hazardPointers;

public:
    LockFreeQueue() : head(nullptr), tail(nullptr) {}

    ~LockFreeQueue() {
        while (Node* node = head.load()) {
            head.store(node->next);
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
        Node* currHead = nullptr;
        HazardPointers<Node>& hp = hazardPointers;

        do {
            currHead = head.load();
            hp.Set(currHead);

            if (currHead != head.load()) {
                // head был обновлен другим потоком
                hp.Clear();
            } else {
                Node* nextNode = currHead->next;
                if (!nextNode) {
                    hp.Clear();
                    return nullptr;
                }

                if (head.compare_exchange_strong(currHead, nextNode)) {
                    hp.Clear();
                    std::shared_ptr<T> result = currHead->data;
                    hp.Set(currHead);
                    RetiredNode retiredNode(currHead, hp);
                    delete retiredNode.node;  // Освобождаем удаленный узел
                    hp.Clear();
                    return result;
                }

                hp.Clear();
            }
        } while (true);
    }
};
