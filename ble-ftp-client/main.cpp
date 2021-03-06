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

  uint16_t port = std::stoi(argv[2]);
  std::cout <<  "Connecting Addr: " << argv[1] << " Port:" << argv[2] << std::endl;

  pi_ble::ble_ftp::BleFtpClient bleClient(port);
  bleClient.set_curr_dir("/home/deniskudia/Downloads");

  if( bleClient.connect_to(argv[1], port)){
    std::cout <<  "Connected" << std::endl;

    std::string command;
    bool active_session = true;
    for(;active_session;){
      std::cout << "> ";
      std::getline(std::cin, command);

      auto cmd = bleClient.recognize_cmd( command );
      switch( cmd.first ){
        case pi_ble::ble_ftp::CmdList::Cmd_Help:
            bleClient.process_cmd_help();
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Quit:
            bleClient.process_cmd_quit();
            active_session = false;
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Pwd:
            bleClient.process_cmd_pwd();
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Cdup:
            bleClient.process_cmd_cdup();
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_List:
            bleClient.process_cmd_list(cmd.second);
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Cwd:
            bleClient.process_cmd_cwd(cmd.second);
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Rmd:
            bleClient.process_cmd_rmdir(cmd.second);
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Mkd:
            bleClient.process_cmd_mkdir(cmd.second);
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Dele:
            bleClient.process_cmd_delete(cmd.second);
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Retr:
            bleClient.process_cmd_retr(cmd.second);
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Stor:
            bleClient.process_cmd_stor(cmd.second);
          break;
        case pi_ble::ble_ftp::CmdList::Cmd_Ls:
            bleClient.process_cmd_ls(cmd.second);
          break;
        default:
          std::cout << "Unknown command" << endl;
      }
    }
  }
  else {
    std::cout <<  "Connection failed" << std::endl;
  }

  bleClient.close_socket();

  std::cout <<  "BLE FTP client finished" << std::endl;
  exit(EXIT_SUCCESS);
}

