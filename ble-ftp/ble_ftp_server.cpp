/*
 * ble_ftp_server.cpp
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

#include "ble_ftp_server.h"

namespace pi_ble {
namespace ble_ftp {

const char TAG[] = "ftpd";

std::string BleFtpServer::helpText = "Commands list:\n\
    HELP - print this help\n\
    QUIT - finish session\n\
    LIST - print list files in current server directory\n\
    PWD  - print current server directory\n\
    CWD  - change current server directory\n\
    CDUP - change current server diectory to parent\n\
    DELE - delete file\n\
    MKD  - make directory\n\
    RMD  - remove directory\n\
    RETR - download file from server\n\
    STOR - upload file from server;\n";

/*
*
*/
bool BleFtpServer::wait_for_connection(){
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__));

    socklen_t addrlen;
#ifdef USE_NET_INSTEAD_BLE
    struct sockaddr_in addr_rem = { 0 };
#else
    struct sockaddr_rc addr_rem;
    memset(&addr_rem, 0, sizeof(addr_rem));
#endif
    addrlen = sizeof(addr_rem);

    if( wait_for_descriptor(_sock_cmd, WAIT_READ) <= 0 ){
        return false;
    }

    _sock_client = accept( _sock_cmd, (struct sockaddr *)&addr_rem, &addrlen );
    if( _sock_client < 0 ){
        logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Accept failed: " + std::to_string(errno));
        return false;
    }

#ifdef USE_NET_INSTEAD_BLE
    char addr[64];
    inet_ntop(AF_INET, (const char*)&addr_rem.sin_addr, addr, sizeof(addr));
#else
    char addr[1024];
    ba2str( &addr_rem.rc_bdaddr, addr );
#endif

    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Accept from: " + addr + " Socket:" + std::to_string(_sock_client));
    return true;
}

/*
*
*/
bool BleFtpServer::close_client() {
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " socket: " + std::to_string(_sock_client) );

    if( _sock_client > 0 ){
        close( _sock_client );
        _sock_client = 0;
    }
    return true;
}


/*
 *
 */
bool BleFtpServer::start(){
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Started");
    return piutils::Threaded::start<BleFtpServer>(this);
}

/*
 *
 */
void BleFtpServer::stop(){
    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Started.");
    close_client();
    close_socket();
    piutils::Threaded::stop();
}

/*
* Process HELP command on server side
*/
bool BleFtpServer::process_cmd_help(){
    const std::string response = prepare_result(200, "HELP") + helpText;
    return  write_data( get_cmd_socket(), response.c_str(), response.length());
}


/*
* FTP over BLE working function
*/
void BleFtpServer::worker(BleFtpServer* owner){
    int res;

    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Started. Initialiing connection");
    owner->to_state(BleFtpStates::Connecting);

    //open CMD socket
    if(!owner->initialize()){
        logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Could not initilize connection");
        owner->to_state(BleFtpStates::Error);
    }
    else{

        if( owner->prepare() ){
            for(;;){

                std::cout <<  " Wait for connection " << std::endl;

                if( !owner->wait_for_connection() ){
                    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Socket error detected");
                    break;
                }

                //Stop signal detected
                if( owner->is_stop_signal() ){
                    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Stop signal detected");
                    break;
                }

                //Detected request for connection
                bool active_session = true;
                for(;active_session;){

                    //waiting for command
                    auto cmd = owner->cmd_receive();
                    switch(cmd.first){
                        case pi_ble::ble_ftp::CmdList::Cmd_Unknown:
                            //do nothing - probably just log it in
                            break;
                        //if end of session detected or error detected - move out from the CMD processing
                        case pi_ble::ble_ftp::CmdList::Cmd_Timeout:
                            active_session = false;
                            owner->set_stop_signal(true);
                            break;
                        case pi_ble::ble_ftp::CmdList::Cmd_Quit:
                            active_session = false;
                            owner->process_cmd_quit();
                            break;
                        case pi_ble::ble_ftp::CmdList::Cmd_Help:
                            owner->process_cmd_help();
                            break;
                        case pi_ble::ble_ftp::CmdList::Cmd_Pwd:
                            owner->process_cmd_pwd();
                            break;
                        case pi_ble::ble_ftp::CmdList::Cmd_List:
                            owner->process_cmd_list(cmd.second);
                            break;
                        case pi_ble::ble_ftp::CmdList::Cmd_Cwd:
                            owner->process_cmd_cwd(cmd.second);
                            break;
                        case pi_ble::ble_ftp::CmdList::Cmd_Cdup:
                            owner->process_cmd_cdup();
                            break;
                        case pi_ble::ble_ftp::CmdList::Cmd_Mkd:
                            owner->process_cmd_mkdir(cmd.second);
                            break;
                        case pi_ble::ble_ftp::CmdList::Cmd_Rmd:
                            owner->process_cmd_rmdir(cmd.second);
                            break;
                        case pi_ble::ble_ftp::CmdList::Cmd_Dele:
                            owner->process_cmd_delete(cmd.second);
                            break;
                    }
                }
                //Close client connection
                owner->close_client();

                //Stop signal detected
                if( owner->is_stop_signal() ){
                    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Stop signal detected");
                    break;
                }
            }
        }
        else{
            logger::log(logger::LLOG::ERROR, TAG, std::string(__func__) + " Could not prepare connection");
            owner->to_state(BleFtpStates::Error);
        }

        //free socket
        owner->close_socket();
        owner->to_state(BleFtpStates::Initial);
    }

    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Finished");
}

/*
* Recive command
*/
const CmdInfo BleFtpServer::cmd_receive()
{
    CmdList cmd = CmdList::Cmd_Unknown;

    int fd = get_cmd_socket();

    int res =  wait_for_descriptor(fd, WAIT_READ, 60, true);
    if( res < 0 ){
        logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Wait_for_descriptor error or Stop signal detected");
        return std::make_pair(cmd, "");
    }

    // Client session timeout (TODO: state detection to avoid break during file uploading/downloading)
    if( res == 0 ){
        return std::make_pair(CmdList::Cmd_Timeout, "");
    }

    std::string command;
    if( read_data(fd, command) <= 0 ){
        logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + "read_data failed");
        return std::make_pair(cmd, "");
    }

    logger::log(logger::LLOG::DEBUG, TAG, std::string(__func__) + " Received: Bytes " + std::to_string(command.length()) + " [" + command + "]");

    return recognize_cmd(command);
}

}
}