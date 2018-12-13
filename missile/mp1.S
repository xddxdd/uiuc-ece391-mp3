# mp1.S - missile-command version
# Good luck, and have fun!

// Color switch. Defining this enables colorful gameplay.
#define ENABLE_COLOR

.data

    # Constants for accessing the fields of a struct missile,
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'
	EXPLOSION = 64

	# Constants I added
	SIZEOF_MISSILE = 33
	SCREEN_WIDTH = 80
	SCREEN_HEIGHT = 25

	CROSSHAIR = 43
	SPACE = 32

	BASE1_POS = 18
	BASE2_POS = 38
	BASE3_POS = 58

	#ifdef ENABLE_COLOR
		CROSSHAIR_COLOR = 0x04
		MISSILE_COLOR = 0x02
		MISSILE_CHAR = 42
		ENEMY_COLOR = 0x05
		ENEMY_CHAR = 101
		EXPLOSION_COLOR = 0x04
	#endif

# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

#ifdef ENABLE_COLOR
base_pic_color:		.byte 0x03, 0x06, 0x04, 0x06, 0x03, 0x00
dead_base_pic_color:.byte 0x07, 0x06, 0x04, 0x06, 0x07, 0x00
#endif

mp1_ioctl_getstatus_intermediate_storage:	.long 0x0	# bridge between kernel and user space

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
	pushl %ebp
	movl %esp, %ebp
	pushal

	pushl %ecx
	pushl %eax
	call mp1_poke_helper
	addl $8, %esp

	popal
	leave
	ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	call update_missiles

	// Redraw bases
	leal base_alive, %esi
	// Draw base 1
	pushl $BASE1_POS
	movsbl 0(%esi), %eax
	pushl %eax
	call draw_base
	addl $8, %esp
	// Draw base 2
	pushl $BASE2_POS
	movsbl 1(%esi), %eax
	pushl %eax
	call draw_base
	addl $8, %esp
	// Draw base 3
	pushl $BASE3_POS
	movsbl 2(%esi), %eax
	pushl %eax
	call draw_base
	addl $8, %esp

	// Redraw crosshair, load position
	movl crosshairs_x, %esi
	movl crosshairs_y, %edi

	// Calculate redraw position
	movl %edi, %eax
	imull $SCREEN_WIDTH, %eax
	addl %esi, %eax	// %eax = %ecx * SCREEN_WIDTH + %ebx
	addl %eax, %eax	// %eax *= 2, each screen position takes 2 bytes to represent

	// Redraw
#ifdef ENABLE_COLOR
	movb $CROSSHAIR_COLOR, %ch
#endif
	movb $CROSSHAIR, %cl
	call mp1_poke

	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
	// dispatch function does not touch the stack
	movl 8(%esp), %ebx	// %ebx = cmd
	// if(cmd < 4) {jmp}
	cmpl $4, %ebx
	ja mp1_ioctl_invalid_cmd
	// arg is the second one from stack top, no need to move
	jmp *mp1_ioctl_jumptable(, %ebx, 4)
mp1_ioctl_invalid_cmd:
	// invalid cmd, return -1 on behalf of sub ioctl functions
	movl $-1, %eax
	ret

mp1_ioctl_jumptable:
	.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame

# ----------------- Functions private to this file -------------------

update_missiles:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl mp1_missile_list, %esi

update_missiles_next_missile:
	// if(mp1_missile_list = 0) return
	cmpl $0, %esi
	je update_missiles_done
	// First, remove the missle from screen
	pushl $SPACE
	pushl %esi
	call draw_missile
	addl $8, %esp

	// then, check if missile is exploding
	cmpl $0, EXPLODED(%esi)
	je update_missiles_missile_not_exploding
update_missiles_missile_exploding:
	// the mssile is exploding, call missile_explode()
	pushl %esi
	call missile_explode
	addl $4, %esp
	// if return value is not zero, we need to send SIGUSR1
	cmpl $0, %eax
	je update_missiles_missile_finished_exploding
	call mp1_notify_user
update_missiles_missile_finished_exploding:
	// decrease EXPLODED(%esi) by 1
	subl $1, EXPLODED(%esi)
	// if EXPLODED(%esi) = 0, remove this missile
	cmpl $0, EXPLODED(%esi)
	je update_missiles_remove_this_missile

update_missiles_missile_not_yet_finish_explode:
	// Draw the missile with explosion character
	pushl $EXPLOSION
	pushl %esi
	call draw_missile
	addl $4, %esp
	// We're done with this missile
	jmp update_missiles_this_missile_done

update_missiles_missile_not_exploding:
	// Update x, y for missile
	movl VX(%esi), %eax
	movl VY(%esi), %ebx
	addl %eax, X(%esi)
	addl %ebx, Y(%esi)

	// Load missile on-screen x, y
	movl X(%esi), %eax
	movl Y(%esi), %ebx
	shrl $16, %eax		// %eax = %eax >> 16
	shrl $16, %ebx		// %ebx = %ebx >> 16
	// If missile went out of screen, remove it
	cmpl $SCREEN_WIDTH, %eax
	jae update_missiles_remove_this_missile
	cmpl $SCREEN_HEIGHT, %ebx
	jae update_missiles_remove_this_missile

	// If missile reaches destination, explode it
	cmpl DEST_X(%esi), %eax
	jne update_missiles_missile_still_moving
	cmpl DEST_Y(%esi), %ebx
	jne update_missiles_missile_still_moving
	jmp update_missiles_missile_exploding

update_missiles_missile_still_moving:
	// Otherwise, just draw the missile
	movsbl C(%esi), %edi
	pushl %edi
	pushl %esi
	call draw_missile
	addl $8, %esp
	// and we're done with this missile
	jmp update_missiles_this_missile_done

update_missiles_remove_this_missile:
	pushl %esi
	movl NEXT(%esi), %esi				// load next missile before removal, prevent segfault
	call remove_missile
	addl $4, %esp
	jmp update_missiles_next_missile	// already switched to next missile, jmp directly

update_missiles_this_missile_done:
	// load next missile
	movl NEXT(%esi), %esi
	jmp update_missiles_next_missile

update_missiles_done:
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

/* void draw_missile(missile_t* missile, int char)
 * Draws a missile.
 * for int char, only last 8 bits are used.
 */
draw_missile:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl 8(%ebp), %esi	// missile coordinate to %esi
	movl X(%esi), %eax
	shrl $16, %eax
	movl Y(%esi), %ebx
	shrl $16, %ebx

	imull $SCREEN_WIDTH, %ebx
	addl %ebx, %eax
	addl %eax, %eax

	movb 12(%ebp), %cl

#ifdef ENABLE_COLOR
	cmpb $EXPLOSION, %cl
	je draw_missile_explosion
	cmpb $ENEMY_CHAR, %cl
	je draw_missile_enemy
	cmpb $MISSILE_CHAR, %cl
	je draw_missile_missile
	movb $0x07, %ch
	jmp draw_missile_color_done
draw_missile_explosion:
	movb $EXPLOSION_COLOR, %ch
	jmp draw_missile_color_done
draw_missile_enemy:
	movb $ENEMY_COLOR, %ch
	jmp draw_missile_color_done
draw_missile_missile:
	movb $MISSILE_COLOR, %ch
draw_missile_color_done:
#endif
	call mp1_poke

	popl %edi
	popl %esi
	popl %ebx
	leave
	ret


/* void remove_missile(missile_t* missile)
 * Removes a missile from memory.
 */
remove_missile:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl 8(%ebp), %eax			// load missile pointer to %eax
	movl mp1_missile_list, %ebx	// load missile head to %ebx
	cmpl %eax, %ebx
	jne remove_missile_find_parent
	// The missile to be deleted is the head of list
	// change the head of list first
	movl NEXT(%eax), %ebx
	movl %ebx, mp1_missile_list
	pushl %eax
	call mp1_free
	addl $4, %esp
	jmp remove_missile_done

remove_missile_find_parent:
	movl NEXT(%ebx), %ecx
	cmpl %eax, %ecx
	je remove_missile_found_parent
	movl %ecx, %ebx
	jmp remove_missile_find_parent

remove_missile_found_parent:
	movl NEXT(%eax), %ecx
	movl %ecx, NEXT(%ebx)
	pushl %eax
	call mp1_free
	addl $4, %esp

remove_missile_done:
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

/* void draw_base(bool alive, int left_distance)
 * Draw a base, either dead or alive, at the bottom of the screen.
 */
draw_base:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl 8(%ebp), %eax
	cmpl $0, %eax
	je draw_base_load_dead_base

	// Load image of alive base
	lea base_pic, %ebx
#ifdef ENABLE_COLOR
	lea base_pic_color, %esi
#endif
	jmp draw_base_calculate_position
draw_base_load_dead_base:
	// Load iamge of dead base
	lea dead_base_pic, %ebx
#ifdef ENABLE_COLOR
	lea dead_base_pic_color, %esi
#endif
draw_base_calculate_position:
	// Calculate initial position
	movl $SCREEN_HEIGHT, %eax
	subl $1, %eax
	imull $SCREEN_WIDTH, %eax	// %eax = index of bottom-left corner
	movl 12(%ebp), %ecx
	addl %ecx, %eax				// %eax = index of left of base
	addl %eax, %eax				// On screen each position takes 2 bytes

draw_base_draw_character:
	// Do the actual drawing
#ifdef ENABLE_COLOR
	movb 0(%esi), %ch
#endif
	movb 0(%ebx), %cl
	cmpb $0, %cl				// If char = 0, reached end of string, return
	je draw_base_done

	call mp1_poke

	addl $2, %eax				// Move to next position & next character
	addl $1, %ebx
#ifdef ENABLE_COLOR
	addl $1, %esi
#endif
	jmp draw_base_draw_character

draw_base_done:
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

/* void mp1_ioctl_startgame()
 * Initialize linked-list, base status, crosshair, scoring.
 */
mp1_ioctl_startgame:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	// Initialize global variables
	movl $0, mp1_missile_list
	leal base_alive, %edi
	movb $1, 2(%edi)
	movb $1, 1(%edi)
	movb $1, 0(%edi)
	movl $40, crosshairs_x
	movl $12, crosshairs_y
	movl $0, mp1_score

	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

/* int mp1_ioctl_addmissile(missile_t* missile)
 * Add a new missile to the linked list.
 * Returns 0 on success, -1 on fail.
 */
mp1_ioctl_addmissile:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl $SIZEOF_MISSILE, %eax
	pushl %eax
	call mp1_malloc
	addl $4, %esp

	// If mp1_alloc returns 0, malloc failed, return -1
	cmpl $0, %eax
	je mp1_ioctl_addmissile_fail

	// Now %eax holds the ptr to new missile structure,
	// copy userspace missile to kernel space
	movl $SIZEOF_MISSILE, %ebx
	pushl %ebx		// push n
	movl 8(%ebp), %ebx
	pushl %ebx		// push *from
	movl %eax, %ebx	// copy %eax to %ebx, preserve memory address through call
	pushl %eax		// push *to
	call ece391_memcpy
	addl $12, %esp	// pop 3 parameters

	// If does not return 0, mem copy failed
	cmpl $0, %eax
	jne mp1_ioctl_addmissile_memcpy_fail

	// Link the new missile's next to head of linked list
	movl mp1_missile_list, %ecx
	movl %ecx, NEXT(%ebx)

	// Link the linked list head to the new missile
	movl %ebx, mp1_missile_list

	// Job done, return 0
	movl $0, %eax
	jmp mp1_ioctl_addmissile_done

mp1_ioctl_addmissile_memcpy_fail:
	// at this point %ebx holds the memory of new missile
	// so we need to free the memory
	pushl %ebx
	call mp1_free
	addl $4,%esp
mp1_ioctl_addmissile_fail:
	// Job failed, return -1
	movl $-1, %eax
mp1_ioctl_addmissile_done:
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

/* int mp1_ioctl_movexhairs(int position)
 * move crosshair as input delta value.
 */
mp1_ioctl_movexhairs:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	// Load original position
	movl crosshairs_x, %esi
	movl crosshairs_y, %edi

	// Calculate original position
	movl %edi, %eax
	imull $SCREEN_WIDTH, %eax
	addl %esi, %eax	// %eax = %ecx * SCREEN_WIDTH + %ebx
	addl %eax, %eax	// %eax *= 2, each screen position takes 2 bytes to represent

	// Remove original crosshair
#ifdef ENABLE_COLOR
	movb $0x07, %ch
#endif
	movb $SPACE, %cl
	call mp1_poke

	// Add current delta
	movswl 8(%ebp), %ebx	// %ebx stores x component, low 16 bits
	movswl 10(%ebp), %ecx	// %ecx stores y component, high 16 bits
	addl %ebx, %esi
	addl %ecx, %edi

	// Check range of parameters, use unsigned so we don't have to compare against 0
	cmpl $SCREEN_WIDTH, %esi		// if (unsigned) x >= SCREEN_WIDTH, return
	jae mp1_ioctl_movexhairs_done
	cmpl $SCREEN_HEIGHT, %edi		// if (unsigned) y >= SCREEN_HEIGHT, return
	jae mp1_ioctl_movexhairs_done

	// Update stored position
	movl %esi, crosshairs_x
	movl %edi, crosshairs_y

mp1_ioctl_movexhairs_done:
	// Reload stored position
	movl crosshairs_x, %esi
	movl crosshairs_y, %edi

	// Calculate redraw position
	movl %edi, %eax
	imull $SCREEN_WIDTH, %eax
	addl %esi, %eax	// %eax = %ecx * SCREEN_WIDTH + %ebx
	addl %eax, %eax	// %eax *= 2, each screen position takes 2 bytes to represent

	// Redraw
#ifdef ENABLE_COLOR
	movb $CROSSHAIR_COLOR, %ch
#endif
	movb $CROSSHAIR, %cl
	call mp1_poke

	movl $0, %eax
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

/* int mp1_ioctl_getstatus(unsigned long* status)
 * Copies the score to low 16 bit of status, and base status to bit 16-18.
 * Returns 0 on success, -1 on fail.
 */
mp1_ioctl_getstatus:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	// Copy score to intermediate storage
	movl mp1_score, %eax
	movl %eax, mp1_ioctl_getstatus_intermediate_storage

	// Prepare base status
	movl $0, %eax
	leal base_alive, %esi
	movsbl 2(%esi), %ebx
	addl %ebx, %eax
	addl %eax, %eax
	movsbl 1(%esi), %ebx
	addl %ebx, %eax
	addl %eax, %eax
	movsbl 0(%esi), %ebx
	addl %ebx, %eax

	// Add base status to intermediate storage
	shll $16, %eax
	addl %eax, mp1_ioctl_getstatus_intermediate_storage

	// Copy score to result
	movl $4, %edi			// push size to copy
	pushl %edi
	leal mp1_ioctl_getstatus_intermediate_storage, %edi	// push source address
	pushl %edi
	movl 8(%ebp), %edi		// push dest address
	pushl %edi
	call ece391_memcpy
	addl $12, %esp			// pop 3 parameters

	// Check if memcpy succeeded
	cmpl $0, %eax
	jne mp1_ioctl_getstatus_fail	// fail if memcpy fail
	jmp mp1_ioctl_getstatus_done

mp1_ioctl_getstatus_fail:
	movl $-1, %eax
mp1_ioctl_getstatus_done:
	movl $0, %eax
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret

/* void mp1_ioctl_endgame()
 * Clears the linked list and return 0.
 */
mp1_ioctl_endgame:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

mp1_ioctl_endgame_remove_next:
	// Check if there's still missiles in the list
	movl mp1_missile_list, %esi
	cmpl $0, %esi
	je mp1_ioctl_endgame_done

	// Remove the missile on the head of the list
	pushl %esi
	call remove_missile
	addl $4, %esp

	jmp mp1_ioctl_endgame_remove_next

mp1_ioctl_endgame_done:
	// We have finished removing all missiles
	movl $0, %eax
	popl %edi
	popl %esi
	popl %ebx
	leave
	ret
