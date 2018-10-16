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

  pi_ble::ble_lib::BleServer* bleServer = new pi_ble::ble_lib::BleServer();
  pi_ble::ble_lib::BleSevicePtr bleSvc = pi_ble::ble_lib::BleSevicePtr(
      new pi_ble::ble_lib::BleService(0xAADD, "Weather Service", "Denis Kudia", "Weather service for Hoime")
    );

  bleSvc->add_protocol_l2cap( 5001 ); // Use PSM port number 5001 (odd number)
  bleSvc->add_protocol_rfcomm( 23 );  // Use  RFCOMM channel 23
  bleSvc->add_protocol_att(); //Add ATT protocol

  if( bleSvc->prepare() ){
    bleServer->add_service(bleSvc);

    std::cout <<  "BLE server. Registering services" << std::endl;
    if( bleServer->sdp_register() ){
        std::cout <<  "BLE server. Services registered" << std::endl;

        //wait for some time (check on client side that registration is visible)
        sleep( 10 );

        std::cout <<  "BLE server. Unregister services" << std::endl;
        bleServer->sdp_unregister();
    }
  }

  delete bleServer;
  std::cout <<  "BLE server. Finished" << std::endl;
  exit(EXIT_SUCCESS);
}

