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

#define SECTONSEC 1000000000 /* sec to nanosec */
# define USECTONSEC 1000 /* microsec to nanosec */
# define MSECTOUSEC 1000 /* milisec to microsec */

const int kNumMeals = 3;
const int no_option = 1;

typedef struct	s_args
{
	int				id;
	pthread_mutex_t	*left;
	pthread_mutex_t	*right;
}				t_args;

/* int	extract_arg(t_args *a, char **argv, int option) */
/* { */
/*  */
/* 	return (0); */
/* } */
/*  */
/* int	arg_is_valid(int argc, char *argv[]) */
/* { */
/* 	return (1); */
/* } */

void	think(int id)
{
	printf("%d is thinking\n", id);
}

void	eat(int id, pthread_mutex_t *left, pthread_mutex_t *right)
{
	pthread_mutex_lock(left);
	usleep(1000 * MSECTOUSEC);
	pthread_mutex_lock(right);
	printf("%d is eating\n", id);
	usleep(2000 * MSECTOUSEC);
	pthread_mutex_unlock(left);
	pthread_mutex_unlock(right);
}

void	*philosopher(void *args)
{
	t_args *a;

	a = (t_args *)args;
	printf("thread started: %d\n", a->id);
	// while number of meals to eat, eat, sleep, think
	int i = 0;
	while (i < kNumMeals || no_option)
	{
		eat(a->id, a->left, a->right);
		printf("%d is sleeping\n", a->id);
		usleep(3000 * MSECTOUSEC);
		printf("%d is thinking\n", a->id);
		i++;
	}
	return (NULL);
}

int main(int argc, char *argv[])
{
	if (argc != 5 && argc != 6)
	{
		printf("Error: Invalid number of arguments\n");
		return (1);
	}

	/* max number of threads? */
	int num_philo = ft_atoi(argv[1]);
	if (num_philo <= 0)
	{
		printf("Error: Invalid arguments\n");
		return (1);
	}
	printf("num_philo: %d\n", num_philo);

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
		pthread_create(&philos[i], NULL, philosopher, (void *)(args + i));
		i++;
	}
	int j = 0;
	while (j < num_philo)
	{
		pthread_join(philos[j], NULL);
		j++;
	}
	return (0);
}

