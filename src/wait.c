#include "octopus.h"

static int process_fd(octopus_t *poll, long timeout)
{
	int count;
	struct octopus_fd_evt *p;

	lvector_reserve(poll->epoll_events, clist_size(poll->fd_events));
	if (pthread_mutex_lock(&poll->fd_events_mutex) == -1)
		return -1;
	count = epoll_wait(poll->epollfd, poll->epoll_events.arr, poll->epoll_events.rsize, timeout);
	if (count < 0)
		return -1;
	poll->epoll_events.size = (size_t)count;
	lvector_foreach(event, poll->epoll_events)
	{
		p = event->data.ptr;
		if (event->events == p->on_event)
			p->callback(poll, p->fd, p->userdata);
	}
	if (pthread_mutex_unlock(&poll->fd_events_mutex) == -1)
		return -1;
	return count;
}

static int move_all_to_add(octopus_t *poll)
{
	if (pthread_mutex_lock(&poll->events_mutex) == -1)
		return -1;
	while (poll->events_to_add != NULL) {
		clist_move_back(poll->events_to_add, poll->events);
	}
	if (pthread_mutex_unlock(&poll->events_mutex) == -1)
		return -1;
	return 0;
}

static int process_event(octopus_t *poll)
{
	do {
		if (move_all_to_add(poll) == -1)
			return -1;
		while (poll->events != NULL) {
			poll->events->object.callback(poll, poll->events->object.userdata);
			clist_erase(poll->events, poll->events);
		}
	} while (poll->events != NULL);
	return 0;
}

int octopus_wait(octopus_t *poll, long timeout)
{
	if (process_fd(poll, timeout) == -1 || process_event(poll) == -1)
		return -1;
	return 0;
}
