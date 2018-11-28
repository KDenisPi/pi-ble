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
#include <functional>

namespace pi_ble {
namespace ble_ftp {

/*
* Send/receive support
*
* Receiver always wait for connection from sender
*/
class BleFtpFile : public BleFtp, public piutils::Threaded {

public:
    /*
    *
    */
    BleFtpFile(const bool is_server, const uint16_t port)
        : BleFtp(port, is_server),  _filename(""), _flength(0), _receiver( false ), _fd(0), _nd(0) {
    }

    /*
    * Set server address
    */
    void set_address(const std::string& address) {
        _address = address;
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " Address: " + _address);
    }

    void set_filename(const std::string& filename) {
        _filename = filename;
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " File: " + _filename);
    }

    void set_filesize(const ssize_t fsize){
        _flength = fsize;
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " File size: " + std::to_string(_flength));
    }

    void set_receiver( const bool receiver ){
        _receiver = receiver;
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " Receiver: " + (_receiver ? "true " : "false "));
    }

    /*
    *
    */
    ~BleFtpFile() {
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__));

        fd_close();
    }

    std::function<void(std::string&)> finish_callback;

    //
    bool start(){
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " Started");
        return piutils::Threaded::start<BleFtpFile>(this);
    }

    //
    void stop(){
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " Started.");
        fd_close();
        piutils::Threaded::stop();
    }

    //temporal
    bool wait_for_finishing() {
        auto fn = [this]{return this->is_stop_signal();};
        {
            std::unique_lock<std::mutex> lk(this->cv_m);
            this->cv.wait(lk, fn);
        }
    }


    //Main server function
    static void worker(BleFtpFile* owner){
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " started");
        owner->send_receive();
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " finished");
    }

    /*
    * Send/receive file
    */
    bool send_receive() {
        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " started " + std::to_string(is_receiver()) + " port: " + std::to_string(get_channel()));

        bool res = false;

        if( prepare_src_dst() ){
            //initialize socket
            if( initialize() ){
                if( is_server() ){
                    if( prepare() ){
                        _nd = wait_connection( WAIT_READ|WAIT_WRITE, 10, true);
                    }
                    else{
                        logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " Could not prepare socket");
                    }
                }
                else{
                    _nd = connect_to_receiver();
                }
            }
            else{
                logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " Could not initialize socket");
            }
        }

        /*
        * Send / receive data here
        */
        std::string result;
        if( _nd > 0 ){
            if( is_receiver() ){
                res = fsend_receive( _nd, _fd ); //Receiver: read from network and write to file
            }
            else{
                res = fsend_receive( _fd, _nd ); //Sender: Read from file and write to network
            }
        }
        else {
            result = "500 Socket error or timeout detected";
            logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " Socket error or timeout detected");
        }

        if( this->finish_callback ){
            result = std::string("200 File successfully") + (is_receiver() ? " received" : "sent");
            this->finish_callback(result);
        }

        // close descriptors
        fd_close();

        //if failed and there is receiver - delete created file (?)
        if( !res && is_receiver() ){

        }

        set_stop_signal(true);

        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " finished");
        return res;
    }

protected:

    char _buffer[8096];

    /*
    * Send receive function
    */
    bool fsend_receive( int r_fd, int w_fd ) {
        ssize_t rres, wres, rlen = 0, wlen = 0;

        for(;;){
            rres = read( r_fd, _buffer, sizeof(_buffer));
            if( rres < 0 ){
                logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " File read error: " + std::to_string(errno));
                break;
            }
            else if( rres == 0 ){
                logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " EOF Read length: " + std::to_string( rlen ));
                break;
            } //EOF
            rlen += rres;

            if( is_stop_signal() ){
                logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " Stop signal detected");
                break;
            }

            wres = write( w_fd, _buffer, rres );
            if( wres < 0 ){
                logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " File write error: " + std::to_string(errno));
                break;
            }
            wlen += wres;

            if( wres != rres ){
                logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " File write lost data: " + std::to_string(wres));
                break;
            }

            if( is_stop_signal() ){
                logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " Stop signal detected");
                break;
            }
        }

        if( is_receiver() && (_flength > 0) && (_flength != wlen)){
            logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " Length of processed data does not match with reported length: " + std::to_string(_flength));
        }

        logger::log(logger::LLOG::DEBUG, "SndRcv", std::string(__func__) + " Processed : " + std::to_string( wlen ) + " bytes");
        return ( (wlen > 0) && (rlen == wlen) );
    }

    /*
    * Get component role (server / receiver)
    */
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

        _fd = open( _filename.c_str(),  get_flags(), get_mode() );
        if( _fd < 0 ){
            logger::log(logger::LLOG::ERROR, "SndRcv", std::string(__func__) + " Error: " + std::to_string(errno) + " " + _filename);
        }

        return ( _fd > 0 );
    }

private:
    std::string _filename;  //full file path
    ssize_t _flength;        //file length (checked on receiver side)
    std::string _address;
    bool _receiver;

    int _fd;    //file descriptor for source/destination
    int _nd;  //network descriptor

    void fd_close() {

        if( _fd > 0 )
            close( _fd );

        if( _nd > 0 )
            close( _nd );
    }

    /*
    * Detect file open flags
    */
    int get_flags() {
       return ( _receiver ? ( O_WRONLY | O_CREAT | O_TRUNC) : (O_RDONLY));
    }

    mode_t get_mode() {
        return ( _receiver ? (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) : (S_IRUSR | S_IRGRP | S_IROTH));
    }
};

}//namespace ble_ftp
}//namespace pi-ble

#endif