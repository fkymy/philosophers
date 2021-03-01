#include <stdlib.h>
#include <stdio.h>

/* Utils */
size_t	ft_strlen(const char *s)
{
	char *p;

	p = (char *)s;
	while (*p)
		p++;
	return (p - s);
}

int	ft_atoi(const char *s)
{
	int				i;
	unsigned int	n;
	int				sign;

	i = 0;
	n = 0;
	while ((s[i] > 8 && s[i] < 14) || s[i] == ' ')
		i++;
	sign = s[i] == '-' ? -1 : 1;
	if (s[i] == '-' || s[i] == '+')
		i++;
	while (s[i] >= '0' && s[i] <= '9')
	{
		n = 10 * n + (s[i] - '0');
		if (n > 2147483647 && sign == 1)
			return (-1);
		if (n > 2147483648 && sign == -1)
			return (-1);
		i++;
	}
	return (sign * n);
}

#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define SECTONSEC 1000000000 /* sec to nanosec */
# define USECTONSEC 1000 /* microsec to nanosec */
# define MSECTOUSEC 1000 /* milisec to microsec */

const int kNumMeals = 3;
const int no_option = 0;

typedef struct	s_args
{
	int				id;
	pthread_mutex_t	*left;
	pthread_mutex_t	*right;
	int				*permits;
	pthread_mutex_t	*permits_lock;
}				t_args;

long	utime(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000 + tv.tv_usec);
}

void	print_status(long utime, int id, char *msg)
{
	long	mtime;

	mtime = utime / 1000;
	printf("%ld %d %s\n", mtime, id, msg);
}

long	gettimestamp(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	wait_for_permission(int *permits, pthread_mutex_t *permits_lock)
{
	while (1)
	{
		pthread_mutex_lock(permits_lock);
		if (*permits > 0)
			break ;
		pthread_mutex_unlock(permits_lock);
		usleep(10);
	}
	(*permits)--;
	pthread_mutex_unlock(permits_lock);
}

void	grant_permission(int *permits, pthread_mutex_t *permits_lock)
{
	pthread_mutex_lock(permits_lock);
	(*permits)++;
	pthread_mutex_unlock(permits_lock);
}

void	eat(int id, pthread_mutex_t *left, pthread_mutex_t *right, int *permits, pthread_mutex_t *permits_lock)
{
	wait_for_permission(permits, permits_lock);
	pthread_mutex_lock(left);
	print_status(utime(), id, "has taken a fork");
	// usleep(1000 * MSECTOUSEC); // for deadlock check
	pthread_mutex_lock(right);
	print_status(utime(), id, "has taken a fork");
	print_status(utime(), id, "is eating");
	usleep(1000 * MSECTOUSEC);
	grant_permission(permits, permits_lock);
	pthread_mutex_unlock(left);
	pthread_mutex_unlock(right);
}

void	*philosopher(void *args)
{
	t_args *a;

	a = (t_args *)args;
	print_status(utime(), a->id, "thread started");
	int i = 0;
	while (i < kNumMeals || no_option)
	{
		eat(a->id, a->left, a->right, a->permits, a->permits_lock);
		print_status(utime(), a->id, "is sleeping");
		usleep(1000 * MSECTOUSEC);
		print_status(utime(), a->id, "is thinking");
		i++;
	}
	return ((void *)a);
}

int main(int argc, char *argv[])
{
	if (argc != 5 && argc != 6)
	{
		printf("Error: Invalid number of arguments\n");
		return (1);
	}

	int num_philo = ft_atoi(argv[1]);
	if (num_philo <= 1)
	{
		printf("Error: Invalid arguments\n");
		return (1);
	}
	printf("num_philo: %d\n", num_philo);

	int permits = num_philo - 1;
	pthread_mutex_t permits_lock;
	/* pthread_mutex_t death_lock; */
	pthread_mutex_t forks[num_philo];
	pthread_t philos[num_philo];
	t_args args[num_philo];

	int i = 0;
	while (i < num_philo)
		pthread_mutex_init(&forks[i++], NULL);
	i = 0;
	while (i < num_philo)
	{
		args[i].id = i;
		args[i].left = &forks[i];
		args[i].right = &forks[(i + 1) % num_philo];
		args[i].permits = &permits;
		args[i].permits_lock = &permits_lock;
		pthread_create(&philos[i], NULL, philosopher, (void *)(args + i));
		i++;
	}

	t_args *ret;
	int j = 0;
	while (j < num_philo)
	{
		pthread_join(philos[j], (void *)&ret);
		printf("ret! j: %d, id: %d\n", j, ret->id);
		j++;
	}
	return (0);
}

