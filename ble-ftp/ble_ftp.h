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

#include "ble_lib.h"
#include "ble_ftp_cmd.h"

namespace pi_ble {
namespace ble_ftp {


class BleFtp
{

public:

    //
    #define WAIT_READ   1
    #define WAIT_WRITE  2

    /*
    * Constructor
    */
    BleFtp(const uint16_t port, const bool is_server) : _port(port), _sock_cmd(0), _server(is_server) {
        logger::log(logger::LLOG::DEBUG, "BleFtp", std::string(__func__) + " Is server: " + (is_server ? "true " : "false ") + " Port: " + std::to_string(port));
    }

    /*
    * Destructor
    */
    virtual ~BleFtp() {
        close_socket();
    }

    //initialize socket
    bool initialize();

    //close socket
    bool close_socket();

    //Set connection
    int connect_to(const std::string daddress, const uint16_t dchannel);

    //Return channel number
    const uint16_t get_channel() const {
        return _port;
    }

    //
    bool prepare();

    //Is we use blocking mode for connection
    const bool is_server() const {
        return _server;
    }

    int wait_connection(const uint8_t wait_for = WAIT_READ, const int wait_interval = 1, const bool break_if_timeout = false);

    /*
    * Currnet directory
    */
    const std::string get_curr_dir() const {
        return _current_dir;
    }

    void set_curr_dir( const std::string current_dir ){
        _current_dir = current_dir;
    }

    void set_address( const std::string& address) {
        _address = address;
    }

    const std::string get_address() const {
        return _address;
    }

protected:
    uint16_t _port;   //server command port number
    std::string _address; //server address

    int _sock_cmd;
    bool _server; //USe blocking of non blocking connection


    char buffer_cmd[MAX_CMD_BUFFER_LENGTH];

    const std::string get_full_path(const std::string& fname ) const {
        return ( fname[0] == '/' ? fname : get_curr_dir() + "/" + fname);
    }

    //Get socket used for sending/receiving data
    virtual int get_cmd_socket() {
        return _sock_cmd;
    }

    /*
    * Service function
    */
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

    /*
    * Wait untill socked will not ready for read/write
    *
    * Wait interval in seconds (defauld 1 sec)
    * Break IF timeout - stop waiting if nothing was detected during wait interval
    */
    int wait_for_descriptor(int fd, const uint8_t wait_for = WAIT_READ, const int wait_interval = 1, const bool break_if_timeout = false);

    std::string _current_dir;

public:
    //Send command to server
    bool cmd_send(const CmdList cmd, const std::string& parameters = "");
    bool cmd_process_response();

    const std::string prepare_result(const uint16_t code, const std::string& message){
        return std::to_string(code) + " " + message + "\n";
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
