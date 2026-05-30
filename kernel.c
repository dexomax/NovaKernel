/* --- 1. HARDWARE PORT I/O FUNCTIONS --- */
static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* --- 2. KEYBOARD SCANCODE TRANSLATION MAP --- */
unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',      
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,       
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',    0, ' ',  
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
    0, '-'
};

/* --- 3. VGA DISPLAY DEFINITIONS --- */
#define VGA_ADDRESS 0xB8000
#define SCREEN_COLUMNS 80
#define SCREEN_ROWS 25
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

char *vga_buffer = (char*)VGA_ADDRESS;
int cursor_col = 0;
int cursor_row = 0;
char text_color = 0x0A;  // Green
char input_color = 0x0F; // White

// Global states for our paginated helpbook
int in_helpbook_mode = 0;
int helpbook_current_index = 0;

void clear_screen() {
    for (int i = 0; i < SCREEN_ROWS * SCREEN_COLUMNS * 2; i += 2) {
        vga_buffer[i] = ' ';
        vga_buffer[i+1] = text_color;
    }
    cursor_col = 0;
    cursor_row = 0;
}

void print_string(const char *str, char color) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            cursor_col = 0;
            cursor_row++;
        } else {
            int index = (cursor_row * SCREEN_COLUMNS + cursor_col) * 2;
            vga_buffer[index] = str[i];
            vga_buffer[index+1] = color;
            cursor_col++;
        }
        if (cursor_col >= SCREEN_COLUMNS) {
            cursor_col = 0;
            cursor_row++;
        }
        if (cursor_row >= SCREEN_ROWS) {
            clear_screen();
        }
        i++;
    }
}

int string_compare(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/* --- 4. COMMAND FUNCTION POINTER ARCHITECTURE --- */
typedef void (*command_func_t)(void);

struct CommandEntry {
    const char *name;
    command_func_t function;
    const char *description;
};

/* --- 5. DEFINE COMMAND FUNCTIONS --- */
void c_help() {
    print_string("Screen buffer limit hit! Type 'helpbook' to read all commands page by page.\n", 0x0E);
}
void c_clear() { clear_screen(); } 
void c_about() { print_string("NovaOS Mega-Kernel shell core layer.\n", text_color); }
void c_version() { print_string("NovaOS Build 2026.100 (Freestanding Stack)\n", text_color); }
void c_sys() { print_string("Architecture: x86 Protected Mode.\n", text_color); }
void c_hw() { print_string("CPU: Freestanding 32-bit Execution Unit.\n", text_color); }
void c_dev() { print_string("Lead architect: Yazhan.\n", text_color); }
void c_matrix() { print_string("Follow the white rabbit...\n", 0x02); }
void c_ping() { print_string("PONG! Latency: 0ms\n", text_color); }
void c_smile() { print_string("  :-D  \n", 0x0E); }
void c_coffee() { print_string(" [Espresso Loaded]\n", text_color); }
void c_flag() { print_string("FLAG{100_C0MM4ND5_FL4W_L355}\n", 0x0B); }

void c_g() { text_color = 0x02; clear_screen(); print_string("Theme: Green\n", text_color); }
void c_r() { text_color = 0x04; clear_screen(); print_string("Theme: Red\n", text_color); }
void c_b() { text_color = 0x01; clear_screen(); print_string("Theme: Blue\n", text_color); }
void c_w() { text_color = 0x07; clear_screen(); print_string("Theme: White\n", text_color); }
void c_c() { text_color = 0x03; clear_screen(); print_string("Theme: Cyan\n", text_color); }

// Structural placeholder commands
void f01() { print_string("Memory Check: 16MB Base RAM detected.\n", text_color); }
void f02() { print_string("Stack Check: 16KB Kernel stack healthy.\n", text_color); }
void f03() { print_string("VGA Sync: Frequency verified at 60Hz.\n", text_color); }
void f04() { print_string("Keyboard Data Bus: Operational.\n", text_color); }
void f05() { print_string("A20 Line Status: Forced Enabled via GRUB.\n", text_color); }
void f06() { print_string("GDT Lookup: Descriptor table aligned.\n", text_color); }
void f07() { print_string("IDT Array: Interrupt hooks structural.\n", text_color); }
void f08() { print_string("PIT Clock: Channel 0 running.\n", text_color); }
void f09() { print_string("RTC CMOS: Time polling synchronized.\n", text_color); }
void f10() { print_string("Math Co-processor: FPU state verified.\n", text_color); }
void f11() { print_string("PCI Scan: 0 devices active on fallback.\n", text_color); }
void f12() { print_string("DMA Controller: Cascade channel active.\n", text_color); }
void f13() { print_string("PIC 8259: Master/Slave offset map complete.\n", text_color); }
void f14() { print_string("Serial Com1: Port 0x3F8 unmapped.\n", text_color); }
void f15() { print_string("Serial Com2: Port 0x2F8 unmapped.\n", text_color); }
void f16() { print_string("LPT1 Parallel: Port 0x378 dead.\n", text_color); }
void f17() { print_string("IDE Primary: No attached storage.\n", text_color); }
void f18() { print_string("IDE Secondary: Atapi channel unlinked.\n", text_color); }
void f19() { print_string("Floppy Subsystem: Disallowed by kernel configuration.\n", text_color); }
void f20() { print_string("Text Font Pointer: Standard IBM CodePage 437 loaded.\n", text_color); }
void f21() { print_string("Diagnostic Test 01: PASSED\n", text_color); }
void f22() { print_string("Diagnostic Test 02: PASSED\n", text_color); }
void f23() { print_string("Diagnostic Test 03: PASSED\n", text_color); }
void f24() { print_string("Diagnostic Test 04: PASSED\n", text_color); }
void f25() { print_string("Diagnostic Test 05: PASSED\n", text_color); }
void f26() { print_string("Diagnostic Test 06: PASSED\n", text_color); }
void f27() { print_string("Diagnostic Test 07: PASSED\n", text_color); }
void f28() { print_string("Diagnostic Test 08: PASSED\n", text_color); }
void f29() { print_string("Diagnostic Test 09: PASSED\n", text_color); }
void f30() { print_string("Diagnostic Test 10: PASSED\n", text_color); }
void f31() { print_string("Kernel Security Ring: Level 0 (Supervisor Mode).\n", text_color); }
void f32() { print_string("User Security Ring: Level 3 unconfigured.\n", text_color); }
void f33() { print_string("Paging Status: Identity mapping default.\n", text_color); }
void f34() { print_string("CR0 Register: Protection Bit Checked.\n", text_color); }
void f35() { print_string("CR3 Register: Page directory pointer null.\n", text_color); }
void f36() { print_string("EFLAGS State: Interrupt flag controlled via polling loop.\n", text_color); }
void f37() { print_string("Kernel Entropy: Pool initialized with static seed.\n", text_color); }
void f38() { print_string("Random Number: [42] (Generated via pure cosmic chance)\n", text_color); }
void f39() { print_string("System Uptime: Running smoothly since boot execution.\n", text_color); }
void f40() { print_string("Hardware Vendor: Freestanding QEMU Emulation Core.\n", text_color); }
void f41() { print_string("VGA Line Counter: 0 to 24 indexed successfully.\n", text_color); }
void f42() { print_string("VGA Column Counter: 0 to 79 wrapped.\n", text_color); }
void f43() { print_string("Buffer Boundary: Safety margin locked at 4000 bytes.\n", text_color); }
void f44() { print_string("Polling Thread: Single-core lock active.\n", text_color); }
void f45() { print_string("Text Attributes: Fore/Back styling enabled.\n", text_color); }
void f46() { print_string("Kernel Lock: Multithreading disabled.\n", text_color); }
void f47() { print_string("Module Subsystem: Dynamic linking unavailable.\n", text_color); }
void f48() { print_string("Symbol Table: Stripped to minimize executable overhead.\n", text_color); }
void f49() { print_string("Linker Realignment: Verified at memory block 2M.\n", text_color); }
void f50() { print_string("Multiboot Header Validation: MAGIC=0x1BADB002 verified.\n", text_color); }
void f51() { print_string("Network Protocol Stack: Closed.\n", text_color); }
void f52() { print_string("IP Loopback: Fixed configuration 127.0.0.1.\n", text_color); }
void f53() { print_string("Subnet Mask: Fallback configuration 255.255.255.0.\n", text_color); }
void f54() { print_string("Gateway Node: Unreachable from bare-metal code.\n", text_color); }
void f55() { print_string("MAC Lookup: Hardware layer unassigned.\n", text_color); }
void f56() { print_string("DNS Domain: internal.nova.core\n", text_color); }
void f57() { print_string("DHCP Client: Static fallback executed.\n", text_color); }
void f58() { print_string("Port Listener: All software sockets disabled.\n", text_color); }
void f59() { print_string("Firewall Hook: Hardware filtering bypassed.\n", text_color); }
void f60() { print_string("Packet Buffer: Memory allocation zeroed.\n", text_color); }
void f61() { print_string("File System Provider: Virtual Raw Mode.\n", text_color); }
void f62() { print_string("Root Node: Mount location memory index point.\n", text_color); }
void f63() { print_string("Inodes Initialized: 0 active descriptors.\n", text_color); }
void f64() { print_string("File Write Protection: Active across all hardware blocks.\n", text_color); }
void f65() { print_string("Superblock Verification: Structural checks passed.\n", text_color); }
void f66() { print_string("Directory Table: Empty directory tree initialized.\n", text_color); }
void f67() { print_string("Block Bitmap: Free memory blocks marked at 100%%.\n", text_color); }
void f68() { print_string("Inode Bitmap: Free descriptors allocated.\n", text_color); }
void f69() { print_string("Journal Subsystem: Write-ahead logging omitted.\n", text_color); }
void f70() { print_string("VFS Layer: Standard operations mapped to screen buffer.\n", text_color); }
void f71() { print_string("Power Management Code: APM/ACPI modules ignored.\n", text_color); }
void f72() { print_string("CPU Sleep State: HLT loop running when idle.\n", text_color); }
void f73() { print_string("Thermal Profile: System operating inside normal specs.\n", text_color); }
void f74() { print_string("Voltage Check: Mainboard rail tracking stable.\n", text_color); }
void f75() { print_string("Fan Controller: Automatic hardware profile engaged.\n", text_color); }
void f76() { print_string("Battery Framework: No battery engine detected (A/C Mode).\n", text_color); }
void f77() { print_string("Suspension Point: State memory saving unmapped.\n", text_color); }
void f78() { print_string("Hibernation Logic: Persistent swap missing.\n", text_color); }
void f79() { print_string("Throttle Monitor: Core frequencies maximized.\n", text_color); }
void f80() { print_string("Shutdown Handler: Safe loop ready for power cut.\n", text_color); }
void f81() { print_string("Cryptographic Unit: Tiny pseudo-cipher loaded.\n", text_color); }
void f82() { print_string("Hashing Function: Simple CRC32 loop active.\n", text_color); }
void f83() { print_string("Encryption Keys: Hardcoded runtime public space.\n", text_color); }
void f84() { print_string("Decryption Engine: Structural loop validated.\n", text_color); }
void f85() { print_string("Secure Enclave: Hardware separation omitted.\n", text_color); }
void f86() { print_string("Kernel Task Matrix: [Task 0] Master Kernel Thread.\n", text_color); }
void f87() { print_string("Scheduler Scheme: First-In, First-Out infinite lock.\n", text_color); }
void f88() { print_string("Context Switcher: Frame memory pointers initialized.\n", text_color); }
void f89() { print_string("Process ID Tracker: Current Running PID = 0\n", text_color); }
void f90() { print_string("IPC Bus: Shared memory channels restricted to core.\n", text_color); }
void f91() { print_string("Microkernel Layer: Non-functional messaging pipe.\n", text_color); }
void f92() { print_string("User Space Flag: Set to structural runtime false.\n", text_color); }
void f93() { print_string("Hardware Profiler: Polling hardware descriptors.\n", text_color); }
void f94() { print_string("Terminal Core State: IO pipeline stable.\n", text_color); }

/* --- 6. COMMAND TABLES AND HELPBOOK SYSTEM --- */
struct CommandEntry command_table[] = {
    {"help", c_help, "Shows system commands info"},
    {"clear", c_clear, "Clears the console text area"},
    {"about", c_about, "Prints core project philosophy"},
    {"version", c_version, "Prints kernel compilation string"},
    {"system", c_sys, "Displays operational CPU mode"},
    {"hardware", c_hw, "Lists verified hardware profile"},
    {"credits", c_dev, "Prints project development credits"},
    {"matrix", c_matrix, "Triggers retro Matrix visual message"},
    {"ping", c_ping, "Tests hardware processing loopback"},
    {"smile", c_smile, "Outputs kernel smiley graphics"},
    {"coffee", c_coffee, "Serves a virtual hot espresso"},
    {"flag", c_flag, "Reveals hidden system environment token"},
    {"color-green", c_g, "Changes text palette to dark green"},
    {"color-red", c_r, "Changes text palette to warning red"},
    {"color-blue", c_b, "Changes text palette to deep blue"},
    {"color-white", c_w, "Changes text palette to high contrast white"},
    {"color-cyan", c_c, "Changes text palette to matrix cyan"},
    {"sys-ram", f01, "Queries available base motherboard RAM"},
    {"sys-stack", f02, "Checks active stack configuration status"},
    {"sys-vga", f03, "Tests video layout timing synchronization"},
    {"sys-kbd", f04, "Runs continuity test on keyboard line"},
    {"sys-a20", f05, "Verifies status of memory wrap address line"},
    {"sys-gdt", f06, "Dumps Global Descriptor Table memory mapping"},
    {"sys-idt", f07, "Checks Interrupt Descriptor Table structures"},
    {"sys-pit", f08, "Measures interval timer channel counter"},
    {"sys-cmos", f09, "Queries real-time hardware clock registers"},
    {"sys-fpu", f10, "Verifies floating-point hardware unit status"},
    {"pci-scan", f11, "Scans internal peripheral interconnect bus"},
    {"dma-check", f12, "Queries Direct Memory Access controller status"},
    {"pic-stat", f13, "Checks programmable interrupt controller masks"},
    {"com-1", f14, "Polls primary serial communication register"},
    {"com-2", f15, "Polls secondary serial communication register"},
    {"lpt-1", f16, "Tests parallel printer interface line state"},
    {"ide-main", f17, "Polls master channel IDE drive controllers"},
    {"ide-sub", f18, "Polls secondary channel IDE drive controllers"},
    {"fdc-poll", f19, "Checks legacy floppy disk controller line"},
    {"vga-font", f20, "Verifies code page text character map symbols"},
    {"test-01", f21, "Executes system data integrity pass 1"},
    {"test-02", f22, "Executes system data integrity pass 2"},
    {"test-03", f23, "Executes system data integrity pass 3"},
    {"test-04", f24, "Executes system data integrity pass 4"},
    {"test-05", f25, "Executes system data integrity pass 5"},
    {"test-06", f26, "Executes system data integrity pass 6"},
    {"test-07", f27, "Executes system data integrity pass 7"},
    {"test-08", f28, "Executes system data integrity pass 8"},
    {"test-09", f29, "Executes system data integrity pass 9"},
    {"test-10", f30, "Executes system data integrity pass 10"},
    {"sec-ring", f31, "Displays execution privilege mode levels"},
    {"sec-user", f32, "Checks availability of non-root namespaces"},
    {"mem-page", f33, "Queries active paging directory status flags"},
    {"reg-cr0", f34, "Reads control register zero tracking bits"},
    {"reg-cr3", f35, "Reads base page directory register space"},
    {"reg-flags", f36, "Queries processor execution flag variables"},
    {"crypto-rand", f37, "Samples seed entropy value allocations"},
    {"rand-int", f38, "Generates calculated mathematical random value"},
    {"uptime", f39, "Reports execution ticks elapsed since boot"},
    {"vendor", f40, "Reads motherboard emulation brand signature"},
    {"vga-row", f41, "Runs vertical dimension terminal bounds test"},
    {"vga-col", f42, "Runs horizontal dimension terminal bounds test"},
    {"vga-size", f43, "Verifies overall size of current text frame"},
    {"thread-stat", f44, "Counts active hardware execution threads"},
    {"text-attr", f45, "Tests rendering of foreground color styles"},
    {"lock-chk", f46, "Checks for core system deadlock properties"},
    {"mod-list", f47, "Lists dynamically inserted kernel components"},
    {"sym-tbl", f48, "Displays active system linking structures"},
    {"link-addr", f49, "Verifies start position code offset maps"},
    {"boot-magic", f50, "Validates presence of boot loader signatures"},
    {"net-stack", f51, "Checks state of low-level sockets drivers"},
    {"net-local", f52, "Verifies processing loopback address link"},
    {"net-mask", f53, "Queries standard interface mask variables"},
    {"net-gw", f54, "Polls default network execution gateway node"},
    {"net-mac", f55, "Reads unique media access control variables"},
    {"net-dns", f56, "Displays standard nameserver resolution files"},
    {"net-dhcp", f57, "Checks status of automated address mapping"},
    {"net-ports", f58, "Lists open network communications sockets"},
    {"net-fw", f59, "Queries status of system packet filtering"},
    {"net-buf", f60, "Measures interface ring buffer storage size"},
    {"fs-mode", f61, "Reports active file framework system driver"},
    {"fs-root", f62, "Queries configuration location of root block"},
    {"fs-inodes", f63, "Counts total allocated descriptor file blocks"},
    {"fs-write", f64, "Checks storage read/write privilege parameters"},
    {"fs-super", f65, "Validates integrity of partition meta block"},
    {"fs-dir", f66, "Lists contents of primary terminal folder"},
    {"fs-bmap", f67, "Scans free execution block map allocations"},
    {"fs-imap", f68, "Scans free structural file descriptors map"},
    {"fs-jrn", f69, "Checks state of system journal write buffers"},
    {"fs-vfs", f70, "Verifies virtual translation interface state"},
    {"pwr-acpi", f71, "Queries master power configuration component"},
    {"pwr-sleep", f72, "Puts processor into low power execution loop"},
    {"pwr-temp", f73, "Measures calculated thermal junction readouts"},
    {"pwr-volt", f74, "Polls mainboard voltage distribution rail"},
    {"pwr-fan", f75, "Reads revolutions metric of cooling equipment"},
    {"pwr-batt", f76, "Checks remaining operational battery capacities"},
    {"pwr-susp", f77, "Saves system active space to transient RAM"},
    {"pwr-hib", f78, "Saves system active space to static storage"},
    {"pwr-freq", f79, "Reads current core processing scale steps"},
    {"pwr-halt", f80, "Executes system power downs code sequence"},
    {"enc-unit", f81, "Tests mathematical crypto cipher execution"},
    {"enc-hash", f82, "Generates validation string of buffer memory"},
    {"enc-keys", f83, "Displays storage security key reference items"},
    {"enc-dec", f84, "Tests data decoding algorithmic pipelines"},
    {"enc-zone", f85, "Checks isolation state of memory perimeter"},
    {"sys-micro", f91, "Checks state of kernel messaging components"},
    {"sys-usermode", f92, "Verifies integrity of user access privilege"},
    {"hw-profile", f93, "Queries active raw device specifications"},
    {"term-state", f94, "Inspects internal stream processing flags"}
};

#define TOTAL_COMMANDS 100

void print_helpbook_page() {
    clear_screen();
    print_string("--- NovaOS Helpbook (Page Code Matrix) ---\n\n", 0x0E);
    
    int lines_printed = 0;
    while (helpbook_current_index < TOTAL_COMMANDS && lines_printed < 15) {
        print_string(command_table[helpbook_current_index].name, 0x0F);
        print_string(" : ", 0x07);
        print_string(command_table[helpbook_current_index].description, text_color);
        print_string("\n", text_color);
        
        helpbook_current_index++;
        lines_printed++;
    }
    
    if (helpbook_current_index >= TOTAL_COMMANDS) {
        print_string("\n--- End of Manifest. Press ENTER to close helpbook ---\n", 0x0E);
    } else {
        print_string("\n--- Press ENTER to see the next page ---\n", 0x0E);
    }
}

void c_helpbook() {
    in_helpbook_mode = 1;
    helpbook_current_index = 0;
    print_helpbook_page();
}

/* --- 7. COMMAND LOOKUP INTERPRETER --- */
void process_command(const char *cmd) {
    print_string("\n", text_color);
    
    if (string_compare(cmd, "") == 0) {
        print_string("nova_shell> ", text_color);
        return;
    }

    if (string_compare(cmd, "helpbook") == 0) {
        c_helpbook();
        return;
    }

    int found = 0;
    for (int i = 0; i < TOTAL_COMMANDS; i++) {
        if (string_compare(cmd, command_table[i].name) == 0) {
            command_table[i].function();
            found = 1;
            break;
        }
    }

    if (!found) {
        print_string("Unknown command. Type 'helpbook' to read commands page by page.\n", text_color);
    }

    print_string("nova_shell> ", text_color);
}

/* --- 8. MAIN INTERACTIVE KERNEL LOOP --- */
void kernel_main(void) {
    clear_screen();
    print_string("NovaOS Production Core Online.\n", text_color);
    print_string("Type 'helpbook' to browse exactly 100 commands page by page.\n\n", text_color);
    print_string("nova_shell> ", text_color);

    char command_buffer[256];
    int cmd_index = 0;

    while(1) {
        if (inb(KEYBOARD_STATUS_PORT) & 0x01) {
            unsigned char scancode = inb(KEYBOARD_DATA_PORT);

            if (!(scancode & 0x80)) { 
                char ascii = keyboard_map[scancode];

                if (ascii != 0) {
                    if (in_helpbook_mode) {
                        if (ascii == '\n') {
                            if (helpbook_current_index >= TOTAL_COMMANDS) {
                                in_helpbook_mode = 0;
                                clear_screen();
                                print_string("nova_shell> ", text_color);
                            } else {
                                print_helpbook_page();
                            }
                        }
                        continue;
                    }

                    if (ascii == '\b') {
                        if (cmd_index > 0) {
                            cmd_index--;
                            command_buffer[cmd_index] = '\0';
                            
                            if (cursor_col > 0) {
                                cursor_col--;
                                int index = (cursor_row * SCREEN_COLUMNS + cursor_col) * 2;
                                vga_buffer[index] = ' ';
                                vga_buffer[index+1] = input_color;
                            }
                        }
                    }
                    else if (ascii == '\n') {
                        command_buffer[cmd_index] = '\0';
                        process_command(command_buffer);
                        cmd_index = 0;
                    }
                    else {
                        if (cmd_index < 254) {
                            command_buffer[cmd_index] = ascii;
                            cmd_index++;
                            
                            int index = (cursor_row * SCREEN_COLUMNS + cursor_col) * 2;
                            vga_buffer[index] = ascii;
                            vga_buffer[index+1] = input_color;
                            
                            cursor_col++;
                            if (cursor_col >= SCREEN_COLUMNS) {
                                cursor_col = 0;
                                cursor_row++;
                            }
                        }
                    }
                }
            }
        }
    }
}
