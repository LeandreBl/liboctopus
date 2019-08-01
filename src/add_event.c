#include "octopus.h"

static int evt_create(struct octopus_evt *evt, octopus_callback_t callback, void *userdata)
{
	evt->callback = callback;
	evt->userdata = userdata;
	return 0;
}

int octopus_add_event(octopus_t *poll, octopus_callback_t callback, void *userdata)
{
	if (pthread_mutex_lock(&poll->events_mutex) == -1)
		return -1;
	clist_emplace_back(poll->events_to_add, NULL, evt_create, callback, userdata);
	if (pthread_mutex_unlock(&poll->events_mutex) == -1)
		return -1;
	return 0;
}
