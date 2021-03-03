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
#include <semaphore.h>

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
	// pthread_mutex_t	death_lock;
	sem_t			*sem_permits;
	sem_t			*sem_forks;
	sem_t			*sem_death;
	sem_t			*sem_last_meal;
}					t_diner;

t_diner	global;

typedef struct		s_params
{
	int				id;
	// pthread_mutex_t	*left;
	// pthread_mutex_t	*right;
	// pthread_mutex_t	last_meal_lock;
	long			last_meal;
	int				num_meals;
}					t_params;

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
	sem_wait(global.sem_death);
	if (global.death)
	{
		sem_post(global.sem_death);
		return (1);
	}
	print_status(utime(), id, msg);
	sem_post(global.sem_death);
	return (0);
}

void	update_last_meal(t_params *p, long utime)
{
	sem_wait(global.sem_last_meal);
	p->num_meals++;
	p->last_meal = utime;
	sem_post(global.sem_last_meal);
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

int		eat(int id, t_params *p)
{
	int	err;

	sem_wait(global.sem_permits);
	sem_wait(global.sem_forks);
	err = update_status(id, "has taken a fork");
	if (err)
	{
		sem_post(global.sem_forks);
		sem_post(global.sem_permits);
		return (err);
	}
	sem_wait(global.sem_forks);
	err = update_status(id, "has taken a fork");
	if (err)
	{
		sem_post(global.sem_forks);
		sem_post(global.sem_forks);
		sem_post(global.sem_permits);
		return (err);
	}
	sem_post(global.sem_permits);
	err = update_status(id, "is eating");
	if (err)
	{
		sem_post(global.sem_forks);
		sem_post(global.sem_forks);
		return (err);
	}
	update_last_meal(p, utime());
	ft_sleep(global.time_to_eat * MSECTOUSEC);
	sem_post(global.sem_forks);
	sem_post(global.sem_forks);
	err = update_status(id, "is sleeping");
	if (err)
		return (err);
	return (0);
}

void	*timer(void *args)
{
	t_params	*p;
	long		current_utime;

	p = args;
	while (1)
	{
		sem_wait(global.sem_last_meal);
		if (p->num_meals == global.num_meals && global.has_option)
			break ;
		current_utime = utime();
		if (current_utime - p->last_meal > global.time_to_die * 1000)
		{
			sem_wait(global.sem_death);
			if (!global.death)
			{
				global.death = 1;
				print_status(current_utime, p->id, "died");
			}
			sem_post(global.sem_death);
			break ;
		}
		sem_post(global.sem_last_meal);
		usleep(10);
	}
	sem_post(global.sem_last_meal);
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
		err = eat(p->id, p);
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

void	start_dining(pthread_t *philosophers, t_params *params)
{
	int	i;

	global.start_time = utime();
	i = 0;
	while (i < global.num_philo)
	{
		params[i].id = i;
		params[i].num_meals = 0;
		params[i].last_meal = utime();
		pthread_create(&philosophers[i], NULL, philosopher, (void *)(params + i));
		i++;
	}
	i = 0;
	while (i < global.num_philo)
	{
		pthread_join(philosophers[i], NULL);
		printf("pthread_join returned! i: %d\n", i);
		i++;
	}
}

void	setup_sem(void)
{
	global.sem_forks = sem_open("/forks", O_CREAT | O_EXCL, 0644, global.num_philo);
	global.sem_permits = sem_open("/permits", O_CREAT | O_EXCL, 0644, global.num_philo / 2);
	global.sem_death = sem_open("/death", O_CREAT | O_EXCL, 0644, 1);
	global.sem_last_meal = sem_open("/last_meal", O_CREAT | O_EXCL, 0644, 1);
	sem_unlink("/forks");
	sem_unlink("/permits");
	sem_unlink("/death");
	sem_unlink("/last_meal");
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
	setup_sem();
	return (0);
}

int	main(int argc, char *argv[])
{
	pthread_t		*philosophers;
	t_params		*params;

	if (setup_diner(argc, argv) < 0)
		return (printf("Error: Invalid arguments\n"));
	philosophers = malloc(sizeof(pthread_t) * global.num_philo);
	params = malloc(sizeof(t_params) * global.num_philo);
	start_dining(philosophers, params);

	/* clean up */
	sem_close(global.sem_forks);
	sem_close(global.sem_permits);
	sem_close(global.sem_death);
	sem_close(global.sem_last_meal);
	free(philosophers);
	free(params);
	return (0);
}

