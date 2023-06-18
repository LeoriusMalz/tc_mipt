#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>

// Потокобезопасная очередь
template<typename T>
class ThreadSafeQueue {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
    };

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;

public:
    ThreadSafeQueue() : head_(nullptr), tail_(nullptr) {}

    ~ThreadSafeQueue() {
        Node* current = head_.exchange(nullptr);
        while (current) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }

    void Push(const T& value) {
        Node* newNode = new Node(value);
        newNode->next = nullptr;
        Node* prevTail = tail_.exchange(newNode);
        if (prevTail) {
            prevTail->next = newNode;
        } else {
            head_.store(newNode);
        }
    }

    bool Pop(T& value) {
        Node* currentHead = head_.load();
        if (currentHead) {
            head_.store(currentHead->next);
            value = currentHead->data;
            delete currentHead;
            return true;
        }
        return false;
    }
};

// Глобальная потокобезопасная очередь строк
ThreadSafeQueue<std::string> globalQueue;

// Первый поток: чтение из файла и добавление в очередь
void ReadFileAndEnqueue(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                globalQueue.Push(line);
            }
        }
        file.close();
    } else {
        std::cout << "Error opening file: " << filename << std::endl;
    }
}

// Второй поток: извлечение из очереди, преобразование в заглавные буквы и вывод на экран
void DequeueAndPrint() {
    std::string line;
    while (globalQueue.Pop(line)) {
        // Преобразование в заглавные буквы
        for (char& c : line) {
            c = std::toupper(c);
        }
        std::cout << "Processed line: " << line << std::endl;
    }
}

int main() {
    // Создание и запуск двух потоков
    std::thread readerThread(ReadFileAndEnqueue, "input.txt");
    std::thread printerThread(DequeueAndPrint);

    // Ожидание завершения работы потоков
    readerThread.join();
    printerThread.join();

    return 0;
}
