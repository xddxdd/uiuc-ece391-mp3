/*
 * Source file for system calls
 */

 #include "syscall.h"

 int32_t halt (uint8_t status)
 {
   return 0;
 }

 int32_t execute (const uint8_t* command)
 {
   return 0;
 }

 int32_t read (int32_t fd, void* buf, int32_t nbytes)
 {
   return 0;
 }

 int32_t write (int32_t fd, const void* buf, int32_t nbytes)
 {
   return 0;
 }

 int32_t open (const uint8_t* filename)
 {
   return 0;
 }

 int32_t close (int32_t fd)
 {
   return 0;
 }

 int32_t getargs (uint8_t* buf, int32_t nbytes)
 {
   return 0;
 }

 int32_t vidmap (uint8_t** screen_start)
 {
   return 0;
 }

 int32_t set_handler (int32_t signum, void* handler_address)
 {
   return 0;
 }

 int32_t sigreturn (void)
 {
   return 0;
 }
