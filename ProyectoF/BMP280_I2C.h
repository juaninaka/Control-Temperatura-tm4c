

#ifndef BMP280_I2C_H_
#define BMP280_I2C_H_

//************************Definciones I2C***************************//
#define I2C_MCS_ACK     0x00000008                                  //Transmmitter Acknowledge Enable
#define I2C_MCS_DATACK  0x00000008                                  //Data Acknowledge Enable
#define I2C_MCS_ADRACK  0x00000004                                  //Acknowledge Address
#define I2C_MCS_STOP    0x00000004                                  //Generate STOP
#define I2C_MCS_START   0x00000002                                  //Generate START
#define I2C_MCS_ERROR   0x00000002                                  //Error
#define I2C_MCS_RUN     0x00000001                                  //I2C Master Enable
//******************************************************************//
//*******************Direcciones del BMP280*************************//
#define Ad_BMP280        0x77                                       ///Dirección del esclavo BMP280 *Se hizo corrección antes 0x076
#define Ad_T1_LSB        0x88
#define Ad_T1_MSB        0x89
#define Ad_T2_LSB        0x8A
#define Ad_T2_MSB        0x8B
#define Ad_T3_LSB        0x8C
#define Ad_T3_MSB        0x8D
#define Ad_P1_LSB        0x8E
#define Ad_P1_MSB        0x8F
#define Ad_P2_LSB        0x90
#define Ad_P2_MSB        0x91
#define Ad_P3_LSB        0x92
#define Ad_P3_MSB        0x93
#define Ad_P4_LSB        0x94
#define Ad_P4_MSB        0x95
#define Ad_P5_LSB        0x96
#define Ad_P5_MSB        0x97
#define Ad_P6_LSB        0x98
#define Ad_P6_MSB        0x99
#define Ad_P7_LSB        0x9A
#define Ad_P7_MSB        0x9B
#define Ad_P8_LSB        0x9C
#define Ad_P8_MSB        0x9D
#define Ad_P9_LSB        0x9E
#define Ad_P9_MSB        0x9F
#define Ad_Id            0xD0
#define Ad_reset         0xE0
#define Ad_ctrl_meas     0XF4
#define Ad_config        0xF5
#define Ad_press_msb     0xF7
#define Ad_press_lsb     0xF8
#define Ad_press_xlsb    0xF9
#define Ad_temp_msb      0xFA
#define Ad_temp_lsb      0xFB
#define Ad_temp_xlsb     0xFC
//******************************************************************//
//*********************Declaración de funciones*********************//
int esperar();
void I2C_Init(void);
void BMP280_I2C_Init();
void ReadDataBMP280_I2C();
float Read_Temp_BMP280_I2C();
float Read_Pres_BMP280_I2C();
float Read_Heig_BMP280_I2C(float);
//******************************************************************//

#endif /* BMP280_I2C_H_ */
