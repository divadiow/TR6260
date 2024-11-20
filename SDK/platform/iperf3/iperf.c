/*
 * iperf, Copyright (c) 2014, 2015, 2017, The Regents of the University of
 * California, through Lawrence Berkeley National Laboratory (subject
 * to receipt of any required approvals from the U.S. Dept. of
 * Energy).  All rights reserved.
 *
 * If you have questions about your rights to use or distribute this
 * software, please contact Berkeley Lab's Technology Transfer
 * Department at TTD@lbl.gov.
 *
 * NOTICE.  This software is owned by the U.S. Department of Energy.
 * As such, the U.S. Government has been granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable,
 * worldwide license in the Software to reproduce, prepare derivative
 * works, and perform publicly and display publicly.  Beginning five
 * (5) years after the date permission to assert copyright is obtained
 * from the U.S. Department of Energy, and subject to any subsequent
 * five (5) year renewals, the U.S. Government is granted for itself
 * and others acting on its behalf a paid-up, nonexclusive,
 * irrevocable, worldwide license in the Software to reproduce,
 * prepare derivative works, distribute copies to the public, perform
 * publicly and display publicly, and to permit others to do so.
 *
 * This code is distributed under a BSD style license, see the LICENSE
 * file for complete information.
 */
#include "iperf_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
//#include <errno.h>
#include <signal.h>
#include <unistd.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <sys/socket.h>
#include <sys/types.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "iperf.h"
#include "iperf_api.h"
#include "units.h"
#include "iperf_locale.h"
#include "net.h"


static int run(struct iperf_test *test);


/**************************************************************************/
#ifndef __TR_SW__
int
main(int argc, char **argv)
#else
int
iperf3_main(int argc , char *argv[])
#endif
{
    struct iperf_test *test;
#ifdef __TR_SW__
    int ret = CMD_RET_SUCCESS;
#endif

    // XXX: Setting the process affinity requires root on most systems.
    //      Is this a feature we really need?
#ifdef TEST_PROC_AFFINITY
    /* didnt seem to work.... */
    /*
     * increasing the priority of the process to minimise packet generation
     * delay
     */
    int rc = setpriority(PRIO_PROCESS, 0, -15);

    if (rc < 0) {
        perror("setpriority:");
        fprintf(stderr, "setting priority to valid level\n");
        rc = setpriority(PRIO_PROCESS, 0, 0);
    }
    
    /* setting the affinity of the process  */
    cpu_set_t cpu_set;
    int affinity = -1;
    int ncores = 1;

    sched_getaffinity(0, sizeof(cpu_set_t), &cpu_set);
    if (errno)
        perror("couldn't get affinity:");

    if ((ncores = sysconf(_SC_NPROCESSORS_CONF)) <= 0)
        err("sysconf: couldn't get _SC_NPROCESSORS_CONF");

    CPU_ZERO(&cpu_set);
    CPU_SET(affinity, &cpu_set);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set) != 0)
        err("couldn't change CPU affinity");
#endif

    test = iperf_new_test();
    if (!test)
        iperf_errexit(NULL, "create new test error - %s", iperf_strerror(i_errno));
    iperf_defaults(test);	/* sets defaults */

    if ((ret = iperf_parse_arguments(test, argc, argv)) < 0) {
#ifdef __TR_SW__
        if (ret != -2)
#endif
        iperf_err(test, "parameter error - %s", iperf_strerror(i_errno));
#ifndef __TR_SW__
        fprintf(stderr, "\n");
        usage_long(stdout);
        exit(1);
#else
        if (ret == -2) {
            iperf_free_test(test);
            return 0;
        } else {
            usage_long();
        }
#endif
    }

    if (run(test) < 0) {
        iperf_errexit(test, "error - %s", iperf_strerror(i_errno));
#ifdef __TR_SW__
	ret = CMD_RET_FAILURE;
#endif
    }

    iperf_free_test(test);

#ifndef __TR_SW__
    return 0;
#else
    return ret;
#endif
}

#ifndef __TR_SW__
static jmp_buf sigend_jmp_buf;

static void __attribute__ ((noreturn))
sigend_handler(int sig)
{
    longjmp(sigend_jmp_buf, 1);
}
#endif

/**************************************************************************/
static int
run(struct iperf_test *test)
{
#ifndef __TR_SW__
    /* Termination signals. */
    iperf_catch_sigend(sigend_handler);
    if (setjmp(sigend_jmp_buf))
	iperf_got_sigend(test);

    /* Ignore SIGPIPE to simplify error handling */
    signal(SIGPIPE, SIG_IGN);
#endif

    switch (test->role) {
        case 's':
#ifndef __TR_SW__            
	    if (test->daemon) {
		int rc;
		rc = daemon(0, 0);
		if (rc < 0) {
		    i_errno = IEDAEMON;
		    iperf_errexit(test, "error - %s", iperf_strerror(i_errno));
		}
	    }
	    if (iperf_create_pidfile(test) < 0) {
		i_errno = IEPIDFILE;
		iperf_errexit(test, "error - %s", iperf_strerror(i_errno));
	    }
#endif
            for (;;) {
		int rc;
		rc = iperf_run_server(test);
		if (rc < 0) {
#ifdef __TR_SW__
		    {
		        struct iperf_stream *sp;

		        /* Close all stream sockets */
		        SLIST_FOREACH(sp, &test->streams, streams) {
			    close(sp->socket);
		        }
		    }
#endif
		    iperf_err(test, "error - %s", iperf_strerror(i_errno));
		    if (rc < -1) {
		        iperf_errexit(test, "exiting");
#ifdef __TR_SW__
			break;
#endif
		    }
                }
                iperf_reset_test(test);
                if (iperf_get_test_one_off(test))
                    break;
#ifdef __TR_SW__
		if (i_errno == IESERVERTERM) {
		    i_errno = IENONE;
		    break;
		}
#endif
            }
#ifndef __TR_SW__
	    iperf_delete_pidfile(test);
#endif
            break;
	case 'c':
	    if (iperf_run_client(test) < 0) {
#ifdef __TR_SW__
		{
	            struct iperf_stream *sp;

		    /* Close all stream sockets */
		    SLIST_FOREACH(sp, &test->streams, streams) {
		        close(sp->socket);
		    }
	        }
#endif

		iperf_errexit(test, "error - %s", iperf_strerror(i_errno));
	    }
            break;
        default:
            usage();
            break;
    }

#ifndef __TR_SW__
    iperf_catch_sigend(SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
#endif

    return 0;
}

#ifdef __TR_SW__
iperf3_test_status_e iperf3_run_status = IPERF3_STATUS_IDLE;
static TaskHandle_t iperf_task_handle = NULL;
char ipef3_clibuf[512] = {0};
#include "lwip/sys.h"
#include "system_priority.h"

static int isblank2(char c)
{
    return (( c == ' ' ) || ( c == '\t' ));
}

#define IPERF3_MAX_ARGC 16
static int iperf3_cmd_parse_line(char *s, char *argv[])
{
	int argc = 0;

	while (argc < IPERF3_MAX_ARGC) {
		while (isblank2(*s))
			s++;

		if (*s == '\0')
			goto out;

		argv[argc++] = s;

		while (*s && !isblank2(*s))
			s++;

		if (*s == '\0')
			goto out;

		*s++ = '\0';
	}

	printf("Too many args\n");

 out:
	argv[argc] = NULL;
	return argc;
}

int is_iperf3_stop_request(void)
{
    return IPERF3_STATUS_STOPPING == iperf3_run_status;
}

static void iperf3_thread(void *arg)
{
    char *argv[IPERF3_MAX_ARGC] = {NULL};
    int argc = 0;
    
    while(1) {
        if (IPERF3_STATUS_RUNNING == iperf3_run_status) {
            argc = iperf3_cmd_parse_line(ipef3_clibuf, argv);
            if (!strlen(ipef3_clibuf) || !argc) {
                iperf3_run_status = IPERF3_STATUS_IDLE;
                continue;
            }
            /*{
                int i = 0;
                for (i = 0; i < argc; ++i) {
                    printf("argv[%d]:[%s]\n", i, argv[i]);
                }
            }*/
            iperf3_main(argc, argv);
            iperf3_run_status = IPERF3_STATUS_IDLE;
        }

        sys_arch_msleep(100);
    }
}

#ifdef IPERF_SUPPORT
int iperf3_test(cmd_tbl_t *t, int argc , char *argv[])
{
    int i = 0;

    if (argc <= 1)
        return CMD_RET_FAILURE;
    if (argc == 2) {
        if (0 == strcmp(argv[1], "stop")) {
            if (iperf3_run_status == IPERF3_STATUS_IDLE) {
                printf("not run, nothing to stop!\n");
            } else {
                if (IPERF3_STATUS_STOPPING != iperf3_run_status)
                    iperf3_run_status = IPERF3_STATUS_STOPPING;
            }
            return CMD_RET_SUCCESS;
        } else if (0 == strcmp(argv[1], "-h")) {
            return iperf3_main(argc, argv);
        }
        return CMD_RET_FAILURE;
    }
    if (IPERF3_STATUS_IDLE != iperf3_run_status) {
        printf("iperf3 is busy now....%d\n", iperf3_run_status);
        return CMD_RET_FAILURE;
    }

    memset(ipef3_clibuf, 0, sizeof(ipef3_clibuf));
	sprintf(ipef3_clibuf, "%s", argv[0]);
	for (i = 1; i < argc; i++) {
        sprintf(ipef3_clibuf, "%s %s", ipef3_clibuf, argv[i]);
	}

    //printf("%s\n", ipef3_clibuf);
    iperf3_run_status = IPERF3_STATUS_RUNNING;

    if(!iperf_task_handle) {
		iperf_task_handle = sys_thread_new("iperf3_thread", iperf3_thread, NULL, \
			LWIP_IPERF_TASK_STACK_SIZE, THREAD_IPERF_PRI);
	}

    return CMD_RET_SUCCESS;
}

CMD(iperf, iperf3_test, "iperf3 cli", "iperf3 test cli");
#endif
#endif
