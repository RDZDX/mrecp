#pragma once
#include "vmsys.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "ResID.h"
#include "string.h"
#include <ctype.h>
#include "vmmm.h"

#define BUF_SIZE 253 // Buffer size for output mode 1, set to 253 if you want to connect to longest domain name :)

extern int scr_w, scr_h;
extern VMUINT8 *layer_bufs[2];
extern VMINT layer_hdls[2];

const unsigned short tr_color = VM_COLOR_888_TO_565(0, 255, 255);

#ifdef WIN32
#define my_printf(...) printf(__VA_ARGS__)
#else
#define my_printf(...)
#endif