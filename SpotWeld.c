#define MCU atmega328p
#define F_CPU 16000000UL;

//Libraries
#include <avr/io.h>
#include <util/delay.h>

//Pins
#define PIN_A PD5
#define PIN_B PB2
#define PIN_C PD4
#define PIN_D PB4
#define PIN_E PB5
#define PIN_F PD6
#define PIN_G PD2
#define PIN_DP PB3
#define PIN_C1 PD7
#define PIN_C2 PB1
#define PIN_C3 PB6
#define PIN_C4 PD3
#define PIN_POT ADC0
#define PIN_WELD PC4
#define PIN_MORSE PC5
#define PIN_WORK_MODE PC3
#define PIN_CM1 ADC1
#define PIN_CM2 ADC2

//Variables
int weldTime = 0;

//Constants
const int displayTime = 5;
const int pinMorseRegMask = B00100000;
const int pinWorkModeRegMask = B00001000;


int main (void){
  //Pin Setup  
  DDRD |= (1 << PIN_A);
  DDRB |= (1 << PIN_B);
  DDRD |= (1 << PIN_C);
  DDRB |= (1 << PIN_D);
  DDRB |= (1 << PIN_E);
  DDRD |= (1 << PIN_F);
  DDRD |= (1 << PIN_G);
  DDRB |= (1 << PIN_DP);
  DDRD |= (1 << PIN_C1);
  DDRB |= (1 << PIN_C2);
  DDRB |= (1 << PIN_C3);
  DDRD |= (1 << PIN_C4);
  DDRC |= (1 << PIN_WELD);

  DDRC &= ~(1 << PIN_MORSE);
  PORTC |= (1 << PIN_MORSE);
  DDRC &= ~(1 << PIN_WORK_MODE);
  PORTC |= (1 << PIN_WORK_MODE);

  ADCSRA = B10000011; //ADC enable, div clock by 8 for 125kHz, single conv mode

  while(1) {
    if(PINC & pinWorkModeRegMask){
      weldMode();
    }
    else{
      ampMode();
    }
  }
  return 1;
}

void ampMode(){
  const int milliVoltMultiplier = 0.375;  //Factors 1023 res over 5v, 1/3 voltage divider & volt to millivolt
  const int milliOhmShunt = 80;
  ADMUX = B11100001; //1.1v IRV, left adjust, ADC1 as input channel
  ADCSRA |= (1 << ADSC);  //start conversion
  int cm1Read = ADCH;
  ADMUX = B11100010; //1.1v IRV, left adjust, ADC2 as input channel
  ADCSRA |= (1 << ADSC);  //start conversion
  int cm2Read = ADCH;
  int milliVolt1 = cm1Read * milliVoltMultiplier;
  int milliVolt2 = cm2Read * milliVoltMultiplier;
  int voltDiff = (abs(milliVolt1-milliVolt2));
  int milliCurrent = voltDiff/milliOhmShunt; //OHM's law over shunt resistor
  write7seg(milliCurrent);
}

void weldMode() {
  int newWeldTime = getWeldTime();
  if (abs(weldTime - newWeldTime)>2){ //requre at least a diff of 2 ms to avoid flickering
    weldTime = newWeldTime;
  }

  write7seg(weldTime);

  if (!(PINC & pinMorseRegMask)){
      weld();
  }
}

int getWeldTime(){
  ADMUX = B11100000; //1.1v IRV, left adjust, ADC0 as input channel
  ADCSRA |= (1 << ADSC);  //start conversion
  int potRead = ADCH;
  int potTime = potRead*0.978;
  if (potTime > 999){potTime = 999;}
  return potTime;
}

void weld(){
  PORTC |= (1 << PIN_WELD);
  for(int i=0; i<weldTime; i++){
    _delay_ms(1);
  }
  PORTC &= ~(1 << PIN_WELD);
  _delay_ms(500);
}

void write7seg(int dispVal){
  if(dispVal<0 || dispVal>999){
    write_null();
    PORTB |= (1 << PIN_C2);
    PORTB |= (1 << PIN_C3);
    PORTD |= (1 << PIN_C4);
    return;
  }
  chooseSymbol(dispVal % 10);
  PORTD |= (1 << PIN_C4);
  _delay_ms(displayTime);
  PORTD &= ~(1 << PIN_C4);
  dispVal = dispVal-(dispVal % 10);

  if(dispVal>=10){
    chooseSymbol((dispVal % 100)/10);
    PORTB |= (1 << PIN_C3);
    _delay_ms(displayTime);
    PORTB &= ~(1 << PIN_C3);
    dispVal = dispVal-(dispVal % 100);
  }

  if(dispVal>=100){
    chooseSymbol((dispVal % 1000)/100);
    PORTB |= (1 << PIN_C2);
    _delay_ms(displayTime);
    PORTB &= ~(1 << PIN_C2);
  }
}

void chooseSymbol(int symbol){

  switch (symbol){

    case 0: write_0();
    break;

    case 1: write_1();
    break;
 
    case 2: write_2();
    break;

    case 3: write_3();
    break;

    case 4: write_4();
    break;

    case 5: write_5();
    break;

    case 6: write_6();
    break;

    case 7: write_7();
    break;

    case 8: write_8();
    break;

    case 9: write_9();
    break;

    default: write_null();
    break;
  }
}

void write_0(){
  PORTD |= (1 << PIN_A);
  PORTB |= (1 << PIN_B);
  PORTD |= (1 << PIN_C);
  PORTB |= (1 << PIN_D);
  PORTB |= (1 << PIN_E);
  PORTD |= (1 << PIN_F);
  PORTD &= ~(1 << PIN_G);
}

void write_1(){
  PORTD &= ~(1 << PIN_A);
  PORTB |= (1 << PIN_B);
  PORTD |= (1 << PIN_C);
  PORTB &= ~(1 << PIN_D);
  PORTB &= ~(1 << PIN_E);
  PORTD &= ~(1 << PIN_F);
  PORTD &= ~(1 << PIN_G);
}

void write_2(){
  PORTD |= (1 << PIN_A);
  PORTB |= (1 << PIN_B);
  PORTD &= ~(1 << PIN_C);
  PORTB |= (1 << PIN_D);
  PORTB |= (1 << PIN_E);
  PORTD &= ~(1 << PIN_F);
  PORTD |= (1 << PIN_G);
}

void write_3(){
  PORTD |= (1 << PIN_A);
  PORTB |= (1 << PIN_B);
  PORTD |= (1 << PIN_C);
  PORTB |= (1 << PIN_D);
  PORTB &= ~(1 << PIN_E);
  PORTD &= ~(1 << PIN_F);
  PORTD |= (1 << PIN_G);
}

void write_4(){
  PORTD &= ~(1 << PIN_A);
  PORTB |= (1 << PIN_B);
  PORTD |= (1 << PIN_C);
  PORTB &= ~(1 << PIN_D);
  PORTB &= ~(1 << PIN_E);
  PORTD |= (1 << PIN_F);
  PORTD |= (1 << PIN_G);
}

void write_5(){
  PORTD |= (1 << PIN_A);
  PORTB &= ~(1 << PIN_B);
  PORTD |= (1 << PIN_C);
  PORTB |= (1 << PIN_D);
  PORTB &= ~(1 << PIN_E);
  PORTD |= (1 << PIN_F);
  PORTD |= (1 << PIN_G);
}

void write_6(){
  PORTD |= (1 << PIN_A);
  PORTB &= ~(1 << PIN_B);
  PORTD |= (1 << PIN_C);
  PORTB |= (1 << PIN_D);
  PORTB |= (1 << PIN_E);
  PORTD |= (1 << PIN_F);
  PORTD |= (1 << PIN_G);
}

void write_7(){
  PORTD |= (1 << PIN_A);
  PORTB |= (1 << PIN_B);
  PORTD |= (1 << PIN_C);
  PORTB &= ~(1 << PIN_D);
  PORTB &= ~(1 << PIN_E);
  PORTD &= ~(1 << PIN_F);
  PORTD &= ~(1 << PIN_G);
}

void write_8(){
  PORTD |= (1 << PIN_A);
  PORTB |= (1 << PIN_B);
  PORTD |= (1 << PIN_C);
  PORTB |= (1 << PIN_D);
  PORTB |= (1 << PIN_E);
  PORTD |= (1 << PIN_F);
  PORTD |= (1 << PIN_G);
}


void write_9(){
  PORTD |= (1 << PIN_A);
  PORTB |= (1 << PIN_B);
  PORTD |= (1 << PIN_C);
  PORTB |= (1 << PIN_D);
  PORTB &= ~(1 << PIN_E);
  PORTD |= (1 << PIN_F);
  PORTD |= (1 << PIN_G);
}

void write_null(){
  PORTD &= ~(1 << PIN_A);
  PORTB &= ~(1 << PIN_B);
  PORTD &= ~(1 << PIN_C);
  PORTB |= (1 << PIN_D);
  PORTB &= ~(1 << PIN_E);
  PORTD &= ~(1 << PIN_F);
  PORTD &= ~(1 << PIN_G);
}