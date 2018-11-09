/*
 * ble_ftp_server.h
 *
 * BLE library. FTP server implementation over RFCOMM
 *
 *  Created on: Oct 24, 2018
 *      Author: Denis Kudia
 */

#ifndef BLE_FTP_SERVER_H
#define BLE_FTP_SERVER_H

#include <thread>
#include <memory>

#include "ble_ftp.h"
#include "Threaded.h"
#include "smallthings.h"

namespace pi_ble {
namespace ble_ftp {

using CmdInfo = std::pair<CmdList, std::string>;

class BleFtpServer : public BleFtp, public piutils::Threaded
{

public:
    /*
    * Constructor
    */
    BleFtpServer(const uint16_t port_cmd) : BleFtp(port_cmd, false), _current_dir("/var/log") {

    }

    /*
    * Destructor
    */
    virtual ~BleFtpServer() {

    }

    //
    bool start();

    //
    void stop();

    //
    bool wait_for_connection();

    //Main server function
    static void worker(BleFtpServer* owner);

    //Close client socket
    bool close_client();

    /*
    * process HELP command
    */
    virtual bool process_cmd_help() override;

    /*
    * process EXIT command
    */
    virtual bool process_cmd_quit() override {
        const std::string response = prepare_result(200, "QUIT Session finished");
        set_stop_signal(true);
        return  write_data( get_cmd_socket(), response.c_str(), response.length());
    }

    /*
    * process PWD command
    */
    virtual bool process_cmd_pwd() override {
        const std::string response = prepare_result(200, "PWD Current directory \"" + _current_dir + "\"");
        return  write_data( get_cmd_socket(), response.c_str(), response.length());
    }

    //process CWD command
    virtual bool process_cmd_cwd(const std::string& dpath) override {
        logger::log(logger::LLOG::DEBUG, "ftpd", std::string(__func__) + " [" + dpath + "]");
        std::string response;

        response = prepare_result(500, "CWD Failed");

        if(!dpath.empty()){
            if( piutils::chkfile(dpath)){
                set_curr_dir( dpath );
                response = prepare_result(200, "CWD Set current directory to \"" + _current_dir + "\"");
            }
        }

        return  write_data( get_cmd_socket(), response.c_str(), response.length());
    }


    /*
    * process LIST command
    */
    virtual bool process_cmd_list( const std::string& ldir = ""  ) override {
        logger::log(logger::LLOG::DEBUG, "ftpd", std::string(__func__) + " [" + ldir + "]");

        std::string response = prepare_result(200, "LIST");
        int res = piutils::get_dir_content( (ldir.empty() ? _current_dir : ldir), response, MAX_CMD_BUFFER_LENGTH - 256);
        if(res < 0 ){
            response += "---------------- cut ----------------\n";
        }
        else if( res > 0 ){
            response = prepare_result(500, "LIST Error: " + std::to_string(res));
        }

        return  write_data( get_cmd_socket(), response.c_str(), response.length());
    }

    //service function
    virtual bool check_stop_signal() override {
        return this->is_stop_signal();
    }

    //temporal
    bool wait_for_finishing() {
        auto fn = [this]{return this->is_stop_signal();};
        {
            std::unique_lock<std::mutex> lk(this->cv_m);
            this->cv.wait(lk, fn);
        }
    }

    static std::string helpText;
protected:
    int _sock_client; //socket returned by accept

    //Get socket used for sending/receiving data
    virtual int get_cmd_socket() {
        return _sock_client;
    }

    /*
    * Currnet directory
    */
    const std::string get_curr_dir() const {
        return _current_dir;
    }

    void set_curr_dir( const std::string current_dir ){
        _current_dir = current_dir;
    }


private:
    std::string _current_dir;

public:
    //Receive CMD result
    const CmdInfo cmd_receive();

};

}
}

#endif
