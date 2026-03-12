# Домашнее задание 3

## Задача H2O

N потоков, которые могут писать в std::cout либо "O" либо "H"

Необходимо синхронизировать потоки таким образом, чтобы каждая тройка букв составляла молекулу

Пример:

```
H H O H O H

// ок: HHO HOH
// ок: OHH HHO
// ок: HOH OHH
//
// не ок: OOH HHH и т.п.
```

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

std::mutex mtx;
std::condition_variable cv;

int h_count = 0;
int o_count = 0;

void hydrogen()
{
    std::cout << "H";
    h_count++;
}


void oxygen()
{
    std::cout << "O";
    o_count++;
}


int main()
{
    std::thread t1([&]{ hydrogen(); });
    std::thread t2([&]{ hydrogen(); });
    std::thread t3([&]{ oxygen(); });
    std::thread t4([&]{ hydrogen(); });
    std::thread t5([&]{ oxygen(); });
    std::thread t6([&]{ hydrogen(); });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t6.join();
    
    std::cout << std::endl;
}
```

## Решение

См. main.cpp

Писал в предположении что число потоков случайно, и что молекула, которую поток производит, тоже
определяется броском правильной монетки
