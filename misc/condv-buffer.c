
/*
 * condv-buffer.c -- 条件変数を使った環状バッファ
 */

#include <stdio.h>	/* printf() */
#include <stdlib.h>	/* malloc(), exit() */
#include <pthread.h>

#define	BUFFER_SIZE	4		/* バッファの大きさ */
struct circular_buffer
{
	int rp ;			/* 読み出す位置 */
	int wp ;			/* 書き込む位置 */
    	int used ;			/* バッファ内の要素数 */
	int data[BUFFER_SIZE];		/* データを保存する場所 */
	pthread_mutex_t	mutex ;		/* この構造体の相互排除のための mutex */
	pthread_cond_t	not_full ;	/* バッファが一杯ではない状態を待つための条件変数 */
	pthread_cond_t	not_empty ;	/* バッファが空ではない状態を待つための条件変数 */
};

void thread_A(struct circular_buffer *b);
void thread_B(struct circular_buffer *b);

void put( struct circular_buffer *b,int x )
{
	pthread_mutex_lock( &b->mutex );
loop:	if( b->used == BUFFER_SIZE )
	{
	    pthread_cond_wait( &b->not_full,&b->mutex );
	    goto loop;
	}
	b->data[ b->wp++ ] = x ;
	if( b->wp >= BUFFER_SIZE )
	    b->wp = 0 ;
	b->used ++ ;
	pthread_cond_broadcast( &b->not_empty );
	pthread_mutex_unlock( &b->mutex );
}

int get( struct circular_buffer *b )
{
    int x ;
	pthread_mutex_lock( &b->mutex );
loop:	if( b->used == 0 )
	{
	    pthread_cond_wait( &b->not_empty,&b->mutex );
	    goto loop;
	}
	x = b->data[ b->rp++ ] ;
	if( b->rp >= BUFFER_SIZE )
	    b->rp = 0 ;
	b->used -- ;
	pthread_cond_broadcast( &b->not_full );
	pthread_mutex_unlock( &b->mutex );
	return( x );
}

int main()
{
    pthread_t t1 ;
    pthread_t t2 ;
    struct circular_buffer *b  ;
	b = (struct circular_buffer *)malloc(sizeof(struct circular_buffer));
    	if( b == NULL )
	{
	    perror("no memory for struct buffer\n");
	    exit( -1 );
	}
	b->rp = 0 ;
	b->wp = 0 ;
	b->used = 0 ;
	pthread_mutex_init( &b->mutex, NULL );
	pthread_cond_init( &b->not_full,NULL );
	pthread_cond_init( &b->not_empty,NULL );
	pthread_setconcurrency( 2 );
	pthread_create( &t1, NULL, (void *)thread_A, (void *)b );
	pthread_create( &t2, NULL, (void *)thread_B, (void *)b );
	pthread_join( t1, NULL );
	pthread_join( t2, NULL );
	return( 0 );
}

void thread_A( struct circular_buffer *b )	/* producer */
{
    int i,x ;
	for( i = 0 ; i<10 ; i++ )
	{
	    x = i ;
	    printf("thread_A(): put( %d )\n",x );
	    put( b,x );
	}
}

void thread_B( struct circular_buffer *b )	/* consumer */
{
    int i, x ;
	for( i = 0 ; i<10 ; i++ )
	{
	    x = get( b );
	    printf("thread_B(): get() %d.\n", x );
	}
}
