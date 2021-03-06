/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufukuya <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 15:39:54 by yufukuya          #+#    #+#             */
/*   Updated: 2021/03/06 15:43:15 by yufukuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "philosophers.h"

t_diner	g_diner;

void	no_free_will(pthread_t *philosophers,
						pthread_mutex_t *forks, t_params *params)
{
	int		i;

	g_diner.start_time = mtime();
	i = 0;
	while (i < g_diner.num_philo)
	{
		start_dining(i, philosophers + i, forks, params + i);
		usleep(50);
		i += 2;
	}
	i = 1;
	while (i < g_diner.num_philo)
	{
		start_dining(i, philosophers + i, forks, params + i);
		usleep(50);
		i += 2;
	}
	i = 0;
	while (i < g_diner.num_philo)
		pthread_join(philosophers[i++], NULL);
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
	return (0);
}

int		setup_locks(pthread_mutex_t *forks, t_params *params)
{
	int	i;

	i = 0;
	while (i < g_diner.num_philo)
	{
		pthread_mutex_init(&forks[i], NULL);
		pthread_mutex_init(&params[i].last_meal_lock, NULL);
		i++;
	}
	pthread_mutex_init(&g_diner.death_lock, NULL);
	return (0);
}

void	cleanup(pthread_t *philosophers, pthread_mutex_t *forks,
					t_params *params)
{
	int	i;

	i = 0;
	while (i < g_diner.num_philo)
	{
		pthread_mutex_destroy(&forks[i]);
		pthread_mutex_destroy(&params[i].last_meal_lock);
		i++;
	}
	pthread_mutex_destroy(&g_diner.death_lock);
	free(forks);
	free(philosophers);
	free(params);
}

int		main(int argc, char *argv[])
{
	pthread_t		*philosophers;
	pthread_mutex_t	*forks;
	t_params		*params;

	if (setup_diner(argc, argv) < 0)
		return (printf("Error: Invalid arguments\n"));
	forks = malloc(sizeof(pthread_mutex_t) * g_diner.num_philo);
	philosophers = malloc(sizeof(pthread_t) * g_diner.num_philo);
	params = malloc(sizeof(t_params) * g_diner.num_philo);
	setup_locks(forks, params);
	no_free_will(philosophers, forks, params);
	cleanup(philosophers, forks, params);
	return (0);
}
