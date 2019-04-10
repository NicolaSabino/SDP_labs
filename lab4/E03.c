// main.c -- Defines the C-code kernel entry point, calls initialisation routines.
//           Made for JamesM's tutorials <www.jamesmolloy.co.uk>

#include "monitor.h"
#include "descriptor_tables.h"
#include "timer.h"
#include "paging.h"

int main(struct multiboot *mboot_ptr)
{
    // Initialise all the ISRs and segmentation
    init_descriptor_tables();
    // Initialise the screen (by clearing it)
    monitor_clear();

    initialise_paging();
    monitor_write("Hello, paging world!  ");

    //u32int *ptr = (u32int*)0xA0000000;
    //u32int do_page_fault = *ptr;
	
	u32int *ptr = (u32int*) 0x00000000;
	u32int page = 0;
	while(page<3){
		*ptr = page;
		monitor_write("  ptr ");
		monitor_write_hex(ptr);
		monitor_write(" (page ");
		monitor_write_dec(page);
		monitor_write(") contains ");
		monitor_write_dec(*ptr);
		monitor_write(" ");
		//u32int point_something = *ptr;
		ptr += 0x00007D00;
		page += 1;
	}
	
	u32int *ptr2 = (u32int*) 0x00000000;
        u32int tmp = *ptr2;
        *ptr2 = *(ptr2 + 0x00007D00);
        monitor_write("page 0 now contains ");
        monitor_write_dec(*ptr2);
        monitor_write(". ");
        ptr2 += 0x00007D00;
        *ptr2 = tmp;

        monitor_write("page 1 now contains ");
        monitor_write_dec(*ptr2);
        monitor_write(". ");  

    return 0;
}
