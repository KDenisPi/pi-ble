/*
 * ble_ftp_cmd.h
 *
 * BLE library. FTP command processing implementation
 *
 *  Created on: Oct 25, 2018
 *      Author: Denis Kudia
 */

#ifndef BLE_FTP_CMD_H
#define BLE_FTP_CMD_H

#include "logger.h"
#include "smallthings.h"

namespace pi_ble {
namespace ble_ftp {

enum BleFtpStates {
    Initial,
    Connecting,
    Connected,
    FileTransfer,
    Error
};

#define USE_NET_INSTEAD_BLE

enum CmdList {
    Cmd_List = 0,    //Get list of files
    Cmd_Help,
    Cmd_Quit,
    Cmd_Pwd,
    Cmd_Cwd,
    Cmd_Cdup,
    Cmd_Rmd,
    Cmd_Mkd,
    Cmd_Dele, //delete file
    Cmd_Retr, //Download file from server
    Cmd_Stor, //Upload file to server
    Cmd_Unknown,
    Cmd_Timeout,
    Cmd_Error
};

#define MAX_CMD_BUFFER_LENGTH   4096
using CmdInfo = std::pair<CmdList, std::string>;

/*
*
*/
class BleFtpCommand {
public:
    BleFtpCommand() : _state(BleFtpStates::Initial) {}
    virtual ~BleFtpCommand() {}

    //Change current state
    void to_state(const BleFtpStates state){
        _state = state;
    }

    const BleFtpStates state() const {
        return _state;
    }

    static std::string cmd_list[];

protected:
    //process HELP command on server side
    virtual bool process_cmd_help() { return false; }
    //process EXIT command
    virtual bool process_cmd_quit() { return false; }
    //process PWD command
    virtual bool process_cmd_pwd() { return false; }
    //process CWD command
    virtual bool process_cmd_cwd(const std::string& dpath, const std::string msg = "CWD") { return false; }
    //process LIST command
    virtual bool process_cmd_list( const std::string& ldir = "" ) { return false; }
    //process CDUP command
    virtual bool process_cmd_cdup() { return false; }
    //process MKD command
    virtual bool process_cmd_mkdir( const std::string& ldir) { return false; }
    //process RMD command
    virtual bool process_cmd_rmdir( const std::string& ldir) { return false; }
    //process DELE command
    virtual bool process_cmd_delete( const std::string& lfile) { return false; }
    //process RETR command
    virtual bool process_cmd_download( const std::string& lfile) { return false; }
    //process STOR command
    virtual bool process_cmd_upload( const std::string& lfile) { return false; }

public:
    static const std::string get_cmd_by_code(int cmd) {
        return  cmd_list[cmd];
    }

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

private:
    BleFtpStates _state;
};

}
}

#endif