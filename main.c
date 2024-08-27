/*
Author: Eoin Keogh (C22456452) & Niamh Flynn (C22388461)
Date Finished: 17/11/2023
Description: The game features three modes: Medium, Hard, and Ducks. In the Medium and Hard modes, the player
must navigate the duck character to avoid bubbles, with varying speeds and difficulty levels. In the Ducks mode,
the player can choose between two duck characters, "OG Duckie" and "Frankie."
The code structure includes initialization functions, game setup, main game loop, and menu handling.
The player can select game modes and duck characters using button inputs. The game keeps track of the player's score and top score.
 */

#include <stm32f031x6.h>
#include <stdbool.h>
#include "display.h"
#include "sound.h"
#include "musical_notes.h"
#include <stdlib.h>

// Define the maximum number of circles
#define MAX_CIRCLES 5

// Define the structure for a circle
typedef struct
{
	int x;	   // X-coordinate of the circle
	int y;	   // Y-coordinate of the circle
	int speed; // Vertical speed of the circle
} Circle;

// Array to store circles
Circle circles[MAX_CIRCLES];

// Function prototypes
void initClock(void);
void initSysTick(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO();
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);
void displayCountdown(void);
void handleDuckMovement(uint16_t *x, uint16_t *y, uint16_t *oldx, uint16_t *oldy);
int isOverlapping(Circle circles[], int currentIndex, int occupiedXPositions[]);

// Global variables
int gamescore = 0;
int topscore = 0;
int selectedGame; // Declare a global variable to store the selected game
int selectedDuck = 0;

volatile uint32_t milliseconds;

// Duck images represented as arrays of pixel values
const uint16_t ogduck[] ={ 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 0, 0, 0, 0, 0, 50364, 50364, 50364, 50364, 50364, 50364, 0, 32527, 32527, 32527, 32527, 32527, 0, 50364, 50364, 50364, 50364, 0, 32527, 32527, 32527, 32527, 32527, 32527, 32527, 0, 50364, 0, 50364, 0, 32527, 32527, 32527, 32527, 0, 0, 32527, 0, 50364, 0, 50364, 0, 32527, 32527, 32527, 32527, 0, 65535, 32527, 16155, 16155, 0, 0, 32527, 32527, 32527, 32527, 32527, 32527, 32527, 32527, 16155, 50364, 0, 65301, 32527, 32527, 32527, 32527, 32527, 32527, 32527, 0, 50364, 50364, 0, 65301, 32527, 32527, 32527, 32527, 32527, 32527, 32527, 0, 50364, 50364, 0, 65301, 32527, 32527, 32527, 32527, 32527, 65301, 65301, 0, 50364, 50364, 50364, 0, 0, 65301, 65301, 65301, 65301, 0, 0, 50364, 50364, 50364, 50364, 50364, 50364, 0, 0, 0, 0, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364};

const uint16_t frankduck[] ={ 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 0, 0, 0, 0, 0, 50364, 50364, 50364, 50364, 50364, 50364, 0, 65313, 65313, 65313, 65313, 65313, 0, 50364, 50364, 50364, 50364, 0, 65313, 65313, 65313, 65313, 65313, 65313, 65313, 0, 50364, 0, 50364, 0, 65313, 65313, 65313, 65313, 0, 0, 65313, 0, 50364, 0, 50364, 0, 65313, 65313, 65313, 65313, 0, 65535, 65313, 16155, 16155, 0, 0, 65313, 65313, 65313, 65313, 65313, 65313, 65313, 65313, 16155, 50364, 0, 30745, 65313, 65313, 65313, 65313, 65313, 65313, 65313, 0, 50364, 50364, 0, 30745, 65313, 65313, 65313, 65313, 65313, 65313, 65313, 0, 50364, 50364, 0, 30745, 65313, 65313, 65313, 65313, 65313, 30745, 30745, 0, 50364, 50364, 50364, 0, 0, 30745, 30745, 30745, 30745, 0, 0, 50364, 50364, 50364, 50364, 50364, 50364, 0, 0, 0, 0, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364};

const uint16_t bubble[] ={ 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 13294, 13294, 13294, 50364, 50364, 65535, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 50364, 50364, 13294, 13294, 13294, 50364, 65535, 65535, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 50364, 65535, 65535, 50364, 50364, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 50364, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 50364, 13294, 13294, 13294, 13294, 13294, 50364, 50364, 50364, 50364, 50364};

// Pointer to the current duck image
const uint16_t *duck = ogduck;

// Function to draw circles on the display
void drawCircles(int x, int y)
{
	// Loop through all circles and draw them on the display
	for (int i = 0; i < MAX_CIRCLES; i++)
	{
		putImage(circles[i].x, circles[i].y, 15, 15, bubble, 0, 0);
	}
}

// Function to display the current game score
void game_score()
{
	// Display the label "Score:"
	printText("Score:", 25, 5, RGBToWord(255, 255, 255), 253.2);

	// Display the current game score
	printNumber(gamescore, 75, 5, RGBToWord(255, 255, 255), 253.2);
}

// Function to display the menu and handle user input
int doMenu(char *options[], int len, int x, int y)
{
	int done = 0;	 // Flag to determine when the menu is complete
	int current = 0; // Index of the currently selected option

	// Display the game title and instructions
	fillRectangle(0, 0, 126, 158, RGBToWord(37, 150, 190));
	printText("Duck Dash", 30, 5, RGBToWord(255, 255, 255), 253.2);
	printText("Avoid the bubbles", 5, 130, RGBToWord(255, 255, 255), 253.2);

	// Display bubbles on the screen for the menu
	putImage(20, 20, 15, 15, bubble, 0, 0);
	putImage(70, 40, 15, 15, bubble, 0, 0);
	putImage(20, 70, 15, 15, bubble, 0, 0);
	putImage(10, 110, 15, 15, bubble, 0, 0);
	putImage(90, 90, 15, 15, bubble, 0, 0);
	putImage(40, 95, 15, 15, bubble, 0, 0);

	// Display the label "TOP SCORE:" and the current top score
	printText("TOP SCORE:", 10, 140, RGBToWord(255, 255, 255), 253.2);
	printNumber(topscore, 80, 140, RGBToWord(255, 255, 255), 253.2);

	// Menu loop
	while (!done)
	{
		// Display menu options
		for (int i = 0; i < len; i++)
		{
			if (i == current)
				// Highlight the currently selected option
				printText(options[i], x, y + (i * 9), RGBToWord(15, 15, 255), 253.2);
			else
				printText(options[i], x, y + (i * 9), RGBToWord(255, 255, 255), 253.2);
		}

		// Check user input
		if ((GPIOA->IDR & (1 << 8)) == 0) // UP button pressed
		{
			playNote(AS6_Bb6);
			current = current - 1;
			if (current < 0)
				current = len - 1;
		}
		if ((GPIOA->IDR & (1 << 11)) == 0) // DOWN button pressed
		{
			playNote(AS6_Bb6);
			current = current + 1;
			if (current >= len)
				current = 0;
		}
		if ((GPIOB->IDR & (1 << 4)) == 0) // RIGHT button pressed
		{
			done = 1;
		}
		if ((GPIOB->IDR & (1 << 5)) == 0) // LEFT button pressed
		{
			done = 1;
		}

		// Add a delay to debounce the buttons
		delay(100);
		playNote(0);

		// Store the selected game option
		selectedGame = current;
	}

	// Return the index of the selected option
	return current;
}
// Array of character menu options
const char *menu[] = {
	"Medium",
	"Hard",
	"Ducks"};

// Function to display the character menu and handle user input
int Charactermenu(char *ducks[], int len, int x, int y)
{
	int done = 0;	 // Flag to determine when the menu is complete
	int current = 0; // Index of the currently selected option

	// Display background and title for the character menu
	fillRectangle(0, 0, 126, 158, RGBToWord(37, 150, 190));
	printText("Choose your duck", 5, 5, RGBToWord(255, 255, 255), 253.2);

	// Display duck images for character selection
	putImage(96, 55, 12, 16, ogduck, 0, 0);
	putImage(83, 65, 12, 16, frankduck, 0, 0);

	// Display bubbles on the screen for the character menu
	putImage(20, 20, 15, 15, bubble, 0, 0);
	putImage(70, 40, 15, 15, bubble, 0, 0);
	putImage(20, 70, 15, 15, bubble, 0, 0);
	putImage(10, 110, 15, 15, bubble, 0, 0);
	putImage(90, 90, 15, 15, bubble, 0, 0);
	putImage(40, 95, 15, 15, bubble, 0, 0);

	// Menu loop
	while (!done)
	{
		// Display menu options
		for (int i = 0; i < len; i++)
		{
			if (i == current)
				// Highlight the currently selected option
				printText(ducks[i], x, y + (i * 9), RGBToWord(15, 15, 255), 253.2);
			else
				printText(ducks[i], x, y + (i * 9), RGBToWord(255, 255, 255), 253.2);
		}

		// Check user input
		if ((GPIOA->IDR & (1 << 8)) == 0) // UP button pressed
		{
			playNote(AS6_Bb6);
			current = current - 1;
			if (current < 0)
				current = len - 1;
		}
		if ((GPIOA->IDR & (1 << 11)) == 0) // DOWN button pressed
		{
			playNote(AS6_Bb6);
			current = current + 1;
			if (current >= len)
				current = 0;
		}
		if ((GPIOB->IDR & (1 << 4)) == 0) // RIGHT button pressed
		{
			done = 1;
		}
		if ((GPIOB->IDR & (1 << 5)) == 0) // LEFT button pressed
		{
			done = 1;
		}

		// Add a delay to debounce the buttons
		delay(100);
		playNote(0);

		// Store the selected duck option
		selectedDuck = current;
	}

	// Return the index of the selected option
	return current;
}

// Array of duck menu options
const char *duckmenu[] = {
	"OG Duckie",
	"Frankie"};

// Function to display game over screen
void gameover()
{
	// Display background and game over message
	fillRectangle(0, 0, 126, 158, RGBToWord(37, 150, 190));
	printTextX2("GAME OVER", 8, 50, RGBToWord(255, 255, 255), 253.2);

	// Display the final score
	printText("Score:", 25, 80, RGBToWord(255, 255, 255), 253.2);
	printNumber(gamescore, 75, 80, RGBToWord(255, 255, 255), 253.2);

	// Reset the game score and add a delay before returning
	gamescore = 0;
	delay(2000);
}
int main()
{
	int hinverted = 0;						   // Flag to indicate horizontal inversion of the duck image
	int vinverted = 0;						   // Flag to indicate vertical inversion of the duck image
	int toggle = 0;							   // Toggle flag for image animation
	int hmoved = 0;							   // Flag to indicate horizontal movement of the duck
	int vmoved = 0;							   // Flag to indicate vertical movement of the duck
	bool gamerunning = true;				   // Flag to control the main game loop
	int occupiedXPositions[MAX_CIRCLES] = {0}; // Array to store occupied X positions for circle generation

	while (1)
	{
		uint16_t x = 50;   // Initial X-coordinate of the duck
		uint16_t y = 100;  // Initial Y-coordinate of the duck
		uint16_t oldx = x; // Previous X-coordinate of the duck
		uint16_t oldy = y; // Previous Y-coordinate of the duck

		// Initialize clock, SysTick, GPIO, and sound
		initClock();
		initSysTick();
		setupIO();
		initSound();

	game_ends:
		// Display the main menu and get the selected game
		doMenu(menu, 3, 40, 60);
		// Clear the screen
		fillRectangle(0, 0, 126, 158, RGBToWord(37, 150, 190));

		// Check the selected game and initialize the circles accordingly
		if (selectedGame == 0)
		{
			// Set initial positions for bubbles
			circles[0].y = 0;
			circles[1].y = -10;
			circles[2].y = -56;
			circles[3].y = -40;
			circles[4].y = 104;

			// Display countdown and clear the screen
			displayCountdown();
			fillRectangle(0, 0, 126, 158, RGBToWord(37, 150, 190));

			// Main game loop
			while (gamerunning)
			{
				// Update positions and draw circles
				for (int i = 0; i < MAX_CIRCLES; i++)
				{
					circles[i].speed = 2; // Adjust the speed as needed

					// Check if the bubble has gone off the screen
					if (circles[i].y > 159)
					{
						// Update game score, reset X position for a new bubble
						gamescore++;
						do
						{
							circles[i].x = rand() % 110; // Change the range as needed
						} while (isOverlapping(circles, i, occupiedXPositions));

						// Update occupied X positions and reset Y position
						occupiedXPositions[i] = circles[i].x;
						circles[i].y = 0;
					}

					// Update top score
					if (topscore < gamescore)
					{
						topscore = gamescore;
					}

					// Draw the circle at its current position
					fillRectangle(circles[i].x, circles[i].y, 15, 15, (RGBToWord(37, 150, 190)));
					circles[i].y += circles[i].speed;
					drawCircles(circles[i].x, circles[i].y);
					game_score();

					// Check for collision with the duck
					if (isInside(circles[i].x, circles[i].y, 15, 15, x, y))
					{
						// Collision occurred, player is killed
						// Handle the game over logic here
						gameover();
						goto game_ends;
					}
				}

				// Handle duck movement
				hmoved = vmoved = 0;
				hinverted = vinverted = 0;

				// Check button presses for duck movement
				if ((GPIOB->IDR & (1 << 4)) == 0) // right pressed
				{
					if (x < 110)
					{
						x = x + 1;
						hmoved = 1;
						hinverted = 0;
					}
				}

				if ((GPIOB->IDR & (1 << 5)) == 0) // left pressed
				{
					if (x > 10)
					{
						x = x - 1;
						hmoved = 1;
						vinverted = 1;
					}
				}

				if ((GPIOA->IDR & (1 << 11)) == 0) // down pressed
				{
					if (y < 140)
					{
						y = y + 1;
						vmoved = 1;
						vinverted = 0;
					}
				}

				if ((GPIOA->IDR & (1 << 8)) == 0) // up pressed
				{
					if (y > 16)
					{
						y = y - 1;
						vmoved = 1;
						vinverted = 1;
					}
				}

				// Redraw duck if there has been movement
				if ((vmoved) || (hmoved))
				{
					fillRectangle(oldx, oldy, 12, 16, RGBToWord(37, 150, 190));
					oldx = x;
					oldy = y;

					if (hmoved)
					{
						// Redraw duck with animation toggle
						if (toggle)
							putImage(x, y, 12, 16, duck, hinverted, 0);
						else
							putImage(x, y, 12, 16, duck, hinverted, 0);

						toggle = toggle ^ 1;
					}
					else
					{
						putImage(x, y, 12, 16, duck, 0, hinverted);
					}

					// Now check for an overlap by checking to see if ANY of the 4 corners of deco are within the target area
				}
			}
		}

		// Check if the selected game is the second option
		else if (selectedGame == 1)
		{
			// Set initial positions for bubbles in the second game mode
			circles[0].y = 0;
			circles[1].y = -10;
			circles[2].y = -56;
			circles[3].y = -98;
			circles[4].y = 104;

			// Display countdown and clear the screen
			displayCountdown();
			fillRectangle(0, 0, 126, 158, RGBToWord(37, 150, 190));

			// Main game loop for the second game mode
			while (gamerunning)
			{
				// Update positions and draw circles
				for (int i = 0; i < MAX_CIRCLES; i++)
				{
					// Adjust the speed of circles as needed for the second game mode
					circles[i].speed = 3;

					// Check if the bubble has gone off the screen
					if (circles[i].y > 159)
					{
						// Update game score, reset X position for a new bubble
						gamescore++;
						do
						{
							circles[i].x = rand() % 110; // Change the range as needed
						} while (isOverlapping(circles, i, occupiedXPositions));

						// Update occupied X positions and reset Y position
						occupiedXPositions[i] = circles[i].x;
						circles[i].y = 0;
					}

					// Draw the circle at its current position
					fillRectangle(circles[i].x, circles[i].y, 15, 15, (RGBToWord(37, 150, 190)));
					circles[i].y += circles[i].speed;
					drawCircles(circles[i].x, circles[i].y);
					game_score();

					// Check for collision with the duck
					if (isInside(circles[i].x, circles[i].y, 14, 14, x, y))
					{
						// Collision occurred, player is killed
						// Handle the game over logic here
						gameover();
						goto game_ends;
					}
				}

				// Handle duck movement
				hmoved = vmoved = 0;
				hinverted = vinverted = 0;

				// Check button presses for duck movement
				if ((GPIOB->IDR & (1 << 4)) == 0) // right pressed
				{
					if (x < 110)
					{
						x = x + 1;
						hmoved = 1;
						hinverted = 0;
					}
				}

				if ((GPIOB->IDR & (1 << 5)) == 0) // left pressed
				{
					if (x > 10)
					{
						x = x - 1;
						hmoved = 1;
						vinverted = 1;
					}
				}

				if ((GPIOA->IDR & (1 << 11)) == 0) // down pressed
				{
					if (y < 140)
					{
						y = y + 1;
						vmoved = 1;
						vinverted = 0;
					}
				}

				if ((GPIOA->IDR & (1 << 8)) == 0) // up pressed
				{
					if (y > 16)
					{
						y = y - 1;
						vmoved = 1;
						vinverted = 1;
					}
				}

				// Redraw duck if there has been movement
				if ((vmoved) || (hmoved))
				{
					fillRectangle(oldx, oldy, 12, 16, RGBToWord(37, 150, 190));
					oldx = x;
					oldy = y;

					if (hmoved)
					{
						// Redraw duck with animation toggle
						if (toggle)
							putImage(x, y, 12, 16, duck, hinverted, 0);
						else
							putImage(x, y, 12, 16, duck, hinverted, 0);

						toggle = toggle ^ 1;
					}
					else
					{
						// Redraw duck without animation toggle
						putImage(x, y, 12, 16, duck, 0, hinverted);
					}

					// Now check for an overlap by checking to see if ANY of the 4 corners of deco are within the target area
				}
			}
		}

		// Check if the selected game is the third option
		else if (selectedGame == 2)
		{
			// Game loop for the third option
			while (gamerunning)
			{
				// Clear the screen and display the character menu
				fillRectangle(0, 0, 126, 158, RGBToWord(37, 150, 190));
				Charactermenu(duckmenu, 2, 30, 60);

				// Check the selected duck and set the appropriate duck image
				if (selectedDuck == 0)
				{
					duck = ogduck;
				}
				else if (selectedDuck == 1)
				{
					duck = frankduck;
				}

				// Exit the game loop and go to the game_ends label
				goto game_ends;
			}
		}

		// Delay to control the loop speed
		delay(50);

		// Return 0 to indicate successful completion of the program
		return 0;
	}
}

void initSysTick(void)
{
	SysTick->LOAD = 48000;
	SysTick->CTRL = 7;
	SysTick->VAL = 10;
	__asm(" cpsie i "); // enable interrupts
}
void SysTick_Handler(void)
{
	milliseconds++;
}
void initClock(void)
{
	// This is potentially a dangerous function as it could
	// result in a system with an invalid clock signal - result: a stuck system
	// Set the PLL up
	// First ensure PLL is disabled
	RCC->CR &= ~(1u << 24);
	while ((RCC->CR & (1 << 25)))
		; // wait for PLL ready to be cleared

	// Warning here: if system clock is greater than 24MHz then wait-state(s) need to be
	// inserted into Flash memory interface

	FLASH->ACR |= (1 << 0);
	FLASH->ACR &= ~((1u << 2) | (1u << 1));
	// Turn on FLASH prefetch buffer
	FLASH->ACR |= (1 << 4);
	// set PLL multiplier to 12 (yielding 48MHz)
	RCC->CFGR &= ~((1u << 21) | (1u << 20) | (1u << 19) | (1u << 18));
	RCC->CFGR |= ((1 << 21) | (1 << 19));

	// Need to limit ADC clock to below 14MHz so will change ADC prescaler to 4
	RCC->CFGR |= (1 << 14);

	// and turn the PLL back on again
	RCC->CR |= (1 << 24);
	// set PLL as system clock source
	RCC->CFGR |= (1 << 1);
}
void delay(volatile uint32_t dly)
{
	uint32_t end_time = dly + milliseconds;
	while (milliseconds != end_time)
		__asm(" wfi "); // sleep
}

void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber)
{
	Port->PUPDR = Port->PUPDR & ~(3u << BitNumber * 2); // clear pull-up resistor bits
	Port->PUPDR = Port->PUPDR | (1u << BitNumber * 2);	// set pull-up bit
}
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode)
{
	/*
	 */
	uint32_t mode_value = Port->MODER;
	Mode = Mode << (2 * BitNumber);
	mode_value = mode_value & ~(3u << (BitNumber * 2));
	mode_value = mode_value | Mode;
	Port->MODER = mode_value;
}
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py)
{
	// checks to see if point px,py is within the rectange defined by x,y,w,h
	uint16_t x2, y2;
	x2 = x1 + w;
	y2 = y1 + h;
	int rvalue = 0;
	if ((px >= x1) && (px <= x2))
	{
		// ok, x constraint met
		if ((py >= y1) && (py <= y2))
			rvalue = 1;
	}
	return rvalue;
}

void setupIO()
{
	RCC->AHBENR |= (1 << 18) + (1 << 17); // enable Ports A and B
	display_begin();
	pinMode(GPIOB, 4, 0);
	pinMode(GPIOB, 5, 0);
	pinMode(GPIOA, 8, 0);
	pinMode(GPIOA, 11, 0);
	enablePullUp(GPIOB, 4);
	enablePullUp(GPIOB, 5);
	enablePullUp(GPIOA, 11);
	enablePullUp(GPIOA, 8);
}

int isOverlapping(Circle *circles, int currentIndex, int *occupiedXPositions)
{
	// Iterate through the existing circles up to the currentIndex
	for (int i = 0; i < currentIndex; i++)
	{
		// Get the X-positions of the two circles being compared
		int x1 = occupiedXPositions[i];	  // X-position of the existing circle
		int x2 = circles[currentIndex].x; // X-position of the new circle

		// Adjust the threshold for non-overlapping as needed
		// Check if the distance between the circles is less than the sum of their radii
		if (abs(x1 - x2) < 15) // Assuming bubble width is 15
		{
			return 1; // Overlapping
		}
	}

	// If no overlap is found, return 0
	return 0;
}
/**
 * Displays a countdown on the screen before the game starts.
 */
void displayCountdown()
{
	int x = 50;

	// Clear the screen with a background color
	fillRectangle(0, 0, 126, 158, RGBToWord(37, 150, 190));

	// Display "Get ready!" message
	printText("Get ready!", 20, 60, RGBToWord(255, 255, 255), 253.2);

	// Display "Starting in:" message
	printText("Starting in:", 20, 80, RGBToWord(255, 255, 255), 253.2);

	// Display "3" with a note and wait for 1 second
	printText("3", x, 100, RGBToWord(255, 255, 255), 253.2);
	playNote(AS5_Bb5);
	delay(1000);
	playNote(0); // Stop the note

	// Display "2" with a note and wait for 1 second
	printText("2", x, 100, RGBToWord(255, 255, 255), 253.2);
	playNote(AS6_Bb6);
	delay(1000);
	playNote(0); // Stop the note

	// Display "1" with a note and wait for 1 second
	printText("1", x, 100, RGBToWord(255, 255, 255), 253.2);
	playNote(A6);
	delay(1000);
	playNote(0); // Stop the note
}


void handleDuckMovement(uint16_t *x, uint16_t *y, uint16_t *oldx, uint16_t *oldy)
{
    int hmoved = 0, vmoved = 0;
    int hinverted = 0, vinverted = 0;
		int toggle = 0;		 // Toggle flag for image animation

    // ... (existing duck movement logic)

    // Redraw duck if there has been movement
    if ((vmoved) || (hmoved))
    {
        fillRectangle(*oldx, *oldy, 12, 16, RGBToWord(37, 150, 190));
        *oldx = *x;
        *oldy = *y;

        if (hmoved)
        {
            // Redraw duck with animation toggle
            if (toggle)
                putImage(*x, *y, 12, 16, duck, hinverted, 0);
            else
                putImage(*x, *y, 12, 16, duck, hinverted, 0);

            toggle = toggle ^ 1;
        }
        else
        {
            // Redraw duck without animation toggle
            putImage(*x, *y, 12, 16, duck, 0, hinverted);
        }
    }
}