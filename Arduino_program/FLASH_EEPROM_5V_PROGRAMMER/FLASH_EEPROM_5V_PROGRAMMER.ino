/* @Author: Eduardo José Tagle, loosely based on the one written by Radouane SAMIR
   @Version: 0.1
   @Chip version: AM29F010 AM29F040

   You are free to modify the code as you want, but please keep my name as is.
*/

#include <avr/io.h>
#include <util/delay.h>
#include <Wire.h>

// Minimum required stabilization time: 1uS
#define ACCESS_DELAY 5 /* original 1us, trying 5uS */

/*
 D54 - D61    | PF0-PF7 => DQ0-DQ7
 D62 - D69    | PK0-PK7 =>  A0- A7
 D52        | PB1     => A8
 D50        | PB3 => A9
 D48        | PL1 => A10
 D46        | PL3 => A11
 D44        | PL5 => A12
 D42        | PL7 => A13
 D40        | PG1 => A14
 D38        | PD7 => A15
 D36        | PC1 => A16
 D28        | PA6 => A17
 D26        | PA4 => A18
 D34        | PC3 => CE
 D32        | PC5 => WE
 D30        | PC7 => OE
 D24        | PA2 => OE_12V
 D22        | PA0 => A9_12V

 I2C programmer
 D21        | SCL
 D20        | SDA
 */

#define CE_PIN 34 /* PC3 */
#define CE_PORT PORTC /*PC3*/
#define CE_BIT  3 /*PC3*/

#define WE_PIN 32 /*PC5*/
#define WE_PORT PORTC /*PC5*/
#define WE_BIT  5 /*PC5*/

#define OE_PIN 30 /*PC7*/
#define OE_PORT PORTC /*PC7*/
#define OE_BIT  7 /*PC7*/

#define OE12V_PIN 24 /*PA2*/
#define OE12V_PORT PORTA /*PA2*/
#define OE12V_BIT  2 /*PA2*/

#define A912V_PIN 22 /*PA0*/
#define A912V_PORT PORTA /*PA0*/
#define A912V_BIT  0 /*PA0*/

#define DQ0_PIN 54
#define DQ1_PIN 55
#define DQ2_PIN 56
#define DQ3_PIN 57
#define DQ4_PIN 58
#define DQ5_PIN 59
#define DQ6_PIN 60
#define DQ7_PIN 61

#define DDR_DATA DDRF
#define PORT_DATA PORTF
#define PIN_DATA PINF

#define A0toA7PORT PORTK

#define A0_PIN 62 /*PK0*/
#define A0_PORT PORTK /*PK0*/
#define A0_BIT  0 /*PK0*/

#define A1_PIN 63 /*PK1*/
#define A1_PORT PORTK /*PK1*/
#define A1_BIT  1 /*PK1*/

#define A2_PIN 64 /*PK2*/
#define A2_PORT PORTK /*PK2*/
#define A2_BIT  2 /*PK2*/

#define A3_PIN 65 /*PK3*/
#define A3_PORT PORTK /*PK3*/
#define A3_BIT  3 /*PK3*/

#define A4_PIN 66 /*PK4*/
#define A4_PORT PORTK /*PK4*/
#define A4_BIT  4 /*PK4*/

#define A5_PIN 67 /*PK5*/
#define A5_PORT PORTK /*PK5*/
#define A5_BIT  5 /*PK5*/

#define A6_PIN 68 /*PK6*/
#define A6_PORT PORTK /*PK6*/
#define A6_BIT  6 /*PK6*/

#define A7_PIN 69 /*PK7*/
#define A7_PORT PORTK /*PK7*/
#define A7_BIT  7 /*PK7*/

#define A8_PIN 52  /*PB1*/
#define A8_PORT PORTB /*PB1*/
#define A8_BIT  1 /*PB1*/

#define A9_PIN 50  /*PB3*/
#define A9_PORT PORTB /*PB3*/
#define A9_BIT  3 /*PB3*/

#define A10_PIN 48 /*PL1*/
#define A10_PORT PORTL /*PL1*/
#define A10_BIT  1 /*PL1*/

#define A11_PIN 46 /*PL3*/
#define A11_PORT PORTL /*PL3*/
#define A11_BIT  3 /*PL3*/

#define A12_PIN 44 /*PL5*/
#define A12_PORT PORTL /*PL5*/
#define A12_BIT  5 /*PL5*/

#define A13_PIN 42 /*PL7*/
#define A13_PORT PORTL /*PL7*/
#define A13_BIT  7 /*PL7*/

#define A14_PIN 40 /*PG1*/
#define A14_PORT PORTG /*PG1*/
#define A14_BIT  1 /*PG1*/

#define A15_PIN 38 /*PD7*/
#define A15_PORT PORTD /*PD7*/
#define A15_BIT  7 /*PD7*/

#define A16_PIN 36 /*PC1*/
#define A16_PORT PORTC /*PC1*/
#define A16_BIT  1 /*PC1*/

#define A17_PIN 28 /*PA6*/
#define A17_PORT PORTA /*PA6*/
#define A17_BIT  6 /*PA6*/

#define A18_PIN 26 /*PA4*/
#define A18_PORT PORTA /*PA4*/
#define A18_BIT  4 /*PA4*/


#define D27M1001 0xC3
#define D27M1001_V2 0x83
#define D29F010 0x20
#define D29F040 0xA4
#define D29040  0x86

int dev_id = -1; // Autodetected device ID

#define force_inline inline __attribute__((always_inline))
static force_inline void flash_control_oe_12v(bool enable)
{
    if (enable)
        OE12V_PORT |= (1U << OE12V_BIT);
    else
        OE12V_PORT &= ~(1U << OE12V_BIT);

    // Takes about 2.5us for this voltage to stabilize
    _delay_us(4);
}

static force_inline void flash_control_a9_12v(bool enable)
{
    if (enable)
        A912V_PORT |= (1U << A912V_BIT);
    else
        A912V_PORT &= ~(1U << A912V_BIT);

    // Takes about 2.5us for this voltage to stabilize
    _delay_us(4);
}

/* Need a fast change mode IN/OUT. If io = 1, configure them as inputs, otherwise as outputs */
static force_inline void flash_change_databus_direction(bool io)
{
    if (io) {
        DDR_DATA = 0; /* Configure all as inputs */
        PORT_DATA = 0; /* Disable pull ups */
    } else
        DDR_DATA = 0xFF; /* Configure all as outputs */
}

static force_inline void flash_control_ce(bool state)
{
    if (state)
        CE_PORT |= (1U<<CE_BIT);  //CE
    else
        CE_PORT &= ~(1U<<CE_BIT);  //CE
}

static force_inline void flash_control_we(bool state)
{
    if (state)
        WE_PORT |= (1U<<WE_BIT);  //WE
    else
        WE_PORT &= ~(1U<<WE_BIT);  //WE
}

static force_inline void flash_control_oe(bool state)
{
    if (state)
        OE_PORT |= (1U<<OE_BIT);  //OE
    else
        OE_PORT &= ~(1U<<OE_BIT);  //OE
}

static force_inline void flash_ctrl_deselect()
{
    flash_control_ce(1);  //CE
    flash_control_we(1);  //WE
    flash_control_oe(1);  //OE
}

static force_inline void flash_ctrl_rd()
{
    flash_control_oe(0);
    flash_control_ce(0);
}

static force_inline void flash_ctrl_wr()
{
    flash_control_we(0);
    flash_control_ce(0);
}

/**
 * Set the address *
 *******************/

static void flash_addr_set(uint32_t addr)
{
#if 0
    if (addr & 1UL)
        A0_PORT |= (1U<<A0_BIT);
    else
        A0_PORT &= ~(1U<<A0_BIT);

    if (addr & 2UL)
        A1_PORT |= (1U<<A1_BIT);
    else
        A1_PORT &= ~(1U<<A1_BIT);

    if (addr & 4UL)
        A2_PORT |= (1U<<A2_BIT);
    else
        A2_PORT &= ~(1U<<A2_BIT);

    if (addr & 8UL)
        A3_PORT |= (1U<<A3_BIT);
    else
        A3_PORT &= ~(1U<<A3_BIT);

    if (addr & 16UL)
        A4_PORT |= (1U<<A4_BIT);
    else
        A4_PORT &= ~(1U<<A4_BIT);

    if (addr & 32UL)
        A5_PORT |= (1U<<A5_BIT);
    else
        A5_PORT &= ~(1U<<A5_BIT);

    if (addr & 64UL)
        A6_PORT |= (1U<<A6_BIT);
    else
        A6_PORT &= ~(1U<<A6_BIT);

    if (addr & 128UL)
        A7_PORT |= (1U<<A7_BIT);
    else
        A7_PORT &= ~(1U<<A7_BIT);
#else
    // All low 7 bits are mapped directly
    A0toA7PORT = (addr & 0xFF);
#endif

    if (addr & 256UL)
        A8_PORT |= (1U<<A8_BIT);
    else
        A8_PORT &= ~(1U<<A8_BIT);

    if (addr & 512UL)
        A9_PORT |= (1U<<A9_BIT);
    else
        A9_PORT &= ~(1U<<A9_BIT);

    if (addr & 1024UL)
        A10_PORT |= (1U<<A10_BIT);
    else
        A10_PORT &= ~(1U<<A10_BIT);

    if (addr & 2048UL)
        A11_PORT |= (1U<<A11_BIT);
    else
        A11_PORT &= ~(1U<<A11_BIT);

    if (addr & 4096UL)
        A12_PORT |= (1U<<A12_BIT);
    else
        A12_PORT &= ~(1U<<A12_BIT);

    if (addr & 8192UL)
        A13_PORT |= (1U<<A13_BIT);
    else
        A13_PORT &= ~(1U<<A13_BIT);

    if (addr & 16384UL)
        A14_PORT |= (1U<<A14_BIT);
    else
        A14_PORT &= ~(1U<<A14_BIT);

    if (addr & 32768UL)
        A15_PORT |= (1U<<A15_BIT);
    else
        A15_PORT &= ~(1U<<A15_BIT);

    if (addr & 65536UL)
        A16_PORT |= (1U<<A16_BIT);
    else
        A16_PORT &= ~(1U<<A16_BIT);

    if (addr & 131072UL)
        A17_PORT |= (1U<<A17_BIT);
    else
        A17_PORT &= ~(1U<<A17_BIT);

    if (addr & 262144UL)
        A18_PORT |= (1U<<A18_BIT);
    else
        A18_PORT &= ~(1U<<A18_BIT);
}

/**
 * Calculate the data given and set change pins state
 *****************************************************/

static force_inline void flash_data_set(uint8_t data)
{
    PORT_DATA = data;
}

/**
 * Get Data from the chip *
 * MIND: the pull-down resistors
 *********************************/

static force_inline uint8_t flash_data_get()
{
    return PIN_DATA;
}

/****
 * Set command sequence and send it to the chip
 ***********************************************/

static void flash_write(uint32_t addr, uint8_t data)
{
    /* Config as outputs */
    flash_change_databus_direction(0);

    /* Send command sequence */
    flash_addr_set(addr);
    flash_data_set(data);
    _delay_us(ACCESS_DELAY); // Give some time for data setup
    flash_ctrl_wr();
    _delay_us(ACCESS_DELAY); // Give some time for write
    flash_ctrl_deselect();
    _delay_us(ACCESS_DELAY); // Give some time for deselection
}

static uint8_t flash_read(uint32_t addr)
{
    flash_change_databus_direction(1); // Config as inputs

    flash_addr_set(addr);
    _delay_us(ACCESS_DELAY); // Give some time for address setup
    flash_ctrl_rd();
    _delay_us(ACCESS_DELAY); // Give some time for read access
    uint8_t d = flash_data_get();
    flash_ctrl_deselect();
    _delay_us(ACCESS_DELAY); // Give some time for deselection

    return d;
}


/*
 * Data polling in programming operation
 *****************************************/
static bool flash_DQ7_byte_poll(uint32_t addr, uint8_t data)
{
    bool ret = true;

    // Toggle bit polling
    flash_change_databus_direction(1); // Config as inputs
    flash_addr_set(addr);

    /* Enable chip access */
    flash_control_ce(1);  //CE=1
    flash_control_oe(0);  //OE=0

    /* Data polling algorithm */
    uint8_t rd = 0;
    uint16_t loops = 4000U; // at 20 mhz, 28us is 560 cycles
    do {
        // Read status
        flash_control_ce(0);  //CE
        _delay_us(ACCESS_DELAY); // Give some time for read access
        rd = flash_data_get();
        flash_control_ce(1);  //CE
        _delay_us(ACCESS_DELAY);
    } while (
        ((rd ^ data) & 0x80U) != 0 && /* Data still not valid */
        (rd & 0x20) == 0 && /* And Q5 = 0 (not timed out) */
        --loops != 0U /* And no internal timeout */
    );

    /* Internal polling timeout: Failure */
    if (!loops) {
        ret = false;
    } else {
        /* Either operation timed out, or success */

        /* If operation timed out */
        if ( (rd & 0x20) != 0 ) {

            // Read status again
            flash_control_ce(0);  //CE
            _delay_us(ACCESS_DELAY); // Give some time for read access
            rd = flash_data_get();
            flash_control_ce(1);  //CE
            _delay_us(ACCESS_DELAY);

            /* If data still not valid, failure*/
            if (((rd ^ data) & 0x80U) != 0) {
                ret = false;
            }
        } /* Success */
    }

    /* Deselect chip */
    flash_control_oe(1);  //OE
    _delay_us(ACCESS_DELAY);

  return ret;
}

bool flash_DQ7_erase_poll()
{
    bool ret = true;

    // Toggle bit polling
    flash_change_databus_direction(1); // Config as inputs
    flash_addr_set(0);

    /* Enable chip access */
    flash_control_ce(1);  //CE
    flash_control_oe(0);  //OE

    /* Data polling algorithm */
    uint8_t rd = 0;
    uint16_t loops = 1000U; // at 20 mhz, 7us is 140 cycles
    do {
        // Read status
        flash_control_ce(0);  //CE
        _delay_us(ACCESS_DELAY); // Give some time for read access
        rd = flash_data_get();
        flash_control_ce(1);  //CE
        _delay_us(ACCESS_DELAY);
  } while (
        (rd & 0x80) == 0 && /* Erase still running */
        (rd & 0x20) == 0 && /* And Q5 = 0 (not timed out) */
        --loops != 0U /* And no internal timeout */
    );

    /* Internal polling timeout: Failure */
    if (!loops) {
        ret = false;
    } else {
        /* Either operation timed out, or success */

        /* If operation timed out */
        if ( (rd & 0x20) != 0 ) {

            // Read status again
            flash_control_ce(0);  //CE
            _delay_us(ACCESS_DELAY); // Give some time for read access
            rd = flash_data_get();
            flash_control_ce(1);  //CE
            _delay_us(ACCESS_DELAY);

            /* If data still not valid, failure*/
            if ((rd & 0x80) == 0) { /* Erase still running */
                ret = false;
            }
        } /* Success */
    }

    /* Deselect chip */
    flash_control_oe(1);  //OE
    _delay_us(ACCESS_DELAY);

    return ret;
}

/****
 * Autoselect mode to get info from the chip
 * It's useful to test the chip before any operation
 ******************************************************/

void flash_device_id_10(bool display)
{
    /* This is used to get code identification
    * from the chip in autoselect mode
    * See the datasheet
    ******************************************/

    // Enter autoselect mode and read ID
    flash_write(0x5555, 0xAA);
    flash_write(0x2AAA, 0x55);
    flash_write(0x5555, 0x90);
    uint8_t d = flash_read(0x01); // Device ID

    dev_id = d; // Store device ID
    if (display) {
        if (d < 0x10U)
            Serial.print("0");
        Serial.print(d, HEX);
        Serial.flush();
    }

    // Exit autoselect mode
    flash_reset_chip();
}

/** AM29F040 **/
/***************/

void flash_device_id_40()
{
    /* This is used to get code identification
    * from the chip in autoselect mode
    * See the datasheet
    ******************************************/

    // Enter autoselect mode
    flash_write(0x555, 0xAA);
    flash_write(0x2AA, 0x55);
    flash_write(0x555, 0x90);
    uint8_t d = flash_read(0x01); // Device ID

    if (d < 0x10U)
        Serial.print("0");
    Serial.print(d, HEX);
    Serial.flush();

    // Exit autoselect mode
    flash_reset_chip();
}

void flash_device_id()
{
    if (dev_id == -1)
        flash_device_id_10(false);

    if (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2) {
        flash_device_id_10(true);
    } else {
        flash_device_id_40();
    }
}

void flash_manufacturer_id_10()
{
    /* This is used to get code identification
    * from the chip in autoselect mode
    * See the datasheet
    ******************************************/

    // Enter autoselect mode for info
    flash_write(0x5555, 0xAA);
    flash_write(0x2AAA, 0x55);
    flash_write(0x5555, 0x90);
    uint8_t d = flash_read(0x00); // Manufacturer ID

    if (d < 0x10U)
        Serial.print("0");
    Serial.print(d, HEX);
    Serial.flush();

    // Exit autoselect mode
    flash_reset_chip();
}

void flash_manufacturer_id_40()
{
    /* This is used to get code identification
    * from the chip in autoselect mode
    * See the datasheet
    ******************************************/

    // Enter autoselect mode for info
    flash_write(0x555, 0xAA);
    flash_write(0x2AA, 0x55);
    flash_write(0x555, 0x90);

    uint8_t d = flash_read(0x00); // Manufacturer ID
    if (d < 0x10U)
        Serial.print("0");
    Serial.print(d, HEX);
    Serial.flush();

    // Exit autoselect mode
    flash_reset_chip();
}

void flash_manufacturer_id()
{
    if (dev_id == -1)
        flash_device_id_10(false);

    if (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2) {
        flash_manufacturer_id_10();
    } else {
        flash_manufacturer_id_40();
    }
}


void flash_read_memory()
{

    if (dev_id == -1)
        flash_device_id_10(false);

    // Determine maximum address to read
    uint32_t max_addr = (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2)
        ? 0x20000UL
        : 0x80000UL;

    /* Read the chip until the address given */
    for (uint32_t addr = 0UL; addr < max_addr; ++addr) {
        uint8_t d = flash_read(addr);
        if (d < 0x10U)
            Serial.print("0");
        Serial.print(d, HEX);
    }
    Serial.flush();
}

/* This is the sequence to program a byte on the memory *
 * 0x5555 0x2AAA is for AM29F010
 * 0x555 0x2AA is for AM29F040 A29F040
 ********************************************************/

bool flash_program_byte_10(uint32_t addr, uint8_t data)
{
    // Enter autoselect mode for program
    flash_write(0x5555, 0xAA);
    flash_write(0x2AAA, 0x55);
    flash_write(0x5555, 0xA0);

    //Program Address & Program data
    flash_write(addr, data);

    /** Data Polling **/
    return flash_DQ7_byte_poll(addr, data);
}

bool flash_program_byte_40(uint32_t addr, uint8_t data)
{
    // Enter autoselect mode for program
    flash_write(0x555, 0xAA);
    flash_write(0x2AA, 0x55);
    flash_write(0x555, 0xA0);

    // Program Address & Program data
    flash_write(addr, data);

    /** Data Polling **/
    return flash_DQ7_byte_poll(addr, data);
}

bool flash_program_byte(uint32_t addr, uint8_t data)
{
    if (dev_id == -1)
        flash_device_id_10(false);

    if (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2) {
        return flash_program_byte_10(addr,data);
    } else {
        return flash_program_byte_40(addr,data);
    }
}

/* This is the sequence to program a byte on the memory *
 * 0x5555 0x2AAA is for AM29F010
 * 0x555 0x2AA is for AM29F040 A29F040
 ********************************************************/

void flash_reset_chip_10()
{
    // Enter autoselect mode for chip reset
    flash_write(0x5555, 0xAA);
    flash_write(0x2AAA, 0x55);
    flash_write(0x5555, 0xF0);

    _delay_ms(1000);
}

void flash_reset_chip_40()
{
    // Enter autoselect mode for chip reset
    flash_write(0x555, 0xAA);
    flash_write(0x2AA, 0x55);
    flash_write(0x555, 0xF0);

    _delay_ms(1000);
}

void flash_reset_chip()
{
    if (dev_id == -1)
        flash_device_id_10(false);

    if (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2) {
        flash_reset_chip_10();
    } else {
        flash_reset_chip_40();
    }
}


/* This is the sequence to program a byte on the memory *
 * 0x5555 0x2AAA is for AM29F010
 * 0x555 0x2AA is for AM29F040 A29F040
 ********************************************************/

void flash_erase_memory_10()
{
    // Enter autoselect mode for chip erase
    flash_write(0x5555, 0xAA);
    flash_write(0x2AAA, 0x55);
    flash_write(0x5555, 0x80);
    flash_write(0x5555, 0xAA);
    flash_write(0x2AAA, 0x55);
    flash_write(0x5555, 0x10);

    bool ret = flash_DQ7_erase_poll();
    flash_reset_chip();

    // Send status to python
    if (ret)
        Serial.print("Erasure Complete");
    else
        Serial.print("Erasure FAIL");
    Serial.flush();
}

/* To erase AM29F040 */
/*********************/

void flash_erase_memory_40()
{
    // Enter autoselect mode for chip erase
    flash_write(0x555, 0xAA);
    flash_write(0x2AA, 0x55);
    flash_write(0x555, 0x80);
    flash_write(0x555, 0xAA);
    flash_write(0x2AA, 0x55);
    flash_write(0x555, 0x10);

    bool ret = flash_DQ7_erase_poll();
    flash_reset_chip();

    // Send status to python
    if (ret)
        Serial.print("Erasure Complete");
    else
        Serial.print("Erasure FAIL");
    Serial.flush();
}

void flash_erase_memory()
{
    if (dev_id == -1)
        flash_device_id_10(false);

    if (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2) {
        flash_erase_memory_10();
    } else {
        flash_erase_memory_40();
    }
}

/* Intiate the programing when the python GUI send a trigger */
/*************************************************************/

void flash_program_memory()
{
    // Ack the Write command
    Serial.print("+");
    Serial.flush();

    int index = 0;
    char tab[10] = {0};

    while (1) {
        // Wait for a character
        while (!Serial.available());

        char car = Serial.read();
        if (car == '\r')
            break;

        tab[index++]=car;
    }
    tab[index] = '\0';

    Serial.print("+");
    Serial.flush();

    // If the number is negative, then we can use the new chunked algorithm
    bool newalgo = false;
    int32_t l = (int32_t)atol(tab);
    if (l < 0L) {
        l = -l;
        newalgo = true;
    }

    if (newalgo) {
        uint32_t todo = (uint32_t) l;
        uint32_t address = 0;

        // Acquire chunks
        while (todo != 0) {

            // Determine remaining chunk size
            uint32_t chunksz = todo;
            if (chunksz > 256)
                chunksz = 256;


            static uint8_t buf[256];

            // Receive chunk from the user
            uint8_t* p = &buf[0];
            uint16_t torx = chunksz;
            do {
                while (!Serial.available());
                *p++ = Serial.read();
            } while (--torx != 0);

            // And program it
            p = &buf[0];
            uint16_t topgm = chunksz;
            do {
                uint8_t data = *p;
								
								uint8_t wrtries = 5;
								while (!flash_program_byte(address, data) && --wrtries != 0);
                if (!wrtries) {

                    // Exit programming mode
                    flash_reset_chip();

                    Serial.print("-");
                    Serial.flush();
                    return;
                }

                ++address;
                ++p;
            } while (--topgm);

            // Ack received byte
            Serial.print("+");
            Serial.flush();

            // Compute remaining bytes
            todo -= chunksz;
        }
        // (all chunks programmed)

    } else {
        uint32_t maxaddr = (uint32_t) l;
        uint32_t address = 0;
        while(address < maxaddr) {

            // Wait for a character
            while (!Serial.available());
            uint8_t data = Serial.read();

            // Try to program byte
						uint8_t wrtries = 5;
						while (!flash_program_byte(address, data) && --wrtries != 0);
            if (!wrtries) {
                Serial.print("-");
                Serial.flush();
                break;
            }

            // Ack received byte
            Serial.print("+");
            Serial.flush();

            ++address;
        }
    }

    // Exit programming mode
    flash_reset_chip();
}

void flash_check_sector_protection_10()
{
    uint8_t sector = 0;
    uint8_t prot = 0;
    do {
        // Enter autoselect mode for info
        flash_write(0x5555, 0xAA);
        flash_write(0x2AAA, 0x55);
        flash_write(0x5555, 0x90);

        // Read sector protection  (A0=0, A6=0, A1=1)
        prot |= (
                flash_read(0x02UL | (((uint32_t)sector) << 14UL)) // Sector Protection verification
                & 1U) << sector; // 1 means protected
        ++sector;
    } while (sector < 8);

    if (prot < 0x10U)
        Serial.print("0");
    Serial.print(prot, HEX);
    Serial.flush();

    // Exit programming mode
    flash_reset_chip();
}

void flash_check_sector_protection_40()
{
    uint8_t sector = 0;
    uint8_t prot = 0;
    do {

        // Enter autoselect mode for info
        flash_write(0x555, 0xAA);
        flash_write(0x2AA, 0x55);
        flash_write(0x555, 0x90);

        // Read sector protection
        prot |= (
                flash_read(0x02UL | (((uint32_t)sector) << 16UL)) // Sector Protection verification
                & 1U) << sector; // 1 means protected

        ++sector;
    } while (sector < 8);

    if (prot < 0x10U)
        Serial.print("0");
    Serial.print(prot, HEX);
    Serial.flush();

    // Exit programming mode
    flash_reset_chip();
}

/* Check sector protection */
void flash_check_sector_protection()
{
    if (dev_id == -1)
    flash_device_id_10(false);

    if (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2) {
            flash_check_sector_protection_10();
    } else {
            flash_check_sector_protection_40();
    }
}

bool flash_all_sectors_protect_10()
{
    bool ret = true;

    uint8_t sector = 0;
    do {
        uint8_t tries = 32;
        uint8_t data1 = 0, data0 = 0;
        do {

            flash_control_oe_12v(0);
            flash_control_a9_12v(0);

            // Unlock for sector protect/unprotect
            flash_write(0x5555, 0xAA);
            flash_write(0x2AAA, 0x55);
            flash_write(0x5555, 0x80);
            flash_write(0x5555, 0xAA);
            flash_write(0x2AAA, 0x55);
            flash_write(0x5555, 0x20);

            // Setup sector address, A9=1, A6=0 (protect), A1=1
            flash_addr_set( 0x201UL | (((uint32_t)sector) << 16UL));
            flash_data_set( 0 );

            flash_control_oe_12v(1);
            flash_control_a9_12v(1);

            // OE = 1, CE = 0
            flash_control_oe(1);  //OE
            flash_control_ce(0);  //CE
            _delay_us(ACCESS_DELAY);

            // Activate WE
            flash_control_we(0);  //WE

            // Programming time: 10us for protect (12ms for unprotect)
            _delay_us(10);

            // Deactivate WE
            flash_control_we(1);  //WE

            // Toggle bit polling
            flash_change_databus_direction(1); // Config as inputs

            // Disable 12 on OE
            flash_control_oe_12v(0);

            // Read first time
            flash_control_oe(0);  //OE
            _delay_us(ACCESS_DELAY); // Give some time for read access
            data0 = flash_data_get();
            flash_control_oe(1);  //OE
            _delay_us(ACCESS_DELAY);

            uint16_t tout = 32000U;
            do {
                // Read second time
                data1 = data0;
                flash_control_oe(0);  //OE
                _delay_us(ACCESS_DELAY); // Give some time for read access
                data0 = flash_data_get();
                flash_control_oe(1);  //OE
                _delay_us(ACCESS_DELAY);
            } while (
                ((data1 ^ data0) & 0x40) != 0 && /* Q6 Toggling means still busy */
                (data0 & 0x20) == 0 && /* Q5 = 0 => But not timedout yet*/
                --tout != 0 /* And internal timeout loop not expired */
            );

            // If timeout, do not bother...
            if (tout == 0) {

                // Failure
                tries = 0;
                break;

            } else {

                // If still toggling...
                if (((data1 ^ data0) & 0x40) != 0) {

                    /* If chip signals timeout ... */
                    if ((data0 & 0x20) != 0) {
                        /* Read twice */

                        // Read first time
                        flash_control_oe(0);  //OE
                        _delay_us(ACCESS_DELAY); // Give some time for read access
                        data1 = flash_data_get();
                        flash_control_oe(1);  //OE
                        _delay_us(ACCESS_DELAY);

                        // Read second time
                        flash_control_oe(0);  //OE
                        _delay_us(ACCESS_DELAY); // Give some time for read access
                        data0 = flash_data_get();
                        flash_control_oe(1);  //OE
                        _delay_us(ACCESS_DELAY);

                        // If still toggling, operation failed
                        if (((data1 ^ data0) & 0x40) != 0) {
                            tries = 0;
                            break;
                        }
                    } /* otherwise, success */
                } /* else success */
            }

            // CE = 1 , A9=1
            flash_control_ce(1);  //CE
            _delay_us(ACCESS_DELAY);

        } while (!(data0 & 1) && --tries != 0);

        // If too many errors, break loop
        if (!tries) {
            ret = false;
            break;
        }
        ++sector;
    } while (sector < 2);

    // Just in case
    flash_control_a9_12v(0);

    // Exit programming mode
    flash_reset_chip();

    // Done
    return ret;
}

bool flash_all_sectors_protect_40()
{
    bool ret = true;

    uint8_t sector = 0;
    do {
        uint8_t tries = 32;
        uint8_t data1 = 0, data0 = 0;
        do {
            flash_control_oe_12v(0);
            flash_control_a9_12v(0);

            // Unlock for sector protect/unprotect
            flash_write(0x555, 0xAA);
            flash_write(0x2AA, 0x55);
            flash_write(0x555, 0x80);
            flash_write(0x555, 0xAA);
            flash_write(0x2AA, 0x55);
            flash_write(0x555, 0x20);

            // Setup sector address, A9=1, A6=0 (protect), A1=1
            flash_addr_set( 0x201UL | (((uint32_t)sector) << 16UL));
            flash_data_set( 0 );

            flash_control_oe_12v(1);
            flash_control_a9_12v(1);

            // OE = 1, CE = 0
            flash_control_oe(1);  //OE
            flash_control_ce(0);  //CE
            _delay_us(ACCESS_DELAY);

            // Activate WE
            flash_control_we(0);  //WE

            // Programming time: 10us for protect (12ms for unprotect)
            _delay_us(10);

            // Deactivate WE
            flash_control_we(1);  //WE

            // Toggle bit polling
            flash_change_databus_direction(1); // Config as inputs

            // Disable 12 on OE
            flash_control_oe_12v(0);

            // Read first time
            flash_control_oe(0);  //OE
            _delay_us(ACCESS_DELAY); // Give some time for read access
            data0 = flash_data_get();
            flash_control_oe(1);  //OE
            _delay_us(ACCESS_DELAY);

            uint16_t tout = 32000U;
            do {
                // Read second time
                data1 = data0;
                flash_control_oe(0);  //OE
                _delay_us(ACCESS_DELAY); // Give some time for read access
                data0 = flash_data_get();
                flash_control_oe(1);  //OE
                _delay_us(ACCESS_DELAY);
            } while (
                ((data1 ^ data0) & 0x40) != 0 && /* Q6 Toggling means still busy */
                (data0 & 0x20) == 0 && /* Q5 = 0 => But not timedout yet*/
                --tout != 0 /* And internal timeout loop not expired */
            );

            // If timeout, do not bother...
            if (tout == 0) {

                // Failure
                tries = 0;
                break;

            } else {

                // If still toggling...
                if (((data1 ^ data0) & 0x40) != 0) {

                    /* If chip signals timeout ... */
                    if ((data0 & 0x20) != 0) {
                        /* Read twice */

                        // Read first time
                        flash_control_oe(0);  //OE
                        _delay_us(ACCESS_DELAY); // Give some time for read access
                        data1 = flash_data_get();
                        flash_control_oe(1);  //OE
                        _delay_us(ACCESS_DELAY);

                        // Read second time
                        flash_control_oe(0);  //OE
                        _delay_us(ACCESS_DELAY); // Give some time for read access
                        data0 = flash_data_get();
                        flash_control_oe(1);  //OE
                        _delay_us(ACCESS_DELAY);

                        // If still toggling, operation failed
                        if (((data1 ^ data0) & 0x40) != 0) {
                            tries = 0;
                            break;
                        }
                    } /* otherwise, success */
                } /* else success */
            }

            // CE = 1 , A9=1
            flash_control_ce(1);  //CE
            _delay_us(ACCESS_DELAY);

        } while (!(data0 & 1) && --tries != 0);

        // If too many errors, break loop
        if (!tries) {
            ret = false;
            break;
        }
        ++sector;
    } while (sector < 8);

    // Just in case
    flash_control_a9_12v(0);

    // Exit programming mode
    flash_reset_chip();

    // Done
    return ret;
}

bool flash_all_sectors_protect()
{
    if (dev_id == -1)
        flash_device_id_10(false);

    if (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2) {
        return flash_all_sectors_protect_10();
    } else {
        return flash_all_sectors_protect_40();
    }
}

bool flash_all_sectors_unprotect_10()
{
    bool ret = true;

    uint8_t sector = 0;
    do {
        uint8_t tries = 32;
        uint8_t data1 = 0, data0 = 0;
        do {

            flash_control_oe_12v(0);
            flash_control_a9_12v(0);

            // Unlock for sector protect/unprotect
            flash_write(0x5555, 0xAA);
            flash_write(0x2AAA, 0x55);
            flash_write(0x5555, 0x80);
            flash_write(0x5555, 0xAA);
            flash_write(0x2AAA, 0x55);
            flash_write(0x5555, 0x20);

            // Setup sector address, A9=1, A6=1 (unprotect), A1=1
            flash_addr_set( 0x241UL | (((uint32_t)sector) << 16UL));
            flash_data_set( 0 );

            flash_control_oe_12v(1);
            flash_control_a9_12v(1);

            // OE = 1, CE = 0
            flash_control_oe(1);  //OE
            flash_control_ce(0);  //CE
            _delay_us(ACCESS_DELAY);

            // Activate WE
            flash_control_we(0);  //WE

            // Programming time: 12ms for unprotect
            _delay_ms(12);

            // Deactivate WE
            flash_control_we(1);  //WE

            // Toggle bit polling
            flash_change_databus_direction(1); // Config as inputs

            // Disable 12 on OE
            flash_control_oe_12v(0);

            // Read first time
            flash_control_oe(0);  //OE
            _delay_us(ACCESS_DELAY); // Give some time for read access
            data0 = flash_data_get();
            flash_control_oe(1);  //OE
            _delay_us(ACCESS_DELAY);

            uint16_t tout = 32000U;
            do {
                // Read second time
                data1 = data0;
                flash_control_oe(0);  //OE
                _delay_us(ACCESS_DELAY); // Give some time for read access
                data0 = flash_data_get();
                flash_control_oe(1);  //OE
                _delay_us(ACCESS_DELAY);
            } while (
                ((data1 ^ data0) & 0x40) != 0 && /* Q6 Toggling means still busy */
                (data0 & 0x20) == 0 && /* Q5 = 0 => But not timedout yet*/
                --tout != 0 /* And internal timeout loop not expired */
            );

            // If timeout, do not bother...
            if (tout == 0) {

                // Failure
                tries = 0;
                break;

            } else {

                // If still toggling...
                if (((data1 ^ data0) & 0x40) != 0) {

                    /* If chip signals timeout ... */
                    if ((data0 & 0x20) != 0) {
                        /* Read twice */

                        // Read first time
                        flash_control_oe(0);  //OE
                        _delay_us(ACCESS_DELAY); // Give some time for read access
                        data1 = flash_data_get();
                        flash_control_oe(1);  //OE
                        _delay_us(ACCESS_DELAY);

                        // Read second time
                        flash_control_oe(0);  //OE
                        _delay_us(ACCESS_DELAY); // Give some time for read access
                        data0 = flash_data_get();
                        flash_control_oe(1);  //OE
                        _delay_us(ACCESS_DELAY);

                        // If still toggling, operation failed
                        if (((data1 ^ data0) & 0x40) != 0) {
                            tries = 0;
                            break;
                        }
                    } /* otherwise, success */
                } /* else success */
            }

            // CE = 1 , A9=1
            flash_control_ce(1);  //CE
            _delay_us(ACCESS_DELAY);

        } while (!(data0 & 1) && --tries != 0);

        // If too many errors, break loop
        if (!tries) {
            ret = false;
            break;
        }
        ++sector;
    } while (sector < 2);

    // Just in case
    flash_control_a9_12v(0);

    // Exit programming mode
    flash_reset_chip();

    // Done
    return ret;
}

bool flash_all_sectors_unprotect_40()
{
    bool ret = true;

    uint8_t sector = 0;
    do {
        uint8_t tries = 32;
        uint8_t data1 = 0, data0 = 0;
        do {

            flash_control_oe_12v(0);
            flash_control_a9_12v(0);

            // Unlock for sector protect/unprotect
            flash_write(0x555, 0xAA);
            flash_write(0x2AA, 0x55);
            flash_write(0x555, 0x80);
            flash_write(0x555, 0xAA);
            flash_write(0x2AA, 0x55);
            flash_write(0x555, 0x20);

            // Setup sector address, A9=1, A6=1 (unprotect), A1=1
            flash_addr_set( 0x241UL | (((uint32_t)sector) << 16UL));
            flash_data_set( 0 );

            flash_control_oe_12v(1);
            flash_control_a9_12v(1);

            // OE = 1, CE = 0
            flash_control_oe(1);  //OE
            flash_control_ce(0);  //CE

            // Activate WE
            flash_control_we(0);  //WE
            _delay_us(ACCESS_DELAY);

            // Programming time: 12ms for unprotect
            _delay_ms(12);

            // Deactivate WE
            flash_control_we(1);  //WE

            // Toggle bit polling
            flash_change_databus_direction(1); // Config as inputs

            // Disable 12 on OE
            flash_control_oe_12v(0);

            // Read first time
            flash_control_oe(0);  //OE
            _delay_us(ACCESS_DELAY); // Give some time for read access
            data0 = flash_data_get();
            flash_control_oe(1);  //OE
            _delay_us(ACCESS_DELAY);

            uint16_t tout = 32000U;
            do {
                // Read second time
                data1 = data0;
                flash_control_oe(0);  //OE
                _delay_us(ACCESS_DELAY); // Give some time for read access
                data0 = flash_data_get();
                flash_control_oe(1);  //OE
                _delay_us(ACCESS_DELAY);
            } while (
                ((data1 ^ data0) & 0x40) != 0 && /* Q6 Toggling means still busy */
                (data0 & 0x20) == 0 && /* Q5 = 0 => But not timedout yet*/
                --tout != 0 /* And internal timeout loop not expired */
            );

            // If timeout, do not bother...
            if (tout == 0) {

                // Failure
                tries = 0;
                break;

            } else {

                // If still toggling...
                if (((data1 ^ data0) & 0x40) != 0) {

                    /* If chip signals timeout ... */
                    if ((data0 & 0x20) != 0) {
                        /* Read twice */

                        // Read first time
                        flash_control_oe(0);  //OE
                        _delay_us(ACCESS_DELAY); // Give some time for read access
                        data1 = flash_data_get();
                        flash_control_oe(1);  //OE
                        _delay_us(ACCESS_DELAY);

                        // Read second time
                        flash_control_oe(0);  //OE
                        _delay_us(ACCESS_DELAY); // Give some time for read access
                        data0 = flash_data_get();
                        flash_control_oe(1);  //OE
                        _delay_us(ACCESS_DELAY);

                        // If still toggling, operation failed
                        if (((data1 ^ data0) & 0x40) != 0) {
                            tries = 0;
                            break;
                        }
                    } /* otherwise, success */
                } /* else success */
            }

            // CE = 1 , A9=1
            flash_control_ce(1);  //CE
            _delay_us(ACCESS_DELAY);

        } while (!(data0 & 1) && --tries != 0);

        // If too many errors, break loop
        if (!tries) {
            ret = false;
            break;
        }
        ++sector;
    } while (sector < 8);

    // Just in case
    flash_control_a9_12v(0);

    // Exit programming mode
    flash_reset_chip();

    // Done
    return ret;

}

bool flash_all_sectors_unprotect()
{
    if (dev_id == -1)
        flash_device_id_10(false);

    if (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2) {
        return flash_all_sectors_unprotect_10();
    } else {
        return flash_all_sectors_unprotect_40();
    }

}

void test_addresses()
{
    flash_ctrl_deselect();
    flash_change_databus_direction(0); // Config as outputs
    flash_addr_set(0UL);
    flash_data_set(0U);

    Serial.println("Testing Address lines");

    Serial.println("A0 must be at +5v, all the others (A1-A18) should be 0v");
    flash_addr_set(1UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A1 must be at +5v, all the others should be 0v");
    flash_addr_set(2UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A2 must be at +5v, all the others should be 0v");
    flash_addr_set(4UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A3 must be at +5v, all the others should be 0v");
    flash_addr_set(8UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A4 must be at +5v, all the others should be 0v");
    flash_addr_set(16UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A5 must be at +5v, all the others should be 0v");
    flash_addr_set(32UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A6 must be at +5v, all the others should be 0v");
    flash_addr_set(64UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A7 must be at +5v, all the others should be 0v");
    flash_addr_set(128UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A8 must be at +5v, all the others should be 0v");
    flash_addr_set(256UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A9 must be at +5v, all the others should be 0v");
    flash_addr_set(512UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A10 must be at +5v, all the others should be 0v");
    flash_addr_set(1024UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A11 must be at +5v, all the others should be 0v");
    flash_addr_set(2048UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A12 must be at +5v, all the others should be 0v");
    flash_addr_set(4096UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A13 must be at +5v, all the others should be 0v");
    flash_addr_set(8192UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A14 must be at +5v, all the others should be 0v");
    flash_addr_set(16384UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A15 must be at +5v, all the others should be 0v");
    flash_addr_set(32768UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A16 must be at +5v, all the others should be 0v");
    flash_addr_set(65536UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A17 must be at +5v, all the others should be 0v");
    flash_addr_set(131072UL);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("A18 must be at +5v, all the others should be 0v");
    flash_addr_set(262144UL);
    while (!Serial.available());
    (void)Serial.read();

    flash_addr_set(0);

    Serial.println("Testing Data lines");

    Serial.println("D0 must be at +5v, all the others D1-D7 should be 0v");
    flash_data_set(1U);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("D1 must be at +5v, all the others should be 0v");
    flash_data_set(2U);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("D2 must be at +5v, all the others should be 0v");
    flash_data_set(4U);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("D3 must be at +5v, all the others should be 0v");
    flash_data_set(8U);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("D4 must be at +5v, all the others should be 0v");
    flash_data_set(16U);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("D5 must be at +5v, all the others should be 0v");
    flash_data_set(32U);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("D6 must be at +5v, all the others should be 0v");
    flash_data_set(64U);
    while (!Serial.available());
    (void)Serial.read();

    Serial.println("D7 must be at +5v, all the others should be 0v");
    flash_data_set(128U);
    while (!Serial.available());
    (void)Serial.read();

    flash_data_set(0);

    Serial.println("Test Done");
}

void eeprom_i2c_write(byte address, byte from_addr, byte data) {
  Wire.beginTransmission(address);
  Wire.write(from_addr);
  Wire.write(data);
  Wire.endTransmission();
}

byte eeprom_i2c_read(int address, int from_addr) {
  Wire.beginTransmission(address);
  Wire.write(from_addr);
  Wire.endTransmission();

  Wire.requestFrom(address, 1);
  if(Wire.available())
    return Wire.read();
  else
    return 0xFF;
}
#define ADDR_24C02 0b01010000

void i2c_read_memory()
{
    // Determine maximum address to read
    uint32_t max_addr = 256;

    /* Read the chip until the address given */
    for (uint32_t addr = 0UL; addr < max_addr; ++addr) {
        uint8_t d = eeprom_i2c_read(ADDR_24C02, addr);
        if (d < 0x10U)
            Serial.print("0");
        Serial.print(d, HEX);
    }
    Serial.flush();
}

void i2c_program_memory()
{
    // Ack the Write command
    Serial.print("+");
    Serial.flush();

    uint32_t maxaddr = 256;
    uint32_t address = 0;
    while(address < maxaddr) {

        // Wait for a character
        while (!Serial.available());
        uint8_t data = Serial.read();

        // Try to program byte
        eeprom_i2c_write(ADDR_24C02, address, data);
        _delay_ms(15); // (to be on the safe side)
        
        // Ack received byte
        Serial.print("+");
        Serial.flush();

        ++address;
    }
}


void setup()
{
  Serial.begin(115200);

  Wire.begin(); // i2c setup
  
  //ANALOG_CONFIG;

  //Set the chip controller
  pinMode(A912V_PIN, OUTPUT);
  pinMode(OE12V_PIN, OUTPUT);
  digitalWrite(A912V_PIN, LOW);
  digitalWrite(OE12V_PIN, LOW);

  pinMode(CE_PIN, OUTPUT);
  pinMode(WE_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);
  digitalWrite(CE_PIN, HIGH);
  digitalWrite(WE_PIN, HIGH);
  digitalWrite(OE_PIN, HIGH);

  //Set data pin mode
  pinMode(DQ0_PIN, INPUT);
  pinMode(DQ1_PIN, INPUT);
  pinMode(DQ2_PIN, INPUT);
  pinMode(DQ3_PIN, INPUT);
  pinMode(DQ4_PIN, INPUT);
  pinMode(DQ5_PIN, INPUT);
  pinMode(DQ6_PIN, INPUT);
  pinMode(DQ7_PIN, INPUT);

  //set the highest address pins
  pinMode(A0_PIN, OUTPUT);
  pinMode(A1_PIN, OUTPUT);
  pinMode(A2_PIN, OUTPUT);
  pinMode(A3_PIN, OUTPUT);
  pinMode(A4_PIN, OUTPUT);
  pinMode(A5_PIN, OUTPUT);
  pinMode(A6_PIN, OUTPUT);
  pinMode(A7_PIN, OUTPUT);
  pinMode(A8_PIN, OUTPUT);
  pinMode(A9_PIN, OUTPUT);
  pinMode(A10_PIN, OUTPUT);
  pinMode(A11_PIN, OUTPUT);
  pinMode(A12_PIN, OUTPUT);
  pinMode(A13_PIN, OUTPUT);
  pinMode(A14_PIN, OUTPUT);
  pinMode(A15_PIN, OUTPUT);
  pinMode(A16_PIN, OUTPUT);
  pinMode(A17_PIN, OUTPUT);
  pinMode(A18_PIN, OUTPUT);

  _delay_ms(1000);

  flash_ctrl_deselect();
}

void loop()
{
    if (!Serial.available())
        return;

    char cmd = Serial.read();
    switch(cmd) {
        case 'C':
            flash_check_sector_protection();
            Serial.println("");
            break;

        case 'R':
            flash_read_memory();
            Serial.println("");
            break;

        case 'E':
            flash_erase_memory();
            Serial.println("");
            break;

        case 'W':
            flash_program_memory();
            Serial.println("");
            break;

        case 'I': // Manufacturer ID
            flash_manufacturer_id();
            Serial.println("");
            break;

        case 'D': // Device ID
            flash_device_id();
            Serial.println("");
            break;

        case 'L': // Sector protect
            flash_all_sectors_protect();
            Serial.println("");
            break;

        case 'U': // Sector unprotect
            flash_all_sectors_unprotect();
            Serial.println("");
            break;

        case 'T': // Test
        case 't': // Test
            test_addresses();
            break;

        /* I2C */
        case 'r':
            i2c_read_memory();
            Serial.println("");
            break;

        case 'w':
            i2c_program_memory();
            Serial.println("");
            break;

        default:
            break;
    }
}
