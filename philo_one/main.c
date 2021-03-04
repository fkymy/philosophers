#include <stdlib.h>
#include <stdio.h>

/* Utils */
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
# define SECTONSEC 1000000000 /* sec to nanosec */
# define USECTONSEC 1000 /* microsec to nanosec */
# define MSECTOUSEC 1000 /* milisec to microsec */

# define STOP 1

typedef struct		s_diner
{
	int				num_philo;
	long			time_to_die;
	long			time_to_eat;
	long			time_to_sleep;
	long			num_meals;
	int				has_option;
	long			start_time;
	int				death;
	pthread_mutex_t	death_lock;
}					t_diner;

t_diner	global;

typedef struct		s_params
{
	int				id;
	pthread_mutex_t	*left;
	pthread_mutex_t	*right;
	pthread_mutex_t	last_meal_lock;
	long			last_meal;
	int				num_meals;
}					t_params;

long	utime(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000 + tv.tv_usec);
}

long	mtime(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	msleep(long mtime)
{
	long	end;
	long	start;

	end = mtime * 1000;
	start = utime();
	while (utime() - start < end)
		usleep(10);
}

void	print_status(long mtime, int id, char *msg)
{
	int		idx;

	idx = id + 1;
	printf("%ld %d %s\n", mtime - global.start_time, idx, msg);
}

int		update_status(int id, char *msg)
{
	pthread_mutex_lock(&global.death_lock);
	if (global.death)
	{
		pthread_mutex_unlock(&global.death_lock);
		return (STOP);
	}
	print_status(mtime(), id, msg);
	pthread_mutex_unlock(&global.death_lock);
	return (0);
}

void	update_last_meal(t_params *p)
{
	pthread_mutex_lock(&p->last_meal_lock);
	p->num_meals++;
	p->last_meal = mtime();
	pthread_mutex_unlock(&p->last_meal_lock);
}

int		pickup(t_params *p, pthread_mutex_t *first, pthread_mutex_t *second)
{
	pthread_mutex_lock(first);
	if (update_status(p->id, "has taken a fork") == STOP)
	{
		pthread_mutex_unlock(first);
		return (STOP);
	}
	pthread_mutex_lock(second);
	if (update_status(p->id, "has taken a fork") == STOP)
	{
		pthread_mutex_unlock(first);
		pthread_mutex_unlock(second);
		return (STOP);
	}
	return (0);
}

void	putdown(pthread_mutex_t *first, pthread_mutex_t *second)
{
	pthread_mutex_unlock(first);
	pthread_mutex_unlock(second);
}

int		eat_sleep(int id, pthread_mutex_t *left, pthread_mutex_t *right, t_params *p)
{
	if (pickup(p, left, right) == STOP)
	{
		putdown(left, right);
		return (STOP);
	}
	if (update_status(id, "is eating") == STOP)
	{
		putdown(left, right);
		return (STOP);
	}
	update_last_meal(p);
	msleep(global.time_to_eat);
	if (update_status(id, "is sleeping") == STOP)
	{
		putdown(left, right);
		return (STOP);
	}
	putdown(left, right);
	msleep(global.time_to_sleep);
	return (0);
}

void	*timer(void *args)
{
	t_params	*p;
	long		now;

	p = args;
	while (1)
	{
		pthread_mutex_lock(&p->last_meal_lock);
		if (p->num_meals == global.num_meals && global.has_option)
			break ;
		now = mtime();
		if (now - p->last_meal > global.time_to_die)
		{
			pthread_mutex_lock(&global.death_lock);
			if (!global.death)
			{
				global.death = 1;
				print_status(now, p->id, "died");
			}
			pthread_mutex_unlock(&global.death_lock);
			break ;
		}
		pthread_mutex_unlock(&p->last_meal_lock);
		usleep(10);
	}
	pthread_mutex_unlock(&p->last_meal_lock);
	return (NULL);
}

void	*philosopher(void *args)
{
	t_params	*p;
	pthread_t	tid;
	int			i;

	p = args;
	if (p->id % 2 == 0)
		usleep(1000);
	pthread_create(&tid, NULL, timer, (void *)p);
	i = 0;
	while (i < global.num_meals || !global.has_option)
	{
		if (eat_sleep(p->id, p->left, p->right, p) == STOP)
			break ;
		if (update_status(p->id, "is thinking") == STOP)
			break ;
		i++;
	}
	pthread_join(tid, NULL);
	return ((void *)p);
}

int	setup_locks(pthread_mutex_t *forks, t_params *params)
{
	int	i;

	i = 0;
	while (i < global.num_philo)
	{
		pthread_mutex_init(&forks[i], NULL);
		pthread_mutex_init(&params[i].last_meal_lock, NULL);
		i++;
	}
	pthread_mutex_init(&global.death_lock, NULL);
	return (0);
}

void	start_dining(pthread_t *philosophers, pthread_mutex_t *forks, t_params *params)
{
	int		i;
	long	now;

	now = mtime();
	global.start_time = now;
	i = 0;
	while (i < global.num_philo)
	{
		params[i].id = i;
		params[i].left = &forks[i];
		params[i].right = &forks[(i + 1) % global.num_philo];
		params[i].num_meals = 0;
		params[i].last_meal = now;
		pthread_create(&philosophers[i], NULL, philosopher, (void *)(params + i));
		i++;
	}
	i = 0;
	while (i < global.num_philo)
		pthread_join(philosophers[i++], NULL);
}

int	setup_diner(int argc, char *argv[])
{
	if (argc != 5 && argc != 6)
		return (-1);
	global.num_philo = ft_atoi(argv[1]);
	global.time_to_die = ft_atoi(argv[2]);
	global.time_to_eat = ft_atoi(argv[3]);
	global.time_to_sleep = ft_atoi(argv[4]); global.num_meals = argc == 6 ? ft_atoi(argv[5]) : 0;
	global.num_meals = argc == 6 ? ft_atoi(argv[5]) : 0;
	global.has_option = argc == 6 ? 1 : 0;
	global.death = 0;
	if (global.num_philo <= 1 || global.num_philo > MAXTHREADS)
		return (-1);
	if (global.time_to_die < 0 || global.time_to_eat < 0 || global.time_to_sleep < 0 || global.num_meals < 0)
		return (-1);
	return (0);
}

int	main(int argc, char *argv[])
{
	pthread_t		*philosophers;
	pthread_mutex_t	*forks;
	t_params		*params;

	if (setup_diner(argc, argv) < 0)
		return (printf("Error: Invalid arguments\n"));
	forks = malloc(sizeof(pthread_mutex_t) * global.num_philo);
	philosophers = malloc(sizeof(pthread_t) * global.num_philo);
	params = malloc(sizeof(t_params) * global.num_philo);
	setup_locks(forks, params);
	start_dining(philosophers, forks, params);

	/* cleanup */
	for (int i = 0; i < global.num_philo; i++)
	{
		pthread_mutex_destroy(&forks[i]);
		pthread_mutex_destroy(&params[i].last_meal_lock);
	}
	pthread_mutex_destroy(&global.death_lock);
	free(forks);
	free(philosophers);
	free(params);
	return (0);
}
