// this is the code for the fan in the running, idle, and error states
// also double cheeck for basic errors if you can
// fan control pin
#define FAN_PIN 9

// led pin definitions these are for examples you just need to add the real ones

#define LED_YELLOW = pin # 
#define LED_BLUE = pin #
#define LED_GREEN = pin #
#define LED_RED = pin #



// state control: disabled = 0; Idle = 1; running = 2; error = 3

// this is the disabled state
volatile unsigned int state = 0; 

// initializing the fan and LED pin
volatile unsigned char* ddr_h = (unsigned char*) 0x101; //ddr for port H
volatile unsgined char* port_h =(unsigned char*) ox 102; // port for port h

void setup(){
  // fan pin setup
  *ddr_h |=  (1 << FAN_PIN);
  *port_h &= ~(1 << FAN_PIN);

  //LED pin setup
  *ddr_h |= (1 << LED_YELLOW)|(1 << LED_BLUE)|(1 << LED_GREEN)|(1 << LED_RED);

  *port_h &= ~((1 << LED_YELLOW)|(1 << LED_BLUE)|(1 << LED_GREEN)|(1 << LED_RED));

  // set inital state (DISABLED)
  state =0;
}

 void loop(){

  switch (state){
    case 0: // disabled
      *port_h |= (0x01 << LED_YELLOW); 
      *port_h &= ~((1 << LED_BLUE)|(1 << LED_GREEN)|(1 << LED_RED));
      *port_h &= ~(1 << FAN_PIN);
      break;

    case 1: // IDLE
      *port_h |= (0x01 << LED_GREEN); 
      *port_h &= ~((1 << LED_YELLOW) | (1 << LED_BLUE) | (1 << LED_RED));
      *port_h &= ~(1 << FAN_PIN);      
      break;
    
    case 2: // RUNNING
      *port_h |= (0x01 << LED_BLUE); 
      *port_h &= ~((1 << LED_YELLOW) | (1 << LED_GREEN) | (1 << LED_RED));
      *port_h &= ~(1 << FAN_PIN);      
      break;
    
    case 3: //ERROR
      *port_h |= (0x01 << LED_RED); 
      *port_h &= ~((1 << LED_YELLOW) | (1 << LED_GREEN) | (1 << LED_RED));
      *port_h &= ~(1 << FAN_PIN);
      break;
    }
 }

// this functio is to toggle between DISABLED and IDLE
void Toggle(){
  if (state == 0){
    state = 1; // go from disabled to Idle
  }else if(state == 1){
    state == 0; // goes from idle to disabled
  }
}
