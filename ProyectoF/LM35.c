/* Programa que  configura el ADC para medir la temperatura
 * con el LM35
 */
//******************Declaración de Librerias ***********************//
#include<stdint.h>
#include<stdbool.h>
#include <math.h>
#include "inc/tm4c1294ncpdt.h"
#include "LM35.h"
//******************************************************************//
//************* Declaración de variables globales ******************//
int DATO;
float TEMP;
int Cuenta_ADC_ISR=0;
//******************************************************************//
//******************Función que configura el ADC0 ******************//
void LM35_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x0010;                                    // Habilita reloj para Puerto E
    while( (SYSCTL_PRGPIO_R & 0x010) ==0);
    GPIO_PORTE_AHB_DIR_R &= ~0x10;                                  // PE4 como entrada
    GPIO_PORTE_AHB_AFSEL_R |= 0x10;                                 // Función Alterna de PE4
    GPIO_PORTE_AHB_DEN_R &= 0x10;                                   // Deshabilita Función Digital de PE4
    GPIO_PORTE_AHB_AMSEL_R |= 0x10;                                 // Habilita Función Analógica de PE4

    SYSCTL_RCGCADC_R = 0x01;                                        // Habilita reloj del ADC0
    while((SYSCTL_PRADC_R&0x01)==0);
    ADC0_PC_R = 0x01;                                               // Configura para 125Ks/s
    ADC0_SSPRI_R = 0x0123;                                          // SS3 con la más alta prioridad
    ADC0_ACTSS_R = 0x0000;                                          // Deshabilita SS3 antes de cambiar configuración de registros
    ADC0_SAC_R = 0X6;                                               // 64x Hardware oversampling
    ADC0_EMUX_R = 0x0000;                                           // Iniciar muestreo por software
    ADC0_SSEMUX3_R = 0x00;                                          // Rango de entradas para bit EMUX0: AIN[15:0]
    ADC0_SSMUX3_R = 0X9;                                            // Para bit MUX0: Canal AIN9 => PE4
    ADC0_SSCTL3_R = 0x0006;                                         // Entrada externa, IN, Ultima muestra, no diferencial
    ADC0_ISC_R = 0x0008;                                            // Limpia la bandera RIS del ADC0
    ADC0_IM_R = 0x0008;                                             // Habilita interrupciones de SS3
    ADC0_ACTSS_R |= 0x0008;                                         // Habilita SS3

    SYSCTL_PLLFREQ0_R |= SYSCTL_PLLFREQ0_PLLPWR;                    // Enciende PLL
    while((SYSCTL_PLLSTAT_R&0x01)==0);                              // Espera a que el PLL fije su frecuencia
    SYSCTL_PLLFREQ0_R &= ~SYSCTL_PLLFREQ0_PLLPWR;                   // Apaga PLL
    //ADC0 SS3 -> #17
    NVIC_EN0_R |= 0X20000;                                          // Habilita interrupción general del secuenciador SS3 de ADC0
    NVIC_PRI4_R &=~0x0000E000;
}
//******************************************************************//
//*************** Funcion que inicia muestreo del ADC **************//
float LM35_ReadTemp(void){
     Cuenta_ADC_ISR=0;
     ADC0_PSSI_R = 0x0008;                                          // Inicia conversión del SS3
     while(Cuenta_ADC_ISR==0){};
     return TEMP;
}
//******************************************************************//
//*************** Servicio de rutina de interrupción ***************//
void ISR_ADC0_SS3(void){
int ADC_Dato;
float Voltaje;

ADC_Dato = (ADC0_SSFIFO3_R & 0xFFF);                                //Guarda FIFO3
Voltaje = ADC_Dato * (3.29/4095.0);                                 //Convierte a voltaje
TEMP=Voltaje*100;                                                   //Convierte a temperatura
Cuenta_ADC_ISR=1;                                                   //Condicion de salida LM35_ReadTemp
ADC0_ISC_R |= 0x0008;                                               //Limpia bandera de interrupción
}
//******************************************************************//
