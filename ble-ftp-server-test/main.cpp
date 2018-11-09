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
  std::cout <<  "BLE FTP server" << std::endl;

  logger::log_init("/var/log/pi-robot/ftpd_log");

  pi_ble::ble_ftp::BleFtpServer ftpd(7000);

  ftpd.start();
  std::cout <<  "BLE FTP server, Started, Wait" << std::endl;

  ftpd.wait_for_finishing();

  std::cout <<  "BLE FTP server, Stopped" << std::endl;
  sleep(3);

  exit(EXIT_SUCCESS);
}

