#include <avr/wdt.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

int buzzer = 12;	// Pin of the active buzzer

// SSD1306 display
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

// Pins for Buttons
int buttonApin = 10;
int buttonBpin = 11;

// Button press status
bool buttonAPressed = false;
bool buttonBPressed = false;

// Button press counter
int buttonACount = 0;
int buttonBCount = 0;

// timer for button inactivity
unsigned long lastButtonPressTime = 0;
bool waitingForInactivity = false;

// Initialize game
int board[3][3] = {
		{ 1, 1, 1 },
	{ 1, 1, 1 },
	{ 1, 1, 1 }
};
	// Initialize the board to all 1s
int player = 2;	// Initialize the player turn 
int moveCount = 0;	// Initialize the moves counter 
int winner = 0;	// Initiatize winner

void humanTurn()
{
	int coord[2] = { 0, 0 };

	int row = 0;
	int col = 0;

	// read coordinates from button press
  readButton(coord); 
	row = coord[0] - 1;
	col = coord[1] - 1;

	if (board[row][col] == 1)
	{
		// Only set the value if the current value is 1
		board[row][col] = 3;	// Set the corresponding element in the board array to 3
		// printBoard();	// print board to console
		displayBoard();	// display board on oled display
		sound(1, 100);	// play the sound
		player = 2;	// Set the player turn to 2 (computer)
		moveCount++;	// Increase move count
		// After move, reset button counter
		buttonACount = 0;
		buttonBCount = 0;
	}
}

void computerTurn()
{
	int i = 0;
	int j = 0;
	int result[2];

	// Find the winning move
	bestMove(board, 25, result);
	if (result[0] != -1 && result[1] != -1)
	{
		int i = result[0];
		int j = result[1];

		computerMove(i, j);
		return;
	}
	else
	{
		// Find the blocking move
		bestMove(board, 9, result);
		if (result[0] != -1 && result[1] != -1)
		{
			int i = result[0];
			int j = result[1];

			computerMove(i, j);
			return;
		}
		else
		{
			// Select a random position
			int i = random(0, 3);
			int j = random(0, 3);

			computerMove(i, j);
			return;
		}
	}
}

void computerMove(int i, int j)
{
	if (board[i][j] == 1)
	{
		// Only set the value if the current value is 1
		board[i][j] = 5;	// Set the corresponding element in the board array to 5 (computer)  
		delay(500);	// Add a small delay 
		displayBoard();	// display board on oled display
		sound(2, 100);	// play the sound
		player = 1;	// Set the player turn to 1 (human)
		moveCount++;	// Increase move count
	}
}

void bestMove(int values[][3], int check, int *best_move)
{
	int row_product[3] = { 1, 1, 1 };
	int col_product[3] = { 1, 1, 1 };
	int diagonal1_product = 1;
	int diagonal2_product = 1;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			row_product[i] *= values[i][j];
			col_product[j] *= values[i][j];
			if (i == j)
			{
				diagonal1_product *= values[i][j];
			}

			if (i + j == 2)
			{
				diagonal2_product *= values[i][j];
			}
		}
	}

	if (check == row_product[0] || check == row_product[1] || check == row_product[2])
	{
		int row;
		for (int i = 0; i < 3; i++)
		{
			if (row_product[i] == check)
			{
				row = i;
				break;
			}
		}

		for (int col = 0; col < 3; col++)
		{
			if (values[row][col] == 1)
			{
				best_move[0] = row;
				best_move[1] = col;
				return;
			}
		}
	}
	else if (check == col_product[0] || check == col_product[1] || check == col_product[2])
	{
		int col;
		for (int j = 0; j < 3; j++)
		{
			if (col_product[j] == check)
			{
				col = j;
				break;
			}
		}

		for (int row = 0; row < 3; row++)
		{
			if (values[row][col] == 1)
			{
				best_move[0] = row;
				best_move[1] = col;
				return;
			}
		}
	}
	else if (check == diagonal1_product)
	{
		for (int i = 0; i < 3; i++)
		{
			if (values[i][i] == 1)
			{
				best_move[0] = i;
				best_move[1] = i;
				return;
			}
		}
	}
	else if (check == diagonal2_product)
	{
		for (int i = 0; i < 3; i++)
		{
			if (values[i][2 - i] == 1)
			{
				best_move[0] = i;
				best_move[1] = 2 - i;
				return;
			}
		}
	}

	best_move[0] = -1;
	best_move[1] = -1;
}

void displayBoard()
{
	// print on OLED display
	oled.clear();
	oled.set1X();
	oled.println("TRISDUINO\n");
	oled.set1X();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (board[i][j] == 1)
			{
				oled.print(" ");
			}
			else if (board[i][j] == 3)
			{
				oled.print("x");
			}
			else if (board[i][j] == 5)
			{
				oled.print("o");
			}

			if (j < 2)
			{
				// Add column separator except for the last column
				oled.print("|");
			}
		}

		if (i < 2)
		{
			// Add row separator except for the last row
			oled.println("\n-+-+-");	// Print the row separator
		}
	}
}

int checkWinner()
{
	for (int i = 0; i < 3; i++)
	{
		if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != 1)
		{
			return board[i][0];
		}

		if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != 1)
		{
			return board[0][i];
		}
	}

	if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != 1)
	{
		return board[0][0];
	}

	if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != 1)
	{
		return board[0][2];
	}

	return 0;
}

void displayWinner(int winner)
{
	if (winner == 3)
	{
		oled.println("\nYOU WIN!");
		sound(1, 500);	// play the sound

	}
	else
	{
		oled.println("\nARDUINO WINS!");
		sound(3, 500);	// play the sound
	}
}

void displayTie()
{
	oled.println("\nNO WINNER!");
	sound(2, 500);	// play the sound

}

void readButton(int *coord)
{
	if (digitalRead(buttonApin) == LOW && !buttonAPressed)
	{
		buttonACount++;
		sound(1, 50);	// play the sound

		if (buttonACount > 3)
		{
			buttonACount = 1;
		}

		buttonAPressed = true;
		delay(100);	// Wait for debouncing
		lastButtonPressTime = millis();
		waitingForInactivity = true;
	}

	if (digitalRead(buttonBpin) == LOW && !buttonBPressed)
	{
		buttonBCount++;
		sound(1, 50);	// play the sound

		if (buttonBCount > 3)
		{
			buttonBCount = 1;
		}

		buttonBPressed = true;
		delay(100);	// Wait for debouncing

		lastButtonPressTime = millis();
		waitingForInactivity = true;
	}

	if (digitalRead(buttonApin) == HIGH)
	{
		buttonAPressed = false;
	}

	if (digitalRead(buttonBpin) == HIGH)
	{
		buttonBPressed = false;
	}

	if (waitingForInactivity && buttonACount > 0 && buttonBCount > 0 && (millis() - lastButtonPressTime > 700))
	{
		waitingForInactivity = false;
		/*
		Serial.print("Final values = ");
		Serial.print(buttonACount);
		Serial.print(",");
		Serial.println(buttonBCount);
		*/
		coord[0] = buttonACount;
		coord[1] = buttonBCount;

		return;
	}

	delay(100);
}

void sound(int f, int t)
{
	//output an frequency
	for (int i = 0; i < int(t / f); i++)
	{
		digitalWrite(buzzer, HIGH);
		delay(f);	
		digitalWrite(buzzer, LOW);
		delay(f);
	}
}

void reset()
{
  // Reset to restart 
	delay(3000);
	wdt_enable(WDTO_15MS);	
	while (true) {}
}

void setup()
{
	Serial.begin(9600);	// Set baud rate to match the serial monitor
	randomSeed(analogRead(0));	// randomize using noise from analog pin 0

	//initialize the buzzer pin as an output
  pinMode(buzzer, OUTPUT);	

	// initialize display
	Wire.begin();
	oled.begin(&Adafruit128x64, I2C_ADDRESS);
	oled.set400kHz();
	oled.setFont(Adafruit5x7);

	// Initialize the buttons
	pinMode(buttonApin, INPUT_PULLUP);
	pinMode(buttonBpin, INPUT_PULLUP);
}

void loop()
{
	while (moveCount < 9)
	{
		if (player == 1)
		{
			// human player's turn
			humanTurn();
		}
		else if (player == 2)
		{
			// computer player's turn
			computerTurn();
		}

		int winner = checkWinner();

		if (winner != 0)
		{
			displayWinner(winner);
			reset();
		}

		delay(100);	
	}

	if (moveCount == 9)
	{
		displayTie();
		reset();
	}
}