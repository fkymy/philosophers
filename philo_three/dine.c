/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dine.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufukuya <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 16:51:32 by yufukuya          #+#    #+#             */
/*   Updated: 2021/03/06 17:00:28 by yufukuya         ###   ########.fr       */
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

void	print_status(time_t mtime, int id, char *msg)
{
	int		idx;

	idx = id + 1;
	printf("%ld %d %s\n", mtime - g_diner.start_time, idx, msg);
}

int		update_status(int id, char *msg)
{
	sem_wait(g_diner.sem_death);
	if (g_diner.death)
	{
		sem_post(g_diner.sem_death);
		return (STOP);
	}
	print_status(mtime(), id, msg);
	sem_post(g_diner.sem_death);
	return (0);
}

void	*timer(void *args)
{
	t_params	*p;

	p = args;
	while (1)
	{
		usleep(100);
		sem_wait(g_diner.sem_last_meal);
		if (p->num_meals == g_diner.num_meals && g_diner.has_option)
			break ;
		if (mtime() - p->last_meal > g_diner.time_to_die)
		{
			sem_wait(g_diner.sem_death);
			if (!g_diner.death)
			{
				g_diner.death = 1;
				print_status(mtime(), p->id, "died");
				exit(1);
			}
			sem_post(g_diner.sem_death);
			break ;
		}
		sem_post(g_diner.sem_last_meal);
	}
	sem_post(g_diner.sem_last_meal);
	return (NULL);
}

void	*philosopher(void *args)
{
	t_params	*p;
	pthread_t	tid;
	int			i;

	p = args;
	p->last_meal = mtime();
	pthread_create(&tid, NULL, timer, (void *)p);
	i = 0;
	while (i < g_diner.num_meals || !g_diner.has_option)
	{
		if (eat_sleep(p->id, p) == STOP)
			exit(1);
		if (update_status(p->id, "is thinking") == STOP)
			exit(1);
		i++;
	}
	pthread_join(tid, NULL);
	exit(0);
	return ((void *)p);
}
