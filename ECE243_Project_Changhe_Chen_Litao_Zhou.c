#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>


/* This files provides address values that exist in the system */

#define BOARD                 "DE1-SoC"

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_END       0xC803FFFF
#define FPGA_CHAR_BASE        0xC9000000
#define FPGA_CHAR_END         0xC9001FFF

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define JP1_BASE              0xFF200060
#define JP2_BASE              0xFF200070
#define PS2_BASE              0xFF200100
#define PS2_DUAL_BASE         0xFF200108
#define JTAG_UART_BASE        0xFF201000
#define JTAG_UART_2_BASE      0xFF201008
#define IrDA_BASE             0xFF201020
#define TIMER_BASE            0xFF202000
#define AV_CONFIG_BASE        0xFF203000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define AUDIO_BASE            0xFF203040
#define VIDEO_IN_BASE         0xFF203060
#define ADC_BASE              0xFF204000

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE        0xFF709000
#define HPS_TIMER0_BASE       0xFFC08000
#define HPS_TIMER1_BASE       0xFFC09000
#define HPS_TIMER2_BASE       0xFFD00000
#define HPS_TIMER3_BASE       0xFFD01000
#define FPGA_BRIDGE           0xFFD0501C

/* ARM A9 MPCORE devices */
#define   PERIPH_BASE         0xFFFEC000    // base address of peripheral devices
#define   MPCORE_PRIV_TIMER   0xFFFEC600    // PERIPH_BASE + 0x0600

/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg
/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs


/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240


volatile int pixel_buffer_start; // global variable

void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void draw_start();
void draw_p1_hand_down();
void draw_p2_hand_down();
void erase_p1_hand_down();
void erase_p2_hand_down();
void erase_p1_hand_up();
void erase_p2_hand_up();
void draw_p1_hand_up();
void draw_p2_hand_up();
void draw_stone(int x, int y, short int line_color);
void draw_start_prompt(short int color_author, short int color_prompt, short int color_title);
void draw_end_prompt(short int color);
void draw_p1_win(short int color);
void draw_p2_win(short int color);
void draw_win_player(short int color);
void draw_wind(int wind, short int color);
void draw_p1_force(int force, short int color);
void draw_p2_force(int force, short int color);

void reset_draw_wind();
void swap(int* x, int* y);
void draw_line(int x0, int y0, int x1, int y1, short int color);
void wait_for_vsync();
void draw_name_word();
void draw_word_component(int x0, int y0, int x1, int y1, short int color);
void draw_title_component(int x0, int y0, int x1, int y1, short int color);
void draw_p1_turn_arrow(short int color);
void draw_p2_turn_arrow(short int color);
void draw_ball_curve_user1(int x_speed, int y_speed);
void draw_ball_curve_user2(int x_speed, int y_speed);
void draw_wind(int wind, short int color);
void draw_p1_life(int life, short int color);
void draw_p2_life(int life, short int color);

const int left_x = 30;
const int left_y = 180;//initial position of user 1 ball

const int right_x = 290;
const int right_y = 180;//initial position of user 2 ball

const int ball_to_xbound = 290;//maximum x distance to ball travel

int space_pressed_time = 0;

bool user1_turn = true;
bool user2_turn = false;
bool start = false;

int user1_speed = 35;
int user2_speed = 35;
int user1_speed_exibit = 41;
int user2_speed_exibit = 41;
int wind_speed = 5;

int user1_life = 10;
int user2_life = 10;

volatile int* PS2_ptr = (int*)0xFF200100;  // PS/2 port address
int PS2_data, RVALID;


int main() {
	volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
	/* Read location of the pixel buffer from the pixel buffer controller */
	pixel_buffer_start = *pixel_ctrl_ptr;

	/* set front pixel buffer to start of FPGA On-chip memory */
	*(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
										// back buffer
	/* now, swap the front/back buffers, to set the front buffer location */
	wait_for_vsync();
	/* initialize a pointer to the pixel buffer, used by drawing functions */
	pixel_buffer_start = *pixel_ctrl_ptr;
	clear_screen(); // pixel_buffer_start points to the pixel buffer
	/* set back pixel buffer to start of SDRAM memory */
	*(pixel_ctrl_ptr + 1) = 0xC0000000;
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	clear_screen();
	char byte1 = 0, byte2 = 0, byte3 = 0;
	*(PS2_ptr) = 0xFF;


	while (1) {
		clear_screen();
		if (start == false) {
			draw_start_prompt(YELLOW, WHITE, ORANGE);
			draw_p1_hand_down();
			draw_p2_hand_down();
		}
		if (start == true) {
			//current force of user
			if (user1_speed == 65) {
				user1_speed = 35;
			}
			else user1_speed = user1_speed + 3;


			if (user2_speed == 65) {
				user2_speed = 35;
			}
			else user2_speed = user2_speed + 3;

			//actual force due to delay
			if (user1_speed_exibit == 65) {
				user1_speed_exibit = 35;
			}
			else user1_speed_exibit = user1_speed_exibit + 3;


			if (user2_speed_exibit == 65) {
				user2_speed_exibit = 35;
			}
			else user2_speed_exibit = user2_speed_exibit + 3;

			draw_start();//draw all
			draw_name_word(); //draw the word of people and wind
			draw_p1_life(user1_life, WHITE);
			draw_p2_life(user2_life, WHITE);
			draw_wind(wind_speed, CYAN);
			if (user1_turn == true) {
				draw_p1_hand_up();
				draw_p2_hand_down();
				draw_p1_turn_arrow(GREEN);
				draw_p1_force(user1_speed_exibit - 35, WHITE);
			}

			if (user2_turn == true) {
				draw_p2_hand_up();
				draw_p1_hand_down();
				draw_p2_turn_arrow(GREEN);
				draw_p2_force(user2_speed_exibit - 35, WHITE);
			}

			if (user1_life == 0) {//p2 win
				draw_p1_force(0, 0x0000);
				erase_p2_hand_down();
				draw_win_player(BLUE);
				erase_p1_hand_up();
				draw_p1_turn_arrow(0x0000);
				draw_end_prompt(YELLOW);
				draw_p2_win(BLUE);
			}

			if (user2_life == 0) {//p1 win
				draw_p2_force(0, 0x0000);
				erase_p1_hand_down();
				draw_win_player(RED);
				erase_p2_hand_up();
				draw_p2_turn_arrow(0x0000);
				draw_end_prompt(YELLOW);
				draw_p1_win(RED);
			}
		}

		PS2_data = *(PS2_ptr); // read the Data register in the PS/2 port
		RVALID = PS2_data & 0x8000;



		if (RVALID) {
			byte2 = byte3;
			byte3 = PS2_data & 0xFF;


			if (space_pressed_time == 0 && byte3 == 0x29 && byte2 == 0xF0) {
				draw_start_prompt(0x0000, 0x0000, 0x0000);
				space_pressed_time++;
				start = true;
			}
			else if (byte3 == 0x29 && (user1_life == 0 || user2_life == 0) && byte2 == 0xF0) {
				draw_end_prompt(0x0000);
				if (user1_life == 0) {
					draw_p2_win(0x0000);
				}
				if (user2_life == 0) {
					draw_p1_win(0x0000);
				}
				start = false;
				user1_life = 10;
				user2_life = 10;
				user2_turn = false;
				user1_turn = true;
				space_pressed_time = 0;
			}
			else if (byte3 == 0x29 && user1_turn == true && byte2 == 0xF0) {
				wind_speed = rand() % 20 - 10;//wind speed
				int user1_x_speed = user1_speed / 5 * 3 + wind_speed;
				int user1_y_speed = user1_speed / 5 * 4;
				draw_ball_curve_user1(user1_x_speed, user1_y_speed); //draw the curve of the flying ball
				user1_turn = false;
				user2_turn = true;
			}
			else if (byte3 == 0x29 && user2_turn == true && byte2 == 0xF0) {
				wind_speed = rand() % 20 - 10;//wind speed
				int user2_x_speed = user2_speed / 5 * 3 + wind_speed;
				int user2_y_speed = user2_speed / 5 * 4;
				draw_ball_curve_user2(user2_x_speed, user2_y_speed); //draw the curve of the flying ball
				user2_turn = false;
				user1_turn = true;
			}
		}
		wait_for_vsync(); // swap front and back buffers on VGA vertical sync
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
	}
}

void wait_for_vsync() {
	volatile int* pixel_ctrl_ptr = (int*)PIXEL_BUF_CTRL_BASE; // Buffer register
	register int status;

	*pixel_ctrl_ptr = 1; // start the synchronization process

	status = *(pixel_ctrl_ptr + 3);
	while ((status & 0x01) != 0) {
		status = *(pixel_ctrl_ptr + 3);
	}
}


// code not shown for clear_screen() and draw_line() subroutines
void plot_pixel(int x, int y, short int line_color)
{
	*(short int*)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}




//write the black into the color
void clear_screen() {
	int i = 0;
	int j = 0;

	for (i = 0; i < 320; i++) {
		for (j = 0; j < 240; j++) {
			plot_pixel(i, j, 0x0000);
		}
	}
}





void swap(int* x, int* y) {
	int z = *x;
	*x = *y;
	*y = z;


}


void draw_line(int x0, int y0, int x1, int y1, short int colour) {
	bool is_sleep;

	if (ABS(y1 - y0) > ABS(x1 - x0)) {
		is_sleep = true;
	}
	else {
		is_sleep = false;
	}

	if (is_sleep) {
		swap(&x0, &y0);
		swap(&x1, &y1);
	}

	if (x0 > x1) {
		swap(&x0, &x1);
		swap(&y0, &y1);
	}

	int deltax = x1 - x0;
	int deltay = ABS(y1 - y0);
	int error = -(deltax / 2);

	int y = y0;
	int y_step = 0;
	if (y0 < y1) {
		y_step = 1;
	}
	else {
		y_step = -1;
	}

	int x = x0;
	for (x = x0; x <= x1; x++) {
		if (is_sleep) {
			plot_pixel(y, x, colour);

		}
		else {
			plot_pixel(x, y, colour);

		}

		error = error + deltay;
		if (error >= 0) {
			y = y + y_step;
			error = error - deltax;
		}




	}

}

//draw the start pages
void draw_start() {
	//draw wind
	for (int i = 117; i <= 203; i++) {
		draw_line(i, 27, i, 53, 0xFFFF);
	}

	for (int i = 120; i <= 200; i++) {
		draw_line(i, 30, i, 50, 0x0000);
	}


	//draw A life
	for (int i = 18; i <= 82; i++) {
		draw_line(i, 38, i, 52, 0xFFFF);
	}

	for (int i = 20; i <= 80; i++) {
		draw_line(i, 40, i, 50, 0x0000);
	}



	//draw B life
	for (int i = 238; i <= 302; i++) {
		draw_line(i, 38, i, 52, 0xFFFF);
	}

	for (int i = 240; i <= 300; i++) {
		draw_line(i, 40, i, 50, 0x0000);
	}

}

void draw_p1_hand_up() {
	//draw the player A head
	for (int i = 50; i <= 60; i++) {
		draw_line(i, 176, i, 189, RED);
	}

	//draw the player A body 
	for (int i = 43; i <= 67; i++) {
		draw_line(i, 191, i, 219, RED);
	}

	//draw the player A left leg
	for (int i = 43; i <= 54; i++) {
		draw_line(i, 221, i, 237, RED);
	}


	//draw the player A right leg
	for (int i = 56; i <= 67; i++) {
		draw_line(i, 221, i, 237, RED);
	}

	//draw the player A left arm
	for (int i = 25; i <= 40; i++) {

		draw_line(40, 205, i, 190, RED);
	}

	for (int i = 35; i <= 40; i++) {

		draw_line(40, 195, i, 190, 0x0000);

	}

	//draw the player A right arm
	for (int i = 70; i <= 77; i++) {
		draw_line(i, 197, i, 215, RED);
	}
}

void erase_p1_hand_up() {
	//draw the player A head
	for (int i = 50; i <= 60; i++) {
		draw_line(i, 176, i, 189, 0x0000);
	}

	//draw the player A body 
	for (int i = 43; i <= 67; i++) {
		draw_line(i, 191, i, 219, 0x0000);
	}

	//draw the player A left leg
	for (int i = 43; i <= 54; i++) {
		draw_line(i, 221, i, 237, 0x0000);
	}


	//draw the player A right leg
	for (int i = 56; i <= 67; i++) {
		draw_line(i, 221, i, 237, 0x0000);
	}

	//draw the player A left arm
	for (int i = 25; i <= 40; i++) {

		draw_line(40, 205, i, 190, 0x0000);
	}

	for (int i = 35; i <= 40; i++) {

		draw_line(40, 195, i, 190, 0x0000);

	}

	//draw the player A right arm
	for (int i = 70; i <= 77; i++) {
		draw_line(i, 197, i, 215, 0x0000);
	}
}

void draw_p2_hand_up() {
	//draw the player B head
	for (int i = 260; i <= 270; i++) {
		draw_line(i, 176, i, 189, BLUE);
	}

	//draw the player B body 
	for (int i = 253; i <= 277; i++) {
		draw_line(i, 191, i, 219, BLUE);
	}

	//draw the player B left leg
	for (int i = 253; i <= 264; i++) {
		draw_line(i, 221, i, 237, BLUE);
	}


	//draw the player B right leg
	for (int i = 266; i <= 277; i++) {
		draw_line(i, 221, i, 237, BLUE);
	}

	//draw the player B right arm
	for (int i = 280; i <= 295; i++) {

		draw_line(280, 205, i, 190, BLUE);
	}

	for (int i = 280; i <= 285; i++) {

		draw_line(280, 195, i, 190, 0x0000);

	}

	//draw the player B left arm
	for (int i = 243; i <= 250; i++) {
		draw_line(i, 197, i, 215, BLUE);
	}
}


void erase_p2_hand_up() {
	//draw the player B head
	for (int i = 260; i <= 270; i++) {
		draw_line(i, 176, i, 189, 0x0000);
	}

	//draw the player B body 
	for (int i = 253; i <= 277; i++) {
		draw_line(i, 191, i, 219, 0x0000);
	}

	//draw the player B left leg
	for (int i = 253; i <= 264; i++) {
		draw_line(i, 221, i, 237, 0x0000);
	}


	//draw the player B right leg
	for (int i = 266; i <= 277; i++) {
		draw_line(i, 221, i, 237, 0x0000);
	}

	//draw the player B right arm
	for (int i = 280; i <= 295; i++) {

		draw_line(280, 205, i, 190, 0x0000);
	}

	for (int i = 280; i <= 285; i++) {

		draw_line(280, 195, i, 190, 0x0000);

	}

	//draw the player B left arm
	for (int i = 243; i <= 250; i++) {
		draw_line(i, 197, i, 215, 0x0000);
	}
}

//draw the stone
void draw_stone(int x, int y, short int line_color) {
	//draw up side
	draw_line(x - 1, y - 6, x + 1, y - 6, line_color);
	draw_line(x - 3, y - 5, x + 3, y - 5, line_color);
	draw_line(x - 4, y - 4, x + 4, y - 4, line_color);
	draw_line(x - 5, y - 3, x + 5, y - 3, line_color);
	draw_line(x - 5, y - 2, x + 5, y - 2, line_color);
	draw_line(x - 6, y - 1, x + 6, y - 1, line_color);

	//draw middle line
	draw_line(x - 6, y, x + 6, y, line_color);

	//draw down side
	draw_line(x - 6, y + 1, x + 6, y + 1, line_color);
	draw_line(x - 5, y + 2, x + 5, y + 2, line_color);
	draw_line(x - 5, y + 3, x + 5, y + 3, line_color);
	draw_line(x - 4, y + 4, x + 4, y + 4, line_color);
	draw_line(x - 3, y + 5, x + 3, y + 5, line_color);
	draw_line(x - 1, y + 6, x + 1, y + 6, line_color);


}


void draw_p1_turn_arrow(short int color) {
	for (int i = 50; i <= 60; i++) {
		draw_line(i, 140, i, 159, color);
	}

	for (int i = 45; i <= 65; i++) {
		draw_line(i, 160, 55, 170, color);
	}

}


void draw_p2_turn_arrow(short int color) {
	for (int i = 260; i <= 270; i++) {
		draw_line(i, 140, i, 159, color);
	}

	for (int i = 255; i <= 275; i++) {
		draw_line(i, 160, 265, 170, color);
	}

}


void draw_name_word() {
	//draw W
	draw_word_component(133, 10, 137, 20, 0xFFFF);
	draw_word_component(137, 20, 140, 10, 0xFFFF);
	draw_word_component(140, 10, 143, 20, 0xFFFF);
	draw_word_component(143, 20, 147, 10, 0xFFFF);

	//draw I
	draw_word_component(155, 10, 155, 20, 0xFFFF);

	//draw N
	draw_word_component(165, 10, 165, 20, 0xFFFF);
	draw_word_component(165, 10, 175, 20, 0xFFFF);
	draw_word_component(175, 10, 175, 20, 0xFFFF);

	//draw D
	draw_word_component(185, 10, 185, 20, 0xFFFF);
	draw_word_component(185, 10, 192, 13, 0xFFFF);
	draw_word_component(185, 20, 192, 16, 0xFFFF);
	draw_word_component(192, 13, 192, 16, 0xFFFF);

	//draw P1
	draw_word_component(40, 10, 40, 30, RED);
	draw_word_component(40, 10, 50, 10, RED);
	draw_word_component(40, 20, 50, 20, RED);
	draw_word_component(50, 10, 50, 20, RED);

	draw_word_component(55, 15, 60, 10, RED);
	draw_word_component(60, 10, 60, 30, RED);
	draw_word_component(55, 30, 65, 30, RED);

	//draw P2
	draw_word_component(260, 10, 260, 30, BLUE);
	draw_word_component(260, 10, 270, 10, BLUE);
	draw_word_component(260, 20, 270, 20, BLUE);
	draw_word_component(270, 10, 270, 20, BLUE);

	draw_word_component(275, 10, 285, 10, BLUE);
	draw_word_component(285, 10, 285, 20, BLUE);
	draw_word_component(275, 20, 285, 20, BLUE);
	draw_word_component(275, 20, 275, 30, BLUE);
	draw_word_component(275, 30, 285, 30, BLUE);
}





void draw_word_component(int x0, int y0, int x1, int y1, short int color) {

	if (y0 == y1) {
		draw_line(x0, y0 - 1, x1, y1 - 1, color);
		draw_line(x0, y0, x1, y1, color);
		draw_line(x0, y0 + 1, x1, y1 + 1, color);

	}
	else {
		draw_line(x0 - 1, y0, x1 - 1, y1, color);
		draw_line(x0, y0, x1, y1, color);
		draw_line(x0 + 1, y0, x1 + 1, y1, color);
	}

}

void draw_title_component(int x0, int y0, int x1, int y1, short int color) {

	if (y0 == y1) {
		draw_line(x0, y0 - 2, x1, y1 - 2, color);
		draw_line(x0, y0 - 1, x1, y1 - 1, color);
		draw_line(x0, y0, x1, y1, color);
		draw_line(x0, y0 + 1, x1, y1 + 1, color);
		draw_line(x0, y0 + 2, x1, y1 + 2, color);

	}
	else {
		draw_line(x0 - 2, y0, x1 - 2, y1, color);
		draw_line(x0 - 1, y0, x1 - 1, y1, color);
		draw_line(x0, y0, x1, y1, color);
		draw_line(x0 + 1, y0, x1 + 1, y1, color);
		draw_line(x0 + 2, y0, x1 + 2, y1, color);
	}

}




// draw p1 force
void draw_p1_force(int force, short int color) {
	for (int i = 78; i <= 112; i++) {
		draw_line(i, 178, i, 192, color);
	}

	for (int i = 80; i <= 110; i++) {
		draw_line(i, 180, i, 190, 0x0000);
	}
	for (int i = 80; i <= 80 + force; i++) {
		draw_line(i, 180, i, 190, color);
	}
}


// draw p2 force
void draw_p2_force(int force, short int color) {
	for (int i = 208; i <= 242; i++) {
		draw_line(i, 178, i, 192, color);
	}

	for (int i = 210; i <= 240; i++) {
		draw_line(i, 180, i, 190, 0x0000);
	}


	for (int i = 240; i >= 240 - force; i--) {
		draw_line(i, 180, i, 190, color);
	}
}


void draw_p1_hand_down() {
	//draw the player A head
	for (int i = 50; i <= 60; i++) {
		draw_line(i, 176, i, 189, RED);
	}

	//draw the player A body 
	for (int i = 43; i <= 67; i++) {
		draw_line(i, 191, i, 219, RED);
	}

	//draw the player A left leg
	for (int i = 43; i <= 54; i++) {
		draw_line(i, 221, i, 237, RED);
	}


	//draw the player A right leg
	for (int i = 56; i <= 67; i++) {
		draw_line(i, 221, i, 237, RED);
	}

	//draw the player A left arm
	for (int i = 33; i <= 40; i++) {

		draw_line(i, 197, i, 215, RED);
	}

	//draw the player A right arm
	for (int i = 70; i <= 77; i++) {
		draw_line(i, 197, i, 215, RED);
	}
}

void draw_p2_hand_down() {
	//draw the player B head
	for (int i = 260; i <= 270; i++) {
		draw_line(i, 176, i, 189, BLUE);
	}

	//draw the player B body 
	for (int i = 253; i <= 277; i++) {
		draw_line(i, 191, i, 219, BLUE);
	}

	//draw the player B left leg
	for (int i = 253; i <= 264; i++) {
		draw_line(i, 221, i, 237, BLUE);
	}


	//draw the player B right leg
	for (int i = 266; i <= 277; i++) {
		draw_line(i, 221, i, 237, BLUE);
	}

	//draw the player B right arm
	for (int i = 280; i <= 287; i++) {

		draw_line(i, 197, i, 215, BLUE);
	}

	//draw the player B left arm
	for (int i = 243; i <= 250; i++) {
		draw_line(i, 197, i, 215, BLUE);
	}
}

void erase_p1_hand_down() {
	//draw the player A head
	for (int i = 50; i <= 60; i++) {
		draw_line(i, 176, i, 189, 0x0000);
	}

	//draw the player A body 
	for (int i = 43; i <= 67; i++) {
		draw_line(i, 191, i, 219, 0x0000);
	}

	//draw the player A left leg
	for (int i = 43; i <= 54; i++) {
		draw_line(i, 221, i, 237, 0x0000);
	}


	//draw the player A right leg
	for (int i = 56; i <= 67; i++) {
		draw_line(i, 221, i, 237, 0x0000);
	}

	//draw the player A left arm
	for (int i = 33; i <= 40; i++) {

		draw_line(i, 197, i, 215, 0x0000);
	}

	//draw the player A right arm
	for (int i = 70; i <= 77; i++) {
		draw_line(i, 197, i, 215, 0x0000);
	}
}


void erase_p2_hand_down() {
	//draw the player B head
	for (int i = 260; i <= 270; i++) {
		draw_line(i, 176, i, 189, 0x0000);
	}

	//draw the player B body 
	for (int i = 253; i <= 277; i++) {
		draw_line(i, 191, i, 219, 0x0000);
	}

	//draw the player B left leg
	for (int i = 253; i <= 264; i++) {
		draw_line(i, 221, i, 237, 0x0000);
	}


	//draw the player B right leg
	for (int i = 266; i <= 277; i++) {
		draw_line(i, 221, i, 237, 0x0000);
	}

	//draw the player B right arm
	for (int i = 280; i <= 287; i++) {

		draw_line(i, 197, i, 215, 0x0000);
	}

	//draw the player B left arm
	for (int i = 243; i <= 250; i++) {
		draw_line(i, 197, i, 215, 0x0000);
	}
}

//draw the start prompt
//clear after the game start
void draw_start_prompt(short int color_author, short int color_prompt, short int color_title) {
	//draw name of author
	draw_word_component(155, 215, 165, 225, color_author);
	draw_word_component(155, 225, 165, 215, color_author);

	//draw ZLT								
	draw_word_component(110, 210, 120, 210, color_author);
	draw_word_component(120, 210, 110, 230, color_author);
	draw_word_component(110, 230, 120, 230, color_author);

	draw_word_component(125, 210, 125, 230, color_author);
	draw_word_component(125, 230, 135, 230, color_author);

	draw_word_component(135, 210, 145, 210, color_author);
	draw_word_component(140, 210, 140, 230, color_author);

	//draw CHC								
	draw_word_component(175, 210, 185, 210, color_author);
	draw_word_component(175, 210, 175, 230, color_author);
	draw_word_component(175, 230, 185, 230, color_author);

	draw_word_component(190, 210, 190, 230, color_author);
	draw_word_component(190, 220, 200, 220, color_author);
	draw_word_component(200, 210, 200, 230, color_author);

	draw_word_component(205, 210, 215, 210, color_author);
	draw_word_component(205, 210, 205, 230, color_author);
	draw_word_component(205, 230, 215, 230, color_author);

	//draw the prompt
	//press
	draw_word_component(80, 120, 90, 120, color_prompt);
	draw_word_component(80, 120, 80, 140, color_prompt);
	draw_word_component(90, 120, 90, 130, color_prompt);
	draw_word_component(80, 130, 90, 130, color_prompt);


	draw_word_component(95, 130, 95, 140, color_prompt);
	draw_word_component(95, 135, 100, 130, color_prompt);


	draw_word_component(105, 130, 115, 130, color_prompt);
	draw_word_component(105, 135, 115, 135, color_prompt);
	draw_word_component(105, 140, 115, 140, color_prompt);
	draw_word_component(105, 130, 105, 140, color_prompt);
	draw_word_component(115, 130, 115, 135, color_prompt);

	draw_word_component(120, 130, 130, 130, color_prompt);
	draw_word_component(120, 135, 130, 135, color_prompt);
	draw_word_component(120, 140, 130, 140, color_prompt);
	draw_word_component(120, 130, 120, 135, color_prompt);
	draw_word_component(130, 135, 130, 140, color_prompt);

	draw_word_component(135, 130, 145, 130, color_prompt);
	draw_word_component(135, 135, 145, 135, color_prompt);
	draw_word_component(135, 140, 145, 140, color_prompt);
	draw_word_component(135, 130, 135, 135, color_prompt);
	draw_word_component(145, 135, 145, 140, color_prompt);

	//space
	draw_word_component(155, 120, 165, 120, color_prompt);
	draw_word_component(155, 120, 155, 130, color_prompt);
	draw_word_component(155, 130, 165, 130, color_prompt);
	draw_word_component(165, 130, 165, 140, color_prompt);
	draw_word_component(155, 140, 165, 140, color_prompt);

	draw_word_component(170, 120, 170, 140, color_prompt);
	draw_word_component(170, 120, 180, 120, color_prompt);
	draw_word_component(180, 120, 180, 130, color_prompt);
	draw_word_component(170, 130, 180, 130, color_prompt);

	draw_word_component(185, 120, 185, 140, color_prompt);
	draw_word_component(185, 120, 195, 120, color_prompt);
	draw_word_component(185, 130, 195, 130, color_prompt);
	draw_word_component(195, 120, 195, 140, color_prompt);

	draw_word_component(200, 120, 200, 140, color_prompt);
	draw_word_component(200, 120, 210, 120, color_prompt);
	draw_word_component(200, 140, 210, 140, color_prompt);

	draw_word_component(215, 120, 225, 120, color_prompt);
	draw_word_component(215, 120, 215, 140, color_prompt);
	draw_word_component(215, 130, 225, 130, color_prompt);
	draw_word_component(215, 140, 225, 140, color_prompt);

	//to
	draw_word_component(115, 150, 115, 170, color_prompt);
	draw_word_component(110, 160, 120, 160, color_prompt);
	draw_word_component(115, 170, 120, 170, color_prompt);

	draw_word_component(125, 160, 135, 160, color_prompt);
	draw_word_component(125, 170, 135, 170, color_prompt);
	draw_word_component(125, 160, 125, 170, color_prompt);
	draw_word_component(135, 160, 135, 170, color_prompt);

	//throw
	draw_word_component(150, 150, 150, 170, color_prompt);
	draw_word_component(145, 160, 155, 160, color_prompt);
	draw_word_component(150, 170, 155, 170, color_prompt);

	draw_word_component(160, 150, 160, 170, color_prompt);
	draw_word_component(160, 160, 170, 160, color_prompt);
	draw_word_component(170, 160, 170, 170, color_prompt);

	draw_word_component(175, 160, 175, 170, color_prompt);
	draw_word_component(175, 165, 180, 160, color_prompt);

	draw_word_component(185, 160, 195, 160, color_prompt);
	draw_word_component(185, 170, 195, 170, color_prompt);
	draw_word_component(185, 160, 185, 170, color_prompt);
	draw_word_component(195, 160, 195, 170, color_prompt);

	draw_word_component(200, 160, 200, 170, color_prompt);
	draw_word_component(200, 170, 205, 160, color_prompt);
	draw_word_component(205, 160, 210, 170, color_prompt);
	draw_word_component(210, 160, 210, 170, color_prompt);


	//THROW it - title
	draw_title_component(20, 20, 50, 20, color_title);
	draw_title_component(35, 20, 35, 60, color_title);

	draw_title_component(60, 20, 60, 60, color_title);
	draw_title_component(90, 20, 90, 60, color_title);
	draw_title_component(60, 40, 90, 40, color_title);

	draw_title_component(110, 20, 140, 20, color_title);
	draw_title_component(110, 40, 140, 40, color_title);
	draw_title_component(110, 20, 110, 60, color_title);
	draw_title_component(140, 20, 140, 40, color_title);
	draw_title_component(110, 40, 140, 60, color_title);
	draw_title_component(113, 40, 143, 60, color_title);


	draw_title_component(160, 20, 190, 20, color_title);
	draw_title_component(160, 60, 190, 60, color_title);
	draw_title_component(160, 20, 160, 60, color_title);
	draw_title_component(190, 20, 190, 60, color_title);

	draw_title_component(200, 20, 210, 60, color_title);
	draw_title_component(210, 60, 220, 30, color_title);
	draw_title_component(220, 30, 230, 60, color_title);
	draw_title_component(230, 60, 240, 20, color_title);

	draw_stone(270, 30, color_author);
	draw_title_component(270, 40, 270, 60, color_title);

	draw_title_component(280, 40, 300, 40, color_title);
	draw_title_component(290, 30, 290, 60, color_title);
	draw_title_component(290, 60, 300, 60, color_title);

}


void draw_end_prompt(short int color) {

	//draw the prompt
	//press
	draw_word_component(80, 120, 90, 120, color);
	draw_word_component(80, 120, 80, 140, color);
	draw_word_component(90, 120, 90, 130, color);
	draw_word_component(80, 130, 90, 130, color);


	draw_word_component(95, 130, 95, 140, color);
	draw_word_component(95, 135, 100, 130, color);


	draw_word_component(105, 130, 115, 130, color);
	draw_word_component(105, 135, 115, 135, color);
	draw_word_component(105, 140, 115, 140, color);
	draw_word_component(105, 130, 105, 140, color);
	draw_word_component(115, 130, 115, 135, color);

	draw_word_component(120, 130, 130, 130, color);
	draw_word_component(120, 135, 130, 135, color);
	draw_word_component(120, 140, 130, 140, color);
	draw_word_component(120, 130, 120, 135, color);
	draw_word_component(130, 135, 130, 140, color);

	draw_word_component(135, 130, 145, 130, color);
	draw_word_component(135, 135, 145, 135, color);
	draw_word_component(135, 140, 145, 140, color);
	draw_word_component(135, 130, 135, 135, color);
	draw_word_component(145, 135, 145, 140, color);

	//space
	draw_word_component(155, 120, 165, 120, color);
	draw_word_component(155, 120, 155, 130, color);
	draw_word_component(155, 130, 165, 130, color);
	draw_word_component(165, 130, 165, 140, color);
	draw_word_component(155, 140, 165, 140, color);

	draw_word_component(170, 120, 170, 140, color);
	draw_word_component(170, 120, 180, 120, color);
	draw_word_component(180, 120, 180, 130, color);
	draw_word_component(170, 130, 180, 130, color);

	draw_word_component(185, 120, 185, 140, color);
	draw_word_component(185, 120, 195, 120, color);
	draw_word_component(185, 130, 195, 130, color);
	draw_word_component(195, 120, 195, 140, color);

	draw_word_component(200, 120, 200, 140, color);
	draw_word_component(200, 120, 210, 120, color);
	draw_word_component(200, 140, 210, 140, color);

	draw_word_component(215, 120, 225, 120, color);
	draw_word_component(215, 120, 215, 140, color);
	draw_word_component(215, 130, 225, 130, color);
	draw_word_component(215, 140, 225, 140, color);

	//to
	draw_word_component(95, 150, 95, 170, color);
	draw_word_component(90, 160, 100, 160, color);
	draw_word_component(95, 170, 100, 170, color);

	draw_word_component(105, 160, 115, 160, color);
	draw_word_component(105, 170, 115, 170, color);
	draw_word_component(105, 160, 105, 170, color);
	draw_word_component(115, 160, 115, 170, color);

	//restart
	draw_word_component(125, 160, 125, 170, color);
	draw_word_component(125, 165, 130, 160, color);

	draw_word_component(135, 160, 145, 160, color);
	draw_word_component(135, 165, 145, 165, color);
	draw_word_component(135, 170, 145, 170, color);
	draw_word_component(135, 160, 135, 170, color);
	draw_word_component(145, 160, 145, 165, color);

	draw_word_component(150, 160, 160, 160, color);
	draw_word_component(150, 165, 160, 165, color);
	draw_word_component(150, 170, 160, 170, color);
	draw_word_component(150, 160, 150, 165, color);
	draw_word_component(160, 165, 160, 170, color);

	draw_word_component(165, 160, 175, 160, color);
	draw_word_component(170, 150, 170, 170, color);
	draw_word_component(170, 170, 175, 170, color);

	draw_word_component(180, 160, 190, 160, color);
	draw_word_component(180, 170, 195, 170, color);
	draw_word_component(180, 160, 180, 170, color);
	draw_word_component(190, 160, 190, 170, color);

	draw_word_component(200, 160, 200, 170, color);
	draw_word_component(200, 165, 205, 160, color);

	draw_word_component(210, 160, 220, 160, color);
	draw_word_component(215, 150, 215, 170, color);
	draw_word_component(215, 170, 220, 170, color);
}


void draw_p1_win(short int color) {

	//draw P

	draw_word_component(70, 70, 70, 110, color);
	draw_word_component(70, 70, 90, 70, color);
	draw_word_component(70, 90, 90, 90, color);
	draw_word_component(90, 70, 90, 90, color);


	//draw 1
	draw_word_component(100, 80, 110, 70, color);
	draw_word_component(110, 70, 110, 110, color);
	draw_word_component(100, 110, 120, 110, color);


	//draw W
	draw_word_component(140, 70, 150, 110, color);
	draw_word_component(150, 110, 160, 70, color);
	draw_word_component(160, 70, 170, 110, color);
	draw_word_component(170, 110, 180, 70, color);


	//draw I

	draw_word_component(200, 70, 200, 110, color);

	//draw N
	draw_word_component(220, 70, 220, 110, color);
	draw_word_component(220, 70, 240, 110, color);
	draw_word_component(240, 70, 240, 110, color);

}

void draw_p2_win(short int color) {

	//draw P
	draw_word_component(70, 70, 70, 110, color);
	draw_word_component(70, 70, 90, 70, color);
	draw_word_component(70, 90, 90, 90, color);
	draw_word_component(90, 70, 90, 90, color);


	//draw 2
	draw_word_component(100, 70, 120, 70, color);
	draw_word_component(100, 90, 120, 90, color);
	draw_word_component(100, 110, 120, 110, color);
	draw_word_component(120, 70, 120, 90, color);
	draw_word_component(100, 90, 100, 110, color);



	//draw W
	draw_word_component(140, 70, 150, 110, color);
	draw_word_component(150, 110, 160, 70, color);
	draw_word_component(160, 70, 170, 110, color);
	draw_word_component(170, 110, 180, 70, color);


	//draw I

	draw_word_component(200, 70, 200, 110, color);

	//draw N
	draw_word_component(220, 70, 220, 110, color);
	draw_word_component(220, 70, 240, 110, color);
	draw_word_component(240, 70, 240, 110, color);
}



void draw_win_player(short int color) {

	//draw the player head
	for (int i = 155; i <= 165; i++) {
		draw_line(i, 176, i, 189, color);
	}

	//draw the player body 
	for (int i = 148; i <= 172; i++) {
		draw_line(i, 191, i, 219, color);
	}

	//draw the player left leg
	for (int i = 148; i <= 159; i++) {
		draw_line(i, 221, i, 237, color);
	}


	//draw the player right leg
	for (int i = 161; i <= 172; i++) {
		draw_line(i, 221, i, 237, color);
	}

	//draw the player right arm
	for (int i = 175; i <= 182; i++) {

		draw_line(i, 197, i, 215, color);
	}

	//draw the player left arm
	for (int i = 138; i <= 145; i++) {
		draw_line(i, 197, i, 215, color);
	}

}


//draw the wind based on the current wind
void draw_wind(int wind, short int color) {
	if (wind > 10 || wind < -10) return;
	else if (wind < 0) {
		for (int i = 160 + wind * 4; i <= 160; i++) {
			draw_line(i, 30, i, 50, color);
		}
	}
	else {
		for (int i = 160; i <= 160 + wind * 4; i++) {
			draw_line(i, 30, i, 50, color);
		}

	}

}



//reset the wind box
void reset_draw_wind() {
	for (int i = 120; i <= 200; i++) {
		draw_line(i, 30, i, 50, 0x0000);
	}
}


void draw_ball_curve_user1(int x_speed, int y_speed) {
	int flying_time = ball_to_xbound / x_speed;//get how many lines we should draw
	int current_x_position = left_x;
	int current_y_position = left_y;
	int next_x_position = 0;
	int next_y_position = 0;
	draw_stone(current_x_position, current_y_position, YELLOW);

	for (int i = 1; i <= flying_time; i++) {
		next_x_position = left_x + x_speed * i;//next x position
		next_y_position = left_y + (-y_speed * i + 5 * i * i);//next y position
		if (next_x_position > 319 || next_y_position > 239) break;
		draw_line(current_x_position, current_y_position, next_x_position, next_y_position, 0x4A69);//draw line between x and y position
		current_x_position = next_x_position;//increment current x position
		current_y_position = next_y_position;//increment current y position
		draw_stone(current_x_position, current_y_position, YELLOW);
	}

	if (current_x_position >= 237 && current_x_position <= 275 && current_y_position >= 173 && current_y_position <= 189) {
		user2_life--;
		user2_life--;
	}
	else if (current_x_position >= 232 && current_x_position <= 295 && current_y_position >= 189 && current_y_position <= 215) {
		user2_life--;
		user2_life--;
	}
	else if (current_x_position >= 247 && current_x_position <= 307 && current_y_position >= 215) {
		user2_life--;
		user2_life--;
	}
}

void draw_ball_curve_user2(int x_speed, int y_speed) {
	int flying_time = 280 / x_speed;//get how many lines we should draw
	int current_x_position = right_x;
	int current_y_position = right_y;
	int next_x_position = 0;
	int next_y_position = 0;
	draw_stone(current_x_position, current_y_position, YELLOW);

	for (int i = 1; i <= flying_time; i++) {
		next_x_position = right_x - x_speed * i;//next x position
		next_y_position = right_y + (-y_speed * i + 5 * i * i);//next y position
		if (next_x_position < 0 || next_y_position>239) break;
		draw_line(current_x_position, current_y_position, next_x_position, next_y_position, 0x4A69);//draw line between x and y position
		current_x_position = next_x_position;//increment current x position
		current_y_position = next_y_position;//increment current y position
		draw_stone(current_x_position, current_y_position, YELLOW);
	}

	if (current_x_position >= 45 && current_x_position <= 88 && current_y_position >= 173 && current_y_position <= 189) {
		user1_life--;
		user1_life--;
	}
	else if (current_x_position >= 25 && current_x_position <= 83 && current_y_position >= 189 && current_y_position <= 215) {
		user1_life--;
		user1_life--;

	}
	else if (current_x_position >= 11 && current_x_position <= 73 && current_y_position >= 215) {
		user1_life--;
		user1_life--;
	}

}

void draw_p1_life(int life, short int color) {
	if (life == 0 || life > 10) {
		for (int i = 20; i <= 80; i++) {
			draw_line(i, 40, i, 50, 0x0000);
		}
	}
	else {

		for (int i = 20; i <= 20 + life * 6; i++) {
			draw_line(i, 40, i, 50, color);
		}
	}

}



void draw_p2_life(int life, short int color) {
	if (life == 0 || life > 10) {
		for (int i = 240; i <= 300; i++) {
			draw_line(i, 40, i, 50, 0x0000);
		}
	}
	else {
		for (int i = 240; i <= 240 + life * 6; i++) {
			draw_line(i, 40, i, 50, color);
		}
	}
}