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

namespace pi_ble {
namespace ble_ftp {

enum BleFtpStates {
    Initial,
    Connecting,
    Connected,
    FileTransfer,
    Error
};

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

private:
    BleFtpStates _state;
};

}
}

#endif