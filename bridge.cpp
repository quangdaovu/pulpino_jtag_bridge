#include "bridge.h"
#include <stdarg.h>

bool platform_pulpino(MemIF* mem, std::list<DbgIF*>* p_list, LogIF *log) {
  p_list->push_back(new DbgIF(mem, 0x1A110000, log));

  return true;
}

Bridge::Bridge(Platforms platform, int portNumber, LogIF *log) {
  initBridge(platform, portNumber, NULL, log);
}

Bridge::Bridge(Platforms platform, MemIF *memIF, LogIF *log) {
  initBridge(platform, -1, memIF, log);
}

void Bridge::initBridge(Platforms platform, int portNumber, MemIF *memIF, LogIF *log) {

  // initialization
  if (log == NULL)
    this->log = this;
  else
    this->log = log;

  mem = new JtagIF();

  platform_pulpino(mem, &dbgifs, this->log);
  cache = new Cache(mem, &dbgifs);

  bp = new BreakPoints(mem, cache);

  rsp = new Rsp(1234, mem, this->log, dbgifs, bp);
}

void Bridge::mainLoop()
{
  // main loop
  while (1) {
    rsp->open();
    while(!rsp->wait_client());
    rsp->loop();
    rsp->close();
  }
}

Bridge::~Bridge()
{
  // cleanup
  delete rsp;

  for (std::list<DbgIF*>::iterator it = dbgifs.begin(); it != dbgifs.end(); it++) {
    delete (*it);
  }

  delete bp;
  delete cache;
  delete mem;
}

void Bridge::user(char *str, ...)
{
  va_list va;
  va_start(va, str);
  vprintf(str, va);
  va_end(va);
}

void Bridge::debug(char *str, ...)
{
  va_list va;
  va_start(va, str);
  vprintf(str, va);
  va_end(va);
}
