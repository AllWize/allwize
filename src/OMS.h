/**
 * @file OMS.h
 * Open Metering System Specification header file
 */

// C-Field values
#define C_SND_NKE                       0x40    // Master, link reset after communication
#define C_SND_UD2                       0x43    // Master, send command with subsequent response (expects RSP-UD or NACK)
#define C_SND_UD                        0x53    // Master, send command (expects ACK or NACK, also 0x73)    
#define C_REQ_UD1                       0x5A    // Master, alarm request (expects ACK or RSP-UD, also 0x7A)    
#define C_REQ_UD2                       0x5B    // Master, data request (expects RSP-UD, also 0x7B)    
#define C_CNF_IR                        0x06    // Master, confirms successful registration
#define C_ACK                           0x00    // Master or slave
#define C_SND_NR                        0x44    // Slave, send spontaneous application data without request (expects CNF-IR or SND-NKE)
#define C_SND_IR                        0x46    // Slave, send manually initiated installation data
#define C_ACC_NR                        0x47    // Slave, empty, allows bidirectionallity
#define C_ACC_DMD                       0x48    // Slave, demand to master for application data (expects ACK)
#define C_NACK                          0x01    // Slave, response in case of error
#define C_RSP_UD                        0x08    // Slave, response of app data after a request from master

#define C_WIZE_INSTPING                 0x46    // C_SND_IR
#define C_WIZE_INSTPONG                 0x06    // C_CNF_IR
#define C_WIZE_DATA                     0x44    // C_SND_NR
#define C_WIZE_COMMAND                  0x43    // C_SND_UD2
#define C_WIZE_RESPONSE                 0x08    // C_RSP_UD

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
#define CI_RESPONSE_UP_SHORT            0x7A

