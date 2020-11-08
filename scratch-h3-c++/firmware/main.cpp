#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <poll.h>

#define read timerfd_read

#include <time.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "debug.h"

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "display.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "storenetwork.h"
#include "storeremoteconfig.h"

#include "networkhandleroled.h"

#include "firmwareversion.h"

static const char SOFTWARE_VERSION[] = "0.0";

#include <h3/shell.h>

/**
 *
 */
#include "ntpclient.h"

#define ARGV1 	3
#define ARGV2 	1
#define ARGV3	10

#include "../include/unistd.h"

extern "C" {

void print_elapsed_time(void) {
	static struct timespec start;
	struct timespec curr;
	static int first_call = 1;
	int secs, nsecs;

	if (first_call) {
		first_call = 0;
		if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
			perror("clock_gettime");
	}

	if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1)
		perror("clock_gettime");

	secs = curr.tv_sec - start.tv_sec;
	nsecs = curr.tv_nsec - start.tv_nsec;

	if (nsecs < 0) {
		secs--;
		nsecs += 1000000000;
	}
	printf("%d.%03d: ", secs, (nsecs + 500000) / 1000000);
}

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();

	NetworkHandlerOled networkHandlerOled;

	nw.SetNetworkDisplay(&networkHandlerOled);
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Init(StoreNetwork::Get());
	nw.Print();

	networkHandlerOled.ShowIp();

	NtpClient ntpClient(nw.GetNtpServerIp());
	ntpClient.SetNtpClientDisplay(&networkHandlerOled);
	ntpClient.Start();
	ntpClient.Print();

	/**
	 *
	 */
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	printf("%d %d\n", tv.tv_sec, tp.tv_sec);

	/**
	 *
	 */
    struct itimerspec new_value;
    int max_exp, fd;
    struct timespec now;
    uint64_t exp = 1, tot_exp=0;
//    int s;

    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
    	perror("clock_gettime");

    /* Create a CLOCK_REALTIME absolute timer with initial
       expiration and interval as specified in command line */

#if 0
	new_value.it_value.tv_sec = now.tv_sec + ARGV1;
	new_value.it_value.tv_nsec = now.tv_nsec;
#else
	new_value.it_value.tv_sec = ARGV1;
	new_value.it_value.tv_nsec = 0;
#endif

	new_value.it_interval.tv_sec = ARGV2;
	max_exp = ARGV3;

	new_value.it_interval.tv_nsec = 0;

    fd = timerfd_create(CLOCK_REALTIME, 0);
    if (fd == -1)
    	perror("timerfd_create");
    else
    	printf("fd=%d", fd);

#if 0
    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
    	perror("timerfd_settime");
#else
    if (timerfd_settime(fd, 0, &new_value, NULL) == -1)
    	perror("timerfd_settime");
#endif

    print_elapsed_time();
    printf("timer started\n");

	RemoteConfig remoteConfig(REMOTE_CONFIG_RDMNET_LLRP_ONLY, REMOTE_CONFIG_MODE_CONFIG, 0);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	Shell shell;

	/**
	 *
	 */

	struct pollfd pollfd[1];
	pollfd[0].fd = fd;

	for (;;) {
		nw.Run();
		ntpClient.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		shell.Run();
		/*
		 *
		 */
		if (tot_exp < (uint64_t) max_exp) {
			exp = 1;
			int i = poll(pollfd, 1, -1);
			printf("exp=%lu, i=%d, pollfd[0].revents=%d\n", exp, i, pollfd[0].revents);

//			s = read(fd, &exp, sizeof(uint64_t));
//
//			if (s != sizeof(uint64_t)) {
//				perror("read");
//			}

			tot_exp += exp;
			print_elapsed_time();
			printf("read: %lu; total=%lu\n", exp, tot_exp);
		}
	}
}

}
