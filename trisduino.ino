#include <Keypad.h>
#include <avr/wdt.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// SSD1306 display
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

// Define the number of rows and cols in the keypad
const byte ROWS = 4; 
const byte COLS = 4; 

// Define the keys on the keypad
char keys[ROWS][COLS] = { 
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// Connect the row and column pins of the keypad
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3};

// Initialize the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

int board[3][3] = {{1,1,1},{1,1,1},{1,1,1}}; // Initialize the board to all 1s
int player = 2; // Initialize the player turn 
int moveCount = 0; // Initialize the moves counter 
int winner = 0; // Initiatize winner

void humanTurn(){
  char key = keypad.getKey(); // Read the key pressed on the keypad
    
  if(key){ // If a key is pressed
    int row = (key - '1') / 3; // Calculate the row of the pressed key
    int col = (key - '1') % 3; // Calculate the column of the pressed key
    if(board[row][col] == 1){ // Only set the value if the current value is 1
      board[row][col] = 3; // Set the corresponding element in the board array to 3
	    printBoard(); // print board to console
      displayBoard(); // display board on oled display

      player = 2; // Set the player turn to 2 (computer)
   	  moveCount++; // Increase move count
    }
  }
}

void computerTurn(){
  int i = 0;
  int j = 0;
  
  // Find the winning move
  int result[2];
  find_best_move(board, 25, result);
  if (result[0] != -1 && result[1] != -1) {
    int i = result[0];
    int j = result[1];
    Serial.print("Winning move: ");
    Serial.print(i);
    Serial.print(",");
    Serial.println(j);
    computer_move(i,j);
    return;
  }
  else {
    // Find the blocking move
    find_best_move(board, 9, result);
    if (result[0] != -1 && result[1] != -1) {
      int i = result[0];
      int j = result[1];
      Serial.print("Blocking move: ");
      Serial.print(i);
      Serial.print(",");
      Serial.println(j);
      computer_move(i,j);
      return;
    }
    else {
      // Select a random position
      int i = random(0, 3);
	  int j = random(0, 3);	
      Serial.print("Random move: ");
      Serial.print(i);
      Serial.print(",");
      Serial.println(j);
      computer_move(i,j);
      return;
    }
  }
}
  
void computer_move(int i, int j) {
  if(board[i][j] == 1){ // Only set the value if the current value is 1
    board[i][j] = 5; // Set the corresponding element in the board array to 5 (computer)  
    delay(500); // Add a small delay 
	  printBoard(); // print board to console
    displayBoard(); // display board on oled display
   player = 1; // Set the player turn to 1 (human)
    moveCount++; // Increase move count
  }
}

void find_best_move(int values[][3], int check, int* best_move) {
    int row_product[3] = {1, 1, 1};
    int col_product[3] = {1, 1, 1};
    int diagonal1_product = 1;
    int diagonal2_product = 1;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            row_product[i] *= values[i][j];
            col_product[j] *= values[i][j];
            if (i == j) {
                diagonal1_product *= values[i][j];
            }
            if (i + j == 2) {
                diagonal2_product *= values[i][j];
            }
        }
    }

    if (check == row_product[0] || check == row_product[1] || check == row_product[2]) {
        int row;
        for (int i = 0; i < 3; i++) {
            if (row_product[i] == check) {
                row = i;
                break;
            }
        }
        for (int col = 0; col < 3; col++) {
            if (values[row][col] == 1) {
                best_move[0] = row;
                best_move[1] = col;
                return;
            }
        }
    } else if (check == col_product[0] || check == col_product[1] || check == col_product[2]) {
        int col;
        for (int j = 0; j < 3; j++) {
            if (col_product[j] == check) {
                col = j;
                break;
            }
        }
        for (int row = 0; row < 3; row++) {
            if (values[row][col] == 1) {
                best_move[0] = row;
                best_move[1] = col;
                return;
            }
        }
    } else if (check == diagonal1_product) {
        for (int i = 0; i < 3; i++) {
            if (values[i][i] == 1) {
                best_move[0] = i;
                best_move[1] = i;
                return;
            }
        }
    } else if (check == diagonal2_product) {
        for (int i = 0; i < 3; i++) {
            if (values[i][2-i] == 1) {
                best_move[0] = i;
                best_move[1] = 2-i;
                return;
            }
        }
    }

    best_move[0] = -1;
    best_move[1] = -1;
}

void printBoard() {
  // Print the matrix with updated values
  for(int i=0; i<3; i++){
    for(int j=0; j<3; j++){
      if(board[i][j] == 1){
        Serial.print(" ");
      }
      else if(board[i][j] == 3){
        Serial.print("x");
      }
      else if(board[i][j] == 5){
        Serial.print("o");
      }
      if(j < 2){ // Add column separator except for the last column
        Serial.print("|");
      }
    }
    if(i < 2){ // Add row separator except for the last row
      Serial.println("\n-+-+-"); // Print the row separator
    }
    else{
      Serial.println("\n"); // Print newline after the last row
    }
  }
}

void displayBoard() {
  // print on OLED display
  oled.clear();  
  oled.set1X();
  oled.println("TRISDUINO\n");
  oled.set1X();
  for(int i=0; i<3; i++){
    for(int j=0; j<3; j++){
      if(board[i][j] == 1){
        oled.print(" ");
      }
      else if(board[i][j] == 3){
        oled.print("x");
      }
      else if(board[i][j] == 5){
        oled.print("o");
      }
      if(j < 2){ // Add column separator except for the last column
        oled.print("|");
      }
    }
    if(i < 2){ // Add row separator except for the last row
      oled.println("\n-+-+-"); // Print the row separator
    }
  }

}
  
void setup() {
  Serial.begin(9600); // Set baud rate to match the serial monitor
  randomSeed(analogRead(0)); // randomize using noise from analog pin 5

  // initialize display
  Wire.begin();         
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.set400kHz();  
  oled.setFont(Adafruit5x7);  

}

int checkWinner() {
  for (int i = 0; i < 3; i++) {
    if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != 1) {
      return board[i][0];
    }
    if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != 1) {
      return board[0][i];
    }
  }
  if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != 1) {
    return board[0][0];
  }
  if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != 1) {
    return board[0][2];
  }
  return 0;
 }

void display_winner(int winner) {
  if (winner == 3) {
    Serial.println("\nYOU WIN!"); 
    oled.println("\nYOU WIN!"); 
  }
  else {
    Serial.println("\nARDUINO WINS!"); 
    oled.println("\nARDUINO WINS!"); 
   }
 }

void display_tie() {
  Serial.println("\nNO WINNER!"); 
  oled.println("\nNO WINNER!"); 

 }

void reset() {
    delay(4000); 
    wdt_enable(WDTO_15MS); // Reset to restart 
    while (true) {}
    }

void loop() {
  Serial.println("\nNew game"); 

  while (moveCount < 9) {
    if(player == 1){ // If it's the human player's turn
      humanTurn();
    }
    else if(player == 2){ // If it's the computer player's turn
      computerTurn();
    }

  int winner = checkWinner();

  if (winner != 0) {
    display_winner(winner);
    reset();
   }
    delay(100); // Add a small delay 
  }
  
  if (moveCount == 9) {
    display_tie(); 
    reset();
  }    
}
