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

    void replaceNumbersToString() {
        while (find_if(this->begin(), this->end(), ::isdigit) != this->end()) {
            iterator it = find_if(this->begin(), this->end(), ::isdigit);
            int nb = *it -
                     '0'; // On soustrait le code ASCII du caractere 0 au code ASCII du caractere pour obtenir pour obtenir le nombre correspondant
            this->replace(it, it + 1, numbers[nb]);
        }
    }
};

using Task = function<void()>;

class Pipeline {
private:
    vector<thread> threads;
    vector<queue<StringPlus>> stringsQueues;
    vector<mutex> mutexes;
    vector<StringPlus> strings;
    mutex m1;
    mutex m2;
    mutex m3;
    mutex m4;

    void addTask(unsigned line, StringPlus &str) {
        stringsQueues[line].push(str);
    }

public:
    Pipeline() {
        threads.resize(4);
        stringsQueues.resize(4);
        vector<StringPlus> tempStrings = {
                "L'artiste a réalisé 4 belle peintures."
        };
        for (StringPlus &str : tempStrings) {
            addTask(str);
        }
    }

    Pipeline(vector<StringPlus> strings) {
        threads.resize(4);
        stringsQueues.resize(4);
        for (StringPlus &str : strings) {
            addTask(str);
        }
    }

    void addTask(StringPlus &str) {
        stringsQueues[0].push(str);
    }

    void showStrings() {
        for (StringPlus &str : strings) {
            str.show();
        }
    }

    void start() {
        auto processToLower = [this] {
            while (!stringsQueues[0].empty()) {
                m1.lock();
                StringPlus str = stringsQueues[0].front();
                str.toLower();
                addTask(1, str);
                stringsQueues[0].pop();
                m1.unlock();
            }
        };

        auto processTokenize = [this] {
            while (!stringsQueues[0].empty() || !stringsQueues[1].empty()) {
                m2.lock();
                if (!stringsQueues[1].empty()) {
                    StringPlus str = stringsQueues[1].front();
                    str.tokenize();
                    addTask(2, str);
                    stringsQueues[1].pop();
                }
                m2.unlock();
            }
        };

        auto processRemovePunctuation = [this] {
            while (!stringsQueues[0].empty() || !stringsQueues[1].empty() || !stringsQueues[2].empty()) {
                m3.lock();
                if (!stringsQueues[2].empty()) {
                    StringPlus str = stringsQueues[2].front();
                    str.removePunctuation();
                    addTask(3, str);
                    stringsQueues[2].pop();
                }
                m3.unlock();
            }
        };

        auto processReplaceNumbersToString = [this] {
            while (!stringsQueues[0].empty() || !stringsQueues[1].empty() || !stringsQueues[2].empty() || !stringsQueues[3].empty()) {
                m4.lock();
                if (!stringsQueues[3].empty()) {
                    StringPlus str = stringsQueues[3].front();
                    str.replaceNumbersToString();
                    stringsQueues[3].pop();
                    strings.push_back(str);
                }
                m4.unlock();
            }
        };

        threads[0] = thread(processToLower);
        threads[1] = thread(processTokenize);
        threads[2] = thread(processRemovePunctuation);
        threads[3] = thread(processReplaceNumbersToString);

        for (unsigned i = 0; i < threads.size(); ++i) {
            if (threads[i].joinable()) {
                threads[i].join();
            }
        }
    }
};

int main() {
    StringPlus stringPlus = "L'artiste a réalisé 4 belle peintures.";
    StringPlus stringPlus2 = "Le maçon a construit 2 grande maison.";
    StringPlus stringPlus3 = "L' a réalisé 4 belle peintures.";

    Pipeline pipeline({stringPlus, stringPlus2, stringPlus3});
    pipeline.start();
    pipeline.showStrings();
}
