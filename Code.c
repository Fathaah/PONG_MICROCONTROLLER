#include "msp430.h"
#include <stdint.h>

// Uncomment if you are using a MAX7221, leave commented for MAX7219
//#define USE_MAX7221

// Port1 USCI pins and CS/Load pin
#define SPI_SIMO  BIT2
#define SPI_CLK   BIT4
#define SPI_CS    BIT3  // Load or /CS

// MAX7219 Register addresses
#define MAX_NOOP  0x00
#define MAX_DIGIT0  0x01
#define MAX_DIGIT1  0x02
#define MAX_DIGIT2  0x03
#define MAX_DIGIT3  0x04
#define MAX_DIGIT4  0x05
#define MAX_DIGIT5  0x06
#define MAX_DIGIT6  0x07
#define MAX_DIGIT7  0x08
#define MAX_DECODEMODE  0x09
#define MAX_INTENSITY 0x0A
#define MAX_SCANLIMIT 0x0B
#define MAX_SHUTDOWN  0x0C
#define MAX_DISPLAYTEST 0x0F
#define INT_BITS 8
int x_dir = 0;
int y_dir = -1;
//int pong_x,pong_y;
int pong_x = 4;
int pong_y = 5;
// Function prototypes
void spi_init();
void spi_max(unsigned char address, unsigned char data);
byte pong_loc;
byte left = B11110000;
byte right = B00001111;
int i ;
byte b[8];
byte flip[8];
byte disp[8];
byte sad[8] ;
// Program start
int main(void){
  play();}
int play(void)
{
  WDTCTL = WDTPW + WDTHOLD;   // Disable WDT
  DCOCTL = CALDCO_1MHZ;     // 1 Mhz DCO
  BCSCTL1 = CALBC1_1MHZ;
  // Setup Port1 pins
  P1DIR |= SPI_SIMO + SPI_CLK + SPI_CS;
  #ifdef USE_MAX7221
  P1OUT |= SPI_CS;
  //P2REN |= BIT0 + BIT2// MAX7221 Chip Select is inactive high
  #endif

  spi_init();     // Init USCI in SPI mode
  // Initialise MAX7219 with 8x8 led matrix
  spi_max(MAX_NOOP, 0x00);  // NO OP (seems needed after power on)
  spi_max(MAX_SCANLIMIT, 0x07);   // Enable all digits (always needed for current/8 per row)
  spi_max(MAX_INTENSITY, 0x08);   // Display intensity (0x00 to 0x0F)
  spi_max(MAX_DECODEMODE, 0); // No BCD decoding for led matrix
  // Clear all rows/digits
  spi_max(MAX_DIGIT0, 0);
  spi_max(MAX_DIGIT1, 0);
  spi_max(MAX_DIGIT2, 0);
  spi_max(MAX_DIGIT3, 0);
  spi_max(MAX_DIGIT4, 0);
  spi_max(MAX_DIGIT5, 0);
  spi_max(MAX_DIGIT6, 0);
  spi_max(MAX_DIGIT7, 0);
  spi_max(MAX_SHUTDOWN, 1);   // Wake oscillators/display up

  x_dir = 1;  
  // Ready to start displaying something!
  uint8_t row, framecounter;
  disp[0] = BIT4 + BIT5 + BIT6;
  disp[5] = BIT4;
  // Loop forever
  while (1) 
  { update_pong();
    if(P2IN&BIT0){
   // __delay_cycles(900000);
    disp[0] = leftRotate(disp[0],1);
    //__delay_cycles(200);
    }
    if(P2IN&BIT2){
    //__delay_cycles(900000);
    disp[0] = rightRotate(disp[0],1);
    //__delay_cycles(200);
    }
    disp[7] = BIT0 + BIT7 + BIT1;
    disp[7] = leftRotate(disp[7],pong_x+x_dir);
    // Loop through some frames
    // Load all 8 row/digit registers with data from number[framecounter]
      flipDisp();
      for (row=0; row<8; row++)
      {
        spi_max(MAX_DIGIT0+row, flip[row]);
      }
        }

}


// Send 16 bits as: xxxxaaaadddddddd (ignore, address, data)
// and use active low Chip Select or active high Load pulse
// depending if the IC is a MAX7221 or MAX7219
void spi_max(uint8_t address, uint8_t data)
{
#ifdef USE_MAX7221
  P1OUT &= ~(SPI_CS);     // MAX7221 uses proper /CS
#endif
  UCA0TXBUF = address & 0b00001111; // Send 4bit address as byte
  while (UCA0STAT & UCBUSY);    // Wait until done
  UCA0TXBUF = data;     // Send byte of data
  while (UCA0STAT & UCBUSY);
  P1OUT |= SPI_CS;      // /CS inactive or Load high
#ifndef USE_MAX7221
  P1OUT &= ~(SPI_CS);     // MAX7219 pulses Load high
#endif
}

// Enable harware SPI
void spi_init()
{
  UCA0CTL1 |= UCSWRST;    // USCI in Reset State (for config)
  // Leading edge + MSB first + Master + Sync mode (spi)
  UCA0CTL0 = UCCKPH + UCMSB + UCMST + UCSYNC;
  UCA0CTL1 |= UCSSEL_2;     // SMCLK as clock source
  UCA0BR0 |= 0x01;    // SPI speed (same as SMCLK)
  UCA0BR1 = 0;
  P1SEL |= SPI_SIMO + SPI_CLK;  // Set port pins for USCI
  P1SEL2 |= SPI_SIMO + SPI_CLK;
  UCA0CTL1 &= ~UCSWRST;     // Clear USCI Reset State
}

int leftRotate(byte n, unsigned int d) 
{ 
      
    /* In n<<d, last d bits are 0. To 
     put first 3 bits of n at  
    last, do bitwise or of n<<d  
    with n >>(INT_BITS - d) */
    return (n << d)|(n >> (INT_BITS - d)); 
} 
  
/*Function to right rotate n by d bits*/
int rightRotate(byte n, unsigned int d) 
{ 
    /* In n>>d, first d bits are 0.  
    To put last 3 bits of at  
    first, do bitwise or of n>>d 
    with n <<(INT_BITS - d) */
    return (n >> d)|(n << (INT_BITS - d)); 
}

void update_pong(){
  pong_loc = disp[pong_y];
   
  switch(pong_y){
    case 0: __delay_cycles(500000);
            GameOver();
          break;
    case 1: if(pong_loc&disp[0]){
              y_dir = 1;
              break;
    }
            if((pong_loc<<1&disp[0])||(pong_loc>>1&disp[0])){
              y_dir = 1;
              x_dir = -x_dir;}
             break;
     case 6: y_dir = - y_dir;
              break;
            break;
    default : 
              break;
            }
    switch(pong_x){
          case 0: x_dir = +1;
                  break;
          case 7: x_dir = -1;
                  break;
          default: break;
      
      }
    pong_x += x_dir;
    pong_y += y_dir;
    for(i = 1 ;i<8;i++){
      disp[i] = 0;
      }
    byte pong = BIT0;
    pong = pong<<(pong_x);
    disp[pong_y] = pong;
    
    __delay_cycles(200000);
    
}
int row;
void GameOver(){
      disp[0] = B00000000;
      disp[1] = B01000010;
      disp[2] = B00111100;
      disp[3] = B00000000;
      disp[4] = B00000000;
      disp[5] = B00000000;
      disp[6] = B01000010;
      disp[7] = B00000000;
      flipDisp();
       while(1){      
              for (row = 0; row < 8; row++)
      {
        spi_max(MAX_DIGIT0+row, flip[row]);
      }
        }}
int k;


void flipDisp(){
      b[0] = B10000000;
      b[1] = B01000000;
      b[2] = B00100000;
      b[3] = B00010000;
      b[4] = B00001000;
      b[5] = B00000100;
      b[6] = B00000010;
      b[7] = B00000001;
  byte x;
  byte f;
  for(i = 0;i<8;i++){
    x = b[i];
    flip[i] = B00000000;
    for(k = 0;k<8;k++){
        f = x&disp[k];
        if (i>k){
          flip[i] += (f<<(i-k)); 
          }
        if(i < k){
          flip[i] += (f>>(k-i)); 
          }
        if(i == k){
          flip[i] += f;
          }
      }
    }
      }
