#include iostream
#include fstream
#include string
#include thread
#include mutex
#include condition_variable
#include cctype

stdstring globalString;
stdmutex mtx;
stdcondition_variable cv;
bool isReady = false;

 Первый поток чтение из файла и запись строки в глобальную переменную
void ReadFileAndWrite(const stdstring& filename) {
    stdifstream file(filename);
    if (file.is_open()) {
        stdstring line;
        while (stdgetline(file, line)) {
            stdunique_lockstdmutex lock(mtx);
            globalString = line;
            isReady = true;
            cv.notify_one();
            cv.wait(lock, []{ return !isReady; });
        }
        file.close();
    } else {
        stdcout  Error opening file   filename  stdendl;
    }
}

 Второй поток ожидание строки, преобразование в заглавные буквы и вывод на экран
void WaitForStringAndPrint() {
    while (true) {
        stdunique_lockstdmutex lock(mtx);
        cv.wait(lock, []{ return isReady; });
        if (!globalString.empty()) {
             Преобразование в заглавные буквы
            for (char& c  globalString) {
                c = stdtoupper(c);
            }
            stdcout  Processed line   globalString  stdendl;
            globalString.clear();
            isReady = false;
            cv.notify_one();
        } else {
             Если строка пустая, значит потоку пора завершиться
            break;
        }
    }
}

int main() {
    stdthread readerThread(ReadFileAndWrite, input.txt);
    stdthread printerThread(WaitForStringAndPrint);

    readerThread.join();
    printerThread.join();

    return 0;
}