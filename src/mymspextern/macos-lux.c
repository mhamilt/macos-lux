#include "macos-lux.h"
#include <stdio.h>
#include <mach/mach.h>
#include <IOKit/IOKitLib.h>

//------------------------------------------------------------------------------
static double updateInterval = 0.1;
static io_connect_t dataPort = 0;
uint64_t prevReading = 0;
uint64_t recordMaximum = 0;
MaxExternalObject* maxExtRef;
//------------------------------------------------------------------------------
void onLuxUpdate(MaxExternalObject* maxObjectPtr, uint64_t lux)
{
    outlet_int(maxObjectPtr->int_out, lux);
}
//------------------------------------------------------------------------------
void updateTimerCallBack(CFRunLoopTimerRef timer, void *info)
{
    maxExtRef->lux_thread_runloop = CFRunLoopGetCurrent();
    kern_return_t kr;
    uint32_t outputs = 2;
    uint64_t values[outputs];
    
    kr = IOConnectCallMethod(dataPort, 0, nil, 0, nil, 0, values, &outputs, nil, 0);
    if (kr == KERN_SUCCESS )
    {
        if (values[0] != prevReading)
        {
            recordMaximum = (values[0] > recordMaximum) ? values[0]: recordMaximum;
            
            onLuxUpdate(maxExtRef,values[0]);
        }
        prevReading = values[0];
        return;
    }
    
    if (kr == kIOReturnBusy)
    {
        return;
    }
    
    mach_error("I/O Kit error:", kr);
    exit(kr);
}
//------------------------------------------------------------------------------
void* lux_main(void* arg)
{
    maxExtRef = (MaxExternalObject*)arg;
    kern_return_t kr;
    io_service_t serviceObject;
    CFRunLoopTimerRef updateTimer;
    
    serviceObject = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleLMUController"));
    if (!serviceObject) {
        fprintf(stderr, "failed to find ambient light sensors\n");
        exit(1);
    }
    
    kr = IOServiceOpen(serviceObject, mach_task_self(), 0, &dataPort);
    IOObjectRelease(serviceObject);
    if (kr != KERN_SUCCESS)
    {
        mach_error("IOServiceOpen:", kr);
        exit(kr);
    }
    
    setbuf(stdout, NULL);
    printf("%8ld %8ld\n", 0L, 0L);
    
    updateTimer = CFRunLoopTimerCreate(kCFAllocatorDefault,
                                       CFAbsoluteTimeGetCurrent() + updateInterval, updateInterval,
                                       0, 0, updateTimerCallBack, NULL);
    
    CFRunLoopAddTimer(CFRunLoopGetCurrent(),
                      updateTimer,
                      kCFRunLoopDefaultMode);
    CFRunLoopRun();
    
    return NULL;
}
