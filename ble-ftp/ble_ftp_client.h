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

#include "logger.h"

#include "ble_ftp.h"
#include "ble_ftp_file_snd_rcv.h"

namespace pi_ble {
namespace ble_ftp {

class BleFtpClient : public BleFtp, public BleFtpCommand
{

public:
    /*
    * Constructor
    */
    BleFtpClient(const uint16_t port_cmd) : BleFtp(port_cmd, false) {
        initialize();

        set_curr_dir("~/Downloads");

        _pfile = std::shared_ptr<BleFtpFile>(new BleFtpFile(false, port_cmd+1));
        _pfile->finish_callback = std::bind(&BleFtpClient::print_file_result, this, std::placeholders::_1);
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
        return process_request(pi_ble::ble_ftp::CmdList::Cmd_Help);
    }

    /*
    * Process EXIT command
    */
    virtual bool process_cmd_quit() override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " QUIT");
        return process_request(pi_ble::ble_ftp::CmdList::Cmd_Quit);
    }

    /*
    * Process PWD command
    */
    virtual bool process_cmd_pwd() override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " PWD");
        return process_request(pi_ble::ble_ftp::CmdList::Cmd_Pwd);
    }

    /*
    * process CWD command
    */
    virtual bool process_cmd_cwd(const std::string& dpath, const std::string msg = "CWD") override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " CWD to: " + dpath);
        return process_request_w_param(pi_ble::ble_ftp::CmdList::Cmd_Cwd, dpath);
    }

    /*
    * process LIST command
    */
    virtual bool process_cmd_list( const std::string& ldir = ""  ) override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " LIST for: " + ldir);
        return process_request(pi_ble::ble_ftp::CmdList::Cmd_List);
    }

    /*
    * process CDUP command
    */
    virtual bool process_cmd_cdup() override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " CDUP");
        return process_request(pi_ble::ble_ftp::CmdList::Cmd_Cdup);
    }

    /*
    * process MKD command
    */
    virtual bool process_cmd_mkdir( const std::string& ldir) override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " MKD for: " + ldir);
        return process_request_w_param(pi_ble::ble_ftp::CmdList::Cmd_Mkd, ldir);
    }

    /*
    * process RMD command
    */
    virtual bool process_cmd_rmdir( const std::string& ldir) override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " RMD for: " + ldir);
        return process_request_w_param(pi_ble::ble_ftp::CmdList::Cmd_Rmd, ldir);
    }

    /*
    * process DELE command
    */
    virtual bool process_cmd_delete( const std::string& lfile) override {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " DELE for: " + lfile);
        return process_request_w_param(pi_ble::ble_ftp::CmdList::Cmd_Dele, lfile);
    }

    /*
    * process RETR command
    */
    virtual bool process_cmd_download( const std::string& lfile) {
        logger::log(logger::LLOG::DEBUG, "ftpc", std::string(__func__) + " RETR for: " + lfile);

        bool res = cmd_send(pi_ble::ble_ftp::Cmd_Retr, lfile);
        if( res ){
            res = cmd_process_response();

            if( res ){ //start file operation
                _pfile->set_filename( lfile );
                _pfile->start();
            }
        }

        return res;
    }


    void print_file_result(std::string& result) const {
        std::cout <<  result << std::endl;
    }

private:

    bool process_request_w_param( pi_ble::ble_ftp::CmdList cmd, const std::string& param ){
        if(param.empty()) {
            return print_result_400_Bad_request(cmd_list[cmd]);
        }

        bool res = cmd_send(cmd, param);
        if( res ){
            res = cmd_process_response();
        }

        return res;
    }

    bool process_request( pi_ble::ble_ftp::CmdList cmd){
        bool res = cmd_send(cmd);
        if( res ){
            res = cmd_process_response();
        }
        return res;
    }

    /*
    * Send receive file object
    */
    std::shared_ptr<BleFtpFile> _pfile;

};

}
}

#endif
