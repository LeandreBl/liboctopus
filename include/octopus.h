#ifndef _LIB_OCTOPUS_H_
#define _LIB_OCTOPUS_H_

#include <sys/epoll.h>
#include <pthread.h>
#include <clist.h>
#include <lvector.h>

typedef struct loctopus_s octopus_t;
typedef void (*octopus_callback_t)(octopus_t *poll, void *userdata);
typedef void (*octopus_fd_callback_t)(octopus_t *poll, int fd, void *userdata);
typedef void (*octopus_delay_callback_t)(octopus_t *poll, int timerfd,
					 void *userdata);

enum octopus_event {
	OCTOPUS_EVENT_READ = EPOLLIN,
	OCTOPUS_EVENT_WRITE = EPOLLOUT,
};

struct octopus_cbs {
    int (* on_trigger)(octopus_t *poll, void *octo_evt);
};

struct octopus_evt {
    struct octopus_cbs cbs;
	octopus_callback_t callback;
	void *userdata;
};

struct octopus_fd_evt {
    struct octopus_cbs cbs;
	octopus_fd_callback_t callback;
	void *userdata;
	int fd;
	enum octopus_event on_event;
};

struct loctopus_s {
	clist(struct octopus_evt) * events;
	pthread_mutex_t events_mutex;

	clist(struct octopus_evt) * events_to_add;

	clist(struct octopus_fd_evt) * fd_events;
	pthread_mutex_t fd_events_mutex;

	lvector(struct epoll_event) epoll_events;

	int epollfd;
}; /* octopus_t */

int octopus_create(octopus_t *poll);

int octopus_add_event(octopus_t *poll, octopus_callback_t callback,
		      void *userdata);

int octopus_add_delayed_event(octopus_t *poll,
			      octopus_delay_callback_t callback, void *userdata,
			      const struct itimerspec *timer);

int octopus_del_delayed_event(octopus_t *poll, int timerfd);

int octopus_monitor_add(octopus_t *poll, octopus_fd_callback_t callback,
			void *userdata, int fd, enum octopus_event on_event);

int octopus_monitor_delete(octopus_t *poll, int fd);

int octopus_wait(octopus_t *poll, long timeout);

void octopus_destroy(octopus_t *poll);

#endif /* !_LIB_OCTOPUS_H_ */
