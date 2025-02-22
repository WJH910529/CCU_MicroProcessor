/****************************************************************************
 * @file     main.c
 * @version  V2.0
 * $Revision: 1 $
 * $Date: 14/12/08 11:49a $
 * @brief    Perform A/D Conversion with ADC single mode.
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "NUC100Series.h"
#define PLL_CLOCK       50000000

/*---------------------------------------------------------------------------------------------------------*/
/* Define Function Prototypes                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void SYS_Init(void);
void UART0_Init(void);
void AdcSingleModeTest(void);

int g_InitValueRed=0;
int g_InitValueBlue=0;
int g_InitValueGreen=0;

int g_InitValueRed_Black=0;
int g_InitValueBlue_Black=0;

//--------------------------------//
int32_t g_diff_red=0,g_diff_blue=0;
int32_t red=0;
int32_t blue=0;
//--------------------------------//

uint8_t result[500];//For final to print.
int result_byte=0;

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32AdcIntFlag;

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC 22.1184MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external XTAL 12MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable ADC module clock */
    CLK_EnableModuleClock(ADC_MODULE);

    /* Select UART module clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_PLL, CLK_CLKDIV_UART(1));

    /* ADC clock source is 22.1184MHz, set divider to 7, ADC clock is 22.1184/7 MHz */
    CLK_SetModuleClock(ADC_MODULE, CLK_CLKSEL1_ADC_S_HIRC, CLK_CLKDIV_ADC(7));

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);
    SYS->GPB_MFP |= SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD;

    /* Disable the GPA0 - GPA3 digital input path to avoid the leakage current. */
    GPIO_DISABLE_DIGITAL_PATH(PA, 0xF);

    /* Configure the GPA0 - GPA3 ADC analog input pins */
    SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA0_Msk | SYS_GPA_MFP_PA1_Msk | SYS_GPA_MFP_PA2_Msk | SYS_GPA_MFP_PA3_Msk) ;
    SYS->GPA_MFP |= SYS_GPA_MFP_PA0_ADC0 | SYS_GPA_MFP_PA1_ADC1 | SYS_GPA_MFP_PA2_ADC2 | SYS_GPA_MFP_PA3_ADC3 ;
    SYS->ALT_MFP1 = 0;

}

/*---------------------------------------------------------------------------------------------------------*/
/* Init UART                                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void UART0_Init()
{
    /* Reset IP */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
}
/*---------------------------------------------------------------------------------------------------------*/
/* ADC interrupt handler                                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
void ADC_IRQHandler(void)
{
    g_u32AdcIntFlag = 1;
    ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT); /* clear the A/D conversion flag */
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/

void Initial_Black()
{
		uint32_t u32ChannelCount;
		int32_t  i32ConversionData;
		int i,loop=50;
		g_InitValueRed_Black=5000;
		g_InitValueBlue_Black=5000;
	
		for(i=0;i<loop;i++)
		{		
						ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_CONTINUOUS, 0xF);
			
            /* Power on ADC module */
            ADC_POWER_ON(ADC);
			
            /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);
			
            /* Enable the ADC interrupt */
						ADC_EnableInt(ADC, ADC_ADF_INT);
            NVIC_EnableIRQ(ADC_IRQn);

            /* Reset the ADC interrupt indicator and Start A/D conversion */
            //g_u32AdcIntFlag = 0;

            ADC_START_CONV(ADC);
						
           /* Wait conversion done */
            while(!ADC_GET_INT_FLAG(ADC, ADC_ADF_INT));

						/* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

            /* Get the conversion result of the ADC channel 2 */
						
						   for(u32ChannelCount = 1; u32ChannelCount < 2; u32ChannelCount++)
            {
                i32ConversionData = ADC_GET_CONVERSION_DATA(ADC, u32ChannelCount);
                printf("Sample value of channel %d(red) :0x%X (%d)\n", u32ChannelCount, i32ConversionData, i32ConversionData);
								if(i32ConversionData  < g_InitValueRed_Black )
								{
									printf("here\n");
									 g_InitValueRed_Black=i32ConversionData;
								}
						}
						
           /* Wait conversion done */
            while(!ADC_GET_INT_FLAG(ADC, ADC_ADF_INT));
						
						/* Stop A/D conversion */
           ADC_STOP_CONV(ADC);
			
						for(u32ChannelCount = 2; u32ChannelCount < 3; u32ChannelCount++)
            {
                i32ConversionData = ADC_GET_CONVERSION_DATA(ADC, u32ChannelCount);
                printf("Sample value of channe2 %d(blue):0x%X (%d)\n", u32ChannelCount, i32ConversionData, i32ConversionData);
								if(i32ConversionData < g_InitValueBlue_Black)
								{
									 g_InitValueBlue_Black=i32ConversionData;
								}
            }
						
					  /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);
						printf("---------------------------------------------\n");
		}
		
		printf("\n--------------------Init Done--------------------\n");
		printf("Red Initial value of resistence for No light: %d\n",g_InitValueRed_Black);
		printf("Blue Initial value of resistence for No light: %d\n",g_InitValueBlue_Black);
		
}

void InitValue()
{
		uint32_t u32ChannelCount;
		int32_t  i32ConversionData;
		int i,loop=50;
		for(i=0;i<loop;i++)
		{		
						ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_CONTINUOUS, 0xF);
			
            /* Power on ADC module */
            ADC_POWER_ON(ADC);
			
            /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);
			
            /* Enable the ADC interrupt */
						ADC_EnableInt(ADC, ADC_ADF_INT);
            NVIC_EnableIRQ(ADC_IRQn);

            /* Reset the ADC interrupt indicator and Start A/D conversion */
            //g_u32AdcIntFlag = 0;

            ADC_START_CONV(ADC);
						
           /* Wait conversion done */
            while(!ADC_GET_INT_FLAG(ADC, ADC_ADF_INT));

						/* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

            /* Get the conversion result of the ADC channel 2 */
						
						   for(u32ChannelCount = 1; u32ChannelCount < 2; u32ChannelCount++)
            {
                i32ConversionData = ADC_GET_CONVERSION_DATA(ADC, u32ChannelCount);
                printf("Sample value of channel %d(red) :0x%X (%d)\n", u32ChannelCount, i32ConversionData, i32ConversionData);
								g_InitValueRed += i32ConversionData;
						}
						
           /* Wait conversion done */
            while(!ADC_GET_INT_FLAG(ADC, ADC_ADF_INT));
						
						/* Stop A/D conversion */
           ADC_STOP_CONV(ADC);
			
						for(u32ChannelCount = 2; u32ChannelCount < 3; u32ChannelCount++)
            {
                i32ConversionData = ADC_GET_CONVERSION_DATA(ADC, u32ChannelCount);
                printf("Sample value of channe2 %d(blue):0x%X (%d)\n", u32ChannelCount, i32ConversionData, i32ConversionData);
								g_InitValueBlue += i32ConversionData;
            }
						
					  /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);
					
						printf("---------------------------------------------\n");
		}
		g_InitValueRed=g_InitValueRed/loop;
		g_InitValueBlue=g_InitValueBlue/loop;
		
		printf("\n--------------------Init Done--------------------\n");
		printf("Initial value of resistence for Red: %d\n",g_InitValueRed);
		printf("Initial value of resistence for Blue: %d\n",g_InitValueBlue);

}


void 	Get_Sample()
{
						uint32_t u32ChannelCount;
						int32_t  i32ConversionData;
		
						ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_CONTINUOUS, 0xF);

            /* Power on ADC module */
            ADC_POWER_ON(ADC);

            /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

            /* Enable the ADC interrupt */
						ADC_EnableInt(ADC, ADC_ADF_INT);
            NVIC_EnableIRQ(ADC_IRQn);

            /* Reset the ADC interrupt indicator and Start A/D conversion */
            //g_u32AdcIntFlag = 0;

            ADC_START_CONV(ADC);
						
           /* Wait conversion done */
            while(!ADC_GET_INT_FLAG(ADC, ADC_ADF_INT));

						/* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

            /* Get the conversion result of the ADC channel 2 */
						
						
						 for(u32ChannelCount = 1; u32ChannelCount < 2; u32ChannelCount++)
            {                                                    
                i32ConversionData = ADC_GET_CONVERSION_DATA(ADC, u32ChannelCount);
                //printf("Conversion result of channel %d(red) :0x%X (%d)\n", u32ChannelCount, i32ConversionData, i32ConversionData);
								red=i32ConversionData;
								//CLK_SysTickDelay(500000);
            }
           /* Wait conversion done */
            while(!ADC_GET_INT_FLAG(ADC, ADC_ADF_INT));
						
						/* Stop A/D conversion */
           ADC_STOP_CONV(ADC);
			
						for(u32ChannelCount = 2; u32ChannelCount < 3; u32ChannelCount++)
            {
                i32ConversionData = ADC_GET_CONVERSION_DATA(ADC, u32ChannelCount);
                //printf("Conversion result of channe2 %d(blue):0x%X (%d)\n\n", u32ChannelCount, i32ConversionData, i32ConversionData);
								blue=i32ConversionData;
								//CLK_SysTickDelay(500000);
            }
					  /* Clear the A/D interrupt flag for safe */
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);
										
						g_diff_red=red - g_InitValueRed;
						g_diff_blue=blue - g_InitValueBlue;
}


void 	light_Cmp(){
	
		int i=0,j=0,k=0;
		int black_flag=0;
		int recevie_char;
		//int error_flag=0;
		uint8_t received[8];//For one byte;
		uint8_t  u8Option;
		//int32_t delaytime;
    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|                      ADC Light_Resistance 		code                     |\n");
    printf("+----------------------------------------------------------------------+\n");
	
		//restart:
		//i=0;
		//printf("%d\n",i);
    while(1)  
    {							
	
//--------------------------First Sample-----------------------------------------------------------------------;		
						red=0,blue=0,j=0;
						CLK_SysTickDelay(700000);	//not sure why need these
						CLK_SysTickDelay(180000);	
						CLK_SysTickDelay(500000);
						
						Get_Sample();
//--------------------------Sample Done-----------------------------------------------------------------------;		
						
						
//--------------------------Detect status of black flag--------------------------------------------------;
						if(red >= g_InitValueRed_Black-200  &&  blue >= g_InitValueRed_Black-200)
						{
							printf("Black_flag_1st\n");
							CLK_SysTickDelay(180000);
							CLK_SysTickDelay(500000);
							CLK_SysTickDelay(500000);
							CLK_SysTickDelay(700000);
							CLK_SysTickDelay(180000);
							CLK_SysTickDelay(500000);// Servo2 Confirm to open
							
							//I think need more delay here, but it seens not very important (because the code line:386). 
							
							black_flag=1;
						}			
//-------------------------------------------------------------------------------------------------------;			
						
					while(black_flag)
					{
						printf("Wait Sender to Sample\n");
						CLK_SysTickDelay(700000);
						CLK_SysTickDelay(180000);
						CLK_SysTickDelay(500000);							
//-----------------------------------Sample again-----------------------------------------------------;
						printf("Sample\n");	
						red=0,blue=0,j=0;
						
						Get_Sample();
						
						printf("Sample Done\n");	
//---------------------------------Sample Done---------------------------------------------------//////////////;			
													
							if( (red >= g_InitValueRed_Black-200)  &&  (blue >= g_InitValueRed_Black-200))
								continue;
												
							if(g_diff_red < g_diff_blue)
							{
									CLK_SysTickDelay(500000);
									//CLK_SysTickDelay(500000);
									//CLK_SysTickDelay(500000);
									printf("The %dth Recevied Red--> signal 1\n\n",i);
									received[i]=1;
									
									//CLK_SysTickDelay(500000);
									//CLK_SysTickDelay(500000);	
									//CLK_SysTickDelay(1000000);
							}
							else if(g_diff_blue < g_diff_red)
							{
									CLK_SysTickDelay(500000);
									//CLK_SysTickDelay(500000);
									//CLK_SysTickDelay(500000);
									printf("The %dth Recevied Blue --> signal 0\n\n",i);
									received[i]=0;
									
									//CLK_SysTickDelay(500000);
									//CLK_SysTickDelay(500000);	
									//CLK_SysTickDelay(1000000);
							}			
						
							i++;
							recevie_char=0;
							
	//---------------------------------Print 0/1---------------------------------------------------//////////////;									
							if(i==8)
							{
									printf("Receive:");
									for(j=0;j<8;j++)
									{
											recevie_char += received[j]*pow(2,j);
											printf("%d ",received[j]);		
									}
									
									result[result_byte]=recevie_char;
																	
									if(recevie_char>=32 && recevie_char<=126){
											printf("Received char:%c ",recevie_char);
									}							
									else{
											printf("Received Wrong ASCII ");
									}				
									
									if(result[result_byte]==35)//ASCII #=35 
									{
											printf("\nWhole received:");
											for(k=0;k<=result_byte;k++)
											{
													printf("%c",result[k]);
											}
											printf("\n---- ---------------END---------------------------\n");		
											i=0;
											black_flag=0;
											result_byte=0;
											
											while(1){
												printf("Press '5' to continue or Press '4' to END\n");
												u8Option=getchar();
												if(u8Option=='5'){
															break;
													}
												else if(u8Option=='4'){
														printf("Bye~\n");
														return ;
												}
											}
											
									}
									result_byte++;
									printf("\n------------------------------------------------------\n");													
									//CLK_SysTickDelay(1000000);
									i=0; //reset i
									//error_flag=0;//reset error_flag
							}			
							black_flag=0;	//reset black_flag
					}//while(black_flag)     				
    }//while(1)
}
//------------------------------------------------------------------------------------------;		


int main(void)
{
		uint8_t  u8Option;
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();

    /*---------------------------------------------------------------------------------------------------------*/
    /* SAMPLE CODE                                                                                             */
    /*---------------------------------------------------------------------------------------------------------*/

		GPIO_SetMode(PC,BIT12,GPIO_PMD_OUTPUT);
		GPIO_SetMode(PC,BIT13,GPIO_PMD_OUTPUT);
		GPIO_SetMode(PC,BIT14,GPIO_PMD_OUTPUT);
		GPIO_SetMode(PC,BIT15,GPIO_PMD_OUTPUT);
		printf("Select input mode:\n");
    printf("  [1] Initial Red/Blue\n");
		printf("  [2] Initial Black\n");
    printf("  [3] Start!\n");
    u8Option = getchar();
    //printf("\nSystem clock rate: %d Hz", SystemCoreClock);
		
		while(1){
			u8Option = getchar();
			
			if(u8Option=='1'){
			printf("---------Initial red/blue for a few second\n---------\n");
			InitValue();	
			}
			else if(u8Option=='2'){
			printf("---------Initial black for a few second\n---------\n");
			Initial_Black();
			}
			else if(u8Option=='3'){
			printf("\n---------Start Receive---------\n");
			light_Cmp();
			}
	}

	/*
    ADC_Close(ADC);

    CLK_DisableModuleClock(ADC_MODULE);

    NVIC_DisableIRQ(ADC_IRQn);
    printf("\nExit ADC sample code\n");
	*/
	//    while(1);

}



