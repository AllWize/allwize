/**
 * @file RC1701HP.h
 * RC1701HP command and memory codes header file
 */

// Module signature
#define MODULE_SIGNATURE                "RC1701"

// Module types
enum {
    
    MODULE_UNKNOWN,
    
    MODULE_MBUS4,
    MODULE_OSP,
    MODULE_WIZE,

    MODULE_MAX

};

// Special characters
#define END_OF_RESPONSE                 '>'
#define START_BYTE                      0x68
#define STOP_BYTE                       0x16

// Special command keys
#define CMD_ENTER_CONFIG                (char) 0x00
#define CMD_EXIT_CONFIG                 (char) 0x58
#define CMD_NO_RESPONSE                 (char) 0xFB
#define CMD_KEY_CHALLENGE               (char) 0xFC
#define CMD_IDLE_ENABLE_RF              (char) 0xFD
#define CMD_IDLE_DISABLE_RF             (char) 0xFF
#define CMD_AWAKE                       (char) 0xFF     // Deprecated
#define CMD_EXIT_MEMORY                 (char) 0xFF     // Deprecated

// Command keys
#define CMD_AUTO_MESSAGE_FLAGS          'A'
#define CMD_BIND                        'B'
#define CMD_CHANNEL                     'C'
#define CMD_PING                        'D'
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
#define CMD_RSSI_CONTINUOUS             's'
#define CMD_TEST_MODE_0                 '0'

// Memory slots
// These are abstract memory slots 
// that are mapped to actual addresses in the MEM_ADDRESS array
enum {
    
    MEM_CHANNEL_TX,
    MEM_CHANNEL = MEM_CHANNEL_TX,
    MEM_CHANNEL_RX,
    MEM_RF_POWER,
    MEM_DATA_RATE_TX,
    MEM_DATA_RATE = MEM_DATA_RATE_TX,
    MEM_DATA_RATE_RX,

    MEM_MBUS_MODE,
    MEM_SLEEP_MODE,
    MEM_RSSI_MODE,
    MEM_PA_TABLE_EXTENDED,
    MEM_PREAMBLE_LENGTH,

    MEM_TIMEOUT,
    MEM_NETWORK_ROLE,
    MEM_MAILBOX,
    MEM_MANUFACTURER_ID,
    MEM_UNIQUE_ID,

    MEM_VERSION,
    MEM_DEVICE,
    MEM_UART_BAUD_RATE,
    MEM_UART_FLOW_CTRL,
    MEM_DATA_INTERFACE,

    MEM_CONFIG_INTERFACE,
    MEM_FREQ_CAL,
    MEM_LED_CONTROL,
    MEM_CONTROL_FIELD,
    MEM_RX_TIMEOUT,

    MEM_INSTALL_MODE,
    MEM_ENCRYPT_FLAG,
    MEM_DECRYPT_FLAG,
    MEM_DEFAULT_KEY,
    MEM_PART_NUMBER,

    MEM_SERIAL_NUMBER,
    MEM_MAC_2_CHECK_ONLY_FLAG,

    MEM_MAX_SLOTS

};

// MBUS4
static const uint8_t MEM_ADDRESS[MODULE_MAX-1][MEM_MAX_SLOTS] = {
    
    /* MODULE_MBUS4 */ 
    { 
        0x00, 0xFF, 0x01, 0x02, 0xFF, 0x03, 0x04, 0x05, 0xFF, 0x0A,
        0x10, 0x12, 0x16, 0x19, 0x1B, 0x1F, 0x20, 0x30, 0x35, 0x36,
        0x37, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x61,
        0x78, 0xFF
    },

    /* MODULE_OSP */ 
    { 
        0x00, 0xFF, 0x01, 0x02, 0xFF, 0x03, 0x04, 0x05, 0xFF, 0x0A,
        0x10, 0x12, 0x16, 0x19, 0x1B, 0x1F, 0x20, 0x30, 0x35, 0x36,
        0x37, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x89,
        0xA9, 0xFF                                                      // Is this 0x9A?
    },

    /* MODULE_WIZE */ 
    { 
        0x00, 0x01, 0x04, 0x02, 0x03, 0x05, 0x06, 0x07, 0x08, 0xFF,
        0x10, 0x12, 0x16, 0x19, 0x1B, 0x1F, 0x20, 0x30, 0x35, 0x36,
        0x37, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x89,
        0x9A, 0x41
    }

};

// Channels
#define CHANNEL_100                     1
#define CHANNEL_110                     2
#define CHANNEL_120                     3
#define CHANNEL_130                     4
#define CHANNEL_140                     5
#define CHANNEL_150                     6

#define CHANNEL_01                      1
#define CHANNEL_02                      2
#define CHANNEL_03                      3
#define CHANNEL_04                      4
#define CHANNEL_05                      5
#define CHANNEL_06                      6
#define CHANNEL_07                      7
#define CHANNEL_08                      8
#define CHANNEL_09                      9
#define CHANNEL_10                      10
#define CHANNEL_11                      11
#define CHANNEL_12                      12
#define CHANNEL_13                      13
#define CHANNEL_14                      14
#define CHANNEL_15                      15
#define CHANNEL_16                      16
#define CHANNEL_17                      17
#define CHANNEL_18                      18
#define CHANNEL_19                      19
#define CHANNEL_20                      20
#define CHANNEL_21                      21
#define CHANNEL_22                      22
#define CHANNEL_23                      23
#define CHANNEL_24                      24
#define CHANNEL_25                      25
#define CHANNEL_26                      26
#define CHANNEL_27                      27
#define CHANNEL_28                      28
#define CHANNEL_29                      29
#define CHANNEL_30                      30
#define CHANNEL_31                      31
#define CHANNEL_32                      32
#define CHANNEL_33                      33
#define CHANNEL_34                      34
#define CHANNEL_35                      35
#define CHANNEL_36                      36
#define CHANNEL_37                      37
#define CHANNEL_38                      38
#define CHANNEL_39                      39
#define CHANNEL_40                      40
#define CHANNEL_41                      41

// Data rates
#define DATARATE_2400bps                0x01    // Only OSP & WIZE
#define DATARATE_4800bps                0x02    // Only OSP & WIZE
#define DATARATE_6400bps                0x03    // Only WIZE
#define DATARATE_19200bps               0x04    // Only OSP
#define DATARATE_6400bps_OSP            0x05    // Only OSP

static const uint32_t DATARATES[4] = {2400, 4800, 6400, 9600};

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

// LED Control
#define LED_CONTROL_DISABLED            0x00
#define LED_CONTROL_RX_TX               0x01
#define LED_CONTROL_UART_RF_IDLE        0x02
#define LED_CONTROL_RF_RX_TX            0x03

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

// Encryption Keys
#define ENCRYPTION_KEY_NONE             0x00
#define ENCRYPTION_KEY_01               0x01
#define ENCRYPTION_KEY_02               0x02
#define ENCRYPTION_KEY_03               0x03
#define ENCRYPTION_KEY_04               0x04
#define ENCRYPTION_KEY_05               0x05
#define ENCRYPTION_KEY_06               0x06
#define ENCRYPTION_KEY_07               0x07
#define ENCRYPTION_KEY_08               0x08
#define ENCRYPTION_KEY_09               0x09
#define ENCRYPTION_KEY_10               0x0A
#define ENCRYPTION_KEY_11               0x0B
#define ENCRYPTION_KEY_12               0x0C
#define ENCRYPTION_KEY_13               0x0D
#define ENCRYPTION_KEY_14               0x0E
#define ENCRYPTION_KEY_CHANGE           0x0F
#define ENCRYPTION_KEY_AUTH             0x10

// Baud rates
#define BAUDRATE_2400                   0x01
#define BAUDRATE_4800                   0x02
#define BAUDRATE_9600                   0x03
#define BAUDRATE_14400                  0x04
#define BAUDRATE_19200                  0x05
#define BAUDRATE_28800                  0x06
#define BAUDRATE_38400                  0x07
#define BAUDRATE_57600                  0x08
#define BAUDRATE_76800                  0x09
#define BAUDRATE_115200                 0x0A
#define BAUDRATE_230400                 0x0B

static const uint32_t BAUDRATES[11] = {2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 76800, 115200, 230400};
