#ifndef JTAG_IF_H
#define JTAG_IF_H

#include "mem.h"

#include <stdint.h>

class JtagIF : public MemIF {
  public:
    JtagIF();
    ~JtagIF();

    bool access(bool write, unsigned int addr, int size, char* buffer);

  private:
  	bool configure_chain();
	bool mem_write(uint32_t addr, int size, bool word, uint32_t wdata);
    bool mem_read(uint32_t addr, uint32_t *rdata);
};

#endif
