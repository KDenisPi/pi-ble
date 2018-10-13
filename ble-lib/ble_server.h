/*
 * ble_server.h
 *
 * BLE library. Server implementation
 *
 *  Created on: Sep 21, 2018
 *      Author: Denis Kudia
 */

#ifndef BLE_SERVER_LIB_H
#define BLE_SERVER_LIB_H

#include <thread>
#include "ble_lib.h"

namespace pi_ble {
namespace ble_lib {

class BleService
{
public:
    /*
    *
    */
    BleService(const uint8_t srv_uuid, const std::string& svc_name, const std::string& svc_descr = "", const std::string& svc_provider="")
        : _svc_name(svc_name), _svc_descr(svc_descr), _svc_provider(svc_provider), _srv_uuid(srv_uuid),
        _prepared(false), _proto_rfcomm_enabled(false), _rfcomm_channel(0), _proto_l2cap_enabled(false), _p2cap_psm(0),
        _proto_att_enabled(false) {

    }

    /*
    * Add RFCOMM protocol configuration for this service
    */
    bool add_protocol_rfcomm(const uint8_t channel){
        if(is_prepared()){
            //Prepared already
            return false;
        }

        if(channel == 0 || channel > 30){
            //Invalid channel number (1-30)
            return false;
        }

        _rfcomm_channel = channel;
        _proto_rfcomm_enabled = true;

        return _proto_rfcomm_enabled;
    }

    /*
    * Add L2CAP protocol configuration for this service
    */
    bool add_protocol_l2cap(const uint16_t psm){
        if(is_prepared()){
            //Prepared already
            return false;
        }

        // 1-4095 reserved numbers
        // PSM is odd number
        if(psm < 4095 || psm > 35765 || ((psm%2) == 0) ){
            //Invalid PSM
            return false;
        }

        _p2cap_psm = psm;
        _proto_l2cap_enabled = true;
        return _proto_l2cap_enabled;
    }

    /*
    *
    */
    virtual ~BleService() {
        clear();
    }

    /*
    * Detect if service configuration was prepared for using
    *
    * I will allow to re-configura prepared service object for now.
    */
    const bool is_prepared() const {
        return _prepared;
    }

private:
    std::string _svc_name;      //service name
    std::string _svc_descr;     //service descriptor
    std::string _svc_provider;  //service provider

    uint8_t _srv_uuid;  //service UUID

    bool _prepared;     //if service configuration prepared
    /*
    * Protocol configurations
    */

    // ---- RFCOMM ----
    bool _proto_rfcomm_enabled;
    uint8_t _rfcomm_channel;    //RFCOMM channel

    // ---- L2CAP ----
    bool _proto_l2cap_enabled;
    uint16_t _p2cap_psm; // Protocol Service Multiplexers (PSM)

    // ---- ATT ----
    bool _proto_att_enabled;

    /*
    * Internal SDP data structures
    */

    /*
    * Release all allocated memory
    */
    void clear(){

    }
};

/*
*
*
*/
class BleServer {
public:
    BleServer() {}
    virtual ~BleServer() {}

   /*
   * Register server on SDP
   *
   */
   virtual bool sdp_register() {

       return false;
   }

   /*
   * UnRegister server on SDP
   *
   */
   virtual bool sdp_unregister() {

       return true;
   }

};

}
}

#endif