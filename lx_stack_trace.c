
#include "asm_powerpc_stk.h"

#define  MAX_STK_DEPTH 30
#define  STK_TRACE_SKIP_CNT 2


#ifdef CONFIG_IRQSTACKS
static __always_inline int my_valid_irq_stack(unsigned long sp, struct task_struct *p,
                                  unsigned long nbytes)
{
        unsigned long stack_page;
        unsigned long cpu = task_cpu(p);

        /*
         * Avoid crashing if the stack has overflowed and corrupted
         * task_cpu(p), which is in the thread_info struct.
         */
        if (cpu < NR_CPUS && cpu_possible(cpu)) {
                stack_page = (unsigned long) hardirq_ctx[cpu];
                if (sp >= stack_page + sizeof(struct thread_struct)
                    && sp <= stack_page + THREAD_SIZE - nbytes)
                        return 1;

                stack_page = (unsigned long) softirq_ctx[cpu];
                if (sp >= stack_page + sizeof(struct thread_struct)
                    && sp <= stack_page + THREAD_SIZE - nbytes)
                        return 1;
        }
        return 0;
}

#else
#define my_valid_irq_stack(sp, p, nb)      0
#endif /* CONFIG_IRQSTACKS */

static __always_inline int my_validate_sp(unsigned long sp, struct task_struct *p,
                       unsigned long nbytes)
{
        unsigned long stack_page = (unsigned long)task_stack_page(p);

        if (sp >= (unsigned long)end_of_stack(p)
            && sp <= stack_page + THREAD_SIZE - nbytes)
                return 1;

        return my_valid_irq_stack(sp, p, nbytes);
}


static __always_inline unsigned int my_get_stack_trace(unsigned long *stk_trace)
{
        unsigned long sp;
        unsigned int i;

        sp = my_get_current_stack_pointer(sp);

        for (i=1; i< STK_TRACE_SKIP_CNT; i++)
        {
                if (!my_validate_sp(sp, current, STACK_FRAME_OVERHEAD))
                        return 0;
                sp = ((unsigned long *)sp)[0];
        }

        for (i=0; i< MAX_STK_DEPTH; i++)
        {
                if (!my_validate_sp(sp, current, STACK_FRAME_OVERHEAD))
                        return i;

                stk_trace[i] = ((unsigned long *)sp)[STACK_FRAME_LR_SAVE];

                sp = ((unsigned long *)sp)[0];
        }

        return MAX_STK_DEPTH;
}


static void my_get_and_print_stack_trace()
{
    unsigned long stk_trace[MAX_STK_DEPTH];
    unsigned int stk_trace_len;


    stk_trace_len = my_get_stack_trace(stk_trace);


    for (k = 0; (k < stk_trace_len); k++) {
        sprintf(page + count, "    %pS\n", (void *)stk_trace[k]);
    }
}
