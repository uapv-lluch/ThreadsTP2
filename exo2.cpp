#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <functional>
#include <queue>

using namespace std;

class StringPlus : public string {
private:
    using string::string;
    vector<string> numbers = {"zero", "un", "deux", "trois", "quatre", "cinq", "six", "sept", "huit", "neuf"};

public:
    void show() {
        cout << *this << endl;
    }

    void toLower() {
        for (char &chr : *this) {
            chr = (char) tolower(chr);
        }
    }

    void tokenize() {
        while (this->find('\'') != string::npos) {
            this->replace(this->find('\''), 1, " ");
        }
    }

    void removePunctuation() {
        this->erase(remove_if(this->begin(), this->end(), ::ispunct), this->end());
    }

    void replaceNumbersToLetters() {
        while (find_if(this->begin(), this->end(), ::isdigit) != this->end()) {
            iterator it = find_if(this->begin(), this->end(), ::isdigit);
            int nb = *it -
                     '0'; // On soustrait le code ASCII du caractere 0 au code ASCII du caractere pour obtenir pour obtenir le nombre correspondant
            this->replace(it, it + 1, numbers[nb]);
        }
    }
};

using Task = function<StringPlus()>;

class Pipeline {
private:
    vector<thread> threads;
    vector<queue<Task>> tasksQueues;
    vector<mutex> mutexes;
    vector<StringPlus> strings;
    mutex m;
    mutex n;

    void addTask(unsigned line, Task task) {
        tasksQueues[line].push(task);
    }

public:
    Pipeline() {
        threads.resize(4);
        tasksQueues.resize(4);
//        mutexes.resize(4);
        strings = {
                "L'artiste a réalisé 4 belle peintures."
        };
        for (StringPlus &str : strings) {
            addTask(str);
        }
    }

    Pipeline(vector<StringPlus> strings) {
        threads.resize(4);
        tasksQueues.resize(4);
//        mutexes.resize(4);
        this->strings = move(strings);
        for (StringPlus &str : this->strings) {
            addTask(str);
        }
    }

    void addTask(StringPlus &str) {
        Task task = [&str] {
            str.toLower();
            return str;
        };
        tasksQueues[0].push(task);
    }

    void showStrings() {
        for (StringPlus &str : strings) {
            str.show();
        }
    }

    void start() {
        auto processToLower = [this] {
            while (!tasksQueues[0].empty()) {
                m.lock();
                Task task = tasksQueues[0].front();
                StringPlus str = task();
                Task nextTask = [&str] {
                    str.tokenize();
                    return str;
                };
                addTask(1, nextTask);
                tasksQueues[0].pop();
                m.unlock();
            }
        };

        auto processTokenize = [this] {
            while (!tasksQueues[0].empty() && !tasksQueues[1].empty()) {
                n.lock();
                Task task = tasksQueues[1].front();
                StringPlus str = task();
                /*Task nextTask = [&str] {
                    str.removePunctuation();
                    return str;
                };
                addTask(2, nextTask);*/
                tasksQueues[1].pop();
                n.unlock();
            }
        };
        threads[0] = thread(processToLower);
        threads[1] = thread(processTokenize);

        for (unsigned i = 0; i < 2; ++i) {
            if (threads[i].joinable()) {
                threads[i].join();
            }
        }
    }
};

int main() {
    StringPlus stringPlus = "L'artiste a réalisé 4 belle peintures.";
    /*stringPlus.toLower();
    stringPlus.tokenize();
    stringPlus.removePunctuation();
    stringPlus.replaceNumbersToLetters();
    cout << stringPlus << endl;*/

    Pipeline pipeline;
    pipeline.start();
    pipeline.showStrings();
}
