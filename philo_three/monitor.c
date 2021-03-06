/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufukuya <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 16:52:29 by yufukuya          #+#    #+#             */
/*   Updated: 2021/03/06 16:58:02 by yufukuya         ###   ########.fr       */
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

static pid_t	wait_philosophers(void)
{
	pid_t	pid;
	int		i;
	int		status;
	int		count;

	count = 0;
	i = -1;
	while (++i < g_diner.num_philo)
	{
		status = 0;
		pid = waitpid(-1, &status, 0);
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) == 1)
				return (pid);
			else if (WEXITSTATUS(status) == 0)
			{
				if (++count == g_diner.num_philo)
					break ;
			}
		}
	}
	return (-1);
}

void			monitor_philosophers(pid_t *philosophers)
{
	pid_t	pid;
	int		i;

	pid = wait_philosophers();
	if (pid != -1)
	{
		i = 0;
		while (i < g_diner.num_philo)
		{
			if (philosophers[i] != pid)
				kill(philosophers[i], SIGKILL);
			i++;
		}
	}
}
