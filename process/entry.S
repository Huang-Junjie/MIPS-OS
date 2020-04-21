NESTED(kernel_thread_entry, 40, ra)
    pushl %edx              # push arg
    call *%ebx              # call fn

    pushl %eax              # save the return value of fn(arg)
    call do_exit            # call do_exit to terminate current thread
END(kernel_thread_entry)
