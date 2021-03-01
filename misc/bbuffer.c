/**
 * Circular bounded buffer with mutual exclusion
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define BBUF_SIZE 128
#define min(a, b) ((a < b) ? a : b)

struct bbuffer {
	char buf[BBUF_SIZE];
	size_t capacity;
	pthread_mutex_t	lock;
	size_t pos;
	size_t len;
	int write_closed;
};

struct bbuffer *bbuf_init(void)
{
	struct bbuffer *b;

	b = malloc(sizeof(struct bbuffer));
	b->capacity = BBUF_SIZE;
	pthread_mutex_init(&b->lock, NULL);
	b->pos = 0;
	b->len = 0;
	b->write_closed = 0;
	return (b);
};

ssize_t bbuf_write(struct bbuffer *b, const char *buf, size_t sz)
{
	pthread_mutex_lock(&b->lock);
	assert(!b->write_closed);
	size_t pos = 0;
	while (pos < sz && b->len < b->capacity) {
		size_t bindex = (b->pos + b->len) % b->capacity;
		size_t bspace = min(b->capacity - bindex, b->capacity - b->len);
		size_t n = min(sz - pos, bspace);
		memcpy(&b->buf[bindex], &buf[pos], n);
		b->len += n;
		pos += n;
	}
	pthread_mutex_unlock(&b->lock);
	if (pos == 0 && sz > 0) {
		return -1;
	} else {
		return pos;
	}
}

ssize_t bbuf_read(struct bbuffer *b, const char *buf, size_t sz)
{
	size_t pos = 0;
	pthread_mutex_lock(&b->lock);
	while (pos < sz && b->len > 0) {
		size_t bspace = min(b->len, b->capacity - b->pos);
		size_t n = min(sz - pos, bspace);
		memcpy(&buf[pos], &b->buf[b->pos], n);
		b->pos = (b->pos + n) % b->capacity;
		b->len -= n;
		pos += n;
	}
	if (pos == 0 && sz > 0 && !b->write_closed) {
		pos = -1;
	}
	pthread_mutex_unlock(&b->lock);
	return pos;
}
