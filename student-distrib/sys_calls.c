#include "sys_calls.h"



// System calls for checkpoint 3.
int32_t halt (uint8_t status){

    return SYSCALL_SUCCESS;
}
int32_t execute (const uint8_t* command){

    return SYSCALL_SUCCESS;
}
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    
    return SYSCALL_SUCCESS;
}
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    
    return SYSCALL_SUCCESS;
}
int32_t open (const uint8_t* filename){
    
    return SYSCALL_SUCCESS;
} 
int32_t close (int32_t fd){
    
    return SYSCALL_SUCCESS;
}


// System calls for checkpoint 4.
int32_t getargs (uint8_t* buf, int32_t nbytes){

    return SYSCALL_SUCCESS;
}
int32_t vidmap (uint8_t** screen_start){

    return SYSCALL_SUCCESS;
}


//Extra credit system calls.
int32_t set_handler (int32_t signum, void* handler_address){

    return SYSCALL_SUCCESS;
}
int32_t sigreturn (void){

    return SYSCALL_SUCCESS;
}
