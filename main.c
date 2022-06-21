 #include "derivative.h" /* include peripheral declarations */

unsigned int reps = 0;
unsigned int cycles = 0;
unsigned int distance = 0;
//Function declarations
void send_inf(void);
int getDigitsize(int cycles);
void loadInfo(unsigned char msg[], int digit_size, int cycles, unsigned char msg_2[], int digit_size_2, int distance);

//Functions

void send_inf(void)
{
	int digit_size = getDigitsize(cycles);
	unsigned char msg[digit_size];
	int digit_size_2 = getDigitsize(distance);
	unsigned char msg_2[digit_size_2];
	
	loadInfo(msg,digit_size,cycles,msg_2,digit_size_2,distance);
	
	unsigned char i = 0;
	unsigned char n = 0;
	
	while ((UART0_S1&(1<<7)) == 0 );
				UART0_D = '_';
				
	while (i < digit_size)
	{
		while ((UART0_S1&(1<<7)) == 0 );
		UART0_D = msg[i++];
	}
	
	while ((UART0_S1&(1<<7)) == 0 );
					UART0_D = '_';
			
	while (n < digit_size_2)
	{
		while ((UART0_S1&(1<<7))==0);
		UART0_D = msg_2[n++];
	}
	
	
}

//Function that has an integer as a parameter and returns an other integer
int getDigitsize(int cycles)
{
	
	int digits = 0;
	int divider = 1;
	if(cycles == 0)
	{
		return 1;
	}
	while(cycles/divider !=0)
	{
		digits++;
		divider *=10;
		
	}
	return digits;
}

//function that modifies the array with desired ascci msg
void loadInfo(unsigned char msg[], int digit_size, int cycles, unsigned char msg_2[], int digit_size_2, int distance)
{
	int ind;
	for(ind=1; (ind <= digit_size); ind++)
	{
		msg[digit_size-ind] = (cycles % 10)+ 48;
		cycles /= 10;  
	}
	
	int ind_2;
		for(ind_2=1; (ind_2 <= digit_size_2); ind_2++)
		{
			msg_2[digit_size_2 - ind_2] = (distance % 10)+ 48;
			distance /= 10;
		}
}

void LPTimer_IRQHandler()
{
	LPTMR0_CSR=(1<<7)+(1<<6)+(1<<0);//borrar bandera, mantener habilitada interrupcion y mantener timer enabled
	GPIOE_PDOR^=(1<<1);			//GPIOE_PDOR=GPIOE_PDOR^0x00040000;//logical OR ^
	send_inf();
}

void PORTA_IRQHandler ()
{
	PORTA_PCR5=(1<<24)|(1<<8)|(9<<16);
	reps++;
	if (reps%4==0)
		{
			cycles++;
			distance = cycles*32;
		}
}

void clk_init (void)
{
// FIRC = 4 MHz. BusClk = 4 MHz
// UART0: FIRC. UART1: BusClk. UART2: BusClk. TPM: FIRC. IIC: BusClk

MCG_C1|=(1<<6) + (1<<1);	//MCGOUTCLK : IRCLK. CG: Clock gate, MCGIRCLK enable pag 116
MCG_C2|=1;					//Mux IRCLK : FIRC (4 MHz) pag 116
MCG_SC=0;					//Preescaler FIRC 1:1 pag 116

SIM_CLKDIV1=0;				//OUTDIV4=OUTDIV1= 1:1 pag 116. Busclk 4 MHz
SIM_SOPT2|=15<<24;			//Seleccion MCGIRCLK tanto para UART0 como para TPM
SIM_SCGC5 = 0;
}

void UART_init(void)
{
	SIM_SCGC5|=(1<<9);		//clk interfaz PORTA
	PORTA_PCR1=(2<<8);		//UART0 Rx		
	PORTA_PCR2=(2<<8);		//UART0 Tx
	
	SIM_SCGC4=(1<<10);		//clk interfaz UART0
	UART0_BDH=0;
	UART0_BDL=26;			//ecuaciÃ³n para definir baud rate
	UART0_C2=12;		// TE=RE=1
}

void LPTimer_init(void)
{
	SIM_SCGC5|=(1<<13)+(1<<0);	//Interface clock PORTB and LPTMR
	PORTE_PCR1 = (1<<8);		//GPIO
		
	GPIOE_PDDR=(1<<1);			//0b0000 0000 0000 0000 0000 0000 0000 0010 (Output)
			
	LPTMR0_CMR=100-1;				//parametro de entrada
	LPTMR0_PSR=5;					//101 : bypass del preescaler, seleccion de LPO (1 KHz)
			
	LPTMR0_CSR=(1<<0) + (1<<6);		//Timer enabled = 1. habilitar Intrrupcion LPTMR
	NVIC_ISER|=(1<<28); 			//Habilitar Interrupcion LPTMR desde el NVIC (pag53-540
}

void PortA_init(void)
{
	SIM_SCGC5|=(1<<9);
	PORTA_PCR5 =(1<<8)|(9<<16);  //GPIO Y CONFIGURACIÓN INTERRUPCIOÓN EN FLACO de subida
	NVIC_ISER|=(1<<30);
}


int main(void)
{
	clk_init();
	UART_init();
	LPTimer_init();
	PortA_init();
	
	while(1);
	
	return 0;
}
