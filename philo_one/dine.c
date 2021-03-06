/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dine.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufukuya <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 15:36:38 by yufukuya          #+#    #+#             */
/*   Updated: 2021/03/06 15:44:28 by yufukuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "philosophers.h"

void	print_status(time_t mtime, int id, char *msg)
{
	int	idx;

	idx = id + 1;
	printf("%ld %d %s\n", mtime - g_diner.start_time, idx, msg);
}

int		update_status(int id, char *msg)
{
	pthread_mutex_lock(&g_diner.death_lock);
	if (g_diner.death)
	{
		pthread_mutex_unlock(&g_diner.death_lock);
		return (STOP);
	}
	print_status(mtime(), id, msg);
	pthread_mutex_unlock(&g_diner.death_lock);
	return (0);
}

void	*timer(void *args)
{
	t_params	*p;

	p = args;
	while (1)
	{
		usleep(100);
		pthread_mutex_lock(&p->last_meal_lock);
		if (p->num_meals == g_diner.num_meals && g_diner.has_option)
			break ;
		if (mtime() - p->last_meal > g_diner.time_to_die)
		{
			pthread_mutex_lock(&g_diner.death_lock);
			if (!g_diner.death)
			{
				g_diner.death = 1;
				print_status(mtime(), p->id, "died");
			}
			pthread_mutex_unlock(&g_diner.death_lock);
			break ;
		}
		pthread_mutex_unlock(&p->last_meal_lock);
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
	pthread_create(&tid, NULL, timer, (void *)p);
	i = 0;
	while (i < g_diner.num_meals || !g_diner.has_option)
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

void	start_dining(int id, pthread_t *tid, pthread_mutex_t *forks,
						t_params *params)
{
	params->id = id;
	params->left = &forks[id];
	params->right = &forks[(id + 1) % g_diner.num_philo];
	params->num_meals = 0;
	params->last_meal = mtime();
	pthread_create(tid, NULL, philosopher, (void *)params);
}
