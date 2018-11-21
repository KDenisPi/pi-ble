/*
 * ble_ftp.h
 *
 * BLE library. FTP implementation over RFCOMM
 *
 *  Created on: Oct 24, 2018
 *      Author: Denis Kudia
 */

#ifndef BLE_FTP_COMMON_H
#define BLE_FTP_COMMON_H

#include "logger.h"
#include "smallthings.h"

#include "ble_lib.h"
#include "ble_ftp_cmd.h"

namespace pi_ble {
namespace ble_ftp {

#define MAX_CMD_BUFFER_LENGTH   4096

using CmdInfo = std::pair<CmdList, std::string>;

class BleFtp : public BleFtpCommand
{

public:
    /*
    * Constructor
    */
    BleFtp(const uint16_t port_cmd, const bool is_server) : _port_cmd(port_cmd), _sock_cmd(0), _server(is_server) {

    }

    /*
    * Destructor
    */
    virtual ~BleFtp() {

    }

    //initialize socket
    bool initialize();

    //close socket
    bool close_socket();

    //Set connection
    bool connect_to(const std::string daddress, const uint16_t dchannel);

    //Return channel number
    const uint16_t get_cmd_channel() const {
        return _port_cmd;
    }

    //
    bool prepare();

    //Is we use blocking mode for connection
    const bool is_server() const {
        return _server;
    }

protected:
    uint16_t _port_cmd;  //FTP command port number
    uint16_t _port_data; //FTP data port number

    int _sock_cmd;
    bool _server; //USe blocking of non blocking connection

    char buffer_cmd[MAX_CMD_BUFFER_LENGTH];

    //Get socket used for sending/receiving data
    virtual int get_cmd_socket() {
        return _sock_cmd;
    }

    // Service function
    virtual bool check_stop_signal() { return false; }

    /*
    * process input command (receive data from recipient and save it to file or memory)
    *
    * If filename is empty put data to string return object otherwise write it to the file
    */
    bool process_incoming_stream(std::string& data);

    /*
    * Low level write data function
    */
    bool write_data(int fd, const void* data, size_t size);

    /*
    * Low level write data function
    */
    int read_data(int fd, std::string& result);

    //
    #define WAIT_READ   1
    #define WAIT_WRITE  2

    /*
    * Wait untill socked will not ready for read/write
    *
    * Wait interval in seconds (defauld 1 sec)
    * Break IF timeour - stop waiting if nothing was detected during wait interval
    */
    int wait_for_descriptor(int fd, const uint8_t wait_for, const int wait_interval = 1, const bool break_if_timeout = false);

public:
    /*
    * Recognize received connamd
    */
   const CmdInfo recognize_cmd(const std::string& command){
        CmdList cmd = CmdList::Cmd_Unknown;
        std::string parameters;

        //recognize received command
        int idx = 0;
        while(get_cmd_by_code(idx).compare("EOF") != 0){
            std::string::size_type pos = command.rfind(get_cmd_by_code(idx));
            if(pos == 0 ){
                cmd = (CmdList) idx;

                parameters = command.substr(get_cmd_by_code(idx).length());
                parameters = piutils::trim( parameters );

                logger::log(logger::LLOG::DEBUG, "BleFtp", std::string(__func__) + " CMD: " + std::to_string(cmd) + " [" + parameters + "]");
                break;
            }
            idx++;
        }

        return std::make_pair(cmd, parameters);
   }

    //Send command to server
    bool cmd_send(const CmdList cmd, const std::string& parameters = "");
    bool cmd_process_response();

    const std::string prepare_result(const uint16_t code, const std::string& message){
        return std::to_string(code) + " " + message + "\n";
    }

    const std::string get_cmd_by_code(int cmd) {
        return  cmd_list[cmd];
    }

    bool  print_result_400_Bad_request(const std::string cmd) {
        std::string result = prepare_result(400, cmd + " Bad request");
        std::cout <<  result << std::endl;
        return false;
    }
};

}
}

#endif
