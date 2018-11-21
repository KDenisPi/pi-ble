/*
 * ble_ftp.cpp
 *
 * BLE library. FTP implementation over RFCOMM
 *
 *  Created on: Oct 24, 2018
 *      Author: Denis Kudia
 */

#include <sys/socket.h>
#include <arpa/inet.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "ble_ftp.h"

namespace pi_ble {
namespace ble_ftp {

const char TAG[] = "ftplib";

std::string BleFtp::cmd_list[] = { "LIST", "HELP", "QUIT", "PWD", "CWD", "CDUP", "RMD", "MKD", "DELE", "EOF" };

//connect socket
bool BleFtp::initialize(){
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Started");

#ifdef USE_NET_INSTEAD_BLE
    _sock_cmd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    _sock_cmd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
#endif

    if( _sock_cmd < 0 ){
        logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Failed: " + std::to_string(errno));
        _sock_cmd = 0;
        return false;
    }

    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " socket: " + std::to_string(_sock_cmd));
    return true;
}

//disconnect and close
bool BleFtp::close_socket(){
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " socket: " + std::to_string(_sock_cmd));

    if( _sock_cmd > 0 ){
        int res = close( _sock_cmd );
        _sock_cmd = 0;
    }

    return true;
}

/*
*
*/
bool BleFtp::prepare(){
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Started");

    int res;
    socklen_t addrlen;

#ifdef USE_NET_INSTEAD_BLE
    struct sockaddr_in addr_loc;
    memset(&addr_loc, 0, sizeof(addr_loc));

    addr_loc.sin_family = AF_INET;
    addr_loc.sin_addr.s_addr = INADDR_ANY;
    //inet_pton(AF_INET, "127.0.0.1", &addr_loc.sin_addr);
    addr_loc.sin_port = htons(get_cmd_channel());
#else
    struct sockaddr_rc addr_loc = { 0 };
    bdaddr_t baddr_any = {0,0,0,0,0,0};
    memset(&addr_loc, 0, sizeof(addr_loc));

    addr_loc.rc_family = AF_BLUETOOTH;
    addr_loc.rc_bdaddr = baddr_any;
    addr_loc.rc_channel = (uint8_t) get_cmd_channel();

#endif

    addrlen = sizeof(addr_loc);
    res = bind( _sock_cmd, (struct sockaddr *)&addr_loc, sizeof(addr_loc));

    if( res < 0 ){
        logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Bind failed: " + std::to_string(errno));
        return false;
    }
    else
        logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Bind successfull");


    if(!is_server()){

        res = listen( _sock_cmd, 1);
        if( res < 0 ){
            logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Listen failed: " + std::to_string(errno));
            return false;
        }
    }

    addrlen = sizeof(addr_loc);
    getsockname( _sock_cmd, (struct sockaddr *)&addr_loc, &addrlen);
#ifdef USE_NET_INSTEAD_BLE
    char addr[64];
    inet_ntop(AF_INET, (const char*)&addr_loc.sin_addr, addr, sizeof(addr));
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Finished successfully: " + addr +  " Port: " + std::to_string(ntohs(addr_loc.sin_port)) + " Socket:" + std::to_string(_sock_cmd));
#else
    char addr[1024];
    ba2str( &addr_loc.rc_bdaddr, addr );
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Finished successfully: " + addr +  " Channel: " + std::to_string(addr_loc.rc_channel) + " Socket:" + std::to_string(_sock_cmd));
#endif

    return true;
}

/*
* Set remote server connection
*
*  Parameters: remote address, remote channel
*/
bool  BleFtp::connect_to(const std::string daddress, const uint16_t dchannel){

#ifdef USE_NET_INSTEAD_BLE
    struct sockaddr_in addr_rem;
    memset(&addr_rem, 0, sizeof(addr_rem));

    addr_rem.sin_family = AF_INET;
    addr_rem.sin_port = htons(dchannel);
    inet_pton(AF_INET, daddress.c_str(), &addr_rem.sin_addr);
#else
    struct sockaddr_rc addr_rem = { 0 };
    memset(&addr_rem, 0, sizeof(addr_rem));
    addr_rem.rc_family = AF_BLUETOOTH;
    addr_rem.rc_channel = (uint8_t) dchannel;
    str2ba( daddress.c_str(), &addr_rem.rc_bdaddr );
#endif

    int res = connect( _sock_cmd, (const struct sockaddr *)&addr_rem, sizeof(addr_rem) );
    if( res < 0 ){
        logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Connection failed: " + std::to_string(errno));
        return false;
    }

    return true;
}

/*
* Low level write data function
*/
bool BleFtp::write_data(int fd, const void* data, size_t size){
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Started FD: " + std::to_string(fd));

    int res = write( fd, data, size);
    if( res == 0 ){
        logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Write failed: " + std::to_string(errno));
        return false;
    }
    else if(res < 0 ){
        logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Write failed: " + std::to_string(errno));
        return false;
    }

    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " write count: " + std::to_string(res));
    return true;
}


/*
* Low level write data function
*/
int BleFtp::read_data(const int fd, std::string& result){
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Started FD: " + std::to_string(fd));

    int res =  wait_for_descriptor(fd, WAIT_READ, 10, true);
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + "wait_for_descriptor: " + std::to_string(res));

    if( res <= 0 ){
        return res;
    }

    memset( buffer_cmd, 0x00, sizeof(buffer_cmd));
    size_t buff_len = sizeof(buffer_cmd) - 1;
    do{
        res = read( fd, buffer_cmd, buff_len);
        if( res < 0 ){
            logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Read failed: " + std::to_string(errno));
            return res;
        }
        buffer_cmd[res] = 0x00;
        logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " read count: " + std::to_string(res));

        result += buffer_cmd;
        if(res < buff_len){
            break;
        }

    } while( res > 0);

    return result.length();
}


/*
* Send command to server
*/
bool BleFtp::cmd_send(const CmdList cmd, const std::string& parameters) {
    std::string commd;
    if(parameters.empty())
        commd = get_cmd_by_code( cmd );
    else
        commd = get_cmd_by_code( cmd ) + " " + parameters;

    return  write_data( get_cmd_socket(), commd.c_str(), commd.length());
}

/*
*
*/
bool BleFtp::cmd_process_response(){
    std::string result;
    int fd = get_cmd_socket();

    int res = read_data( fd, result);
    if( res <= 0 ){
        result = prepare_result(500, "Internal error");
    }

    std::cout <<  result << std::endl;
    return ( res > 0 );
}

/*
* Wait untill socked will not ready for read/write
*
* Wait interval in seconds (defauld 1 sec)
* Break IF timeour - stop waiting if nothing was detected during wait interval
*/
int BleFtp::wait_for_descriptor(int fd, const uint8_t wait_for, const int wait_interval, const bool break_if_timeout){
    fd_set readfds, writefds;
    struct timeval tmout;

    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " started for " + std::to_string(fd));

    for(;;){
        //Use select for descriptor waiting
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        if( (wait_for & WAIT_READ) != 0 )
            FD_SET(fd, &readfds);
        if( (wait_for & WAIT_WRITE) != 0 )
            FD_SET(fd, &writefds);

        //wait for one second
        tmout.tv_usec = 0;
        tmout.tv_sec = wait_interval;
        int res = select(fd + 1, &readfds, &writefds, NULL, &tmout);

        if( check_stop_signal() ){
            logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " -2 Stop signal detected");
            return -2;
        }

        if( res < 0 ){ //error occurred
            logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " -1 Select failed: " + std::to_string(errno));
            return -1;
        }
        else if( res == 0 ){ //timeout
            if( break_if_timeout ){
                return 0;
            }
            continue;
        }

        logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Ready for: " + (FD_ISSET(fd, &readfds) ? "Read " : "") +
                + (FD_ISSET(fd, &writefds) ? "Write" : "") + " Res: " + std::to_string(res));
/*
        std::cout <<  std::string(__func__) + " Ready for: " + (FD_ISSET(fd, &readfds) ? "Read " : "") +
                + (FD_ISSET(fd, &writefds) ? "Write" : "") + " Res: " + std::to_string(res) + " Timeout: " + std::to_string(wait_interval) << std::endl;
*/
        break;
    }

    return 1;
}

}
}