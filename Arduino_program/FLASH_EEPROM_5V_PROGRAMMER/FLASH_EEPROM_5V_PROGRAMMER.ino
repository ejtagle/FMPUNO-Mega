/* @Author: Radouane SAMIR
   @Version: 0.1
   @Chip version: AM29F010 A29040
   
   This code is tested on AM29F010.
   
   For AM29F040B the command sequence are little bit different:
   0x555 rather than 0x5555 see the datasheet. But the timing is the same
   
   You are free to modify the code as you want, but please keep my name as is.
*/

#include <avr/io.h>
#include <util/delay.h>

#define ACCESS_DELAY 1

/*
 D54 - D61=> DQ0-DQ7
 D62 - D69=> A0-A7
 D52 - A8
 D50 - A9
 D48 - A10
 D46 - A11
 D44 - A12
 D42 - A13
 D40 - A14
 D38 - A15
 D36 - A16
 D34 - CE
 D32 - WE
 D30 - OE
 */

#define DQ0_PIN 54
#define DQ1_PIN 55
#define DQ2_PIN 56
#define DQ3_PIN 57
#define DQ4_PIN 58
#define DQ5_PIN 59
#define DQ6_PIN 60
#define DQ7_PIN 61
  
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

#define CE_PIN 34
#define WE_PIN 32
#define OE_PIN 30

#define DDR_DATA DDRF
#define PORT_DATA PORTF
#define PIN_DATA PINF

#define D27M1001 0xC3
#define D27M1001_V2 0x83
#define D29F010 0x20
#define D29F040 0xA4
#define D29040  0x86
int dev_id = -1; // Autodetected device ID

/* Need a fast change mode IN/OUT. If io = 1, configure them as inputs, otherwise as outputs */
void flash_change_databus_direction(boolean io)
{
  if (io) {
    /* Configure all as inputs */
    DDR_DATA = 0;
  }
  else
  {
    /* Configure all as outputs */
    DDR_DATA = 0xFF;
  }
}

void flash_ctrl_deselect()
{
  digitalWrite(CE_PIN, HIGH);  //CE
  digitalWrite(WE_PIN, HIGH);  //WE
  digitalWrite(OE_PIN, HIGH);  //OE
}

void flash_ctrl_rd()
{
  digitalWrite(CE_PIN, LOW);
  digitalWrite(OE_PIN, LOW);
}

void flash_ctrl_wr()
{
  digitalWrite(CE_PIN, LOW);
  digitalWrite(WE_PIN, LOW);
}  

/**************************************************
   Function to access to 
   the highest addresses in the chip 
   Highest 16bits is given to the function
   and do a bit manipulation to set pins High/Low
***************************************************/

void flash_set_a16_to_a18_addresses(uint16_t addr)
{ 
  /* Highest addresses */
  
  if (addr & 1U)
    A16_PORT |= (1U<<A16_BIT);
  else
    A16_PORT &= ~(1U<<A16_BIT);
  
  if (addr & 2U)
    A17_PORT |= (1U<<A17_BIT);
  else
    A17_PORT &= ~(1U<<A17_BIT);
  
  if (addr & 4U)
    A18_PORT |= (1U<<A18_BIT);
  else
    A18_PORT &= ~(1U<<A18_BIT);
}


/** 
 * Calculated the given address
 **************************************/ 

void flash_set_a0_to_a15_addresses(uint16_t addr)
{ 
  if (addr & 1U)
    A0_PORT |= (1U<<A0_BIT);
  else
    A0_PORT &= ~(1U<<A0_BIT);

  if (addr & 2U)
    A1_PORT |= (1U<<A1_BIT);
  else
    A1_PORT &= ~(1U<<A1_BIT);

  if (addr & 4U)
    A2_PORT |= (1U<<A2_BIT);
  else
    A2_PORT &= ~(1U<<A2_BIT);

  if (addr & 8U)
    A3_PORT |= (1U<<A3_BIT);
  else
    A3_PORT &= ~(1U<<A3_BIT);

  if (addr & 16U)
    A4_PORT |= (1U<<A4_BIT);
  else
    A4_PORT &= ~(1U<<A4_BIT);

  if (addr & 32U)
    A5_PORT |= (1U<<A5_BIT);
  else
    A5_PORT &= ~(1U<<A5_BIT);

  if (addr & 64U)
    A6_PORT |= (1U<<A6_BIT);
  else
    A6_PORT &= ~(1U<<A6_BIT);

  if (addr & 128U)
    A7_PORT |= (1U<<A7_BIT);
  else
    A7_PORT &= ~(1U<<A7_BIT);

  if (addr & 256U)
    A8_PORT |= (1U<<A8_BIT);
  else
    A8_PORT &= ~(1U<<A8_BIT);

  if (addr & 512U)
    A9_PORT |= (1U<<A9_BIT);
  else
    A9_PORT &= ~(1U<<A9_BIT);

  if (addr & 1024U)
    A10_PORT |= (1U<<A10_BIT);
  else
    A10_PORT &= ~(1U<<A10_BIT);

  if (addr & 2048U)
    A11_PORT |= (1U<<A11_BIT);
  else
    A11_PORT &= ~(1U<<A11_BIT);

  if (addr & 4096U)
    A12_PORT |= (1U<<A12_BIT);
  else
    A12_PORT &= ~(1U<<A12_BIT);

  if (addr & 8192U)
    A13_PORT |= (1U<<A13_BIT);
  else
    A13_PORT &= ~(1U<<A13_BIT);

  if (addr & 16384U)
    A14_PORT |= (1U<<A14_BIT);
  else
    A14_PORT &= ~(1U<<A14_BIT);

  if (addr & 32768U)
    A15_PORT |= (1U<<A15_BIT);
  else
    A15_PORT &= ~(1U<<A15_BIT);

}

/**
 * Set the address *
 *******************/

void flash_addr_set(uint32_t addr)
{
  uint16_t LSB = addr;
  uint16_t MSB = (addr>>16);
  
  flash_set_a0_to_a15_addresses(LSB);
  flash_set_a16_to_a18_addresses(MSB);
}

/**
 * Calculate the data given and set change pins state
 *****************************************************/

void flash_data_set(uint8_t data)
{
  PORT_DATA = data;
}

/** 
 * Get Data from the chip *
 * MIND: the pull-down resistors
 *********************************/

byte flash_data_get()
{
  return PIN_DATA;
}

/*
 * Data polling in programming operation
 *****************************************/
boolean flash_DQ7_byte_poll(uint32_t addr, uint8_t data)
{
  boolean ret = true;
  
  uint8_t octect = 0;
  flash_change_databus_direction(1); // Config as inputs
  flash_addr_set(addr);
  _delay_us(ACCESS_DELAY); // Give some time for address stabilization
  flash_ctrl_rd();
  _delay_us(ACCESS_DELAY); // Give some time for read access
  
  uint8_t loops = 140U; // at 20 mhz, 7us is 140 cycles
  octect = flash_data_get();
  while (((octect ^ data) & 0x80) != 0 && --loops != 0U)
    octect = flash_data_get();
  if (!loops) {
    ret = false;
/*     
    Serial.println("Timeout on flash_DQ7_byte_poll");
    Serial.print(" read:");
    Serial.print(octect,HEX);
    Serial.print(" sent:");
    Serial.println(data,HEX);
 */
  }

  flash_ctrl_deselect();
  _delay_us(ACCESS_DELAY); // Give some time for deselection
  return ret;
}

boolean flash_DQ7_erase_poll()
{
  boolean ret = true;
  
  uint8_t octect = 0;
  flash_change_databus_direction(1); // Config as inputs
  flash_addr_set(0);
  _delay_us(ACCESS_DELAY); // Give some time for address stabilization
  flash_ctrl_rd();
  _delay_us(ACCESS_DELAY); // Give some time for read access
  
  uint32_t loops = 1280000000UL; // Up to 64 seconds, at 20 mhz
  octect = flash_data_get();
  while ((octect & 0x80) != 0x80 && --loops != 0UL)
    octect = flash_data_get();

  if (!loops) {
    //Serial.println("Timeout on flash_DQ7_erase_poll");
    ret = false;
  } else {
    if (octect) {
      //Serial.println("Erasure complete");
      ret = true;
    }
  }
  
  flash_ctrl_deselect();
  _delay_us(ACCESS_DELAY); // Give some time for deselection
  return ret;
}

/****
 * Set command sequence and send it to the chip
 ***********************************************/

void flash_send_command(uint32_t addr, uint8_t data)
{
  /* Send command sequence */
  flash_addr_set(addr);
  flash_data_set(data);
  _delay_us(ACCESS_DELAY); // Give some time for data setup
  flash_ctrl_wr();
  _delay_us(ACCESS_DELAY); // Give some time for write
  flash_ctrl_deselect();
  _delay_us(ACCESS_DELAY); // Give some time for deselection
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
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode
  flash_send_command(0x5555, 0xAA);
  flash_send_command(0x2AAA, 0x55);
  flash_send_command(0x5555, 0x90);
 
  _delay_ms(1000);
  
  flash_change_databus_direction(1); // Config as inputs
  flash_addr_set(0x01); // Device ID
  _delay_us(ACCESS_DELAY); // Give some time for address setup
  flash_ctrl_rd();
  _delay_us(ACCESS_DELAY); // Give some time for read access
    
  byte d = flash_data_get();
  dev_id = d; // Store device ID
  if (display) {
    Serial.print(d, HEX);
    Serial.flush();
    //Serial.print("Get device ID complete, Please wait restarting the chip ... ");
  }
  _delay_ms(1);
  
  // Exit autoselect mode
  flash_reset_chip();
  //Serial.println("Done.");
}

/** AM29F040 **/
/***************/

void flash_device_id_40()
{
  /* This is used to get code identification 
   * from the chip in autoselect mode
   * See the datasheet
   ******************************************/
  
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode
  flash_send_command(0x555, 0xAA);
  flash_send_command(0x2AA, 0x55);
  flash_send_command(0x555, 0x90);
 
  _delay_ms(1000);
  
  flash_change_databus_direction(1); // Config as inputs

  flash_addr_set(0x01); // Device ID
  _delay_us(ACCESS_DELAY); // Give some time for address setup
  flash_ctrl_rd();
  _delay_us(ACCESS_DELAY); // Give some time for read access
  
  Serial.print(flash_data_get(), HEX);
  Serial.flush();
  //Serial.print("Get device ID complete, Please wait restarting the chip ... ");
  _delay_ms(1);
  
  // Exit autoselect mode
  flash_reset_chip();
  //Serial.println("Done.");
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
   
  byte data = 0x00;
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode for info
  flash_send_command(0x5555, 0xAA);
  flash_send_command(0x2AAA, 0x55);
  flash_send_command(0x5555, 0x90);

  _delay_ms(1000);
  
  flash_change_databus_direction(1); // Config as inputs
  
  flash_addr_set(0x00); // Manufacturer ID
  _delay_us(ACCESS_DELAY); // Give some time for address setup
  flash_ctrl_rd();
  _delay_us(ACCESS_DELAY); // Give some time for read access
  
  data = flash_data_get();
  if (data < 0x10) 
    Serial.print("0");
  Serial.print(data, HEX);
  Serial.flush();
  
  flash_ctrl_deselect();
  _delay_us(ACCESS_DELAY); // Give some time for deselection
  
  // Exit autoselect mode
  flash_reset_chip();
}

void flash_manufacturer_id_40()
{
  /* This is used to get code identification 
   * from the chip in autoselect mode
   * See the datasheet
   ******************************************/
   
  byte data = 0x00;
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode for info
  flash_send_command(0x555, 0xAA);
  flash_send_command(0x2AA, 0x55);
  flash_send_command(0x555, 0x90);

  _delay_ms(1000);
  
  flash_change_databus_direction(1); // Config as inputs
  
  flash_addr_set(0x00); // Manufacturer ID
  _delay_us(ACCESS_DELAY); // Give some time for address setup
  flash_ctrl_rd();
  _delay_us(ACCESS_DELAY); // Give some time for read access
  
  data = flash_data_get();
  if (data < 0x10) 
    Serial.print("0");
  Serial.print(data, HEX);
  Serial.flush();
  
  flash_ctrl_deselect();
  _delay_us(ACCESS_DELAY); // Give some time for deselection
  
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
  uint32_t addr;
  if (dev_id == -1)
    flash_device_id_10(false);

  // Determine maximum address to read
  addr = (dev_id == D29F010 || dev_id == D27M1001 || dev_id == D27M1001_V2) ? 0x20000UL : 0x80000UL;
    
  /* Read the chip until the address given */  
  flash_change_databus_direction(1); // Config as inputs
  
  for (uint32_t i = 0UL; i < addr; i++)
  {
    flash_addr_set(i);
    _delay_us(ACCESS_DELAY); // Give some time for address setup
    flash_ctrl_rd();
    _delay_us(ACCESS_DELAY); // Give some time for read access
    
    byte data = flash_data_get();
    flash_ctrl_deselect();
    _delay_us(ACCESS_DELAY); // Give some time for deselection
    
    if (data < 0x10) 
        Serial.print("0");
    Serial.print(data, HEX);
    Serial.flush();
  }
}

/* This is the sequence to program a byte on the memory *
 * 0x5555 0x2AAA is for AM29F010
 * 0x555 0x2AA is for AM29F040 A29F040
 ********************************************************/

boolean flash_program_byte_10(uint32_t addr, uint8_t data)
{
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode for program
  flash_send_command(0x5555, 0xAA); 
  flash_send_command(0x2AAA, 0x55);
  flash_send_command(0x5555, 0xA0);
 
  //Program Address & Program data
  flash_send_command(addr, data);
  
  /** Data Polling **/
  flash_DQ7_byte_poll(addr, data);

  return true;
}

boolean flash_program_byte_40(uint32_t addr, uint8_t data)
{
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode for program
  flash_send_command(0x555, 0xAA); 
  flash_send_command(0x2AA, 0x55);
  flash_send_command(0x555, 0xA0);
  
  //Program Address & Program data
  flash_send_command(addr, data);
  
  /** Data Polling **/
  flash_DQ7_byte_poll(addr, data);
  
  return true;
}

boolean flash_program_byte(uint32_t addr, uint8_t data)
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
  /* This is reset command for the AM29F010 */
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode for chip reset
  flash_send_command(0x5555, 0xAA);
  flash_send_command(0x2AAA, 0x55);
  flash_send_command(0x5555, 0xF0);
  
  _delay_ms(1000);
}

void flash_reset_chip_40()
{
  /* This is reset command for the AM29F010 */
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode for chip reset
  flash_send_command(0x555, 0xAA);
  flash_send_command(0x2AA, 0x55);
  flash_send_command(0x555, 0xF0);
   
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
  boolean ret = true;
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode for chip erase
  flash_send_command(0x5555, 0xAA);
  flash_send_command(0x2AAA, 0x55);
  flash_send_command(0x5555, 0x80);
  flash_send_command(0x5555, 0xAA);
  flash_send_command(0x2AAA, 0x55);
  flash_send_command(0x5555, 0x10);
 
  _delay_ms(1000);
  ret = flash_DQ7_erase_poll();
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
  boolean ret = true;
  flash_change_databus_direction(0); // Config as outputs
  
  // Enter autoselect mode for chip erase
  flash_send_command(0x555, 0xAA);
  flash_send_command(0x2AA, 0x55);
  flash_send_command(0x555, 0x80);
  flash_send_command(0x555, 0xAA);
  flash_send_command(0x2AA, 0x55);
  flash_send_command(0x555, 0x10);
  
  _delay_ms(1000);
  ret = flash_DQ7_erase_poll();
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
    uint32_t address = 0;
    
    flash_change_databus_direction(0); // Config as outputs
    
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
    
    uint32_t maxaddr = (uint32_t)atol(tab);
    while(address <= maxaddr) {
      // Wait for a character
      while (!Serial.available());

        uint8_t data = Serial.read();
      flash_program_byte(address++, data); 
      // Ack received byte
      Serial.print("+"); 
      Serial.flush();
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

void setup()
{
  Serial.begin(115200);
  //ANALOG_CONFIG;
  
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

  //Set the chip controller
  pinMode(CE_PIN, OUTPUT);
  pinMode(WE_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);

  digitalWrite(CE_PIN, HIGH);
  digitalWrite(WE_PIN, HIGH);
  digitalWrite(OE_PIN, HIGH);
  
  _delay_ms(1000);
  
  flash_ctrl_deselect();
}

void loop()
{ 
  if (!Serial.available())
      return;
  
  char cmd = Serial.read();
  switch(cmd)
  {
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
  
    case 'T': // Test
    case 't': // Test
        test_addresses();
    break;
    
  
    default:
        break;
  }
}
