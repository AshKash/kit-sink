
#include "tos.h"
#include "LEDS.h"


#define TOS_FRAME_TYPE AM_temp_frame
TOS_FRAME_BEGIN(AM_temp_frame) {
        char leds_on;
}
TOS_FRAME_END(AM_temp_frame);


char TOS_COMMAND(LEDS_INIT)(){
  VAR(leds_on) = 0;
  SET_RED_LED_PIN();
  SET_YELLOW_LED_PIN();
  SET_GREEN_LED_PIN();
  return 1;

}


char TOS_COMMAND(RED_LED_ON)(){

  printf("R_on");
  CLR_RED_LED_PIN();
  VAR(leds_on) |= 0x1;
  return 1;
}

char TOS_COMMAND(RED_LED_OFF)(){

  printf("R_off");
  SET_RED_LED_PIN();
  VAR(leds_on) &= 0x6;
  return 1;
}

char TOS_COMMAND(RED_LED_TOGGLE)(){
  if(VAR(leds_on) & 0x1){
	return TOS_CALL_COMMAND(RED_LED_OFF)();
  }else{
	return TOS_CALL_COMMAND(RED_LED_ON)();
  } 
}


char TOS_COMMAND(GREEN_LED_ON)(){

  printf("G_on");
  CLR_GREEN_LED_PIN();
  VAR(leds_on) |= 0x2;
  return 1;
}

char TOS_COMMAND(GREEN_LED_OFF)(){

  printf("G_off");
  SET_GREEN_LED_PIN();
  VAR(leds_on) &= 0x5;
  return 1;
}
char TOS_COMMAND(GREEN_LED_TOGGLE)(){
  if(VAR(leds_on) & 0x2){
	return TOS_CALL_COMMAND(GREEN_LED_OFF)();
  }else{
	return TOS_CALL_COMMAND(GREEN_LED_ON)();
  } 
}

char TOS_COMMAND(YELLOW_LED_ON)(){

  printf("Y_on");
  CLR_YELLOW_LED_PIN();
  VAR(leds_on) |= 0x4;
  return 1;
}

char TOS_COMMAND(YELLOW_LED_OFF)(){

  printf("Y_off");
  SET_YELLOW_LED_PIN();
  VAR(leds_on) &= 0x3;
  return 1;
}
char TOS_COMMAND(YELLOW_LED_TOGGLE)(){
  if(VAR(leds_on) & 0x4){
	return TOS_CALL_COMMAND(YELLOW_LED_OFF)();
  }else{
	return TOS_CALL_COMMAND(YELLOW_LED_ON)();
  } 
}

