#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

struct bbuffer {
    static constexpr size_t bcapacity = 128;
    char bbuf_[bcapacity];
    size_t bpos_ = 0;
    size_t blen_ = 0;
    bool write_closed_ = false;
	std::mutex mutex_;
	std::condition_variable_any non_full_;
    ssize_t read(char* buf, size_t sz);
    ssize_t write(const char* buf, size_t sz);
    void shutdown_write();
};

ssize_t bbuffer::write(const char* buf, size_t sz) {
	this->mutex_.lock();
    assert(!this->write_closed_);
	while (this->blen_ == bcapacity) {
		this->non_full_.wait(mutex_);
	}
    size_t pos = 0;
    while (pos < sz && this->blen_ < bcapacity) {
        size_t bindex = (this->bpos_ + this->blen_) % bcapacity;
        size_t bspace = std::min(bcapacity - bindex, bcapacity - this->blen_);
        size_t n = std::min(sz - pos, bspace);
        memcpy(&this->bbuf_[bindex], &buf[pos], n);
        this->blen_ += n;
        pos += n;
    }
	this->mutex_.unlock();
    if (pos == 0 && sz > 0) {
        return -1;  // try again
    } else {
        return pos;
    }
}

ssize_t bbuffer::read(char* buf, size_t sz) {
	this->mutex_.lock();
    size_t pos = 0;
    while (pos < sz && this->blen_ > 0) {
        size_t bspace = std::min(this->blen_, bcapacity - this->bpos_);
        size_t n = std::min(sz - pos, bspace);
        memcpy(&buf[pos], &this->bbuf_[this->bpos_], n);
        this->bpos_ = (this->bpos_ + n) % bcapacity;
        this->blen_ -= n;
        pos += n;
    }
	if (pos > 0) {
		this->non_full_.notify_all();
	}
    if (pos == 0 && sz > 0 && !this->write_closed_) {
        return -1;  // try again
    }
	this->mutex_.unlock();
    return pos;
}

