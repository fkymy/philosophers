/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufukuya <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 16:14:32 by yufukuya          #+#    #+#             */
/*   Updated: 2021/03/06 17:00:21 by yufukuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
#include <signal.h>
#include "philosophers.h"

t_diner	g_diner;

void	start_dining(pid_t *philosophers, t_params *params)
{
	int	i;

	g_diner.start_time = mtime();
	i = 0;
	while (i < g_diner.num_philo)
	{
		params[i].id = i;
		params[i].num_meals = 0;
		if ((philosophers[i] = fork()) < 0)
			exit(1);
		else if (philosophers[i] == 0)
			philosopher(params + i);
		i++;
	}
	monitor_philosophers(philosophers);
}

void	setup_sem(void)
{
	g_diner.sem_forks = sem_open("/forks", O_CREAT | O_EXCL,
			S_IRWXU, g_diner.num_philo);
	g_diner.sem_permits = sem_open("/permits", O_CREAT | O_EXCL, S_IRWXU, 1);
	g_diner.sem_death = sem_open("/death", O_CREAT | O_EXCL, S_IRWXU, 1);
	g_diner.sem_last_meal = sem_open("/last_meal", O_CREAT | O_EXCL,
			S_IRWXU, 1);
	sem_unlink("/forks");
	sem_unlink("/permits");
	sem_unlink("/death");
	sem_unlink("/last_meal");
}

int		setup_diner(int argc, char *argv[])
{
	if (argc != 5 && argc != 6)
		return (-1);
	g_diner.num_philo = ft_atoi(argv[1]);
	g_diner.time_to_die = ft_atoi(argv[2]);
	g_diner.time_to_eat = ft_atoi(argv[3]);
	g_diner.time_to_sleep = ft_atoi(argv[4]);
	g_diner.num_meals = argc == 6 ? ft_atoi(argv[5]) : 0;
	g_diner.has_option = argc == 6 ? 1 : 0;
	g_diner.death = 0;
	if (g_diner.num_philo < 1 || g_diner.num_philo > MAXTHREADS)
		return (-1);
	if (g_diner.time_to_die < 0 || g_diner.time_to_eat < 0
			|| g_diner.time_to_sleep < 0 || g_diner.num_meals < 0)
		return (-1);
	setup_sem();
	return (0);
}

int		main(int argc, char *argv[])
{
	pid_t		*philosophers;
	t_params	*params;

	if (setup_diner(argc, argv) < 0)
		return (printf("Error: Invalid arguments\n"));
	params = malloc(sizeof(t_params) * g_diner.num_philo);
	philosophers = malloc(sizeof(pid_t) * g_diner.num_philo);
	start_dining(philosophers, params);
	sem_close(g_diner.sem_forks);
	sem_close(g_diner.sem_permits);
	sem_close(g_diner.sem_death);
	sem_close(g_diner.sem_last_meal);
	free(philosophers);
	free(params);
	return (0);
}
