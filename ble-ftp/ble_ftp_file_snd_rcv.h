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

class BleFtpFile {
public:
    BleFtpFile(const bool role, const std::string& fname) : _role(role), _filename(fname) {

    }

    ~BleFtpFile() {

    }

private:
    bool _role; //true - receiver, false - sender
    std::string _filename; //full file path
};

}//namespace ble_ftp
}//namespace pi-ble

#endif