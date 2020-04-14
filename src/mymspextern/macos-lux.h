#ifndef macos_lux_h
#define macos_lux_h

#include <stdio.h>
#include <pthread.h>
#include <CoreFoundation/CoreFoundation.h>
#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "buffer.h"

/** @struct
 The MaxMSP object
 */
typedef struct _MaxExternalObject
{
    t_pxobject x_obj;
    t_symbol* x_arrayname;
    pthread_t print_lux_thread;
    CFRunLoopRef lux_thread_runloop;
    short inletConnection;
    double gain;
    void* float_out;
} MaxExternalObject;

void* lux_main(void* arg);
void onLuxUpdate(MaxExternalObject* maxObjectPtr, float lux);


#endif
