#include "octopus.h"

static int fd_evt_create(struct octopus_fd_evt *evt, const struct octopus_fd_evt *src, octopus_t *poll)
{
	struct epoll_event epevent = {
		.events = src->on_event,
		.data.ptr = evt,
	};
	if (epoll_ctl(poll->epollfd, EPOLL_CTL_ADD, src->fd, &epevent) == -1)
		return -1;
	*evt = *src;
	return 0;
}

int octopus_monitor_add(octopus_t *poll, octopus_fd_callback_t callback, void *userdata, int fd,
			enum octopus_event on_event)
{
	struct octopus_fd_evt evt;

	if (pthread_mutex_lock(&poll->fd_events_mutex) == -1)
		return -1;
	evt.callback = callback;
	evt.userdata = userdata;
	evt.fd = fd;
	evt.on_event = on_event;
	clist_emplace_back(poll->fd_events, NULL, fd_evt_create, &evt, poll);
	if (pthread_mutex_unlock(&poll->fd_events_mutex) == -1)
		return -1;
	return 0;
}

static int remove_by_fd(octopus_t *poll, int fd)
{
	clist_foreach(fd_evt, poll->fd_events)
	{
		if (fd_evt->fd == fd) {
			clist_erase(poll->fd_events, clist_from_object(fd_evt));
			return 0;
		}
	}
	return -1;
}

int octopus_monitor_delete(octopus_t *poll, int fd)
{
	if (pthread_mutex_lock(&poll->fd_events_mutex) == -1 || epoll_ctl(poll->epollfd, EPOLL_CTL_DEL, fd, NULL) == -1
	    || remove_by_fd(poll, fd) == -1 || pthread_mutex_unlock(&poll->fd_events_mutex) == -1)
		return -1;
	return 0;
}
