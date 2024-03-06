/*
 *   Ctrlr_Test - Program for the PC-FX to test controllers
 *
 *   Copyright (C) 2024 David Shadoff
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <eris/types.h>
#include <eris/std.h>
#include <eris/v810.h>
#include <eris/king.h>
#include <eris/low/7up.h>
#include <eris/tetsu.h>
#include <eris/romfont.h>
#include <eris/timer.h>
#include <eris/pad.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define JOY_I            1
#define JOY_II           2
#define JOY_III          4
#define JOY_IV           8
#define JOY_V            16
#define JOY_VI           32
#define JOY_SELECT       64
#define JOY_RUN          128
#define JOY_UP           256
#define JOY_RIGHT        512
#define JOY_DOWN         1024
#define JOY_LEFT         2048
#define JOY_MODE1        4096
#define JOY_MODE2        16384

#define MOUSE_BUT1       0x10000
#define MOUSE_BUT2       0x20000


// Test positioning MACROS
//
#define LEFT_EDGE        11
#define TITLE_LINE       2

#define SUBTITLE1_LINE   6
#define TEXT1_LINE       9

#define SUBTITLE2_LINE   17
#define TEXT2_LINE       20


// Palettes to use for text
//
#define PAL_TEXT      0
#define PAL_INVERSE   1
#define PAL_DIM       2
#define PAL_UNKNOWN   3
#define PAL_MULTITAP  4
#define PAL_JOYPAD    5
#define PAL_MOUSE     6


void printsjis(char *text, int x, int y);
void print_narrow(u32 sjis, u32 kram);
void print_wide(u32 sjis, u32 kram);

void print_at(int x, int y, int pal, char* str);
void putch_at(int x, int y, int pal, char c);
void putnumber_at(int x, int y, int pal, int digits, int value);

extern u8 font[];

// interrupt-handling variables
volatile int sda_frame_count = 0;
volatile int last_sda_frame_count = 0;

/* HuC6270-A's status register (RAM mapping). Used during VSYNC interrupt */
volatile uint16_t * const MEM_6270A_SR = (uint16_t *) 0x80000400;


char buffer[2048];

int stepval = 0;


///////////////////////////////// Joypad routines
volatile u32 joypad;
volatile u32 joypad_last;
volatile u32 joytrg;

volatile u32 joypad2;
volatile u32 joypad2_last;
volatile u32 joytrg2;

__attribute__ ((noinline)) void joyread(void)
{
u32 temp;

   joypad_last = joypad;
   joypad2_last = joypad2;

   temp = eris_pad_read(0);

   if ((temp >> 28) == PAD_TYPE_FXPAD) {  // PAD TYPE
      joytrg = (~joypad_last) & joypad;
   }
   else {
      joytrg = 0;
   }
   joypad = temp;

   temp = eris_pad_read(1);

   if ((temp >> 28) == PAD_TYPE_FXPAD) {  // PAD TYPE
      joytrg2 = (~joypad2_last) & joypad2;
   }
   else {
      joytrg2 = 0;
   }
   joypad2 = temp;
}

///////////////////////////////// Interrupt handler
__attribute__ ((interrupt_handler)) void my_vblank_irq (void)
{
   uint16_t vdc_status = *MEM_6270A_SR;

   if (vdc_status & 0x20) {
      sda_frame_count++;
   }
   joyread();
}

void vsync(int numframes)
{
   while (sda_frame_count < (last_sda_frame_count + numframes + 1));

   last_sda_frame_count = sda_frame_count;
}


///////////////////////////////// CODE

//
// for setting breakpoints - add a call to step() and breakpoint on it
// or watchpoint on stepval.
//
__attribute__ ((noinline)) void step(void)
{
   stepval++;;
}
//////////


void init(void)
{
	int i, j;
//	u32 str[256];
	u16 microprog[16];
	u16 a, img;

	eris_low_sup_init(0);
	eris_low_sup_init(1);
	eris_king_init();
	eris_tetsu_init();
	
	eris_tetsu_set_priorities(0, 0, 1, 0, 0, 0, 0);
	eris_tetsu_set_7up_palette(0, 0);
	eris_tetsu_set_king_palette(0, 0, 0, 0);
	eris_tetsu_set_rainbow_palette(0);

	eris_king_set_bg_prio(KING_BGPRIO_3, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, 0);
	eris_king_set_bg_mode(KING_BGMODE_4_PAL, 0, 0, 0);
	eris_king_set_kram_pages(0, 0, 0, 0);

	for(i = 0; i < 16; i++) {
		microprog[i] = KING_CODE_NOP;
	}

	microprog[0] = KING_CODE_BG0_CG_0;
	eris_king_disable_microprogram();
	eris_king_write_microprogram(microprog, 0, 16);
	eris_king_enable_microprogram();

	//eris_tetsu_set_palette(3, 0x602C);
	//eris_tetsu_set_palette(4, 0x5080);
	//eris_tetsu_set_palette(5, 0xC422);
	//eris_tetsu_set_palette(6, 0x9999);
	//eris_tetsu_set_palette(7, 0x1234);

	/* Font uses sub-palette #1 for FG, #2 for BG */
//	/* palette #0 is default - light green background, bright white foreground */
//	eris_tetsu_set_palette(0x00, 0x2A66);
//	eris_tetsu_set_palette(0x01, 0xFC88);
//	eris_tetsu_set_palette(0x02, 0x2A66);

	/* palette #0 is default - black background, white foreground */
	eris_tetsu_set_palette(0x00, 0x0088);
	eris_tetsu_set_palette(0x01, 0xE088);
	eris_tetsu_set_palette(0x02, 0x0088);

	/* palette #1 is selection/inverse - white background, black foreground */
	eris_tetsu_set_palette(0x10, 0xE088);
	eris_tetsu_set_palette(0x11, 0x0088);
	eris_tetsu_set_palette(0x12, 0xE088);

	/* palette #2 is disabled/dimmed - black background, dimmed white foreground */
	eris_tetsu_set_palette(0x20, 0x0088);
	eris_tetsu_set_palette(0x21, 0x9088);
	eris_tetsu_set_palette(0x22, 0x0088);

	/* palette #3 is Unknown/Red - black background, bright red foreground */
	eris_tetsu_set_palette(0x30, 0x0088);
	eris_tetsu_set_palette(0x31, 0x2A2F);
	eris_tetsu_set_palette(0x32, 0x0088);

	/* palette #4 is Multitap/yellow - black background, bright yellow foreground */
	eris_tetsu_set_palette(0x40, 0x0088);
	eris_tetsu_set_palette(0x41, 0xDF09);
	eris_tetsu_set_palette(0x42, 0x0088);

	/* palette #5 is Joypad/Turquoise - black background, turquoise foreground */
	eris_tetsu_set_palette(0x50, 0x0088);
	eris_tetsu_set_palette(0x51, 0x9BB1);
	eris_tetsu_set_palette(0x52, 0x0088);

	/* palette #6 is Mouse/Green - black background, green foreground */
	eris_tetsu_set_palette(0x60, 0x0088);
	eris_tetsu_set_palette(0x61, 0x8006);
	eris_tetsu_set_palette(0x62, 0x0088);


	eris_tetsu_set_video_mode(TETSU_LINES_262, 0, TETSU_DOTCLOCK_7MHz, TETSU_COLORS_16,
				TETSU_COLORS_16, 1, 0, 1, 0, 0, 0, 0);
	eris_king_set_bat_cg_addr(KING_BG0, 0, 0);
	eris_king_set_bat_cg_addr(KING_BG0SUB, 0, 0);
	eris_king_set_scroll(KING_BG0, 0, 0);
	eris_king_set_bg_size(KING_BG0, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256);
	eris_low_sup_set_control(0, 0, 1, 0);
	eris_low_sup_set_access_width(0, 0, SUP_LOW_MAP_64X32, 0, 0);
	eris_low_sup_set_scroll(0, 0, 0);
	//eris_low_sup_set_video_mode(0, 2, 2, 4, 0x1F, 0x11, 2, 239, 2); // 5MHz numbers
	eris_low_sup_set_video_mode(0, 3, 3, 6, 0x2B, 0x11, 2, 239, 2);

	eris_king_set_kram_read(0, 1);
	eris_king_set_kram_write(0, 1);
	// Clear BG0's RAM
	for(i = 0; i < 0x1E00; i++) {
		eris_king_kram_write(0);
	}
	eris_king_set_kram_write(0, 1);

	eris_low_sup_set_vram_write(0, 0);
	for(i = 0; i < 0x800; i++) {
		eris_low_sup_vram_write(0, 0x120); // 0x80 is space
	}


	eris_low_sup_set_vram_write(0, 0x1200);
	// load font into video memory
	for(i = 0; i < 0x60; i++) {
		// first 2 planes of color
		for (j = 0; j < 8; j++) {
			img = font[(i*8)+j] & 0xff;
			a = (~img << 8) | img;
			eris_low_sup_vram_write(0, a);
		}
		// last 2 planes of color
		for (j = 0; j < 8; j++) {
			eris_low_sup_vram_write(0, 0);
		}
	}

	eris_pad_init(0); // initialize joypad


        // Disable all interrupts before changing handlers.
        irq_set_mask(0x7F);

        // Replace firmware IRQ handlers for the Timer and HuC6270-A.
        //
        // This liberis function uses the V810's hardware IRQ numbering,
        // see FXGA_GA and FXGABOAD documents for more info ...
        irq_set_raw_handler(0xC, my_vblank_irq);

        // Enable Timer and HuC6270-A interrupts.
        //
        // d6=Timer
        // d5=External
        // d4=KeyPad
        // d3=HuC6270-A
        // d2=HuC6272
        // d1=HuC6270-B
        // d0=HuC6273
        irq_set_mask(0x77);

        // Allow all IRQs.
        //
        // This liberis function uses the V810's hardware IRQ numbering,
        // see FXGA_GA and FXGABOAD documents for more info ...
        irq_set_level(8);

        // Enable V810 CPU's interrupt handling.
        irq_enable();

        eris_low_sup_setreg(0, 5, 0x88);  // Set Hu6270 BG to show, and VSYNC Interrupt

}


void show_joy_init(int leftedge, int titleline, int textline, int pal)
{
   print_at(leftedge + 11, titleline, pal, "Joypad  (F)");

   print_at(leftedge, textline, pal,     "    Mode 1: A-B       ");
   print_at(leftedge, textline + 1, pal, "    Mode 2: A-B       ");
   print_at(leftedge, textline + 2, pal, " ^                    ");
   print_at(leftedge, textline + 3, pal, "<+>            4 5 6  ");
   print_at(leftedge, textline + 4, pal, " v             3 2 1  ");
   print_at(leftedge, textline + 5, pal, "      Sel Run         ");
}

void show_joy_val(int joyval, int leftedge, int textline, int pal)
{
   print_at((leftedge + 13),  textline,      pal, (joyval & JOY_MODE1) ?  ">" : "<");
   print_at((leftedge + 13), (textline + 1), pal, (joyval & JOY_MODE2) ?  ">" : "<");
   print_at((leftedge + 1),  (textline + 2), pal, (joyval & JOY_UP) ?     "^" : " ");
   print_at( leftedge ,      (textline + 3), pal, (joyval & JOY_LEFT) ?   "<" : " ");
   print_at((leftedge + 2),  (textline + 3), pal, (joyval & JOY_RIGHT) ?  ">" : " ");
   print_at((leftedge + 15), (textline + 3), pal, (joyval & JOY_IV) ?     "4" : "-");
   print_at((leftedge + 17), (textline + 3), pal, (joyval & JOY_V) ?      "5" : "-");
   print_at((leftedge + 19), (textline + 3), pal, (joyval & JOY_VI) ?     "6" : "-");
   print_at((leftedge + 1),  (textline + 4), pal, (joyval & JOY_DOWN) ?   "v" : " ");
   print_at((leftedge + 15), (textline + 4), pal, (joyval & JOY_III) ?    "3" : "-");
   print_at((leftedge + 17), (textline + 4), pal, (joyval & JOY_II) ?     "2" : "-");
   print_at((leftedge + 19), (textline + 4), pal, (joyval & JOY_I) ?      "1" : "-");
   print_at((leftedge + 6),  (textline + 5), pal, (joyval & JOY_SELECT) ? "Sel" : "---");
   print_at((leftedge + 10), (textline + 5), pal, (joyval & JOY_RUN) ?    "Run" : "---");
}

void show_mouse_init(int leftedge, int titleline, int textline, int pal)
{
   print_at(leftedge + 11, titleline, pal, "Mouse   (D)");

   print_at(leftedge, textline, pal,     "  Button:  1  2       ");
   print_at(leftedge, textline + 1, pal, "       ###            ");
   print_at(leftedge, textline + 2, pal, "        ^             ");
   print_at(leftedge, textline + 3, pal, "  ### < + > ###       ");
   print_at(leftedge, textline + 4, pal, "        v             ");
   print_at(leftedge, textline + 5, pal, "       ###            ");
}

void show_mouse_val(int joyval, int leftedge, int textline, int pal)
{
int abs;
char buf[20];
signed char x_amt, y_amt;

   x_amt = ((joyval>>8) & 0xff);
   y_amt = (joyval & 0xff);

   print_at((leftedge + 11), textline,      pal, (joyval & MOUSE_BUT1) ?  "1" : "-");
   print_at((leftedge + 14), textline,      pal, (joyval & MOUSE_BUT2) ?  "2" : "-");

   if (x_amt == 0)
      print_at((leftedge +  2), textline + 3, pal, "      +      ");
   else if (x_amt < 0)
   {
      abs = -x_amt;
      sprintf(buf, "%3d", abs);
      print_at((leftedge +  6), textline + 3, pal, "< +      ");
      print_at((leftedge +  2), textline + 3, pal, buf);
   }
   else
   {
      sprintf(buf, "%3d", x_amt);
      print_at((leftedge +  2), textline + 3, pal, "      + >");
      print_at((leftedge + 12), textline + 3, pal, buf);
   }

   if (y_amt == 0)
   {
      print_at((leftedge + 7), textline + 1, pal, "   ");
      print_at((leftedge + 8), textline + 2, pal, " ");
      print_at((leftedge + 8), textline + 4, pal, " ");
      print_at((leftedge + 7), textline + 5, pal, "   ");
   }
   else if (y_amt < 0)
   {
      abs = -y_amt;
      sprintf(buf, "%3d", abs);
      print_at((leftedge + 7), textline + 1, pal, buf);
      print_at((leftedge + 8), textline + 2, pal, "^");
      print_at((leftedge + 8), textline + 4, pal, " ");
      print_at((leftedge + 7), textline + 5, pal, "   ");
   }
   else
   {
      sprintf(buf, "%3d", y_amt);
      print_at((leftedge + 7), textline + 1, pal, "   ");
      print_at((leftedge + 8), textline + 2, pal, " ");
      print_at((leftedge + 8), textline + 4, pal, "v");
      print_at((leftedge + 7), textline + 5, pal, buf);
   }
}

void show_none_init(int leftedge, int titleline, int textline, int pal)
{
   print_at(leftedge + 11, titleline, pal, "None    (0)");

   print_at(leftedge, textline, pal,     "                      ");
   print_at(leftedge, textline + 1, pal, "                      ");
   print_at(leftedge, textline + 2, pal, "                      ");
   print_at(leftedge, textline + 3, pal, "                      ");
   print_at(leftedge, textline + 4, pal, "                      ");
   print_at(leftedge, textline + 5, pal, "                      ");
}

void show_unknown_init(int leftedge, int titleline, int textline, int pal, int joytype)
{
char hexdata[16];

   sprintf(hexdata, "Unknown (%1.1X)", joytype);
   print_at(leftedge + 11, titleline, pal, hexdata);

   print_at(leftedge, textline, pal,     "                      ");
   print_at(leftedge, textline + 1, pal, "                      ");
   print_at(leftedge, textline + 2, pal, "                      ");
   print_at(leftedge, textline + 3, pal, "                      ");
   print_at(leftedge, textline + 4, pal, "                      ");
   print_at(leftedge, textline + 5, pal, "                      ");
}

int main(int argc, char *argv[])
{
char hexdata[16];
static u32 joy1 = 0;
static u32 joy2 = 0;
u32 prevjoy1, prevjoy2;
u8 firsttime = 1;

   init();

   print_at(12, TITLE_LINE, PAL_TEXT, "FX Controller Test");
   print_at(33, TITLE_LINE, PAL_TEXT, "v0.1");


   print_at(9, SUBTITLE1_LINE, PAL_TEXT, "Port 1: ");
   print_at(9, SUBTITLE2_LINE, PAL_TEXT, "Port 2: ");
//

   while (1)
   {
      prevjoy1 = joy1;
      prevjoy2 = joy2;
      joy1 = joypad;
      joy2 = joypad2;

      //
      // Port # 1:
      //

      // Joypad:
      //
      if (((joy1 >> 28) & 0x0F) == PAD_TYPE_FXPAD)
      {
          if ((firsttime == 1) || (prevjoy1 != joy1))
             show_joy_init(LEFT_EDGE, SUBTITLE1_LINE, TEXT1_LINE, PAL_JOYPAD);

          sprintf(hexdata, "%7.7X", (joy1 & 0x0FFFFFFF));
          print_at(30, SUBTITLE1_LINE + 1, PAL_JOYPAD, hexdata);

          show_joy_val(joy1, LEFT_EDGE, TEXT1_LINE, PAL_JOYPAD);
      }
      // Mouse:
      //
      else if (((joy1 >> 28) & 0x0F) == PAD_TYPE_MOUSE)
      {
          if ((firsttime == 1) || (prevjoy1 != joy1))
             show_mouse_init(LEFT_EDGE, SUBTITLE1_LINE, TEXT1_LINE, PAL_MOUSE);

          sprintf(hexdata, "%7.7X", (joy1 & 0x0FFFFFFF));
          print_at(30, SUBTITLE1_LINE + 1, PAL_MOUSE, hexdata);

          show_mouse_val(joy1, LEFT_EDGE, TEXT1_LINE, PAL_MOUSE);
      }
      // Nothing:
      //
      else if (((joy1 >> 28) & 0x0F) == PAD_TYPE_NONE)
      {
          if ((firsttime == 1) || (prevjoy1 != joy1))
             show_none_init(LEFT_EDGE, SUBTITLE1_LINE, TEXT1_LINE, PAL_DIM);

          sprintf(hexdata, "%7.7X", (joy1 & 0x0FFFFFFF));
          print_at(30, SUBTITLE1_LINE + 1, PAL_DIM, hexdata);
      }
      // Unknown:
      //
      else
      {
          if ((firsttime == 1) || (prevjoy1 != joy1))
             show_unknown_init(LEFT_EDGE, SUBTITLE1_LINE, TEXT1_LINE, PAL_UNKNOWN, ((joy1>>28) & 0xFF));

          sprintf(hexdata, "%7.7X", (joy1 & 0x0FFFFFFF));
          print_at(30, SUBTITLE1_LINE + 1, PAL_UNKNOWN, hexdata);
      }


      //
      // Port # 2:
      //

      // Joypad:
      //
      if (((joy2 >> 28) & 0x0F) == PAD_TYPE_FXPAD)
      {
          if ((firsttime == 1) || (prevjoy2 != joy2))
             show_joy_init(LEFT_EDGE, SUBTITLE2_LINE, TEXT2_LINE, PAL_JOYPAD);

          sprintf(hexdata, "%7.7X", (joy2 & 0x0FFFFFFF));
          print_at(30, SUBTITLE2_LINE + 1, PAL_JOYPAD, hexdata);

          show_joy_val(joy2, LEFT_EDGE, TEXT2_LINE, PAL_JOYPAD);
      }
      // Mouse:
      //
      else if (((joy2 >> 28) & 0x0F) == PAD_TYPE_MOUSE)
      {
          if ((firsttime == 1) || (prevjoy2 != joy2))
             show_mouse_init(LEFT_EDGE, SUBTITLE2_LINE, TEXT1_LINE, PAL_MOUSE);

          sprintf(hexdata, "%7.7X", (joy2 & 0x0FFFFFFF));
          print_at(30, SUBTITLE2_LINE + 1, PAL_MOUSE, hexdata);

          show_mouse_val(joy2, LEFT_EDGE, TEXT2_LINE, PAL_MOUSE);
      }
      // Nothing:
      //
      else if (((joy2 >> 28) & 0x0F) == PAD_TYPE_NONE)
      {
          if ((firsttime == 1) || (prevjoy2 != joy2))
             show_none_init(LEFT_EDGE, SUBTITLE2_LINE, TEXT2_LINE, PAL_DIM);

          sprintf(hexdata, "%7.7X", (joy2 & 0x0FFFFFFF));
          print_at(30, SUBTITLE2_LINE + 1, PAL_DIM, hexdata);
      }
      // Unknown:
      //
      else
      {
          if ((firsttime == 1) || (prevjoy2 != joy2))
             show_unknown_init(LEFT_EDGE, SUBTITLE2_LINE, TEXT2_LINE, PAL_UNKNOWN, ((joy2>>28) & 0xFF));

          sprintf(hexdata, "%7.7X", (joy2 & 0x0FFFFFFF));
          print_at(30, SUBTITLE2_LINE + 1, PAL_UNKNOWN, hexdata);
      }


      firsttime = 0;
      vsync(0);
   }

   return 0;
}

// print with first 7up (HuC6270 #0)
//
void print_at(int x, int y, int pal, char* str)
{
	int i;
	u16 a;

	i = (y * 64) + x;

	eris_low_sup_set_vram_write(0, i);
	for (i = 0; i < strlen8(str); i++) {
		a = (pal * 0x1000) + str[i] + 0x100;
		eris_low_sup_vram_write(0, a);
	}
}

void putch_at(int x, int y, int pal, char c)
{
        int i;
        u16 a;

        i = (y * 64) + x;

        eris_low_sup_set_vram_write(0, i);

        a = (pal * 0x1000) + c + 0x100;
        eris_low_sup_vram_write(0, a);
}

void putnumber_at(int x, int y, int pal, int len, int value)
{
        int i;
        u16 a;
	char str[64];

        i = (y * 64) + x;

	if (len == 2) {
	   sprintf(str, "%2d", value);
	}
	else if (len == 4) {
	   sprintf(str, "%4d", value);
	}
	else if (len == 5) {
	   sprintf(str, "%5d", value);
	}

        eris_low_sup_set_vram_write(0, i);

	for (i = 0; i < strlen8(str); i++) {
                a = (pal * 0x1000) + str[i] + 0x100;
                eris_low_sup_vram_write(0, a);
        }
}

// functions related to printing with KING processor
//

void printsjis(char *text, int x, int y)
{

int offset;
u8 ch, ch2;
u32 sjis;
u32 kram;

   offset = 0;
   kram = x + (y <<5);

   ch = *(text+offset);

   while (ch != 0)
   {
      if ((ch < 0x81) || ((ch >= 0xA1) && (ch <= 0xDF)))
      {
         sjis = ch;
         print_narrow(sjis, kram);
         kram++;
      }
      else
      {
         offset++;
         ch2 = *(text+offset);
         sjis = (ch << 8) + ch2;
         print_wide(sjis, kram);
         kram += 2;
      }

      offset++;
      ch = *(text+offset);
   }

}

void print_narrow(u32 sjis, u32 kram)
{
        u16 px;
        int x, y;
        u8* glyph;

        glyph = eris_romfont_get(sjis, ROMFONT_ANK_8x16);

        for(y = 0; y < 16; y++) {
                eris_king_set_kram_write(kram + (y << 5), 1);
                px = 0;
                for(x = 0; x < 8; x++) {
                        if((glyph[y] >> x) & 1) {
                                px |= 1 << (x << 1);
                        }
                }
                eris_king_kram_write(px);
        }
}

void print_wide(u32 sjis, u32 kram)
{
        u16 px;
        int x, y;
        u16* glyph;

        glyph = (u16*) eris_romfont_get(sjis, ROMFONT_KANJI_16x16);

        for(y = 0; y < 16; y++) {
                eris_king_set_kram_write(kram + (y << 5), 1);
                px = 0;
                for(x = 0; x < 8; x++) {
                        if((glyph[y] >> x) & 1) {
                                px |= 1 << (x << 1);
                        }
                }
                eris_king_kram_write(px);

                eris_king_set_kram_write(kram + (y << 5) + 1, 1);
                px = 0;
                for(x = 0; x < 8; x++) {
                        if((glyph[y] >> (x+8)) & 1) {
                                px |= 1 << (x << 1);
                        }
                }
                eris_king_kram_write(px);
        }
}

