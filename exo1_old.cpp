#include <iostream>
#include <thread>
#include <cmath>
#include <vector>
#include <mutex>
#include <functional>


using namespace std;

unsigned long long fact(int n) {
    unsigned long long fact = 1;
    for (int i = 1 ; i <= n ; ++i) {
        fact *= i;
    }
    return fact;
}

double long formule(int i) {
    return pow(-1, i) / fact(i);
}

class ThreadPool {
private:
    vector<thread> workers;
    vector<int> tasksQueue;
    double long result = 0;
    unsigned nbThreads = 4;
    mutex m;
public:
    ThreadPool() {
//        nbThreads = thread::hardware_concurrency();
        workers.resize(nbThreads);
    }

    ThreadPool(ThreadPool&&) {};

    long double getResult() const {
        return result;
    }

    void addTask(int task) {
        tasksQueue.push_back(task);
    }

    void showTasks() {
        for (int task : tasksQueue) {
            cout << task << endl;
        }
    }

    void start() {
        function<void()> searchAndExecute = [this] () {
            double long temp = 0;
            while (!tasksQueue.empty()) {
                int i = tasksQueue.back();
                tasksQueue.pop_back();
                temp = formule(i);
                m.lock();
//                cout << this_thread::get_id() << " : " << i << endl; // Affiche l'id du thread suivi du nombre pour lequel est executé le calcul
                result += temp;
                m.unlock();
            }
        };
        for (unsigned i = 0 ; i < nbThreads ; ++i) {
            workers[i] = thread(searchAndExecute);
//            workers.push_back(thread(searchAndExecute));
        }
        for (unsigned i = 0 ; i < workers.size() ; ++i) {
            if (workers[i].joinable()) {
                workers[i].join();
            }
        }
    }
};

double long eMoinsUnSequentiel() {
    int n = 50;
    double long result = 0;
    for (int i = 0 ; i < n ; ++i) {
        result += formule(i);
    }
    return result;
}

int main() {
    int n = 50;
    ThreadPool threadPool = ThreadPool();
    for (int i = 0 ; i < n ; ++i) {
        threadPool.addTask(i);
    }
    threadPool.start();
    cout << "Calcul séquenciel : " << eMoinsUnSequentiel() << endl;
    cout << "Calcul threadé : " << threadPool.getResult() << endl;
    return 0;
}

