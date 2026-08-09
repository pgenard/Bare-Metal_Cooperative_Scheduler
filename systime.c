#include "systime.h"
#include "gpt.h"

systime_t systime_get(void) {
    return gpt_read_cnt() / 16000;
}
