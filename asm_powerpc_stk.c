
void my_get_current_stack_pointer(unsigned long sp)
{
    asm("mr %0,1" : "=r" (sp));
}
