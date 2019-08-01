#include <string.h>
#include <unistd.h>

#include "octopus.h"

int octopus_create(octopus_t *poll)
{
	memset(poll, 0, sizeof(*poll));
	poll->epollfd = epoll_create1(EPOLL_CLOEXEC);
	if (poll->epollfd == -1)
		goto error;
	if (pthread_mutex_init(&poll->events_mutex, NULL) == -1
	    || pthread_mutex_init(&poll->fd_events_mutex, NULL) == -1)
		goto error;
	return 0;
error:
	octopus_destroy(poll);
	return -1;
}

void octopus_destroy(octopus_t *poll)
{
	if (poll->epollfd != -1)
		close(poll->epollfd);
	clist_destroy(poll->events);
	clist_destroy(poll->fd_events);
	clist_destroy(poll->events_to_add);
	lvector_destroy(poll->epoll_events);
}
