
#include<avr/io.h>
#include<util/delay.h>


//MACRO Definitions
#define TRUE 	1
#define FALSE 	0

//MicroController Pin Definitions
#define POE_PWR_DETECT					PA0	//POE Plugin detect;If DC Jack Not Plugged in, Set high to enable POE Power to carrier and application board
#define APP_PWR_DETECT					PA1	//DC Jack Plugin Detect; If detected, set pin PA2 low to disable POE Mosfet Driver
#define POE_MOSFET_DRIVE				PA2	//POE Mosfet drive Enable;If DC Jack not plugged in,Set to high to enable POE Output
#define APP_MOSFET_DRIVE				PA3	//Application Board Mosfet drive Enable;Set to high to enable power from Application board to carrier board,feature not in use currently
#define BOOTLOADER_MODE_SELECT			PA7	//Boot Mode selection to bootloader,Set to low to Enable Bootloader
#define DEFAULT_BOOT_MODE_SELECT		PB0	//Boot Nortmal mode,System enters to this mode by default
#define FAST_BOOT_DETECT				PB2	//When detected, Switch the system to bootloader mode by 
#define LED_TOGGLE						PB1	//When detected, Switch the system to bootloader mode by 


#define BIT0							0x01
#define BIT1							0x02
#define BIT2							0x04
#define BIT3							0x08
#define BIT4							0x10
#define BIT5							0x20
#define BIT6							0x40
#define BIT7							0x80

#define POE_PWR_DETECT_BIT				BIT0	//POE Plugin detect;If DC Jack Not Plugged in, Set high to enable POE Power to carrier and application board
#define APP_PWR_DETECT_BIT				BIT1	//DC Jack Plugin Detect; If detected, set pin PA2 low to disable POE Mosfet Driver
#define POE_MOSFET_DRIVE_BIT			BIT2	//POE Mosfet drive Enable;If DC Jack not plugged in,Set to high to enable POE Output
#define APP_MOSFET_DRIVE_BIT			BIT3	//Application Board Mosfet drive Enable;Set to high to enable power from Application board to carrier board,feature not in use currently
#define BOOTLOADER_MODE_SELECT_BIT		BIT7	//Boot Mode selection to bootloader,Set to low to Enable Bootloader
#define DEFAULT_BOOT_MODE_SELECT_BIT	BIT0	//Boot Nortmal mode,System enters to this mode by default
#define FAST_BOOT_DETECT_BIT			BIT2	//When detected, Switch the system to bootloader mode by 
#define LED_TOGGLE_BIT					BIT1	//When detected, Switch the system to bootloader mode by 

#define POE_POWER_SOURCE				0x00
#define APP_BOARD_POWER_SOURCE			0x01

//Global Variable Declarations
unsigned char Power_Source_Selected;
unsigned char Boot_Selection_Not_Completed = TRUE;
//Global functions Declarations
void Configure_GPIOs(void);
void oscillator_calibration(void);


void Configure_GPIOs(void){
	DDRA &= ~POE_PWR_DETECT_BIT;				// Setting Direction as input for POE Power Input detect
	DDRA &= ~APP_PWR_DETECT_BIT;				// Setting Direction as input for Application board/DC Jack Power Input detect
	DDRA |= POE_MOSFET_DRIVE_BIT;				// Setting Direction as output for POE Mosfet driver output
	PORTA &= ~POE_MOSFET_DRIVE_BIT;				// Set to low to disable POE Mosfet driver output
	DDRA |= APP_MOSFET_DRIVE_BIT;				// Setting Direction as output for application board mosfet drive
	PORTA &= ~APP_MOSFET_DRIVE_BIT;				// Set to low to disable Application board Mosfet driver output
	DDRA |= BOOTLOADER_MODE_SELECT_BIT;			// Setting Direction as output for switching to bootloader mode
	PORTA &= ~BOOTLOADER_MODE_SELECT_BIT;		// Set to low to avoid jumping to default boot mode at startup
	DDRB |= DEFAULT_BOOT_MODE_SELECT_BIT;		// Setting Direction as output for switching to normal mode of operation
	PORTB &= ~DEFAULT_BOOT_MODE_SELECT_BIT;		// Set to low to avoid jumping to bootloader at startup
	DDRB &= ~FAST_BOOT_DETECT_BIT;				// Setting Direction as input to detect fast USB plugin detect(J3,USB FastBoot)
	DDRB |= LED_TOGGLE_BIT;						// Setting Direction as input for POE Power plugin detect
}

void oscillator_calibration(void){
	unsigned int osc_trim_value = 0;
	unsigned int osc_calibrated_value = 57;
	while(osc_trim_value < (osc_calibrated_value + 1)){
		while(!(CLKCR & (1 << OSCRDY)));						// Wait until the clock is stable
		OSCCAL0 = ++osc_trim_value;
	}
	_delay_ms(1000);											// Added delay to ensure stability of the oscillator
}

int main(void){
	oscillator_calibration();
	Configure_GPIOs();
	
	//Control Power Mosfet to select power source for the board
	if(PINA & APP_PWR_DETECT_BIT){						//check if carrier board powered through DC Jack
		PORTA &= ~POE_MOSFET_DRIVE_BIT;					// Set to low to disable POE Mosfet driver output
		PORTA |= APP_MOSFET_DRIVE_BIT;					// Set to low to enable Application board Mosfet driver output for carrier board
		Power_Source_Selected  = APP_BOARD_POWER_SOURCE;
	}
	else if (PINA & POE_PWR_DETECT_BIT){				//carrier board powered through POE, Enable POE Mosfet Driver,Disable Application Board Mosfet Driver
		PORTA &= ~APP_MOSFET_DRIVE_BIT;					// Set to low to disable Application board Mosfet driver output
		PORTA |= POE_MOSFET_DRIVE_BIT;					// Set to high to enable POE Mosfet driver output
		Power_Source_Selected  = POE_POWER_SOURCE;
	}
	
	//Boot selection
	if(PINB & FAST_BOOT_DETECT_BIT){					//Check if Fast USB Boot Mode enabled or Volume down button is pressed, if so then go to bootloader mode 
		PORTA |= BOOTLOADER_MODE_SELECT_BIT;			//Set PA7 low to stop entering into bootloader mode(This will set BTN_PHONE_ON_N signal pin of SOM to low)
		PORTB |= DEFAULT_BOOT_MODE_SELECT_BIT;			//Set PB0 high to enter default boot mode(This will set KEY_VOLP_N signal pin of SOM to low)
		_delay_ms(2000);								//wait for sometime to get detected by SOM
		PORTB &= ~DEFAULT_BOOT_MODE_SELECT_BIT;			//clear the pin status to default state(This will set BTN_PHONE_ON_N signal pin of SOM to its default state)	
		PORTA &= ~BOOTLOADER_MODE_SELECT_BIT;			//clear the pin status to default state(This will set KEY_VOLP_N signal pin of SOM to its default state)
	}
	else{//switch to normal boot mode
		PORTA &= ~BOOTLOADER_MODE_SELECT_BIT;		//Set PA7 low to stop entering into bootloader mode(This will not affect KEY_VOLP_N signal pin of SOM from its default state)
		PORTB |= DEFAULT_BOOT_MODE_SELECT_BIT;		//Set PB0 high to enter default boot mode(This will set BTN_PHONE_ON_N signal pin of SOM to low)
		_delay_ms(2000);							//wait for sometime to get detected by SOM
		PORTB &= ~DEFAULT_BOOT_MODE_SELECT_BIT;		//clear the pin status to default state(This will set BTN_PHONE_ON_N signal pin of SOM to its default state)
	}
	//_delay_ms(5000);	
	//PORTB |= DEFAULT_BOOT_MODE_SELECT_BIT;			//Set PB0 high to enter default boot mode(This will set KEY_VOLP_N signal pin of SOM to low)
	//PORTA |= BOOTLOADER_MODE_SELECT_BIT;			//Set PA7 low to stop entering into bootloader mode(This will set BTN_PHONE_ON_N signal pin of SOM to low)
	//_delay_ms(4000);								//wait for sometime to get detected by SOM
	//PORTB &= ~DEFAULT_BOOT_MODE_SELECT_BIT;			//clear the pin status to default state(This will set BTN_PHONE_ON_N signal pin of SOM to its default state)	
	//PORTA &= ~BOOTLOADER_MODE_SELECT_BIT;
	
	while(1){
		if(Power_Source_Selected == POE_POWER_SOURCE){			//check if carrier board was being powered by POE currently
			if(PINA & APP_PWR_DETECT_BIT){						//check if power from application board/DC_Jack is available, if so then use application board power for carrier board
				PORTA |= APP_MOSFET_DRIVE_BIT;					// Set to high to enable Application board Mosfet driver output for carrier board
				PORTA &= ~POE_MOSFET_DRIVE_BIT;					// Set to low to disable POE Mosfet driver output
				Power_Source_Selected  = APP_BOARD_POWER_SOURCE;
			}
			while(1);//do nothing
		}
		else
			while(1);//do nothing in case system powered through DC Jack/Application board
	}	
	return 0; 
}

/*
//Test code for led toggling on PB1
	PORTB  &= ~LED_TOGGLE_BIT;
	_delay_ms(500);
	PORTB  |= LED_TOGGLE_BIT;
	_delay_ms(500); 
*/