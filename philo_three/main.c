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
#include <semaphore.h>

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
	int				done;
	sem_t			*sem_permits;
	sem_t			*sem_forks;
	sem_t			*sem_death;
	sem_t			*sem_stop;
	sem_t			*sem_done;
	sem_t			*sem_last_meal;
}					t_diner;

t_diner	global;

typedef struct		s_philo
{
	int				id;
	pid_t			pid;
	long			last_meal;
	int				num_meals;
}					t_philo;

typedef struct		s_params
{
	int				id;
	long			last_meal;
	int				num_meals;
}					t_params;

long	utime(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000 + tv.tv_usec);
}

long	mtime(void)
{
	struct timeval tv;

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
	sem_wait(global.sem_death);
	if (global.death)
	{
		sem_post(global.sem_death);
		return (STOP);
	}
	print_status(mtime(), id, msg);
	sem_post(global.sem_death);
	return (0);
}

void	update_last_meal(t_params *p)
{
	sem_wait(global.sem_last_meal);
	p->num_meals++;
	p->last_meal = mtime();
	sem_post(global.sem_last_meal);
}

int		ask_for_forks(int id)
{
	sem_wait(global.sem_permits);
	sem_wait(global.sem_forks);
	if (update_status(id, "has taken a fork") == STOP)
	{
		sem_post(global.sem_forks);
		sem_post(global.sem_permits);
		return (STOP);
	}
	sem_wait(global.sem_forks);
	if (update_status(id, "has taken a fork") == STOP)
	{
		sem_post(global.sem_forks);
		sem_post(global.sem_forks);
		sem_post(global.sem_permits);
		return (STOP);
	}
	sem_post(global.sem_permits);
	return (0);
}

int		eat_sleep(int id, t_params *p)
{
	if (ask_for_forks(id) == STOP)
		return (STOP);
	if (update_status(id, "is eating") == STOP)
	{
		sem_post(global.sem_forks);
		sem_post(global.sem_forks);
		return (STOP);
	}
	update_last_meal(p);
	msleep(global.time_to_eat);
	sem_post(global.sem_forks);
	sem_post(global.sem_forks);
	if (update_status(id, "is sleeping") == STOP)
		return (STOP);
	msleep(global.time_to_sleep);
	return (0);
}

void	*timer(void *args)
{
	t_philo	*p;
	long		now;

	p = args;
	while (1)
	{
		sem_wait(global.sem_last_meal);
		if (p->num_meals == global.num_meals && global.has_option)
			break ;
		now = mtime();
		if (now - p->last_meal > global.time_to_die)
		{
			sem_wait(global.sem_death);
			if (!global.death)
			{
				global.death = 1;
				print_status(now, p->id, "died");
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
	int			status;
	pid_t		pid;

	p = args;

	pid = fork();
	if (pid == 0)
	{
		pthread_create(&tid, NULL, timer, (void *)p);
		i = 0;
		while (i < global.num_meals || !global.has_option)
		{
			if (eat_sleep(p->id, p) == STOP)
				exit(1);
			if (update_status(p->id, "is thinking") == STOP)
				exit(1);
			i++;
		}
		pthread_join(tid, NULL);
		exit(0);
	}

	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
	{
		if (WEXITSTATUS(status) == 1)
		{
			// kill all except p->id
			sem_post(global.sem_stop);
		}
		else if (WEXITSTATUS(status) == 0)
		{
			sem_wait(global.sem_done);
			global.done++;
			if (global.done == global.num_philo)
				sem_post(global.sem_stop);
			sem_post(global.sem_done);
		}
	}
	return ((void *)p);
}

void	*philosopher_old(void *args)
{
	t_params	*p;
	pthread_t	tid;
	int			i;

	p = args;
	pthread_create(&tid, NULL, timer, (void *)p);
	i = 0;
	while (i < global.num_meals || !global.has_option)
	{
		if (eat_sleep(p->id, p) == STOP)
			break ;
		if (update_status(p->id, "is thinking") == STOP)
			break ;
		i++;
	}
	pthread_join(tid, NULL);
	return ((void *)p);
}

void	monitor_processes(t_philo *philosophers)
{
	int	i;
	int	status;
	int	count;

	count = 0;
	while (1)
	{
		i = 0;
		while (i < global.num_philo)
		{
			status = 0;
			waitpid(philosophers[i].pid, &status, WNOHANG);
			if (status == 256 || status == 0)
			{
				if (status == 0)
					if (++count != global.num_meals)
						continue ;
				break ;
			}
			i++;
		}
		// if (status == 256 || count == global.num_meals)
		// kill all
	}
}

void	start_dining_act(t_philo *philosophers)
{
	int		i;
	long	now;

	now = mtime();
	global.start_time = now;
	i = 0;
	while (i < global.num_philo)
	{
		philosophers[i].id = i;
		philosophers[i].num_meals = 0;
		philosophers[i].last_meal = now;
		if ((philosophers[i].pid = fork()) < 0)
			exit(1);
		else if (philosophers[i].pid == 0)
		{
			philosopher(philosophers + i);
			exit(global.death);
		}
		i++;
	}
	monitor_processes(philosophers);
}

void	start_dining(pthread_t *philosophers, t_params *params)
{
	int		i;
	long	now;

	now = mtime();
	global.start_time = now;
	i = 0;
	while (i < global.num_philo)
	{
		params[i].id = i;
		params[i].num_meals = 0;
		params[i].last_meal = now;
		pthread_create(&philosophers[i], NULL, philosopher, (void *)(params + i));
		pthread_detach(philosophers[i]);
		i++;
	}
	sem_wait(global.sem_stop);
	printf("sem_stop posted!\n");
}

void	start_dining_old(pthread_t *philosophers, t_philo *params)
{
	int		i;
	long	now;

	now = mtime();
	global.start_time = now;
	i = 0;
	while (i < global.num_philo)
	{
		params[i].id = i;
		params[i].num_meals = 0;
		params[i].last_meal = now;
		pthread_create(&philosophers[i], NULL, philosopher, (void *)(params + i));
		i++;
	}
	i = 0;
	while (i < global.num_philo)
		pthread_join(philosophers[i++], NULL);
}

void	setup_sem(void)
{
	// these can return SEM_FAILED
	global.sem_forks = sem_open("/forks", O_CREAT | O_EXCL, 0644, global.num_philo);
	global.sem_permits = sem_open("/permits", O_CREAT | O_EXCL, 0644, 1);
	global.sem_death = sem_open("/death", O_CREAT | O_EXCL, 0644, 1);
	global.sem_last_meal = sem_open("/last_meal", O_CREAT | O_EXCL, 0644, 1);
	global.sem_stop = sem_open("/stop", O_CREAT | O_EXCL, 0644, 0);
	global.sem_done = sem_open("/done", O_CREAT | O_EXCL, 0644, 1);
	sem_unlink("/forks");
	sem_unlink("/permits");
	sem_unlink("/death");
	sem_unlink("/last_meal");
	sem_unlink("/stop");
	sem_unlink("/done");
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
	global.done = 0;
	if (global.num_philo <= 1 || global.num_philo > MAXTHREADS)
		return (-1);
	if (global.time_to_die < 0 || global.time_to_eat < 0 || global.time_to_sleep < 0 || global.num_meals < 0)
		return (-1);
	setup_sem();
	return (0);
}

int	main(int argc, char *argv[])
{
	pthread_t	*philosophers;
	t_params	*params;

	if (setup_diner(argc, argv) < 0)
		return (printf("Error: Invalid arguments\n"));
	params = malloc(sizeof(t_params) * global.num_philo);
	philosophers = malloc(sizeof(pthread_t) * global.num_philo);
	start_dining(philosophers, params);

	/* clean up */
	sem_close(global.sem_forks);
	sem_close(global.sem_permits);
	sem_close(global.sem_death);
	sem_close(global.sem_last_meal);
	free(philosophers);
	return (0);
}

