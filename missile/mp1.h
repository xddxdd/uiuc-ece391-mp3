#ifndef _ASM

#define RTC_STARTGAME 	0
#define RTC_ADDMISSILE 	1
#define RTC_MOVEXHAIRS	2
#define RTC_GETSTATUS 	3
#define RTC_ENDGAME 	4

struct missile {
    struct missile* next;   /* pointer to next missile in linked list */
    int x, y;		    /* x,y position on screen                 */
    int vx, vy;		    /* x,y velocity vector                    */
    int dest_x, dest_y;     /* location at which the missile explodes */
    int exploded;           /* explosion duration counter             */
    char c;                 /* character to draw for this missile     */
};

extern void mp1_rtc_tasklet();
extern int mp1_ioctl(unsigned long arg, unsigned int cmd);

#endif
