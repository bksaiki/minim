#ifndef _MINIM_H_
#define _MINIM_H_

#include "common/buffer.h"
#include "common/common.h"
#include "core/builtin.h"
#include "core/env.h"
#include "core/eval.h"
#include "core/global.h"
#include "core/module.h"
#include "core/number.h"
#include "core/parser.h"
#include "core/print.h"
#include "core/read.h"
#include "gc/gc.h"

#define MINIM_FLAG_LOAD_LIBS        0x1
#define MINIM_FLAG_NO_RUN           0x2
#define MINIM_FLAG_NO_CACHE         0x4
#define MINIM_FLAG_COMPILE          0x8

#define IF_FLAG_RAISED(x, fl, t, f)     ((((x) & (fl)) == 0) ? (f) : (t))

#endif
