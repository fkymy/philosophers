/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   eat_sleep.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufukuya <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 16:05:28 by yufukuya          #+#    #+#             */
/*   Updated: 2021/03/06 16:05:41 by yufukuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
#include "philosophers.h"

void	update_last_meal(t_params *p)
{
	sem_wait(g_diner.sem_last_meal);
	p->num_meals++;
	p->last_meal = mtime();
	sem_post(g_diner.sem_last_meal);
}

int		ask_for_forks(int id)
{
	sem_wait(g_diner.sem_permits);
	sem_wait(g_diner.sem_forks);
	if (update_status(id, "has taken a fork") == STOP)
	{
		sem_post(g_diner.sem_forks);
		sem_post(g_diner.sem_permits);
		return (STOP);
	}
	sem_wait(g_diner.sem_forks);
	if (update_status(id, "has taken a fork") == STOP)
	{
		sem_post(g_diner.sem_forks);
		sem_post(g_diner.sem_forks);
		sem_post(g_diner.sem_permits);
		return (STOP);
	}
	sem_post(g_diner.sem_permits);
	return (0);
}

int		eat_sleep(int id, t_params *p)
{
	if (ask_for_forks(id) == STOP)
		return (STOP);
	if (update_status(id, "is eating") == STOP)
	{
		sem_post(g_diner.sem_forks);
		sem_post(g_diner.sem_forks);
		return (STOP);
	}
	update_last_meal(p);
	msleep(g_diner.time_to_eat);
	sem_post(g_diner.sem_forks);
	sem_post(g_diner.sem_forks);
	if (update_status(id, "is sleeping") == STOP)
		return (STOP);
	msleep(g_diner.time_to_sleep);
	return (0);
}
