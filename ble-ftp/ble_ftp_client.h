/*
 * ble_ftp_client.h
 *
 * BLE library. FTP client implementation over RFCOMM
 *
 *  Created on: Oct 29, 2018
 *      Author: Denis Kudia
 */

#ifndef BLE_FTP_CLIENT_H
#define BLE_FTP_CLIENT_H

#include "ble_ftp.h"
#include "logger.h"

namespace pi_ble {
namespace ble_ftp {

class BleFtpClient : public BleFtp
{

public:
    /*
    * Constructor
    */
    BleFtpClient(const uint16_t port_cmd = 0) : BleFtp(port_cmd, true) {
        initialize();
    }

    /*
    * Destructor
    */
    virtual ~BleFtpClient() {

    }

    /*
    * Process HELP command
    */
    virtual bool process_cmd_help() override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " HELP");

        bool res = cmd_send(pi_ble::ble_ftp::CmdList::Cmd_Help);
        if( res ){
            res = cmd_process_response();
        }
        return res;
    }

    /*
    * Process EXIT command
    */
    virtual bool process_cmd_quit() override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " QUIT");

        bool res = cmd_send(pi_ble::ble_ftp::CmdList::Cmd_Quit);
        if( res ){
            res = cmd_process_response();
        }
        return res;
    }

    /*
    * Process PWD command
    */
    virtual bool process_cmd_pwd() override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " PWD");

        bool res = cmd_send(pi_ble::ble_ftp::CmdList::Cmd_Pwd);
        if( res ){
            res = cmd_process_response();
        }
        return res;
    }

    /*
    * process CWD command
    */
    virtual bool process_cmd_cwd(const std::string& dpath) override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " CWD to: " + dpath);

        if(dpath.empty())
            return false;

        bool res = cmd_send(pi_ble::ble_ftp::CmdList::Cmd_Cwd, dpath);
        if( res ){
            res = cmd_process_response();
        }

        return res;
    }

    /*
    * process LIST command
    */
    virtual bool process_cmd_list( const std::string& ldir = ""  ) override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " LIST for: " + ldir);

        bool res = cmd_send(pi_ble::ble_ftp::CmdList::Cmd_List, ldir);
        if( res ){
            res = cmd_process_response();
        }

        return res;
    }



};

}
}

#endif
