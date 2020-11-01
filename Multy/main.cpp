#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <mutex>

//스레드 디버그창 확인해기 
//실행후 중단. 디버그 -> 창 -> 쓰래드
//병목확인 Concurrency 분석도구로 병목을 확인할 수 있음

using namespace std;

const int MaxCount = 150000;
const int threadCount = 8;

bool IsPrimeNumber(int number) {
    if (number == 1)
        return false;
    if (number == 2 || number == 3)
        return true;
    for (int i = 2; i < number - 1; i++) {
        if ((number % i) == 0)
            return false;
    }
    return true;
}
void PrintNumbers(const vector<int>& primes) {
    for (int v : primes) {
        cout << v << endl;
    }
}

int main()
{
    int num = 1;
    recursive_mutex num_mutex;

    vector<int> primes;
    recursive_mutex primes_mutex;

    auto t0 = chrono::system_clock::now();

    vector<shared_ptr<thread>> threads;

    for (int i = 0; i < threadCount; i++) {
        shared_ptr<thread> thread(new thread([&]() {
            while (true) {
                int n;
                {
                    lock_guard<recursive_mutex> num_lock(num_mutex);
                    n = num;
                    num++;
                }
                if (n >= MaxCount)
                    break;

                if (IsPrimeNumber(n)) {
                    lock_guard<recursive_mutex> primes_lock(primes_mutex);
                    primes.push_back(n);
                }
            }
        }));
        threads.push_back(thread);
    }
    for (auto thread : threads) {
        thread->join();
    }

    auto t1 = chrono::system_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

    cout << "Took " << duration << " milliseconds." << endl;

    return 0;

}