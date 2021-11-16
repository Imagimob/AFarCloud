/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

// EUI
// Little-endian. When copying an EUI from TTN, reverse the bytes.
#define APP_EUI                                        \
    {                                                  \
        0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x00 \
    }

// DEVEUI
// Little-endian. When copying an EUI from TTN, reverse the bytes.
#define DEV_EUI                                        \
    {                                                  \
        0xDE, 0xAD, 0xBE, 0xEF, 0x02, 0x00, 0x00, 0x00 \
    }

// APPKEY
// Big-endian. Copy directly from TTN
#define APP_KEY                                                                                       \
    {                                                                                                 \
        0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x0, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00 \
    }
