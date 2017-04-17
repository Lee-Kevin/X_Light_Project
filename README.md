# RGBLED_Ligh Controller Hardware Part
For this project, the hardware part is very import,


# RGBLED_Ligh Controller Software Part
------
In this project, the software has two part, one is the slave part and the another one is master part.

The slave part will connect to a RGB light, and control the color of the light, the communication between slave and master is wireless/RF.

## Slave Part

This part will recive the command from the master part.

The data frame is:

Head  -  Address  -  Command  -  End

0xAA  -    Addr   -  Command  -  0x55

The Address is a uint8_t value range from 0~255, and 255 is the boardcast address, and 0 is a reserved address.

The Command is something like this:
``` C 
enum LEDstatus {
    led_on            = 0,
    led_off           = 1,
    led_down          = 2,
    led_up            = 3,
    led_red           = 4,
    led_green         = 5,
    led_blue          = 6,
    led_white         = 7,
    led_lightSalmon   = 8,
    led_orangeRed     = 9,
    led_orange        = 10,
    led_yellow        = 11,
    led_paleGreen     = 12,
    led_skyBlue       = 13,
    led_turquoise     = 14,
    led_royalBlue     = 15,
    led_steelBlue     = 16,
    led_purple        = 17,
    led_rebeccapurple = 18,
    led_orchid        = 19,
    led_flash         = 20,
    led_strobe        = 21,
    led_fade          = 22,
    led_smooth        = 23,
    
};
typedef enum LEDstatus ledstatus_t;
ledstatus_t eLEDStatus;
```

The slave part can also set the address via UART, and the command data is like this:

Head  --  Addr  --  Command  -- New Addr  --  END

0xAA  --  0xFF  --   0xFF    --  New Addr --  0x55

Then you can also get the slave address by send the following command:

Head  --  Addr  --  Command  --  END

0xAA  --  0xFF  --   0xFE    --  0x55

Note: the 0xFF is the boardcast addr.


## Master Part

For this system, there can be many master parts to control the light, the only thing that master part need to do is sending commands.

The Master part1 is used for sending command to the slave part, and this part has the following functions.
1. Recive IR command from the IR controller.
2. Recive comand from the computer(just recive data from uart,and send it out via software serial).
3. Change the display mode by a button.
4. Turn ON/OFF the light by a button



More master part to be continued.

## The Light Number 

The following picture shows how the Light is arrranged.

## The finished work



