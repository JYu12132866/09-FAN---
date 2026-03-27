///*****************************************
//  Copyright (C) 2009-2023
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <global_hid.h>
//   @author Kenneth.Hung@ite.com.tw
//   @date   2023/06/28
//   @fileversion: IT66353_SAMPLE_1.12
//******************************************/
#ifndef _global_hid_h_
#define _global_hid_h_

#define MAX_EP0_PACKET_SIZE 64

//
//  Reports:
//

#define RID_EEPROM_OP 0x9A
    #define REP_SIZE_EEPROM_OP 8      // 8 bit
    #define REP_COUNT_EEPROM_OP 48    // 48 x REP_SIZE
    #define REP_LENGTH_EEPROM_OP (1+(REP_SIZE_EEPROM_OP*REP_COUNT_EEPROM_OP)/8)
    #define TAG_ERASE1 0x3C
    #define TAG_ERASE2 0xA5
    #define TAG_ERASE3 0xB9


#define RID_IO_64    0x97
    #define REP_SIZE_IO_64    8
    #define REP_COUNT_IO_64    63
    #define REP_LENGTH_IO_64    (1+(REP_COUNT_IO_64*REP_SIZE_IO_64)/8)  //64 <= 64 (64*1)

#define RID_IO_128    0x9A
    #define REP_SIZE_IO_128    8
    #define REP_COUNT_IO_128    127
    #define REP_LENGTH_IO_128    (1+(REP_COUNT_IO_128*REP_SIZE_IO_128)/8)  //128 <= 128 (64*2)

#define RID_IO_192    0x9D
    #define REP_SIZE_IO_192    8
    #define REP_COUNT_IO_192    191
    #define REP_LENGTH_IO_192    (1+(REP_COUNT_IO_192*REP_SIZE_IO_192)/8)  //192 <= 192 (64*3)

#define RID_IO_256    0xA0
    #define REP_SIZE_IO_256    8
    #define REP_COUNT_IO_256    255
    #define REP_LENGTH_IO_256    (1+(REP_COUNT_IO_256*REP_SIZE_IO_256)/8)  //256 <= 256 (64*4)

#define RID_IO_320    0xA3
    #define REP_SIZE_IO_320    16
    #define REP_COUNT_IO_320    159
    #define REP_LENGTH_IO_320    (1+(REP_COUNT_IO_320*REP_SIZE_IO_320)/8)  //319 <= 320 (64*5)

#define RID_IO_384    0xA6
    #define REP_SIZE_IO_384    16
    #define REP_COUNT_IO_384    191
    #define REP_LENGTH_IO_384    (1+(REP_COUNT_IO_384*REP_SIZE_IO_384)/8)  //383 <= 384 (64*6)

#define RID_IO_448    0xA9
    #define REP_SIZE_IO_448    16
    #define REP_COUNT_IO_448    223
    #define REP_LENGTH_IO_448    (1+(REP_COUNT_IO_448*REP_SIZE_IO_448)/8)  //447 <= 448 (64*7)

#define RID_IO_512    0xAC
    #define REP_SIZE_IO_512    16
    #define REP_COUNT_IO_512    255
    #define REP_LENGTH_IO_512    (1+(REP_COUNT_IO_512*REP_SIZE_IO_512)/8)  //511 <= 512 (64*8)

#define RID_IO_576    0xAF
    #define REP_SIZE_IO_576    32
    #define REP_COUNT_IO_576    143
    #define REP_LENGTH_IO_576    (1+(REP_COUNT_IO_576*REP_SIZE_IO_576)/8)  //573 <= 576 (64*9)

#define RID_IO_640    0xB2
    #define REP_SIZE_IO_640    32
    #define REP_COUNT_IO_640    159
    #define REP_LENGTH_IO_640    (1+(REP_COUNT_IO_640*REP_SIZE_IO_640)/8)  //637 <= 640 (64*10)

#define RID_IO_704    0xB5
    #define REP_SIZE_IO_704    32
    #define REP_COUNT_IO_704    175
    #define REP_LENGTH_IO_704    (1+(REP_COUNT_IO_704*REP_SIZE_IO_704)/8)  //701 <= 704 (64*11)

#define RID_IO_768    0xB8
    #define REP_SIZE_IO_768    32
    #define REP_COUNT_IO_768    191
    #define REP_LENGTH_IO_768    (1+(REP_COUNT_IO_768*REP_SIZE_IO_768)/8)  //765 <= 768 (64*12)

#define RID_IO_832    0xBB
    #define REP_SIZE_IO_832    32
    #define REP_COUNT_IO_832    207
    #define REP_LENGTH_IO_832    (1+(REP_COUNT_IO_832*REP_SIZE_IO_832)/8)  //829 <= 832 (64*13)

#define RID_IO_896    0xBE
    #define REP_SIZE_IO_896    32
    #define REP_COUNT_IO_896    223
    #define REP_LENGTH_IO_896    (1+(REP_COUNT_IO_896*REP_SIZE_IO_896)/8)  //893 <= 896 (64*14)

#define RID_IO_960    0xC1
    #define REP_SIZE_IO_960    32
    #define REP_COUNT_IO_960    239
    #define REP_LENGTH_IO_960    (1+(REP_COUNT_IO_960*REP_SIZE_IO_960)/8)  //957 <= 960 (64*15)

#define RID_IO_1024    0xC4
    #define REP_SIZE_IO_1024    32
    #define REP_COUNT_IO_1024    255
    #define REP_LENGTH_IO_1024    (1+(REP_COUNT_IO_1024*REP_SIZE_IO_1024)/8)  //1021 <= 1024 (64*16)



#ifndef A51_INC
    enum
    {
        OP_WRITE_64, // because the ep0 has only 64 byte
        OP_READ_64,
        OP_WRITE_L8,
        OP_READ_L8,
        OP_GET_PORT,

        OP_SET_PORT,
        OP_WRITE_L16,
        OP_READ_L16,
        OP_OTHERS,
        OP_ERASE_FW,
        OP_SET_OPTIONS,
        OP_GET_OPTIONS,
    };

    enum // sub opcode for 'BYTE OpCode2;'
    {
        OP2_SET_I2C_RATE,
        OP2_I2C_FUNC_SEL,
        OP2_USB_PERF_TEST_SET,
        OP2_USB_PERF_TEST_GET,

        OP2_SET_PORT_CONFIG,
        OP2_SET_PORT_VALUE,
        OP2_GET_FW_VERSION,

        OP2_SET_I2C_DEV,

        OP2_SET_PWM_DUTY_CYCLE,
        OP2_GET_PWM_DUTY_CYCLE,

        OP2_GET_PORT_CONFIG,
        OP2_GET_PORT_VALUE,
        OP2_GET_I2C_DEV,
        OP2_GET_I2C_RATE,

        OP2_SET_IO_LED,
        OP2_GET_IO_LED,

        OP2_SET_EC_REG1,
        OP2_GET_EC_REG1,
        OP2_SET_EC_REG2,
        OP2_GET_EC_REG2,

        OP2_SET_ADC,
        OP2_GET_ADC,

        OP2_SET_LCD,
        OP2_GET_LCD,
    };

    enum
    {
        ADCOP_INIT,
        ADCOP_CHANNEL_ENABLE,
        ADCOP_CHECK_CHANNEL_DATA,
        ADCOP_GET_CHANNEL_DATA,
        ADCOP_GET_ALL_CHANNEL_DATA,
        ADCOP_MODULE_ENABLE,
    };

    enum
    {
        LCDOP_INIT,
        LCDOP_SETDATA,
        LCDOP_SETCTRL,
    };


#define I2C_FLAG_DONOT_SEND_OFFSET 0x01
#define I2C_FLAG_SEND_OFFSET_ONLY 0x02
#define I2C_FLAG_SEND_ADDRESS_ONLY 0x04
#define I2C_FLAG_DONOT_SEND_START 0x08
#define I2C_FLAG_DONOT_SEND_STOP 0x10
#define I2C_FLAG_ASSIGN_I2C_DEV 0x20
#define I2C_FLAG_SEND_RAW_DATA 0x40

#define OPTION_TAG 0x5A

    // byte1
    enum
    {
        OPTION_OP_SET_I2C_MODE,
    };
    // byte2
    enum
    {
        OPTION_I2C_MODE_SOFTWARE,
        OPTION_I2C_MODE_HARDWARE,
    };

    enum
    {
        RESULT_SUCCESS,
        RESULT_FAIL,
        RESULT_LOSE_ARBIT,
        RESULT_WRITE_ADDR_NOACK,
        RESULT_WRITE_OFFSET_NOACK,
        RESULT_WRITE_DATA_NOACK,
        RESULT_READ_ADDR_NOACK,
        RESULT_UNKNOWN_PORT,
        RESULT_IO_LENGTH_ERROR,
        RESULT_UNSUPPORT_OPCODE,
        RESULT_PARAM_ERROR,
    };

#ifdef _IT6350_USBFW_
#else
#pragma pack(push)
#pragma pack(1)
#endif

    struct kIRP
    {
        union
        {
            BYTE Op;
            BYTE Result;
        }Req;

        union
        {
            BYTE Buffer[10]; // NEVER modify this
            /*
            struct
            {
                BYTE Byte00;
                BYTE Byte01;
                BYTE Byte02;
                BYTE Byte03;
                BYTE Byte04;

                BYTE Byte05;
                BYTE Byte06;
                BYTE Byte07;
                BYTE Byte08;
                BYTE Byte09;

                BYTE Byte10;
                BYTE Byte11;
                BYTE Byte12;
                BYTE Byte13;
                BYTE Byte14;
            }Padding2;
            */
            struct
            {
                BYTE Addr;
                BYTE Off;
                BYTE Len;
                BYTE Flag1;
                BYTE Flag2;
            }Read;

            struct
            {
                BYTE Addr;
                BYTE Off;
                WORD Len;
            }Read2;

            struct
            {
                WORD Addr;
                WORD Off;
                WORD Len;
            }Read3;

            struct
            {
                BYTE Addr;
                BYTE Off;
                BYTE Len;
                BYTE Flag1;
                BYTE Flag2;
            }Write;

            struct
            {
                BYTE Addr;
                BYTE Off;
                BYTE Len;
            }WriteL8;

            struct
            {
                BYTE Addr;
                BYTE Off;
                WORD Len;
            }WriteL16;

            struct
            {
                WORD Addr;
                WORD Off;
                WORD Len;
            }Write2;

            struct
            {
                BYTE Tag1;
                BYTE Tag2;
                BYTE Tag3;
            }EEPRom;

            struct
            {
                BYTE Port;
                BYTE Mask;
                BYTE Value;
            }PortOp;

            struct
            {
                BYTE Opt1;
                BYTE Opt2;
                BYTE Opt3;
                BYTE Opt4;
            }GetOption;

            struct
            {
                BYTE Opt1;
                BYTE Opt2;
                BYTE Opt3;
                BYTE Opt4;
            }SetOption;

            struct
            {
                BYTE OpCode2;
                union
                {
                    struct
                    {
                        BYTE IoPort;
                        BYTE InOut;
                        BYTE Type;
                    }Config;
                    struct
                    {
                        BYTE IoPort;
                        BYTE Value;
                    }SetIo;
                    struct
                    {
                        BYTE Channel;
                        BYTE Value;
                    }PwmDutyCycle;
                }u;

            }SetPort;

            struct
            {
                BYTE OpCode2;
                union
                {
                    struct
                    {
                        BYTE IoPort;
                        BYTE InOut;
                        BYTE Type;
                    }Config;
                    struct
                    {
                        BYTE IoPort;
                        BYTE Value;
                    }GetIo;
                    struct
                    {
                        BYTE Channel;
                        BYTE Value;
                    }PwmDutyCycle;
                }u;

            }GetPort;

            struct
            {
                BYTE OpCode2;
                union
                {
                    struct
                    {
                        BYTE PreScale;
                        BYTE DelayType;
                        BYTE DelayValue;
                    }SetRate;

                    struct
                    {
                        BYTE PreScale;
                        BYTE DelayType;
                        BYTE DelayValue;
                    }GetRate;

                    struct
                    {
                        BYTE Function;
                    }I2cSel;

                    struct
                    {
                        BYTE Tag1;
                        BYTE Tag2;
                        BYTE Ver1;
                        BYTE Ver2;
                    }FirmwareVersion;

                    struct
                    {
                        BYTE I2cDevId;
                    }SetI2cDev;

                    struct
                    {
                        BYTE I2cDevId;
                    }GetI2cDev;

                    struct
                    {
                        BYTE SetLEDWithIO;
                    }IoLED;

                    struct
                    {
                        BYTE OffsetL;
                        BYTE OffsetH;
                        BYTE Value;
                    }SetEcReg1;

                    struct
                    {
                        BYTE OffsetL;
                        BYTE OffsetH;
                        BYTE Value;
                    }GetEcReg1;

                    struct
                    {
                        BYTE OffsetL;
                        BYTE OffsetH;
                        BYTE Value;
                    }SetEcReg2;

                    struct
                    {
                        BYTE OffsetL;
                        BYTE OffsetH;
                        BYTE Value;
                    }GetEcReg2;

                    struct
                    {
                        BYTE IoLengthL;
                        BYTE IoLengthH;
                    }PerfTest;

                    struct
                    {
                        BYTE ADCOp;
                        union
                        {
                            struct
                            {
                                BYTE Channel;
                                BYTE Enable;
                            }ChannelEnable;
                            struct
                            {
                                BYTE DataValid;
                            }CheckChannelData;
                            struct
                            {
                                BYTE Channel;
                                BYTE DataL;
                                BYTE DataH;
                            }GetChannelData;
                            struct
                            {
                                BYTE Flags;
                            }GetAllChannelData;
                            struct
                            {
                                BYTE Flags;
                            }ADCInit;
                            struct
                            {
                                BYTE Enable;
                                BYTE Flags;
                            }ADCModuleEnable;
                        }u;
                    }ADC;

                    struct
                    {
                        BYTE LCDOp;
                        union
                        {
                            struct
                            {
                                BYTE Data;
                            }LCDData;
                            struct
                            {
                                BYTE Data;
                            }LCDCtrl;
                        }u;
                    }LCD;
                }u;

            }Others;
        }u;
#ifdef _IT6350_USBFW_
    }__attribute__((__packed__));
#else
    };
#pragma pack(pop)
#endif

#endif

#endif
