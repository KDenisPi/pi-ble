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

#include "Threaded.h"
#include "smallthings.h"

#include "ble_ftp.h"
#include "ble_ftp_file_snd_rcv.h"

namespace pi_ble {
namespace ble_ftp {

class BleFtpServer : public BleFtp, public BleFtpCommand, public piutils::Threaded
{

public:
    /*
    * Constructor
    */
    BleFtpServer(const uint16_t port_cmd) : BleFtp(port_cmd, false) {
        set_curr_dir("/tmp");
        _pfile = std::shared_ptr<BleFtpFile>(new BleFtpFile(true, port_cmd+1));
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
        const std::string response = prepare_result(200, "PWD Current directory \"" + get_curr_dir() + "\"");
        return  write_data( get_cmd_socket(), response.c_str(), response.length());
    }

    //process CWD command
    virtual bool process_cmd_cwd(const std::string& dpath, const std::string msg = "CWD") override {
        std::string fpath = get_full_path(dpath);
        logger::log(logger::LLOG::DEBUG, "ftpd", std::string(__func__) + " [" + dpath + "]" + " Full: " + fpath);

        std::string response;
        response = prepare_result(500, msg + " Failed");
        if(!dpath.empty()){
            if( piutils::chkfile(fpath)){
                set_curr_dir( fpath );
                response = prepare_result(200, msg + " Set current directory to \"" + get_curr_dir() + "\"");
            }
        }
        else{
            response = prepare_result(400, msg + " Directory name is empty.");
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


    /*
    * process CDUP command
    */
    virtual bool process_cmd_cdup() override {
        const std::string response = prepare_result(200, "CDUP Current directory \"" + _current_dir + "\"");
        size_t off = _current_dir.rfind('/', _current_dir.length());
        if( off == 0 ) // root folder - no parent
        {
            std::string response = prepare_result(400, "CDUP No parent directory");
            return  write_data( get_cmd_socket(), response.c_str(), response.length());
        }

        return process_cmd_cwd( _current_dir.substr(0, off), "CDUP");
    }

    /*
    * process MKD command
    */
    virtual bool process_cmd_mkdir( const std::string& ldir ) override {

        std::string fpath = get_full_path(ldir);
        logger::log(logger::LLOG::DEBUG, "ftpd", std::string(__func__) + " MKD [" + ldir + "]" + " Full: " + fpath);

        std::string response;
        if(!ldir.empty()){
            int res = mkdir( fpath.c_str(), S_IWUSR|S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH );
            if( res == 0 || (res == -1 && errno == EEXIST)){
                response = prepare_result(200, "MKD Directory \"" + fpath + "\" created");
            }
            else
                response = prepare_result(500, "MKD Failed Error: " + std::to_string(errno));
        }
        else {
            response = prepare_result(400, "MKD  Directory name is empty.");
        }

        return  write_data( get_cmd_socket(), response.c_str(), response.length());
    }

    /*
    * process RMD command
    */
    virtual bool process_cmd_rmdir( const std::string& ldir ) override {
        std::string fpath = get_full_path(ldir);
        logger::log(logger::LLOG::DEBUG, "ftpd", std::string(__func__) + " RMD [" + ldir + "]" + " Full: " + fpath);

        std::string response;
        if(!ldir.empty()){
            int res = rmdir( fpath.c_str() );
            if( res == 0 ){
                response = prepare_result(200, "RMD Directory \"" + fpath + "\" removed");
            }
            else
                response = prepare_result(500, "RMD Failed Error: " + std::to_string(errno));
        }
        else {
            response = prepare_result(400, "RMD  Directory name is empty.");
        }

        return  write_data( get_cmd_socket(), response.c_str(), response.length());
    }

    /*
    * process DELE command
    */
    virtual bool process_cmd_delete( const std::string& lfile) override {
        std::string fpath = get_full_path(lfile);
        logger::log(logger::LLOG::DEBUG, "ftpd", std::string(__func__) + " DELE [" + lfile + "]" + " Full: " + fpath);

        std::string response;
        if(!lfile.empty()){
            int res = remove( fpath.c_str() );
            if( res == 0 ){
                response = prepare_result(200, "DELE File \"" + fpath + "\" deleted");
            }
            else
                response = prepare_result(500, "DELE Failed Error: " + std::to_string(errno));
        }
        else {
            response = prepare_result(400, "DELE  Filename name is empty.");
        }

        return  write_data( get_cmd_socket(), response.c_str(), response.length());
    }

    /*
    * process RETR command
    */
    virtual bool process_cmd_retr( const std::string& lfile) override {
        std::string fpath = get_curr_dir() + "/" + lfile;
        logger::log(logger::LLOG::DEBUG, "ftpd", std::string(__func__) + " RETR [" + lfile + "]" + " Full: " + fpath);

        std::string response;
        if(!lfile.empty()){
            if( piutils::chkfile(fpath)){
                response = prepare_result(200, "RETR File \"" + fpath + "\"");
                if( _pfile->is_stopped()){
                    _pfile->set_filename( fpath );
                    _pfile->start();
                }
                else {
                    response = prepare_result(400, "RETR  Server busy. Try later.");
                }
            }
            else {
                response = prepare_result(400, "RETR  Filename not exist or access denied");
            }
        }
        else {
            response = prepare_result(400, "RETR  Filename name is empty.");
        }

        return  write_data( get_cmd_socket(), response.c_str(), response.length());
    }

    /*
    * process STOR command
    */
    virtual bool process_cmd_stor( const std::string& lfile) override {
        std::string fpath = get_curr_dir() + "/" + lfile;
        logger::log(logger::LLOG::DEBUG, "ftpd", std::string(__func__) + " STOR [" + lfile + "]" + " Full: " + fpath);

        std::string response;
        if(!lfile.empty()){
            response = prepare_result(200, "STOR File \"" + fpath + "\"");
            if( _pfile->is_stopped()){
                _pfile->set_receiver(true);
                _pfile->set_filename( fpath );
                _pfile->start();
            }
            else {
                response = prepare_result(400, "STOR  Server busy. Try later.");
            }
        }
        else {
            response = prepare_result(400, "STOR  Filename name is empty.");
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

private:
    /*
    * Send receive file object
    */
    std::shared_ptr<BleFtpFile> _pfile;

public:
    //Receive CMD result
    const CmdInfo cmd_receive();

};

}
}

#endif
