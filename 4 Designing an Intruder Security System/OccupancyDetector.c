/*
 * OccupancyDetector.c
  *  Created on: February 20, 2023
 *      Author: Ethan Struble
 *      Version: 1.0
 */
#define Amred_State 0
#define Warning_State 1
#define Alert_State 2
#include <msp430.h>

//Global Variables
unsigned int Time = 0;             // To change in between states
char State = Amred_State;                         // What state we are in
void gpioInit();

int main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    gpioInit(); // Initializing all of the ports

    PM5CTL0 &= ~LOCKLPM5;

    __bis_SR_register(GIE);                 // Enter LPM3 w/interrupt

    while(1)
    {
        //This is the reset condition
        if(!(P4IN & BIT1))        //When reset is pressed(P4.1)
        {
          P1OUT &= ~BIT0;        // Turing off the RED LED
          Time = 0;               // Restarting time condition
        }
        //Going through the states
        switch(State)
        {
            //Starting State
            case Amred_State:
            {
                P1OUT &= ~BIT0;                 // Keeps the RED LED off
                P6OUT ^= BIT6;                  // Toggles the Green LED
                __delay_cycles(100000);         // Blinks Green LED for 1 second(So its easier with the math)
                if(!(P2IN &= BIT3))             //When input is press(2.3)
                {
                    P6OUT &= ~BIT6;             // Turns off of Green LED
                    State = Warning_State;       // Goes into the next state because the "Sensor" went off
                }
                break;
            }
            case Warning_State:
            {
                P6OUT &= ~BIT6;             //Keeps the Green LED off
                P1OUT ^= BIT0;              //Toggles the RED LED
                __delay_cycles(50000);        //Blinks RED LED for half a second(This is so its faster then the green LED blink, Also easier math)
                Time++;                     //Increments our timing condition
                if(Time >= 19)              // After 10 seconds or 20 RED LED blinks
                {
                    State = Alert_State;    //Goes into next state because the "Sensor went off for more then 10 second
                }else if((P2IN &= BIT3))       // The input is released(P2.3)
                {
                    State = Amred_State;    //Starting from Amred_State if the input is released
                }
                break;
            }
            case Alert_State:
            {
                P1OUT |= BIT0;      // Keeps RED LED on
                P6OUT &= ~BIT6;     // Keeps Green LED off
                break;
            }
        }
    }
}
#pragma vector=PORT4_VECTOR      //This is checking Port 4 for an interrupt
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;             //This resets the interrupt flag
    State = Amred_State;        //Sets the State to alert
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)    //This is checking port 2 for an interrupt
{
    P2IFG &= ~BIT3;              //This resets the interrupt flag
    State = Alert_State;         //Sets the State to armed

}

void gpioInit()
{
    P1OUT &= ~BIT0;                 // Starting up the RED LED
    P1DIR |= BIT0;                  // Sets it as an Output

    P6OUT &= ~BIT6;                 // Starting up the Green LED
    P6DIR |= BIT6;                  // Sets it as an Output

    P2OUT |= BIT3;                  // Staring up the Input Button (P2.3)
    P2REN |= BIT3;                  // Make this a pull up resistor
    P2IE |= BIT3;                   // Setting up Interrupt edge
    P2IES &= ~BIT3;                 // Setting up Interrupt edge select
    P2IFG &= ~BIT3;                 // Clearing the interrupt flag

    P4OUT |= BIT1;                  // Staring up the Reset Button(P4.1)
    P4REN |= BIT1;                  // Make this a pull up resistor
    P4IE |= BIT1;                   // Setting up Interrupt Edge
    P4IES &= ~BIT1;                 // Setting up Interrupt Edge select
    P4IFG &= ~BIT1;                 // Clearing the interrupt flag
}




