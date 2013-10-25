/* IRQ.C */
#ifndef _letkuos_irq_h
#define _letkuos_irq_h _letkuos_irq_h
#include "isr.h"

extern void init_irq();
extern void irq_install_handler(int irq, void (*handler)(struct registers *r));
#endif
