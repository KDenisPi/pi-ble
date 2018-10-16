#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fstream>

using namespace std;

#include "ble_server.h"

int main (int argc, char* argv[])
{
  std::cout <<  "BLE server" << std::endl;

  pi_ble::ble_lib::BleServer bleServer;
  pi_ble::ble_lib::BleService bleSvc(0xAADD, "Weather Service", "Denis Kudia", "Weather service for Hoime");
  bleSvc.add_protocol_l2cap( 5000 ); // Use PSM port number 5000
  bleSvc.add_protocol_rfcomm( 23 );  // Use  RFCOMM channel 23
  bleSvc.add_protocol_att(); //Add ATT protocol

  if( bleSvc.prepare() ){
    bleServer.add_service(pi_ble::ble_lib::BleSevicePtr(&bleSvc));

    std::cout <<  "BLE server. Registering services" << std::endl;
    if( bleServer.sdp_register() ){
        std::cout <<  "BLE server. Services registered" << std::endl;

        //wait for some time (check on client side that registration is visible)
        sleep( 10 );

        std::cout <<  "BLE server. Unregister services" << std::endl;
        bleServer.sdp_unregister();
    }
  }

  std::cout <<  "BLE server. Finished" << std::endl;
  exit(EXIT_SUCCESS);
}

