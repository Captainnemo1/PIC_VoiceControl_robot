/*
Obstacle avoider using PIC16F877A
 * Code by: MCA A2 043_042_035_037
 * Dated: 13-10-2021
  */

#include <xc.h>

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define _XTAL_FREQ 20000000
#define trigger RC2 //34 is Trigger
#define echo RC3//35 is Echo 

int time_taken;
int distance;

     
     void back_off() //used to drive the robot backward
     {
      RB0 = 1; RB1 = 1;
      RD5=0; RD4=1; //Motor 1 reverse
      RD6=0; RD7=1; //Motor 2 reverse  
      __delay_ms(1000);   
     }
     
     int calculate_distance() //function to calculate distance of US
     {
        int distance=0;
        TMR1=0;
        // Send Trigger Pulse To The Sensor
        trigger=1;
        __delay_us(10);
        trigger=0;
        // Wait For The Echo Pulse From The Sensor
        while(!echo);
        // Turn ON Timer Module
        TMR1ON=1;
        // Wait Until The Pulse Ends
        while(echo);
        // Turn OFF The Timer
        TMR1ON=0;
        // Calculate The Distance Using The Equation
        distance=TMR1/58.82;
        return distance;  
     }
     //******Initialize SART********//
    void USART_Init()
    {
       //Set the pins of RX and TX//
        TRISC6=1;
        TRISC7=1;

      //Set the baud rate using//
        BRGH=1;      //high speed baud rate with Bluetooth
        SPBRG=129;

        //Turn on Serial Port//
        SYNC=0;
        SPEN=1;

        //Set 8-bit reception and transmission
        RX9=0;
        TX9=0;

       //Enable transmission and reception//
        TXEN=1; 
        CREN=1; 

        //Enable global and ph. interrupts//
        GIE = 1;
        PEIE= 1;

        //Enable interrupts for Tx. and Rx.//
        RCIE=1;
        TXIE=1;
    }
    //___________BT initialized_____________//

    //Function to load the buffer with one char.//
    void USART_char(char byte)  
    {
        TXREG = byte;
        while(!TXIF);  
        while(!TRMT);
    }
    //End of function//

    //Function to Load buffer with string//
    void USART_string(char* string)
    {
        while(*string)
        USART_char(*string++);
    }
    //End of function//

    //Function to send data from RX. buffer//
    void send_data()
    {
      TXREG = 13;  
      __delay_ms(500);
    }
    //End of function//

    //Function to get a char from buffer of Bluetooth//
    char USART_get_char(void)   
    {
        if(OERR) // check for over run error 
        {
            CREN = 0;
            CREN = 1; //Reset CREN
        }

        if(RCIF==1) //if the user has sent a char return the char (ASCII value)
        {
        while(!RCIF);  
        return RCREG;
        }
        else //if user has sent no message return 0
            return 0;
    }
    //End of function/
    
    void bltoh()
    {
        int get_value;
        RB0=1;
        RB1=1;
        get_value = USART_get_char(); //Read the char. received via BT

        //If we receive a '0'//
            if (get_value=='s')
              {
                 RD4=0;
                 RD5=0;
                 RD6=0;
                 RD7=0;
                 USART_string("MOTOR turned OFF");
                 send_data();
              }

        //If we receive a '1'//   
            if (get_value=='f')
              {
                 RD4=1;
                 RD5=0;
                 RD6=1;
                 RD7=0;
                 USART_string("MOTOR turned ON");
                 send_data();
              }

    

}

     
void main()
{
     //int get_value;
     //IO PORTS initialization//
     TRISC2 = 0; //Trigger pin of US sensor is sent as output pin
     TRISC3 = 1; //Echo pin of US sensor is set as input pin       
     TRISD2 = 1; TRISD3 = 1; //Both the IR sensor pins are declared as input
     TRISD4 = 0; TRISD5 = 0; //Motor 1 pins declared as output
     TRISD6 = 0; TRISD7 = 0; //Motor 2 pins declared as output
     TRISB0 = 0; TRISB1 = 0; //Motor enable pins
     
     //IO PORTS Finished//
     
     // Clear The Pre-Scaler Select Bits
     T1CKPS0=0;
     T1CKPS1=0;
     
     // Choose The Local Clock As Clock Source
     TMR1CS=0;
     T1CON=0x20;
     
     //Blue-tooth starting
     USART_Init(); //lets get our blue-tooth ready for action
    
     
     
     
     while(1)
         {  
         bltoh();
            calculate_distance();  
           
            if (distance>5)
            {                       
             RB0 = 1; RB1 = 1;
             RD4=0; RD5=1; //Motor 1 forward
             RD6=0; RD7=1; //Motor 2 forward 
            }

            calculate_distance();   
             if (RD2==0 && RD3==1 && distance<=5) //Left sensor is blocked
             {
             back_off();
             RB0 = 1; RB1 = 1;
             RD4=1; RD5=1; //Motor 1 stop
             RD6=1; RD7=0; //Motor 2 forward

             __delay_ms(500);
             }

            calculate_distance();   
             if (RD2==1 && RD3==0 && distance<=5) //Right sensor is blocked
             {
             back_off();
             RB0 = 1; RB1 = 1;
             RD4=1; RD5=0; //Motor 1 forward
             RD6=1; RD7=1; //Motor 2 stop

             __delay_ms(500);
             } 

            calculate_distance();   
             if (RD2==0 && RD3==0 && distance<=5)//Both sensor is open
             {
             back_off();
             RB0 = 1; RB1 = 1;
             RD4=0; RD5=1; //Motor 1 forward
             RD6=1; RD7=1; //Motor 2 stop

             __delay_ms(500);
             }

            calculate_distance();   
             if (RD2==1 && RD3==1 && distance<=5)//Both sensor is blocked
             {
             back_off(); 
             RB0 = 1; RB1 = 1;
             RD4=1; RD5=0; //Motor 1 reverse
             RD6=1; RD7=1; //Motor 2 stop

             __delay_ms(1000);
             }
             
         }
}
