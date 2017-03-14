/*
* Shamelessy stolen from archshift, sorry bud, but it's to regroup everything in the same repo (note that ARM9 kern dump is by me NexoCube)
*/

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global_backdoor.h"
#include "kernel_patches.h"

#define KERN_MEM        0xDFF00000
#define KERN_MEM_SIZE   0x100000

static u8 buf[0x400];
static size_t mem_ctr;

PrintConsole topScreen, bottomScreen;

s32 __attribute__ ((noinline)) dump_chunk()
{
    memcpy(buf, (void*)(KERN_MEM + mem_ctr), 0x400);
    return 0;
}

s32 __attribute__ ((naked)) dump_chunk_wrapper()
{
    __asm__ __volatile__ ("cpsid aif");
    printf("Interrupts disabled\n");
    dump_chunk();
}

int main()
{

    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    if(checkSvcGlobalBackdoor()) {
    initsrv_allservices();
    patch_svcaccesstable();
    } else {
        goto exit;
    }

    FILE* out_file = fopen("arm11_kernel_dump.bin", "wb");

    printf("Dumping kernel memory... Wait ...\n\n");

    for (mem_ctr = 0; mem_ctr < KERN_MEM_SIZE; mem_ctr += 0x400) {
        svcBackdoor(dump_chunk_wrapper);
        fwrite(buf, 1, 0x400, out_file);
    }

    printf("file : sd_root:/3ds/app_folder/arm11_kernel_dump.bin\n\n");
    printf("Press START to exit !\n");

    while(aptMainLoop())
    {
        hidScanInput();
        if(hidKeysDown() & KEY_START)
            break;


    }

    exit:
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
    gfxExit();
    return 0;
}