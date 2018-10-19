#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fstream>

using namespace std;

#include "ble_ftp_cmd.h"
#include "ble_ftp_server.h"


int main (int argc, char* argv[])
{
  uint16_t cmd_port = 20;

  if(argc > 1){
      cmd_port = std::atoi(argv[1]);
  }

  std::cout <<  "BLE FTP server port: " << std::to_string(cmd_port) << std::endl;

  logger::log_init("/var/log/pi-robot/ftpd_log");

  pi_ble::ble_ftp::BleFtpServer ftpd( cmd_port );

  ftpd.start();
  std::cout <<  "BLE FTP server, Started, Wait" << std::endl;
  ftpd.wait_for_finishing();

  std::cout <<  "BLE FTP server, Stopped" << std::endl;
  sleep(1);

  exit(EXIT_SUCCESS);
}

