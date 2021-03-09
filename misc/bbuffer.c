#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define BBUF_SIZE 128
#define min(a,b) ({\
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })

struct bbuf {
	char buf[BBUF_SIZE];
	size_t capacity;
	pthread_mutex_t	lock;
	size_t pos;
	size_t len;
	int write_closed;
};

struct bbuf *bbuf_init(void)
{
	struct bbuf *b;

	b = malloc(sizeof(struct bbuf));
	b->capacity = BBUF_SIZE;
	pthread_mutex_init(&b->lock, NULL);
	b->pos = 0;
	b->len = 0;
	b->write_closed = 0;
	return (b);
};

void bbuf_destory(struct bbuf *b)
{
	pthread_mutex_destroy(&b->lock);
	/* shutdown write... */
}

ssize_t bbuf_write(struct bbuf *b, const char *buf, size_t size)
{
	pthread_mutex_lock(&b->lock);
	assert(!b->write_closed);
	size_t pos = 0;
	while (pos < size && b->len < b->capacity) {
		size_t bindex = (b->pos + b->len) % b->capacity;
		size_t bspace = min(b->capacity - bindex, b->capacity - b->len);
		size_t n = min(size - pos, bspace);
		memcpy(&b->buf[bindex], &buf[pos], n);
		b->len += n;
		pos += n;
	}
	pthread_mutex_unlock(&b->lock);
	if (pos == 0 && size > 0) {
		return -1;
	} else {
		return pos;
	}
}

ssize_t bbuf_read(struct bbuf *b, char *buf, size_t size)
{
	pthread_mutex_lock(&b->lock);
	size_t pos = 0;
	while (pos < size && b->len > 0) {
		size_t bspace = min(b->len, b->capacity - b->pos);
		size_t n = min(size - pos, bspace);
		memcpy(&buf[pos], &b->buf[b->pos], n);
		b->pos = (b->pos + n) % b->capacity;
		b->len -= n;
		pos += n;
	}
	if (pos == 0 && size > 0 && !b->write_closed) {
		pos = -1;
	}
	pthread_mutex_unlock(&b->lock);
	return pos;
}
