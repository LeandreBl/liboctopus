#include <sys/timerfd.h>
#include <unistd.h>

#include "octopus.h"

static int emplace_timerfd(struct octopus_fd_evt *evt, const struct octopus_fd_evt *src, octopus_t *poll)
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

static void destroy_timerfd(struct octopus_fd_evt *evt)
{
	if (evt->fd > 0)
		close(evt->fd);
}

static int on_trigger(octopus_t *poll, void *data)
{
    struct octopus_fd_evt *evt = data;
    uint64_t tmp;
    ssize_t ret;

    (void)poll;
    ret = read(evt->fd, &tmp, sizeof(tmp));
    if (ret <= 0 || ret != sizeof(tmp))
        return -1;
    return 0;
}

int octopus_add_delayed_event(octopus_t *poll,
			      octopus_delay_callback_t callback, void *userdata,
			      const struct itimerspec *timer)
{
	struct itimerspec oldval;
    struct octopus_fd_evt evt;
	int fd;

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
	if (fd == -1)
		return -1;
	if (timerfd_settime(fd, 0, timer, &oldval) < 0) {
		close(fd);
		return -1;
	}
	if (pthread_mutex_lock(&poll->fd_events_mutex) == -1)
		return -1;
    evt.callback = callback;
    evt.fd = fd;
    evt.userdata = userdata;
    evt.on_event = OCTOPUS_EVENT_READ;
    evt.cbs.on_trigger = &on_trigger;
	clist_emplace_back(poll->fd_events, destroy_timerfd, emplace_timerfd, &evt, poll);
	if (pthread_mutex_unlock(&poll->fd_events_mutex) == -1)
		return -1;
	return 0;
}

int octopus_del_delayed_event(octopus_t *poll, int timerfd)
{
	return octopus_monitor_delete(poll, timerfd);
}