# liboctopus

requires lvector, clist


code example
```c
#include <stdio.h>
#include <octopus.h>
#include <unistd.h>
#include <stdbool.h>

static void print_message(octopus_t *poll, void *userdata)
{
	/* This message will be printed after all monitored file descriptor are processed */
	printf("All events have been processed !\n");
}

static void display_output(octopus_t *poll, int fd, void *userdata)
{
	ssize_t rd;
	size_t total = 0;
	char buff[512];
	bool *running = userdata;

	do {
		rd = read(fd, buff, sizeof(buff));
		if (rd < 0)
			return;
		total += rd;
	} while (rd == sizeof(buff));
	printf("read: %zu bytes\n", total);

	/* If there is no data to read (after a ctrl + D for example) */
	if (total == 0)
		*running = false;
}

int main(void)
{
	octopus_t poll;
	bool running;

	/* Create an octopus instance */
	if (octopus_create(&poll) == -1)
		return -1;

	/* Tells octopus to monitor this file descriptor, and call the "display_output" callback if there is any readable data */
	if (octopus_monitor_add(&poll, &display_output, &running, 0, OCTOPUS_EVENT_READ) == -1) {
		fprintf(stderr, "monitor add failed\n");
		goto end;
	}

	running = true;
	while (running == true) {

		/* Add an event, that will be processed after all monitored file descriptor */
		if (octopus_add_event(&poll, &print_message, NULL) == -1) {
			fprintf(stderr, "add event failed\n");
			goto end;
		}

		/* Wait for any event to happend, here -1 means that we don't want to timeout */
		if (octopus_wait(&poll, -1) == -1) {
			fprintf(stderr, "Wait failed\n");
			goto end;
		}
	}
end:
	octopus_destroy(&poll);
	return 0;
}
```
