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

/*
** diner is read only
*/
typedef struct	s_diner
{
	int			num_philo;
	long		time_to_die;
	long		time_to_eat;
	long		time_to_sleep;
	long		num_meals;
	int			has_option;
	long		start_time;
}				t_diner;

typedef struct		s_params
{
	t_diner			*diner;
	int				id;
	pthread_mutex_t	*left;
	pthread_mutex_t	*right;
	pthread_mutex_t	*timer_lock;
	long			last_meal;
	int				num_meals;
	int				dead;
}					t_params;

typedef struct s_timer
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

void	update_last_meal(t_params *p, long utime)
{
	pthread_mutex_lock(p->timer_lock);
	p->num_meals++;
	p->last_meal = utime;
	pthread_mutex_unlock(p->timer_lock);
}

void	eat(int id, pthread_mutex_t *left, pthread_mutex_t *right, t_diner *d)
{
	print_debug(utime(), id, "waiting for left fork");
	pthread_mutex_lock(left);
	print_status(utime(), id, "has taken a fork");
	// usleep(1000 * MSECTOUSEC); // for deadlock check
	pthread_mutex_lock(right);
	print_status(utime(), id, "has taken a fork");
	print_status(utime(), id, "is eating");
	usleep(d->time_to_eat * MSECTOUSEC);
	print_status(utime(), id, "is sleeping");
	pthread_mutex_unlock(left);
	pthread_mutex_unlock(right);
}

void	*philosopher(void *args)
{
	t_params	*p;
	t_diner		*d;
	int			i;

	p = args;
	d = p->diner;
	print_debug(utime(), p->id, "thread started");
	i = 0;
	while (i < d->num_meals || !d->has_option)
	{
		eat(p->id, p->left, p->right, d);
		usleep(d->time_to_die * 1000);
		usleep(d->time_to_sleep * MSECTOUSEC);
		print_status(utime(), p->id, "is thinking");
		i++;
	}
	return ((void *)p);
}

int	setup_diner(t_diner *d, int argc, char *argv[])
{
	if (argc != 5 && argc != 6)
		return (-1);
	d->num_philo = ft_atoi(argv[1]);
	d->time_to_die = ft_atoi(argv[2]);
	d->time_to_eat = ft_atoi(argv[3]);
	d->time_to_sleep = ft_atoi(argv[4]); d->num_meals = argc == 6 ? ft_atoi(argv[5]) : 0;
	d->has_option = argc == 6 ? 1 : 0;
	if (d->num_philo <= 1 || d->num_philo > MAXTHREADS)
		return (-1);
	if (d->time_to_die < 0 || d->time_to_eat < 0 || d->time_to_sleep < 0 || d->num_meals < 0)
		return (-1);
	return (0);
}

int	main(int argc, char *argv[])
{
	t_diner			d;
	pthread_t		*philosophers;
	pthread_mutex_t	*forks;
	t_params		*params;

	if (setup_diner(&d, argc, argv) < 0)
		return (printf("Error: Invalid arguments\n"));
	/* init */
	forks = malloc(sizeof(pthread_mutex_t) * d.num_philo);
	philosophers = malloc(sizeof(pthread_t) * d.num_philo);
	params = malloc(sizeof(t_params) * d.num_philo);

	int i = 0;
	while (i < d.num_philo)
		pthread_mutex_init(&forks[i++], NULL);
	i = 0;
	d.start_time = utime();
	g_start_time = utime();
	while (i < d.num_philo)
	{
		params[i].diner = &d;
		params[i].id = i;
		params[i].left = &forks[i];
		params[i].right = &forks[(i + 1) % d.num_philo];
		pthread_create(&philosophers[i], NULL, philosopher, (void *)(params + i));
		i++;
	}

	t_params *ret;
	int j = 0;
	while (j < d.num_philo)
	{
		pthread_join(philosophers[j], (void *)&ret);
		printf("pthread_join returned! j: %d, id: %d\n", j, ret->id);
		j++;
	}
	free(forks);
	free(philosophers);
	free(params);
	return (0);
}

