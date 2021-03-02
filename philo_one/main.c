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

# define MAXTHREADS 200
#define SECTONSEC 1000000000 /* sec to nanosec */
# define USECTONSEC 1000 /* microsec to nanosec */
# define MSECTOUSEC 1000 /* milisec to microsec */
long	g_start_time;
int		num_philo;
long	time_to_die;
long	time_to_eat;
long	time_to_sleep;
long	num_meals;
int		no_option;

const int kNumMeals = 3;

typedef struct	s_args
{
	int				id;
	pthread_mutex_t	*left;
	pthread_mutex_t	*right;
	int				*permits;
	pthread_mutex_t	*permits_lock;
	pthread_mutex_t	*timer_lock;
	long			last_meal;
	int				num_meals;
	int				dead;
}				t_args;

typedef struct s_timer
{
	int		id;
	int		num_meals;
	long	last_meal;
}	t_timer;

long	last_meal[5];
pthread_mutex_t	last_meal_lock;

long	utime(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000 + tv.tv_usec);
}

void	print_status(long utime, int id, char *msg)
{
	long	mtime;
	int		idx;

	long diff = utime - g_start_time;
	mtime = diff / 1000;
	idx = id + 1;
	printf("%ld %d %s\n", mtime, id, msg);
}

void	print_debug(long utime, int id, char *msg)
{
	long	mtime;
	int		idx;

	long diff = utime - g_start_time;
	mtime = diff / 1000;
	idx = id + 1;
	printf("					%ld %d %s\n", mtime, id, msg);
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

void	update_last_meal(t_args *a, long utime)
{
	pthread_mutex_lock(a->timer_lock);
	a->num_meals++;
	a->last_meal = utime;
	pthread_mutex_unlock(a->timer_lock);
}

void	eat(int id, pthread_mutex_t *left, pthread_mutex_t *right, int *permits, pthread_mutex_t *permits_lock, t_args *a)
{
	print_debug(utime(), id, "waiting for permission");
	wait_for_permission(permits, permits_lock);
	print_debug(utime(), id, "waiting for left fork");
	pthread_mutex_lock(left);
	print_status(utime(), id, "has taken a fork");
	// usleep(1000 * MSECTOUSEC); // for deadlock check
	pthread_mutex_lock(right);
	print_status(utime(), id, "has taken a fork");
	print_status(utime(), id, "is eating");
	update_last_meal(a, utime());
	usleep(time_to_eat * MSECTOUSEC);
	print_status(utime(), a->id, "is sleeping");
	grant_permission(permits, permits_lock);
	pthread_mutex_unlock(left);
	pthread_mutex_unlock(right);
}

void	*timer(void *args)
{
	int status = 0;
	t_args	*a = args;
	int	current_num_meals;

	print_debug(utime(), a->id, "timer thread started");
	// meal could already be updated here if time to eat is 0
	pthread_mutex_lock(a->timer_lock);
	current_num_meals = a->num_meals;
	pthread_mutex_unlock(a->timer_lock);

	while (1)
	{
		pthread_mutex_lock(a->timer_lock);
		if (current_num_meals != a->num_meals)
		{
			print_debug(utime(), a->id, "timer thread notices update num");
			break ;
		}
		if (!no_option && a->num_meals == num_meals)
		{
			print_debug(utime(), a->id, "timer thread notices full num");
			break ;
		}
		if (utime() - a->last_meal > time_to_die * 1000)
		{
			status = 1;
			break ;
		}
		pthread_mutex_unlock(a->timer_lock);
		usleep(30);
	}
	if (status == 1)
		print_status(utime(), a->id, "IS DEAD!!!!!!");
	pthread_mutex_unlock(a->timer_lock);
	return (NULL);
}

void	*philosopher(void *args)
{
	t_args *a;

	a = (t_args *)args;
	print_status(utime(), a->id, "thread started");
	int i = 0;
	while (i < num_meals || no_option)
	{
		pthread_t	timer_thread;
		pthread_create(&timer_thread, NULL, timer, (void *)a);
		pthread_detach(timer_thread);
		eat(a->id, a->left, a->right, a->permits, a->permits_lock, a);
		usleep(time_to_die * 1000);

		// TODO: can die during sleep
		// TODO: simulation stops when someone dies
		usleep(time_to_sleep * MSECTOUSEC);
		print_status(utime(), a->id, "is thinking");
		i++;
	}
	return ((void *)a);
}

int	main(int argc, char *argv[])
{
	if (argc != 5 && argc != 6)
	{
		printf("Error: Invalid number of arguments\n");
		return (1);
	}
	num_philo = ft_atoi(argv[1]);
	time_to_die = ft_atoi(argv[2]);
	time_to_eat = ft_atoi(argv[3]);
	time_to_sleep = ft_atoi(argv[4]);
	num_meals = argc == 6 ? ft_atoi(argv[5]) : 0;
	no_option = argc == 6 ? 0 : 1;
	if (num_philo <= 1 || num_philo > MAXTHREADS)
	{
		printf("Error: Invalid arguments\n");
		return (1);
	}
	if (time_to_die < 0 || time_to_eat < 0 || time_to_sleep < 0 || num_meals < 0)
	{
		printf("Error: Invalid arguments\n");
		return (1);
	}

	int permits = num_philo - 1;
	pthread_mutex_t permits_lock;

	pthread_mutex_t timers[num_philo];
	pthread_mutex_t forks[num_philo];
	pthread_t philos[num_philo];
	t_args args[num_philo];

	g_start_time = utime();
	pthread_mutex_init(&last_meal_lock, NULL);
	pthread_mutex_init(&permits_lock, NULL);
	int i = 0;
	while (i < num_philo)
	{
		pthread_mutex_init(&timers[i], NULL);
		pthread_mutex_init(&forks[i], NULL);
		i++;
	}
	i = 0;
	while (i < num_philo)
	{
		args[i].id = i;
		args[i].left = &forks[i];
		args[i].right = &forks[(i + 1) % num_philo];
		args[i].permits = &permits;
		args[i].permits_lock = &permits_lock;
		args[i].timer_lock = &timers[i];
		args[i].last_meal = utime();
		args[i].num_meals = 0;
		last_meal[i] = utime();
		pthread_create(&philos[i], NULL, philosopher, (void *)(args + i));
		i++;
	}

	t_args *ret;
	int j = 0;
	while (j < num_philo)
	{
		pthread_join(philos[j], (void *)&ret);
		printf("pthread_join returned! j: %d, id: %d\n", j, ret->id);
		j++;
	}
	return (0);
}

