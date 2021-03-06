//
// Created by zz on 2022/4/16.
//
#include <boost/noncopyable.hpp>
#include <thread>

#ifndef IO_MULTIPLEXING_MUTEX_H
#define IO_MULTIPLEXING_MUTEX_H

#endif //IO_MULTIPLEXING_MUTEX_H

class MutexLock : boost::noncopyable {
public:
    MutexLock() : holder_(0) {
        pthread_mutex_init(&mutex_, nullptr);
    };

    ~MutexLock() {
        assert(holder_ == 0);
        pthread_mutex_destroy(&mutex_);
    };

    void lock() {
        pthread_mutex_lock(&mutex_);
        holder_ = CurrentThread::tid();
    }

    void unlock() {
        holder_ = 0;
        pthread_mutex_unlock(&mutex_);
    }

    bool isLockedByThisThread() {
        return holder_ == CurrentThread::tid();
    }

    pthread_mutex_t *getPthreadMutex() {    // 仅供 Condition 调用，禁止用户代码调用
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;
    pid_t holder_;
};

class MutexLockGuard : boost::noncopyable {
public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) {
        mutex_.lock();
    };

    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    MutexLock &mutex_;
};

#define MutexLockGuard(x) static_assert(false, "missing mutex guard var name")
