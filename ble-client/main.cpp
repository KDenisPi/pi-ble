#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fstream>

using namespace std;

#include "ble_lib.h"

int main (int argc, char* argv[])
{
  std::cout <<  "BLE client" << std::endl;

  pi_ble::ble_lib::BleLib blelib;
  uint16_t services[] = {  0x1800 //GATT General Service
    };

  std::string addr = (argc > 1 ? argv[1] : blelib.find_partner("raspberrypi_1"));

  if(!addr.empty()){
    std::cout <<  "Detected: " << addr << std::endl;
    int len = sizeof(services)/sizeof(uint16_t);
    blelib.detect_service_over_sdp(addr, services, 0);
  }

  exit(EXIT_SUCCESS);
}

