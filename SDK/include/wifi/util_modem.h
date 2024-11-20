#ifndef UTIL_MODEM_H
#define UTIL_MODEM_H

#include "system.h"

uint32_t util_modem_compute_snr(uint32_t signal, uint32_t noise);
uint32_t util_modem_compute_snr_i(uint32_t signal, uint32_t noise);
uint32_t log10_i(uint32_t v);

#endif
