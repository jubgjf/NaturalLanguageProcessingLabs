cmake_minimum_required(VERSION 3.0.0)
project(NaturalLanguageProcessingLabs VERSION 0.1.0)


# ===== Lab 1 =====

add_executable(
        max_match
        lab1/max_match/main.c
        lab1/max_match/max_match.c
        lab1/max_match/max_match.h
        lab1/max_match/max_match_fast.c
        lab1/max_match/max_match_fast.h
        lab1/max_match/max_match_threaded.c
        lab1/max_match/max_match_threaded.h
        lab1/string/str.c
        lab1/string/str.h
        lab1/stack/stack.c
        lab1/stack/stack.h
        lab1/hashmap/hashmap.c
        lab1/hashmap/hashmap.h
)

target_link_libraries(max_match PRIVATE Threads::Threads)

# ===== <pthread.h> 多线程 =====
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
