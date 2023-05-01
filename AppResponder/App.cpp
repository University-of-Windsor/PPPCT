
// App.cpp : Defines the entry point for the console application.
#include <stdio.h>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <signal.h>

#include "EnclaveResponder_u.h"
#include "sgx_eid.h"
#include "sgx_urts.h"

#include "fifo_def.h"
#include "datatypes.h"
#include <iostream>

#include "CPTask.h"
#include "CPServer.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

uint32_t print_ocall(uint32_t *digit);


#define UNUSED(val) (void)(val)
#define TCHAR   char
#define _TCHAR  char
#define _T(str) str
#define scanf_s scanf
#define _tmain  main

CPTask * g_cptask = NULL;
CPServer * g_cpserver = NULL;


void signal_handler(int sig)
{
    switch(sig)
    {
        case SIGINT:
        case SIGTERM:
        {
            if (g_cpserver)
                g_cpserver->shutDown();
        }
        break;
    default:
        break;
    }

    exit(1);
}

void cleanup()
{
    if(g_cptask != NULL)
        delete g_cptask;
    if(g_cpserver != NULL)
        delete g_cpserver;
}

int  main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // create server instance, it would listen on sockets and proceeds client's requests
    g_cptask = new (std::nothrow) CPTask;
    g_cpserver = new (std::nothrow) CPServer(g_cptask);

    if (!g_cptask || !g_cpserver)
         return -1;

    atexit(cleanup);

    // register signal handler so to respond to user interception
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    g_cptask->start();

    if (g_cpserver->init() != 0)
    {
         printf("fail to init server\n");
    }else
    {
         printf("Server is ON...\n");
         printf("Press Ctrl+C to exit...\n");
         g_cpserver->doWork();
    }
    
    return 0;
}

/*extern "C" uint32_t print_ocall(uint32_t * digit)
{
	//memcpy(outbound, (*inbound), 200);

std::cout << "value: " << (*digit) << "\n";
	return (uint32_t)0;
}

*/