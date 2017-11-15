Heaploop:
	The number of instruction pages:	12
	The number of data pages:	154
	The most frequently accessed instruction page is 0x400, where most instructions(e.g. main() and heap_loop()) are stored.
	The most frequently accessed data page is 0xfff000, which is a page in stack. It's frequently because some local variables (e.g. loop counter i) are stored in this page. Although this process accesses heaps, but since each page contains at most 4 krec objects, each page did not get accessed very frequently.

Matmul:
	The number of instruction pages:	14
	The number of data pages:	417
	The most frequently accessed instruction page is 0x400, for the same reason as in Heaploop.
	The most frequently accessed data pages are 0xfff000 and 0x602. 0xfff000 is frequently accessed because it stores local variables. 0x602 is frequently accessed probably because is stores the matrix that are being multiplied.


