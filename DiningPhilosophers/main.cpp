#include <iostream>
#include <thread>
#include <semaphore>
#include <mutex>
#include <string>

using namespace std;

// Количество философов
const int philosophersCount = 5;
// Количество требуемых поеданий у философа
const int philosopherNumOfEating = 5;
// Время в секундах, на которое философ задумывается перед тем как захотеть есть
const int philosopherThinkTimeSec = 3;
// Время в секундах, за которое философ ест
const int philosopherEatTimeSec = 2;
// Время задержки в миллисекундах после вывода текста
const int afterOutputPauseTimeMilliSec = 250;

// Мьютекс для синхронизации вывода текста
mutex outputMutex;

// Класс вилки
class Fork {

private:
    // Семафор для ожидания освобождения вилки, если она занята другим философом
    binary_semaphore takeSem = binary_semaphore(1);

public:
    void take() {
        takeSem.acquire();
    }

    void drop() {
        takeSem.release();
    }
};

// Метод синхронизированного вывода текста с переходом на новую строку
void syncPrintLine(const string &str) {
    outputMutex.lock();
    cout << str << endl;
    this_thread::sleep_for(chrono::milliseconds(afterOutputPauseTimeMilliSec));
    outputMutex.unlock();
}

// Многопоточный метод, симулирующий философа
void bePhilosopher(Fork* forks, int id) {
    // Выбор вилки с наименьшим и наибольшим номером
    int firstForkInd = id;
    int secondForkInd = (id + 1) % philosophersCount;
    if (firstForkInd > secondForkInd)
        swap(firstForkInd, secondForkInd);
    Fork &firstFork = forks[firstForkInd];
    Fork &secondFork = forks[secondForkInd];

    for (int i = 0; i < philosopherNumOfEating; ++i) {
        syncPrintLine(string("Philosopher ") + to_string(id) + " is thinking");
        this_thread::sleep_for(chrono::seconds(philosopherThinkTimeSec));

        // Философ сначала берет вилку с наименьшим номером
        syncPrintLine(
                string("Philosopher ") + to_string(id) + " wants to take first fork (fork " + to_string(id) + ")");
        firstFork.take();
        syncPrintLine(string("Philosopher ") + to_string(id) + " took first fork (fork " + to_string(id) + ")");

        // И только потом уже берет вильку с наибольшим номером
        syncPrintLine(
                string("Philosopher ") + to_string(id) + " wants to take second fork (fork " +
                to_string((id + 1) % philosophersCount) + ")");
        secondFork.take();
        syncPrintLine(string("Philosopher ") + to_string(id) + " took second fork (fork " +
                      to_string((id + 1) % philosophersCount) + ")");
        this_thread::sleep_for(chrono::seconds(philosopherEatTimeSec));
        syncPrintLine(string("Philosopher ") + to_string(id) + " has eaten");

        // Кладет сначала вилку с наибольшим номером, затем с наименьшим
        secondFork.drop();
        firstFork.drop();
    }
}

int main() {
    Fork* forks = new Fork[philosophersCount];
    thread* philosophers = new thread[philosophersCount];
    for (int i = 0; i < philosophersCount; ++i) {
        philosophers[i] = thread(bePhilosopher, forks, i);
    }
    for (int i = 0; i < philosophersCount; ++i) {
        philosophers[i].join();
    }

    delete[] forks;
    delete[] philosophers;
    cout << "Program successfully executed without deadlocks" << endl;
    return 0;
}
