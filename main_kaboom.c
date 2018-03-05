// Written by:      Steven Diamante
// Due Date:        4/5/13
// ECE 2534:        Lab 3 - Kaboom!
// Purpose:         This program will produce a game on the OLED display. This program
//                  will use the accelerometer to move the bucket that will catch bombs
//                  that were created in the Chrfont.c file.
// Resources:       PmodOLED.c uses Timer1 for 1-ms delays
//                  main_kaboom.c uses Timer2 to generate 1-ms interrupts

#include <plib.h>
#include <stdlib.h>
#include "PmodOLED.h"
#include "OledChar.h"
#include "OledGrph.h"
#include "delay.h"

// Cerebot board configuration
#pragma config ICESEL       = ICS_PGx1  // ICE/ICD Comm Channel Select
#pragma config DEBUG        = OFF       // Debugger Disabled for Starter Kit
#pragma config FNOSC        = PRIPLL	// Oscillator selection
#pragma config POSCMOD      = XT	// Primary oscillator mode
#pragma config FPLLIDIV     = DIV_2	// PLL input divider
#pragma config FPLLMUL      = MUL_20	// PLL multiplier
#pragma config FPLLODIV     = DIV_1	// PLL output divider
#pragma config FPBDIV       = DIV_8	// Peripheral bus clock divider
#pragma config FSOSCEN      = OFF	// Secondary oscillator enable

// Global variables
unsigned sec1000_constant = 0; // This is updated 1000 times per second by interrupt handler
unsigned sec_1000 = 0;         // This is updated 1000 times per second by interrupt handler
int difficulty = 1000; // determines speed of bomb drops
char bucket = '_'; // bucket
char bomb_t = '*'; // top section bomb
char bomb_b = '+'; // bottom section bomb
char blank = ' '; // clear bomb
char person = '&'; // a person will jump out of the bucket and run away when a bomb explodes
char person_top = '%'; // person is displayed in the top half of the 8x8 grid
int Y_check=0; // used to compare two accelerometer values
int Kaboom=0;   // If this value is ever equal to 1 the game is over
int stay, x_pos=4, Y, xd0, xd1, yd0, yd1, zd0, zd1;

// Interrupt handler - respond to timer-generated interrupt
#pragma interrupt InterruptHandler ipl1 vector 0
void InterruptHandler( void )
{
    if( mT2GetIntFlag() ) // Verify source of interrupt
{
sec1000_constant++; // Update global variable
sec_1000++; // Update global variable
 mT2ClearIntFlag();    // Acknowledge interrupt
}
}

void Bucket_move();

void Bucket_move()
{
        SpiChnPutC(SPI_CHANNEL3, 0x2D);
        SpiChnPutC(SPI_CHANNEL3, 0x8);
        SpiChnGetC(SPI_CHANNEL3);
        SpiChnGetC(SPI_CHANNEL3);

        SpiChnPutC(SPI_CHANNEL3, 0xF2);
        SpiChnPutC(SPI_CHANNEL3, 0);
        SpiChnPutC(SPI_CHANNEL3, 0);
        SpiChnPutC(SPI_CHANNEL3, 0);
        SpiChnPutC(SPI_CHANNEL3, 0);
        SpiChnPutC(SPI_CHANNEL3, 0);
        SpiChnPutC(SPI_CHANNEL3, 0);

        SpiChnGetC(SPI_CHANNEL3);

        // Set up the SPI to read values for x, y, and z (both pos and neg)
        xd0 = SpiChnGetC(SPI_CHANNEL3);
        xd1 = SpiChnGetC(SPI_CHANNEL3);
        yd0 = SpiChnGetC(SPI_CHANNEL3);
        yd1 = SpiChnGetC(SPI_CHANNEL3);
        zd0 = SpiChnGetC(SPI_CHANNEL3);
        zd1 = SpiChnGetC(SPI_CHANNEL3);

        Y = (yd0-yd1)/2/10; // gets average of positive and negative values of Y
       stay = abs(Y-Y_check); // compares current and previous accelerometer values
       if(Y>0&&stay>1&&x_pos>0) // moves bucket to the left
           x_pos--;
       else if(Y<=0&&stay>1&&x_pos<8) // moves bucket to the right
           x_pos++;
       else
       { } // bucket stays still
       OledSetCursor(x_pos, 3);
       OledPutChar(bucket);
       Y_check = Y;
       DelayMs(100);
       OledSetCursor(x_pos, 3); // removes previous bucket location
       OledPutChar(blank);
       OledUpdate();
}


void Drop_bomb(int x, int y)
{
    for(y=0; y<2; y++)
    {
        OledSetCursor(x,y);
        OledPutChar(blank);
        sec_1000=0; //resets timer
        while(sec_1000 < difficulty) // delays 250 - 1000 ms depending on level
        {
        Bucket_move(); // calls bucket_move function
        OledSetCursor(x,y);
        OledPutChar(bomb_t); // drops bomb at random x location
        }
        sec_1000=0;
        OledSetCursor(x,y);
        OledPutChar(blank);
        while(sec_1000 < difficulty)
        {
        Bucket_move();
        OledSetCursor(x,y);
        OledPutChar(bomb_b); // bomb is dropped to the lower half of the grid
        }
        OledSetCursor(x,y);
        OledPutChar(blank); // after delay the bomb is removed from the (x,y) location
    }
}

Drop_next(int x2, int x)
{   // This function drops the first bomb the rest of the way down the screen
    // and also drops another bomb half way down the screen simultaneously.
    int y, y2=0;
    for(y=2; y<4; y++)
    {
        sec_1000=0; //resets timer
        while(sec_1000 < difficulty)
        { // delays difficulty time and drops both bombs simultaneously
        Bucket_move();
        OledSetCursor(x,y);
        OledPutChar(bomb_t);
        OledSetCursor(x2,y2);
        OledPutChar(bomb_t);
        }
        sec_1000=0;
        OledSetCursor(x,y);
        OledPutChar(blank);
        OledSetCursor(x2,y2);
        OledPutChar(blank);
        while(sec_1000 < difficulty)
        {
        Bucket_move();
        OledSetCursor(x,y);
        OledPutChar(bomb_b);
        OledSetCursor(x2,y2);
        OledPutChar(bomb_b);
        }
        if(x==x_pos)
        {
            Kaboom = 0; // if the bucket is in the same column as the bomb
                        // ,then it is a catch and the game is still going.
        }
        else
            Kaboom = 1; // GAME OVER (bomb not caught)
      OledSetCursor(x,y);
      OledPutChar(blank);
      OledSetCursor(x2,y2);
      OledPutChar(blank);
      y2++; // increment second bomb in the loop
    }
}

Drop_another(int x3, int x2)
{   // This function operates in the same way as Drop_next
    // except that it switches the y locations so that it can drop the next two bombs simultaneously
    int y2, y3=0;
    for(y2=2; y2<4; y2++)
    {
            sec_1000=0; //resets timer
        while(sec_1000 < difficulty)
        {
        Bucket_move();
        OledSetCursor(x2,y2);
        OledPutChar(bomb_t);
        OledSetCursor(x3,y3);
        OledPutChar(bomb_t);
        }
        sec_1000=0;
        OledSetCursor(x2,y2);
        OledPutChar(blank);
        OledSetCursor(x3,y3);
        OledPutChar(blank);
        while(sec_1000 < difficulty)
        {
        Bucket_move();
         if(x2==x_pos)
        {
            Kaboom = 0;
        }
        else
            Kaboom = 1;
        OledSetCursor(x2,y2);
        OledPutChar(bomb_b);
        OledSetCursor(x3,y3);
        OledPutChar(bomb_b);
        }
      OledSetCursor(x2,y2);
      OledPutChar(blank);
      OledSetCursor(x3,y3);
      OledPutChar(blank);
      y3++;
    }
}



enum modes{

	TITLE,				// Display Kaboom! title screen
	GAMESTART,                      // Go to game screen
        BOMB1,				// Drops first bomb
        BOMB2,                          // First bomb is caught or explodes
        BOMB3,                          // Second bomb is dropped and caught/explodes
        UPDATE2,                        // Updates the score and level as it changes
        UPDATE3,                        // Updates the score and level as it changes
        GAMEOVER,                       // Game over screen
	};
               enum modes mode;

int main()
{
char buf[16];    // Temp string for OLED display
int level = 1, score = 0, count = 0;   // initialize counters

//**********************************************************************************//
// Open up SPI
SpiChnOpen (SPI_CHANNEL3, SPI_OPEN_MSTEN | SPI_OPEN_MSSEN | SPI_OPEN_CKP_HIGH | SPI_OPEN_MODE8  | SPI_OPEN_ENHBUF | SPI_OPEN_ON, 10);

// Initialize delay timer and OLED
DelayInit();
OledInit();
// Set up timer 2 to roll over every ms
OpenTimer2(T2_ON | T2_IDLE_CON | T2_SOURCE_INT | T2_PS_1_16 | T2_GATE_OFF, 625);
// f = 10MHz/16/625 = 1 kHz
// Set up CPU to respond to interrupts from Timer2
mT2SetIntPriority(1);
INTEnableSystemSingleVectoredInt();
mT2IntEnable(1);

mode = TITLE; // First time through the program displays the title screen
int x, x2, x3;
while(1)
{
   switch(mode){
	case TITLE:     // Title Screen Display
		OledSetCursor(0, 1);
                OledPutString("Cerebot Kaboom!");
                OledSetCursor(0, 2);
                OledPutString("Steven Diamante");
                OledUpdate();
                sec_1000 = 0; // reset sec1000
                while(sec_1000<5000) // delay 5 seconds
                {}
                OledClearBuffer();
                mode = GAMESTART;                
                break;
        case GAMESTART:
            OledSetCursor(11,0);
            OledPutString("LEVEL");
            OledSetCursor(15,1);
            sprintf(buf, "%d", level);
            OledPutString(buf);
            OledSetCursor(11,2);
            OledPutString("SCORE");
            OledSetCursor(15,3);
            sprintf(buf, "%d", score);
            OledPutString(buf);
            mode = BOMB1;
            break;
       case BOMB1:
            x = rand() % 8; // generate random postion to drop a bomb half way down the screen
            Drop_bomb(x,0);
            x3 = x;
            mode = BOMB2;
            break;
       case BOMB2:
           x2 = rand() % 8; // generate random x-postion to drop next bomb
           Drop_next(x2,x3);
           if(Kaboom==1)
           {
               mode=GAMEOVER; // ends game immediately if bomb is not caught
               break;
           }
           count++; // increment count, which will advance levels
           score=score+(1*level); // adds x amount of points per bomb (x = level)
           mode = UPDATE2; // update score after bomb is caught
           break;
       case BOMB3:
           x3 = rand() % 8; // generate random x postion to drop bomb
           Drop_another(x3,x2);
           if(Kaboom==1)
           {
               mode=GAMEOVER;
               break;
           }
           count++;
           score = score+(1*level);
           if(count%4==0)
           {
               level++;
               if(difficulty>250)
                difficulty = difficulty-250; // increases speed and level when 4 bombs are caught
           }
           mode = UPDATE3; // updates score after a bomb is caught
         break;
       case UPDATE2:
            OledSetCursor(15,1);
            sprintf(buf, "%d", level);
            OledPutString(buf);
            if(score>9)
                OledSetCursor(14,3); // if score is double digits this will move the cursor back one space
            else
                OledSetCursor(15,3);
            sprintf(buf, "%d", score);
            OledPutString(buf);
            OledUpdate();
            mode = BOMB3;
            break;
       case UPDATE3:
           OledSetCursor(15,1);
            sprintf(buf, "%d", level);
            OledPutString(buf);
            if(score>9)
                OledSetCursor(14,3); // if score is double digits this will move the cursor back one space
            else
                OledSetCursor(15,3);
            sprintf(buf, "%d", score);
            OledPutString(buf);
            OledUpdate();
            mode = BOMB2; // repeats game process
            break;
       case GAMEOVER:
           // If a bomb explodes, a little man will jump off of the bucket and run away
                OledSetCursor(x_pos,3);
                OledPutChar(bucket);
                OledSetCursor(x_pos,2);
                OledPutChar(person);
                sec_1000 = 0; // reset sec_1000
                while(sec_1000<500) // delay 500 milliseconds
                {}
                OledSetCursor(x_pos,2);
                OledPutChar(blank);
                if(x_pos<4)
                {
                    OledSetCursor(x_pos+1,3);
                    OledPutChar(person_top);
                    sec_1000 = 0; // reset sec_1000
                    while(sec_1000<500) // delay 500 milliseconds
                    {}
                    OledSetCursor(x_pos+1,3);
                    OledPutChar(blank);
                    OledSetCursor(x_pos+2,3);
                    OledPutChar(person);
                    sec_1000 = 0; // reset sec_1000
                    while(sec_1000<500) // delay 500 milliseconds
                    {}
                    OledSetCursor(x_pos+2,3);
                    OledPutChar(blank);
                    for((x_pos+3);x_pos<11;x_pos++)
                    {
                        OledSetCursor(x_pos+3,3);
                        OledPutChar(person);
                        sec_1000 = 0; // reset sec_1000
                        while(sec_1000<500) // delay 500 milliseconds
                        {}
                        OledSetCursor(x_pos+3,3);
                        OledPutChar(blank);
                    }
                    sec_1000 = 0; // reset sec_1000
                    while(sec_1000<500) // delay 500 milliseconds
                    {}
                }
                else if(x_pos>=4)
                {
                    OledSetCursor(x_pos-1,3);
                    OledPutChar(person_top);
                    sec_1000 = 0; // reset sec_1000
                    while(sec_1000<500) // delay 500 milliseconds
                    {}
                    OledSetCursor(x_pos-1,3);
                    OledPutChar(blank);
                    OledSetCursor(x_pos-2,3);
                    OledPutChar(person);
                    sec_1000 = 0; // reset sec_1000
                    while(sec_1000<500) // delay 500 milliseconds
                    {}
                    OledSetCursor(x_pos-2,3);
                    OledPutChar(blank);
                    for((x_pos-3);x_pos>0;x_pos--)
                    {
                        OledSetCursor(x_pos-3,3);
                        OledPutChar(person);
                        sec_1000 = 0; // reset sec_1000
                        while(sec_1000<500) // delay 500 milliseconds
                        {}
                        OledSetCursor(x_pos-3,3);
                        OledPutChar(blank);
                    }
                sec_1000 = 0; // reset sec_1000
                while(sec_1000<500) // delay 500 milliseconds
                {}
                }
                OledClearBuffer(); // Clear buffer
                OledSetCursor(0, 0); // Display end game screen
                OledPutString("KABOOOOOOOOOOOM!");
                OledSetCursor(0, 2);
                sprintf(buf, "Score = %d", score);
                OledPutString(buf);
                OledSetCursor(0, 3);
                sprintf(buf, "Level = %d", level);
                OledPutString(buf);
                OledUpdate();
                score=0; // resets score, level, and difficulty for next game
                level=1;
                difficulty=1000;
                sec_1000 = 0; // reset sec_1000
                while(sec_1000<5000) // delay 5 seconds
                {}
                OledClearBuffer();
                mode = GAMESTART;
                break;
   }
   }
//**********************************************************************************//
return 0;
}