/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   eat_sleep.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufukuya <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 15:33:13 by yufukuya          #+#    #+#             */
/*   Updated: 2021/03/06 15:41:06 by yufukuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "philosophers.h"

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

int		eat_sleep(int id, pthread_mutex_t *left, pthread_mutex_t *right,
		t_params *p)
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
	msleep(g_diner.time_to_eat);
	if (update_status(id, "is sleeping") == STOP)
	{
		putdown(left, right);
		return (STOP);
	}
	putdown(right, left);
	msleep(g_diner.time_to_sleep);
	return (0);
}
