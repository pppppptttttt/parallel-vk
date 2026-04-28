# Домашнее задание 7

## 1

Отчет для суммирования по строкам и столбцам:

```sh
mode: rows
size: 10000x10000
repeats: 5
time_ns: 15890325
time_ms: 15.8903
checksum: 49997291202

 Performance counter stats for './matrix_sum rows 10000 5':

       397 135 680      task-clock                       #    0,998 CPUs utilized             
     1 815 266 714      cycles                           #    4,571 GHz                         (83,21%)
     6 371 941 647      instructions                     #    3,51  insn per cycle              (83,37%)
        76 091 650      cache-references                 #  191,601 M/sec                       (83,41%)
         2 682 689      cache-misses                     #    3,53% of all cache refs           (83,39%)
       707 628 559      branches                         #    1,782 G/sec                       (83,38%)
        14 207 636      branch-misses                    #    2,01% of all branches             (83,24%)

       0,397792414 seconds time elapsed

       0,246425000 seconds user
       0,151261000 seconds sys


mode: cols
size: 10000x10000
repeats: 5
time_ns: 248993007
time_ms: 248.993
checksum: 50003046499

 Performance counter stats for './matrix_sum cols 10000 5':

       633 684 650      task-clock                       #    0,999 CPUs utilized             
     2 892 015 944      cycles                           #    4,564 GHz                         (83,30%)
     6 817 193 154      instructions                     #    2,36  insn per cycle              (83,28%)
       451 011 847      cache-references                 #  711,729 M/sec                       (83,27%)
       144 530 562      cache-misses                     #   32,05% of all cache refs           (83,35%)
       781 534 435      branches                         #    1,233 G/sec                       (83,43%)
        14 302 160      branch-misses                    #    1,83% of all branches             (83,37%)

       0,634533403 seconds time elapsed

       0,482195000 seconds user
       0,152061000 seconds sys
```

Из этого вывода почти очевидно, что в том, что суммирование по колонкам медленно, виноваты
кэш миссы. Процессор читает данные кэш линиями, и в `sum_rows` соседние `int` используются
уже в следующих итерациях.

## 2

Чтобы улучшить `sum_cols`, можно сделать массив `cols`, в каждый элемент которого проссумировать
элемент `m[i][col]`. Потом вернуть сумму `cols`. Это намного более кеш френдли.

Этот подход я и выбрал.

```sh
mode: rows
size: 10000x10000
repeats: 5
time_ns: 15884239
time_ms: 15.8842
checksum: 50005687458

 Performance counter stats for './matrix_sum rows 10000 5':

       400 384 440      task-clock                       #    0,998 CPUs utilized             
     1 810 439 797      cycles                           #    4,522 GHz                         (83,29%)
     6 289 609 433      instructions                     #    3,47  insn per cycle              (83,26%)
        75 580 031      cache-references                 #  188,769 M/sec                       (83,25%)
         2 488 156      cache-misses                     #    3,29% of all cache refs           (83,29%)
       709 343 434      branches                         #    1,772 G/sec                       (83,54%)
        14 247 965      branch-misses                    #    2,01% of all branches             (83,38%)

       0,401080865 seconds time elapsed

       0,253993000 seconds user
       0,146996000 seconds sys


mode: cols
size: 10000x10000
repeats: 5
time_ns: 258935672
time_ms: 258.936
checksum: 50000034707

 Performance counter stats for './matrix_sum cols 10000 5':

       641 395 350      task-clock                       #    0,999 CPUs utilized             
     2 920 802 094      cycles                           #    4,554 GHz                         (83,33%)
     6 711 989 485      instructions                     #    2,30  insn per cycle              (83,32%)
       450 842 125      cache-references                 #  702,908 M/sec                       (83,32%)
       144 741 028      cache-misses                     #   32,10% of all cache refs           (83,33%)
       786 320 312      branches                         #    1,226 G/sec                       (83,32%)
        14 489 503      branch-misses                    #    1,84% of all branches             (83,38%)

       0,642125165 seconds time elapsed

       0,494988000 seconds user
       0,146996000 seconds sys


mode: cols_opt
size: 10000x10000
repeats: 5
time_ns: 20071674
time_ms: 20.0717
checksum: 50002437047

 Performance counter stats for './matrix_sum cols_opt 10000 5':

       404 022 150      task-clock                       #    0,998 CPUs utilized             
     1 827 526 800      cycles                           #    4,523 GHz                         (83,17%)
     6 408 562 553      instructions                     #    3,51  insn per cycle              (83,17%)
        89 598 117      cache-references                 #  221,765 M/sec                       (83,43%)
         2 934 993      cache-misses                     #    3,28% of all cache refs           (83,42%)
       709 744 003      branches                         #    1,757 G/sec                       (83,42%)
        14 254 559      branch-misses                    #    2,01% of all branches             (83,39%)

       0,404797591 seconds time elapsed

       0,249831000 seconds user
       0,154895000 seconds sys
```

В честь этого можно было бы заменить `vector<vector<int>>` на один плоский массив. `std::vector` аллоцирует
каждую строку отдельно, и для больших матриц это много отдельных аллокаций, которые лежат не подряд в памяти.
Но это противоречит условию `Matrix = vector<vector<int>>`, так что оставлю в качестве мысленного эксперимента
