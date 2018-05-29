#include "debug.h"
#include <string.h>


Debug g_debug;


void debug_init()
{
	memset( &g_debug, 0, sizeof(g_debug));
}


