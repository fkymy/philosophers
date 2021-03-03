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
# define SECTONSEC 1000000000 /* sec to nanosec */
# define USECTONSEC 1000 /* microsec to nanosec */
# define MSECTOUSEC 1000 /* milisec to microsec */

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

typedef struct	s_timer
{
	int		id;
	int		num_meals;
	long	last_meal;
}	t_timer;

long	utime(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000 + tv.tv_usec);
}

void	ft_sleep(long end)
{
	long	start;

	start = utime();
	while (utime() - start < end)
		usleep(10);
}

void	print_debug(long utime, int id, char *msg)
{
	long	mtime;
	int		idx;

	long diff = utime - global.start_time;
	mtime = diff / 1000;
	idx = id + 1;
	printf("					%ld %d %s\n", mtime, id, msg);
}

void	print_status(long utime, int id, char *msg)
{
	long	mtime;
	int		idx;
	long	diff;

	diff = utime - global.start_time;
	mtime = diff / 1000;
	idx = id + 1;
	printf("%ld %d %s\n", mtime, id, msg);
}

int		update_status(int id, char *msg)
{
	pthread_mutex_lock(&global.death_lock);
	if (global.death)
	{
		pthread_mutex_unlock(&global.death_lock);
		return (1);
	}
	print_status(utime(), id, msg);
	pthread_mutex_unlock(&global.death_lock);
	return (0);
}

void	update_last_meal(t_params *p, long utime)
{
	pthread_mutex_lock(&p->last_meal_lock);
	p->num_meals++;
	p->last_meal = utime;
	pthread_mutex_unlock(&p->last_meal_lock);
}

int		pickup(t_params *p, pthread_mutex_t *first, pthread_mutex_t *second)
{
	int	err;

	pthread_mutex_lock(first);
	err = update_status(p->id, "has taken a fork");
	if (err)
	{
		pthread_mutex_unlock(first);
		return (1);
	}
	pthread_mutex_lock(second);
	err = update_status(p->id, "has taken a fork");
	if (err)
	{
		pthread_mutex_unlock(first);
		pthread_mutex_unlock(second);
		return (1);
	}
	return (0);
}

void	putdown(pthread_mutex_t *first, pthread_mutex_t *second)
{
	pthread_mutex_unlock(first);
	pthread_mutex_unlock(second);
}

int		eat(int id, pthread_mutex_t *left, pthread_mutex_t *right, t_params *p)
{
	int	err;

	if (p->id % 2 == 0)
		err = pickup(p, left, right);
	else
		err = pickup(p, right, left);
	if (err)
	{
		putdown(left, right);
		return (err);
	}
	err = update_status(id, "is eating");
	if (err)
	{
		putdown(left, right);
		return (err);
	}
	update_last_meal(p, utime());
	ft_sleep(global.time_to_eat * MSECTOUSEC);
	err = update_status(id, "is sleeping");
	if (err)
	{
		putdown(left, right);
		return (err);
	}
	if (p->id % 2 == 0)
		putdown(left, right);
	else
		putdown(right, left);
	return (0);
}

void	*timer(void *args)
{
	t_params	*p;
	long		current_utime;

	p = args;
	while (1)
	{
		pthread_mutex_lock(&p->last_meal_lock);
		if (p->num_meals == global.num_meals && global.has_option)
			break ;
		current_utime = utime();
		if (current_utime - p->last_meal > global.time_to_die * 1000)
		{
			pthread_mutex_lock(&global.death_lock);
			if (!global.death)
			{
				global.death = 1;
				print_status(current_utime, p->id, "died");
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
	int			err;

	p = args;
	pthread_create(&tid, NULL, timer, (void *)p);
	i = 0;
	err = 0;
	while (i < global.num_meals || !global.has_option)
	{
		err = eat(p->id, p->left, p->right, p);
		if (err)
			break ;
		usleep(global.time_to_sleep * MSECTOUSEC);
		err = update_status(p->id, "is thinking");
		if (err)
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
	int	i;

	global.start_time = utime();
	i = 0;
	while (i < global.num_philo)
	{
		params[i].id = i;
		params[i].left = &forks[i];
		params[i].right = &forks[(i + 1) % global.num_philo];
		params[i].num_meals = 0;
		params[i].last_meal = utime();
		pthread_create(&philosophers[i], NULL, philosopher, (void *)(params + i));
		i++;
	}
	i = 0;
	while (i < global.num_philo)
	{
		pthread_join(philosophers[i], NULL);
		/* printf("pthread_join returned! i: %d\n", i); */
		i++;
	}
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

