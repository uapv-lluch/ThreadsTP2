#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <functional>

using namespace std;

using Task = function<void()>;

int main () {
    vector<thread> threads;
//    mutex m1;
    mutex m2;
    int taskNb = 0;
    vector<int> taskCompleted;
    queue<Task> q1;
    queue<Task> q2;
    m2.lock();
    for (unsigned i = 0 ; i < 5 ; ++i) {
        ++taskNb;
        q1.emplace([] () {
            cout << "1" << endl;
            this_thread::sleep_for(chrono::seconds(1));
        });
        q2.emplace([] {
            cout << "2" << endl;
            this_thread::sleep_for(chrono::seconds(1));
        });
    }
    taskCompleted.push_back(0);
    taskCompleted.push_back(0);

    threads.emplace_back([&] {
        while (true) {
            Task task;
            {
                if (q1.empty()) {
                    while(taskCompleted[1] != taskNb) {
                        m2.unlock();
                    }
                    break;
                }
                task = move(q1.front());
                q1.pop();
            }
            task();
            ++taskCompleted[0];
            m2.unlock();
        }
    });

    threads.emplace_back([&] {
        while (true) {
            Task task;
            {
                m2.lock();
                if (q2.empty()) {
                    break;
                }
                task = move(q2.front());
                q2.pop();
            }
            task();
            ++taskCompleted[1];
        }
    });

    for (auto &v: threads) {
        v.join();
    }
    return 0;
}