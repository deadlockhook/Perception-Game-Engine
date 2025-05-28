#pragma once

#ifdef PLATFORM_WINDOWS

#include <synchapi.h>
class pltf_mutex {
public:
    void lock_shared() { AcquireSRWLockShared(&lock); }
    void unlock_shared() { ReleaseSRWLockShared(&lock); }

    void lock_exclusive() { AcquireSRWLockExclusive(&lock); }
    void unlock_exclusive() { ReleaseSRWLockExclusive(&lock); }
private:
    SRWLOCK lock = SRWLOCK_INIT;
};
#endif

class pltf_lock_guard {
public:
    pltf_lock_guard(pltf_mutex& m) : mutex(&m) { mutex->lock_exclusive(); }
    ~pltf_lock_guard() { mutex->unlock_exclusive(); }
private:
    pltf_mutex* mutex;
};

class pltf_shared_guard {
public:
    pltf_shared_guard(pltf_mutex& m) : mutex(&m) { mutex->lock_shared(); }
    ~pltf_shared_guard() { mutex->unlock_shared(); }
private:
    pltf_mutex* mutex;
};
