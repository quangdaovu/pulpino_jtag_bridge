#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <stdarg.h>
#include <string.h> 
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "chain_commands.h"
#include "cable_common.h"
#include "adv_dbg_commands.h"
#include "errcodes.h"
#ifdef __cplusplus
}
#endif

#include "jtagif.h"

#define debug(...) fprintf(stderr, __VA_ARGS__)
#define IDCODE_PULPINO 0x249511c3

uint32_t *idcodes = NULL;
int num_devices = 0;
int target_dev_pos = 0;
const char *name_pulpino = "PULPino";

JtagIF::JtagIF() 
{
  uint32_t info;
  uint32_t data;
  int err;
  uint32_t err_data[2];
  err_data[0] = 0;
  err_data[1] = 0;

  cable_setup();

  if(cable_init() != APP_ERR_NONE)
  {
    printf("Failed to initialize cable, aborting.\n");
    exit(1);
  }

  this->configure_chain();

  adbg_select_module(0);  
}

JtagIF::~JtagIF() 
{

}

// Resets JTAG, and sets up DEBUG scan chain
bool JtagIF::configure_chain()
{
  int i;
  unsigned int manuf_id;
  uint32_t cmd;  
  uint32_t id_read;
  const char *name;
  int irlen;
  int err = APP_ERR_NONE;

  err |= tap_reset();
  err |= jtag_enumerate_chain(&idcodes, &num_devices);

  if(err != APP_ERR_NONE) 
  {
    printf("Error %s enumerating JTAG chain, aborting.\n", get_err_string(err));
    return false;
  }

  printf("\nDevices on JTAG chain:\n");
  printf("Index\tName\t\tID Code\t\tIR Length\n");
  printf("----------------------------------------------------------------\n");
  for(i = 0; i < num_devices; i++)
  {
    if(idcodes[i] == IDCODE_PULPINO) {
      name = name_pulpino;
      irlen = 4;
    }
    printf("%d: \t%s \t0x%08X \t%d\n", i, name, idcodes[i], irlen);
  }
  printf("\n");

  printf("Target device %i, JTAG ID = 0x%08x\n", target_dev_pos, idcodes[target_dev_pos]);

  config_set_IR_size(irlen);
  config_set_IR_postfix_bits(0);
  config_set_IR_prefix_bits(0);  
  config_set_DR_prefix_bits(num_devices - target_dev_pos - 1);
  config_set_DR_postfix_bits(target_dev_pos);

  cmd = 0x8; // Debug cmd
  config_set_debug_cmd(cmd);

  // Do a sanity test
  cmd = 0x2; // IDCODE cmd
  err |= jtag_get_idcode(cmd, &id_read);

  if(err != APP_ERR_NONE) 
  {
    printf("Error %s checking IDCODE, aborting.\n", get_err_string(err));
    return false;
  }

  if(id_read == idcodes[target_dev_pos])
    printf("IDCODE sanity test passed, chain OK!\n");
  else
    printf("Warning: IDCODE sanity test failed.  Read IDCODE 0x%08X, expected 0x%08X\n", id_read, idcodes[target_dev_pos]);

  // Select the debug unit in the TAP.
  if(err |= tap_enable_debug_module()) 
  {  
    printf("Error %s enabling debug module, aborting.\n", get_err_string(err));
    return false;
  }

  return true;
}

bool JtagIF::access(bool write, unsigned int addr, int size, char* buffer)
{
  bool retvar = true;
  uint32_t rdata;
  int remain;

  if (write)
  {
    remain = size % 4;
    size /= 4;

    while (size > 0)
    {
      retvar = retvar && this->mem_write(addr, 1, true, *((uint32_t*)buffer));

      addr   += 4;
      size   -= 1;
      buffer += 4;
    }

    if (remain > 0)
      retvar = retvar && this->mem_write(addr, remain, false, *((uint32_t*)buffer));
  }
  else
  {
    while (size >= 4)
    {
      retvar = retvar && this->mem_read(addr, &rdata);  

      buffer[0] = rdata;
      buffer[1] = rdata >> 8;
      buffer[2] = rdata >> 16;
      buffer[3] = rdata >> 24;

      addr   += 4;
      size   -= 4;
      buffer += 4;
    }
    
    if (size > 0)
    {
      retvar = retvar && this->mem_read(addr, &rdata);  

      if (size == 2)
      {
        buffer[0] = rdata;
        buffer[1] = rdata >> 8;
      }
      else
      {
        buffer[0] = rdata;
      }
    }
  }

  return retvar;
}

bool JtagIF::mem_write(uint32_t addr, int size, bool word, uint32_t wdata)
{
  int err = APP_ERR_NONE;

  if (word)
    err |= adbg_wb_burst_write((void*)&wdata, 4, size, addr);
  else
    err |= adbg_wb_burst_write((void*)&wdata, 1, size, addr);

  if (err != APP_ERR_NONE)
    return false;
  else
    return true;
}

bool JtagIF::mem_read(uint32_t addr,  uint32_t *rdata)
{
  int err = APP_ERR_NONE;

  err |= adbg_wb_burst_read(4, 1, addr, rdata);

  if (err != APP_ERR_NONE)
    return false;
  else
    return true;
}


// data |= 0x1 << 16;
// err |= adbg_wb_burst_write((void*)&data, 4, 1, 0x1A110000);
// sleep(1);
// data = 0;
// err |= adbg_wb_burst_write((void*)&data, 4, 1, 0x1A110000);
