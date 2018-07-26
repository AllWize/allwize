// -----------------------------------------------------------------------------
// RC1701HP
// -----------------------------------------------------------------------------

// Command keys
#define CMD_AUTO_MESSAGE_FLAGS          'A'
#define CMD_BIND                        'B'
#define CMD_CHANNEL                     'C'
#define CMD_ENCRYPT                     'E'
#define CMD_CONTROL_FIELD               'F'
#define CMD_MBUS_MODE                   'G'
#define CMD_INSTALL_MODE                'I'
#define CMD_KEY_REGISTER                'K'
#define CMD_LIST_BINDING                'L'
#define CMD_WRITE_MEMORY                'M'
#define CMD_ACCESS_NUMBER               'N'
#define CMD_READ_AUTO_MESSAGE_FLAGS     'O'
#define CMD_RF_POWER                    'P'
#define CMD_QUALITY                     'Q'
#define CMD_READ_MAILBOX                'R'
#define CMD_RSSI                        'S'
#define CMD_DESTINATION                 'T'
#define CMD_TEMPERATURE                 'U'
#define CMD_VOLTAGE                     'V'
#define CMD_WRITE_MAILBOX               'W'
#define CMD_READ_MEMORY                 'Y'
#define CMD_SLEEP                       'Z'
#define CMD_TEST_MODE_0                 '0'

// Memory addresses
#define MEM_CHANNEL                     0x00
#define MEM_RF_POWER                    0x01
#define MEM_DATA_RATE                   0x02
#define MEM_MBUS_MODE                   0x03
#define MEM_SLEEP_MODE                  0x04
#define MEM_RSSI_MODE                   0x05
#define MEM_PREAMBLE_LENGTH             0x0A
#define MEM_TIMEOUT                     0x10
#define MEM_NETWORK_ROLE                0x12
#define MEM_MAILBOX                     0x16
#define MEM_MANUFACTURER_ID             0x19
#define MEM_UNIQUE_ID                   0x1B
#define MEM_VERSION                     0x1F
#define MEM_DEVICE                      0x20
#define MEM_UART_BAUD_RATE              0x30
#define MEM_UART_FLOW_CTRL              0x35
#define MEM_DATA_INTERFACE              0x36
#define MEM_CONFIG_INTERFACE            0x37
#define MEM_FREQ_CAL                    0x39
#define MEM_LED_CONTROL                 0x3A
#define MEM_CONTROL_FIELD               0x3B
#define MEM_RX_TIMEOUT                  0x3C
#define MEM_INSTALL_MODE                0x3D
#define MEM_ENCRYPT_FLAG                0x3E
#define MEM_DECRYPT_FLAG                0x3F
#define MEM_DEFAULT_KEY                 0x40
#define MEM_PART_NUMBER_OLD             0x61
#define MEM_SERIAL_NUMBER_OLD           0x71
#define MEM_PART_NUMBER_NEW             0x89
#define MEM_SERIAL_NUMBER_NEW           0xA9

// Channels
#define CHANNEL_01                      0x01
#define CHANNEL_02                      0x02
#define CHANNEL_03                      0x03
#define CHANNEL_04                      0x04
#define CHANNEL_05                      0x05
#define CHANNEL_06                      0x06
#define CHANNEL_07                      0x07
#define CHANNEL_08                      0x08
#define CHANNEL_09                      0x09
#define CHANNEL_10                      0x10
#define CHANNEL_11                      0x11
#define CHANNEL_12                      0x12
#define CHANNEL_13                      0x13
#define CHANNEL_14                      0x14
#define CHANNEL_15                      0x15
#define CHANNEL_16                      0x16
#define CHANNEL_17                      0x17
#define CHANNEL_18                      0x18
#define CHANNEL_19                      0x19
#define CHANNEL_20                      0x20
#define CHANNEL_21                      0x21
#define CHANNEL_22                      0x22
#define CHANNEL_23                      0x23
#define CHANNEL_24                      0x24
#define CHANNEL_25                      0x25
#define CHANNEL_26                      0x26
#define CHANNEL_27                      0x27
#define CHANNEL_28                      0x28
#define CHANNEL_29                      0x29
#define CHANNEL_30                      0x30
#define CHANNEL_31                      0x31
#define CHANNEL_32                      0x32
#define CHANNEL_33                      0x33
#define CHANNEL_34                      0x34
#define CHANNEL_35                      0x35
#define CHANNEL_36                      0x36
#define CHANNEL_37                      0x37
#define CHANNEL_38                      0x38
#define CHANNEL_39                      0x39
#define CHANNEL_40                      0x40
#define CHANNEL_41                      0x41

// Data rates
#define DATARATE_2400bps                0x01
#define DATARATE_4800bps                0x02
#define DATARATE_19200bps               0x04
#define DATARATE_6400bps                0x05

// Power modes
#define POWER_14dBm                     0x01
#define POWER_17dBm                     0x02
#define POWER_20dBm                     0x03
#define POWER_24dBm                     0x04
#define POWER_27dBm                     0x05

// MBus modes
#define MBUS_MODE_S2                    0x00
#define MBUS_MODE_T1                    0x01
#define MBUS_MODE_T2                    0x02
#define MBUS_MODE_S1                    0x03
#define MBUS_MODE_R                     0x04
#define MBUS_MODE_T1_C                  0x0A
#define MBUS_MODE_T2_C                  0x0B
#define MBUS_MODE_N2                    0x10
#define MBUS_MODE_N1                    0x11
#define MBUS_MODE_OSP                   0x12

// Operation modes
#define INSTALL_MODE_NORMAL             0x00
#define INSTALL_MODE_INSTALL            0x01
#define INSTALL_MODE_HOST               0x02

// Sleep modes
#define SLEEP_MODE_DISABLE              0x00
#define SLEEP_MODE_AFTER_TX             0x01
#define SLEEP_MODE_AFTER_TX_RX          0x03
#define SLEEP_MODE_AFTER_TX_TIMEOUT     0x05
#define SLEEP_MODE_AFTER_TX_RX_TIMEOUT  0x07

// Network roles
#define NETWORK_ROLE_SLAVE              0x00
#define NETWORK_ROLE_MASTER             0x01
#define NETWORK_ROLE_REPEATER           0x02

// Timeouts
#define TIMEOUT_32MS                    0x01
#define TIMEOUT_48MS                    0x02
#define TIMEOUT_64MS                    0x03
#define TIMEOUT_2S                      0x7C
#define TIMEOUT_4S                      0xF9

// LED Control
#define LED_CONTROL_DISABLED            0x00
#define LED_CONTROL_RX_TX               0x01
#define LED_CONTROL_UART_RF_IDLE        0x02

// Encrypt/Decrypt flags
#define ENCRYPT_DISABLED                0x00
#define ENCRYPT_ENABLED                 0x01
#define ENCRYPT_ENABLED_CRC             0x03

// Data interface
#define DATA_INTERFACE_ID_ADDR          0x00
#define DATA_INTERFACE_APP_ONLY         0x01
#define DATA_INTERFACE_APP_ACK          0x03
#define DATA_INTERFACE_START_STOP       0x04
#define DATA_INTERFACE_CRC              0x08
#define DATA_INTERFACE_CRC_START_STOP   0x0C

// Preamble Length
#define PREAMBLE_FORMAT_A               0x00
#define PREAMBLE_FORMAT_B               0x02
