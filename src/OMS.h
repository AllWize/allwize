// -----------------------------------------------------------------------------
// Open Metering System Specification
// -----------------------------------------------------------------------------

// C-Field values (master to slave)
#define C_SND_NKE                       0x40    // Master, link reset after communication
#define C_SND_UD                        0x53    // Master, send command (expects ACK or NACK, also 0x73)    
#define C_SND_UD2                       0x43    // Master, send command with subsequent response (expects RSP-UD or NACK)
#define C_REQ_UD1                       0x5A    // Master, alarm request (expects ACK or RSP-UD, also 0x7A)    
#define C_REQ_UD2                       0x5B    // Master, data request (expects RSP-UD, also 0x7B)    
#define C_ACK_MUC                       0x00    // Master, acknowledge of ACC-DMD
#define C_CNF_IR                        0x06    // Master, confirms successful registration

// C-Field values (slave to master)
#define C_SND_NR                        0x44    // Slave, send spontaneous application data without request (expects CNF-IR or SND-NKE)
#define C_SND_IR                        0x46    // Slave, send manually initiated installation data
#define C_ACC_NR                        0x47    // Slave, empty, allows bidirectionallity
#define C_ACC_DMD                       0x48    // Slave, demand to master for application data (expects ACK)
#define C_RSP_UD                        0x08    // Slave, response of app data after a request from master
#define C_NACK                          0x01    // Slave, response in case of error
#define C_ACK_METER                     0x00    // Slave, ACK

// CI-Field values
#define CI_WIZE                         0x20
#define CI_COMMAND_DOWN_NONE            0x51
#define CI_SELECT_DOWN_NONE             0x52
#define CI_COMMAND_DOWN_SHORT           0x5A
#define CI_COMMAND_DOWN_LONG            0x5B
#define CI_APP_RESPONSE_UP_NONE         0x66
#define CI_APP_RESPONSE_UP_SHORT        0x67
#define CI_APP_RESPONSE_UP_LONG         0x68
#define CI_RESPONSE_UP_LONG             0x72
#define CI_RESPONSE_UP_NONE             0x78
#define CI_RESPONSE_UP_SHORT            0x7A
#define CI_TRANSPORT_DOWN_LONG          0x80
#define CI_TRANSPORT_UP_SHORT           0x8A
#define CI_TRANSPORT_UP_LONG            0x8B

// Device types
#define METER_OTHER			            0x00
#define METER_OIL			            0x01
#define METER_ELECTRICITY			    0x02
#define METER_GAS			            0x03
#define METER_HEAT			            0x04
#define METER_STEAM			            0x05
#define METER_WARMWATER			        0x06
#define METER_WATER			            0x07
#define METER_HEATCOSTALLOCATOR			0x08
#define METER_COMPRESSEDAIR			    0x09
#define METER_COOLINGLOADMETEROUTLET	0x0A
#define METER_COOLINGLOADMETERINLET		0x0B
#define METER_HEATINLET			        0x0C
#define METER_HEATCOOLINGLOADMETER		0x0D
#define METER_BUSSYSTEMCOMPONENT		0x0E
#define METER_UNKNOWNMEDIUM			    0x0F
#define METER_CALORIFICVALUE			0x14
#define METER_HOTWATER			        0x15
#define METER_COLDWATER			        0x16
#define METER_DUALREGISTER			    0x17
#define METER_PRESSURE			        0x18
#define METER_ADCONVERTER			    0x19
#define METER_SMOKEDETECTOR			    0x1A
#define METER_ROOMSENSOR			    0x1B
#define METER_GASDETECTOR			    0x1C
#define METER_ELECTRICITYBREAKER		0x20
#define METER_VALVE			            0x21
#define METER_CUSTOMERUNIT			    0x25
#define METER_WASTEWATER			    0x28
#define METER_GARBAGE			        0x29
#define METER_COMMUNICATIONCONTROLLER	0x31
#define METER_UNIDIRECTIONALREPEATER	0x32
#define METER_BIDIRECTIONALREPEATER	    0x33
#define METER_RADIOCONVERTERSYSTEMSIDE	0x36
#define METER_RADIOCONVERTERMETERSIDE	0x37

// Data Information Field
#define DIF_NONE						0x00
#define DIF_INT8						0x01
#define DIF_INT16						0x02
#define DIF_INT24						0x03
#define DIF_INT32						0x04
#define DIF_FLOAT32						0x05
#define DIF_INT48						0x06
#define DIF_INT64						0x07
#define DIF_READOUT						0x08
#define DIF_BCD2						0x09
#define DIF_BCD4						0x0a
#define DIF_BCD6						0x0b
#define DIF_BCD8						0x0c
#define DIF_VARLEN						0x0d
#define DIF_BCD12						0x0e
#define DIF_SPECIAL						0x0f