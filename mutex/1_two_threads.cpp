#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <cctype>

std::string globalString;
std::atomic<bool> isStringEmpty(true);
std::atomic<bool> finished(false);

void readLinesFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        finished = true;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            while (!isStringEmpty) {
                // Ждем, пока строка не будет пустой
            }

            globalString = line;
            isStringEmpty = false;
        }
    }

    finished = true;
}

void processString() {
    while (!finished) {
        if (!isStringEmpty) {
            for (char& c : globalString) {
                c = std::toupper(c);
            }

            std::cout << "Processed string: " << globalString << std::endl;

            globalString.clear();
            isStringEmpty = true;
        }
    }
}

int main() {
    std::string filename = "input.txt"; // Укажите путь к вашему текстовому файлу

    std::thread readerThread(readLinesFromFile, filename);
    std::thread processorThread(processString);

    readerThread.join();
    processorThread.join();

    return 0;
}
