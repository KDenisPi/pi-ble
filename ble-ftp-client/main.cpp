#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fstream>

using namespace std;

#include "ble_ftp_client.h"

const char* help_msg = "Usage: bleftpclient server_id server_channel";
int main (int argc, char* argv[])
{
  std::cout <<  "BLE FTP client started" << std::endl;

  logger::log_init("/var/log/pi-robot/ftpcl_log");

  if(argc < 3){
    std::cout <<  "BLE FTP client" << std::endl;
    exit(EXIT_SUCCESS);
  }

  pi_ble::ble_ftp::BleFtpClient bleClient;

  if( bleClient.connect_to("127.0.0.1", 7000)){
    std::cout <<  "Connected" << std::endl;

    //bleClient.process_cmd_help();
    //bleClient.process_cmd_pwd();
    //bleClient.process_cmd_cdup();
    //bleClient.process_cmd_cwd("/var/log/pi-robot");
    //bleClient.process_cmd_mkdir("/var/log/pi-robot/denis");
    //bleClient.process_cmd_list();
    bleClient.process_cmd_rmdir("/var/log/pi-robot/denis");
    bleClient.process_cmd_delete("/var/log/pi-robot/ftpcl_log_2018-11-08_16-52");

    sleep(1);

    bleClient.process_cmd_quit();
  }

  bleClient.close_socket();

  std::cout <<  "BLE FTP client finished" << std::endl;
  exit(EXIT_SUCCESS);
}

