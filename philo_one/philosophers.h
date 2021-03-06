/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosophers.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufukuya <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 15:27:23 by yufukuya          #+#    #+#             */
/*   Updated: 2021/03/06 15:45:53 by yufukuya         ###   ########.fr       */
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
	pthread_mutex_t	death_lock;
}					t_diner;

extern t_diner		g_diner;

typedef struct		s_params
{
	int				id;
	pthread_mutex_t	*left;
	pthread_mutex_t	*right;
	pthread_mutex_t	last_meal_lock;
	time_t			last_meal;
	int				num_meals;
}					t_params;

time_t				utime(void);
time_t				mtime(void);
void				msleep(time_t end);
int					ft_atoi(const char *s);

void				update_last_meal(t_params *p);
int					pickup(t_params *p,
						pthread_mutex_t *first, pthread_mutex_t *second);
void				putdown(pthread_mutex_t *first, pthread_mutex_t *second);
int					eat_sleep(int id, pthread_mutex_t *left,
						pthread_mutex_t *right, t_params *p);

void				print_status(time_t mtime, int id, char *msg);
int					update_status(int id, char *msg);
void				*timer(void *args);
void				*philosopher(void *args);
void				start_dining(int id, pthread_t *tid, pthread_mutex_t *forks,
						t_params *params);

#endif
