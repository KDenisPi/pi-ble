/*
 * ble_ftp_file_snd_rcv.h
 *
 * BLE library. File send/receive implementation
 *
 *  Created on: Nov 20, 2018
 *      Author: Denis Kudia
 */

#ifndef BLE_FTP_FILE_SND_RCV_H
#define BLE_FTP_FILE_SND_RCV_H

#include <string>

namespace pi_ble {
namespace ble_ftp {

/*
* Send/receive support
*
* Receiver always wait for connection from sender
*/
class BleFtpFile : public BleFtp, public piutils::Threaded {

public:
    BleFtpFile(const bool receiver, const std::string& address, const uint16_t port, const std::string& fname)
        : BleFtp(receiver, port), _address( address ), _filename(fname), _receiver( receiver ), _fd(0) {

    }

    ~BleFtpFile() {
        if( _fd > 0 )
            close( _fd );
    }

    //Main server function
    static void worker(BleFtpFile* owner){

        owner->send_receive();

    }

    /*
    * Send/receive file
    */
    bool send_receive() {
        int sock = -1;

        if( prepare_src_dst() ){
            if( is_receiver() ){
                if( initialize() ){
                    if( prepare() ){
                        sock = wait_connection( WAIT_READ, 10, true);
                        if( sock < 0 ){
                            logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " Socket error detected");
                        }
                    }
                    else{
                        logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " Could not prepare socket");
                    }
                }
                else{
                    logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " Could not initilize connection");
                }
            }
            else{
                sock = connect_to_receiver();
            }
        }

        /*
        * Send / receive data here
        */

        if( sock > 0 ){
            close( sock );
        }
    }

protected:
    bool is_receiver() const {
        return _receiver;
    }

    /*
    *
    */
    int connect_to_receiver() {
        return connect_to(_address, get_channel());
    }

    /*
    * Prepare source / destination file
    */
    bool prepare_src_dst() {

        if( _filename.empty() )
            return false;

        if( is_receiver() ){ //create empty file for writing
            _fd = open( _filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
        }
        else{ //open existing file for reading
            _fd = open( _filename.c_str(), O_RDONLY, S_IRUSR | S_IWUSR );
        }

        if( _fd < 0 ){
            logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " Error: " + std::to_string(errno) + " " + _filename);
        }

        return ( _fd > 0 );
    }

private:
    std::string _filename; //full file path
    std::string _address;
    bool _receiver;

    int _fd; //file descriptoer for source/destination
};

}//namespace ble_ftp
}//namespace pi-ble

#endif