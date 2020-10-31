// ThreadUse.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <mutex>

//스레드 디버그창 확인해기 
//실행후 중단. 디버그 -> 창 -> 쓰래드


using namespace std;

const int MaxCount = 15000000;
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

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
