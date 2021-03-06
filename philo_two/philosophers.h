/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosophers.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufukuya <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 15:27:23 by yufukuya          #+#    #+#             */
/*   Updated: 2021/03/06 16:12:09 by yufukuya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILOSOPHERS_H
# define PHILOSOPHERS_H

# define MAXTHREADS 200
# define SECTONSEC 1000000000
# define USECTONSEC 1000
# define MSECTOUSEC 1000
# define STOP 1

typedef struct		s_diner
{
	int				num_philo;
	time_t			time_to_die;
	time_t			time_to_eat;
	time_t			time_to_sleep;
	int				num_meals;
	int				has_option;
	time_t			start_time;
	int				death;
	sem_t			*sem_permits;
	sem_t			*sem_forks;
	sem_t			*sem_death;
	sem_t			*sem_last_meal;
}					t_diner;

extern t_diner		g_diner;

typedef struct		s_params
{
	int				id;
	time_t			last_meal;
	int				num_meals;
}					t_params;

time_t				utime(void);
time_t				mtime(void);
void				msleep(time_t end);
int					ft_atoi(const char *s);

void				print_status(time_t mtime, int id, char *msg);
int					update_status(int id, char *msg);
void				*timer(void *args);
void				*philosopher(void *args);
void				start_dining(pthread_t *philosophers, t_params *params);

void				update_last_meal(t_params *p);
int					ask_for_forks(int id);
int					eat_sleep(int id, t_params *p);

#endif
