#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"
#include "stack.S"

.set PIN_1_RTC_NUM, 4 //GPIO34
.set PIN_2_RTC_NUM, 5 //GPIO35
.set EVT_ARRAY_SIZE_WORDS, 20 
.set EVT_ARRAY_SIZE_BYTES, 80 // 4 times of EVT_ARRAY_SIZE_WORDS

/* Define variables, which go into .bss section (zero-initialized data) */
	.bss
	.global debug_why2
debug_why2: .long 0
	.global loop_counter
loop_counter: .long 0

bool_wake: .long 0
pin1_status: .long 0

.global pin2_status
pin2_status: .long 0
pin1_last: .long 0 
.global pin2_last
pin2_last: .long 0 

/* ring buffers for timestamp */
	.global timestamp_low_array
timestamp_low_array: 
	.skip EVT_ARRAY_SIZE_BYTES
	.global timestamp_low_array_end
timestamp_low_array_end: .long 0
	.global timestamp_mid_array
timestamp_mid_array: 
	.skip EVT_ARRAY_SIZE_BYTES
	.global timestamp_mid_array_end
timestamp_mid_array_end: .long 0
	.global timestamp_hi_array
timestamp_hi_array: 
	.skip EVT_ARRAY_SIZE_BYTES
	.global timestamp_hi_array_end
timestamp_hi_array_end: .long 0

/* ring buffer for io number and pin status*/
	.global gpio_evt_array
gpio_evt_array: 
	.skip EVT_ARRAY_SIZE_BYTES
	.global gpio_evt_array_end
gpio_evt_array_end: 
	.long 0

/* buffer index counter for all ring buffers */
	.global  gpio_evt_n
gpio_evt_n: 
	.long 0

/* function stack */
	.global stack
stack:
	.skip 100
	.global stackEnd
stackEnd:
	.long 0



/* Code goes into .text section */
	.text
	.global entry
entry:
	/* sacrafice registor r3, use it as function stack pointer */
	move r3,stackEnd
	/* Trigger update of rtc time register */
    WRITE_RTC_REG(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_UPDATE_S, 1, 1)
check_time_valid:
	/* Check if RTC_CNTL_TIME_VALID bit is 1, otherwise repeat */
    READ_RTC_REG(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_VALID_S, 1)
    and r0, r0, 1
    jump check_time_valid, eq
	move r0, bool_wake
	move r1, 0
	st r1, r0, 0

	psr
	jump pin1_check
	psr
	jump pin2_check

	move r1, loop_counter
	ld r0, r1, 0
	add r0 ,r0, 1
	st r0, r1, 0

	move r0, bool_wake
	ld r0, r0, 0
	jumpr wakeup, 1, eq
	
exit:
	halt

wakeup:
	wake
	halt

pin1_check:
	move r0, pin1_status
	ld r0, r0, 0
	move r2, pin1_last
	st r0, r2, 0
	/*read input registor of RTC_GPIO 0-15 into r0*/
    READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S, 16)
	move r2, PIN_1_RTC_NUM
	rsh r0, r0, r2
	and r0, r0, 1
	move r2, pin1_status
	st r0, r2, 0
	move r2, pin1_last
	ld r2, r2, 0
	add r0, r0, r2
	and r0, r0, 1

	jump pin1_check_ret, eq
	psr
	jump pin1_edge_intr
pin1_check_ret:
	ret

pin2_check:
	move r0, pin2_status
	ld r0, r0, 0
	move r2, pin2_last
	st r0, r2, 0
	/*read input registor of RTC_GPIO 0-15 into r0*/
    READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S, 16)
	move r2, PIN_2_RTC_NUM
	rsh r0, r0, r2
	and r0, r0, 1
	move r2, pin2_status
	st r0, r2, 0
	move r2, pin2_last
	ld r2, r2, 0
	add r0, r0, r2
	and r0, r0, 1

	jump pin2_check_ret, eq
	psr
	jump pin2_edge_intr
pin2_check_ret:
	ret

pin1_edge_intr:
	move r2, PIN_1_RTC_NUM
	lsh r0, r2, 4
	move r2, pin1_status
	ld r2, r2, 0
	or r0, r0, r2
	and r0, r0, 0xfff
	psr
	jump new_event
	ret

pin2_edge_intr:
	move r2, PIN_2_RTC_NUM
	lsh r0, r2, 4
	move r2, pin2_status
	ld r2, r2, 0
	or r0, r0, r2
	and r0, r0, 0xfff
	psr
	jump new_event
	ret

new_event:
	move r1, bool_wake
	move r2, 1
	st r2, r1, 0

	/* store new data in array*/
	move r1, gpio_evt_n
	ld r1, r1, 0
	move r2, gpio_evt_array
	add r2, r2, r1
	st r0, r2, 0
	/* store three 16bit parts of rtc timestamp*/
	move r2, timestamp_low_array
	add r2, r2, r1
	READ_RTC_REG(RTC_CNTL_TIME0_REG, 0, 16)
	st r0, r2, 0
	move r2, timestamp_mid_array
	add r2, r2, r1
	READ_RTC_REG(RTC_CNTL_TIME0_REG, 16, 16)
	st r0, r2, 0
	move r2, timestamp_hi_array
	add r2, r2, r1
	READ_RTC_REG(RTC_CNTL_TIME1_REG, 0, 16)
	st r0, r2, 0

	/* update array index for next data*/
	add r0, r1, 1
	jumpr array_ptr_reset, EVT_ARRAY_SIZE_WORDS, ge
	jump array_ptr_continue
array_ptr_reset:
	move r0, 0
array_ptr_continue:
	move r1, gpio_evt_n
	st r0, r1, 0
	ret

	