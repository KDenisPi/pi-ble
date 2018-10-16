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
#include <map>
#include <memory>

#include "ble_lib.h"

namespace pi_ble {
namespace ble_lib {

class BleServer;

class BleService
{
public:

    friend BleServer;
    /*
    *
    */
    BleService(const uint32_t srv_uuid, const std::string& svc_name, const std::string& svc_provider="", const std::string& svc_descr = "")
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
            std::cout <<  std::string(__func__) << " Already prepeared" << std::endl;
            return false;
        }

        if(channel == 0 || channel > 30){
            //Invalid channel number (1-30)
            std::cout <<  std::string(__func__) << " Invalid channel number" << std::endl;
            return false;
        }

        _rfcomm_channel = channel;
        _proto_rfcomm_enabled = true;

        std::cout <<  std::string(__func__) << " Configured" << std::endl;
        return _proto_rfcomm_enabled;
    }

    /*
    * Add L2CAP protocol configuration for this service
    */
    bool add_protocol_l2cap(const uint16_t psm){
        if(is_prepared()){
            std::cout <<  std::string(__func__) << " Already prepeared" << std::endl;
            //Prepared already
            return false;
        }

        // 1-4095 reserved numbers
        // PSM is odd number
        if(psm < 4095 || psm > 35765 || ((psm%2) == 0) ){
            std::cout <<  std::string(__func__) << " Invalid PSM number" << std::endl;
            //Invalid PSM
            return false;
        }

        _p2cap_psm = psm;
        _proto_l2cap_enabled = true;

        std::cout <<  std::string(__func__) << " Configured" << std::endl;
        return _proto_l2cap_enabled;
    }

    /*
    * Add ATT protocol configuration
    */
    bool add_protocol_att(){
        if(is_prepared()){
            std::cout <<  std::string(__func__) << " Already prepeared" << std::endl;
            //Prepared already
            return false;
        }

        _proto_att_enabled = true;

        std::cout <<  std::string(__func__) << " Configured" << std::endl;
        return _proto_att_enabled;
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

    bool prepare() {
        uint32_t service_uuid_int[] = { 0, 0, 0, 0 };
        uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, att_uuid, svc_uuid;
        _prepared = false;

        if( _srv_uuid == 0 ){
            //UUID empty
            std::cout <<  std::string(__func__) << " Invalid UUID" << std::endl;
            return false;
        }

        service_uuid_int[3] = _srv_uuid;

        _record = sdp_record_alloc();
        if( _record == NULL ){
            std::cout <<  std::string(__func__) << " Record allocation error. Err:" << std::to_string(errno) << std::endl;
            return false;
        }

        // set the general service ID
        sdp_uuid128_create( &svc_uuid, &service_uuid_int );
        sdp_set_service_id( _record, svc_uuid );

        // make the service record publicly browsable
        sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
        _root_list = sdp_list_append(0, &root_uuid);
        sdp_set_browse_groups( _record, _root_list );

        // L2CAP
        if( is_l2cap_enabled()){
            // set l2cap information
            sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
            _l2cap_list = sdp_list_append( 0, &l2cap_uuid );

            //Set PSM
            _psm = sdp_data_alloc(SDP_UINT16, &_p2cap_psm);
            sdp_list_append( _l2cap_list, _psm );

            //Add protocol
            _proto_list = sdp_list_append( 0, _l2cap_list );
        }

        // RFCOMM
        if( is_rfcomm_enabled() ){
            sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
            _rfcomm_list = sdp_list_append( 0, &rfcomm_uuid );

            _channel = sdp_data_alloc(SDP_UINT8, &_rfcomm_channel);
            sdp_list_append( _rfcomm_list, _channel );

            sdp_list_append( _proto_list, _rfcomm_list );
        }

        // ATT
        if(is_att_enabled()){
            sdp_uuid16_create(&att_uuid, ATT_UUID);
            _att_list = sdp_list_append( 0, &att_uuid );

            sdp_list_append( _proto_list, _att_list );
        }

        // attach protocol information to service record
        _access_proto_list = sdp_list_append( 0, _proto_list );
        sdp_set_access_protos( _record, _access_proto_list );

        // set the name, provider, and description
        sdp_set_info_attr(_record, _svc_name.c_str(), _svc_provider.c_str(), _svc_descr.c_str());

        _prepared = true;
        return _prepared;
    }

    /*
    *
    */
    const bool is_l2cap_enabled() const {
        return _proto_l2cap_enabled;
    }

    const bool is_rfcomm_enabled() const {
        return _proto_rfcomm_enabled;
    }

    const bool is_att_enabled() const {
        return _proto_att_enabled;
    }

    // Get service UUID
    const uint32_t uuid() const {
        return _srv_uuid;
    }

    // Get service name
    const std::string get_name() const {
        return _svc_name;
    }

protected:
    sdp_record_t * get_record(){
        return ( is_prepared() ? _record : nullptr );
    }

    const bool is_registered() const {
        return _registered;
    }

    void set_registered( const bool reg ){
        _registered = reg;
    }

private:
    std::string _svc_name;      //service name
    std::string _svc_descr;     //service descriptor
    std::string _svc_provider;  //service provider

    uint32_t _srv_uuid;  //service UUID

    bool _prepared;     //if service configuration prepared

    bool _registered;  //if service was successfully registered on SDP
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
    sdp_record_t *_record = nullptr;

    sdp_list_t *_l2cap_list = nullptr;
    sdp_list_t *_rfcomm_list = nullptr;
    sdp_list_t *_att_list = nullptr;
    sdp_list_t *_root_list = nullptr;
    sdp_list_t *_proto_list = nullptr;
    sdp_list_t *_access_proto_list = nullptr;

    sdp_data_t *_channel = nullptr;
    sdp_data_t *_psm = nullptr;

    /*
    * Release all allocated memory
    */
    void clear(){
        std::cout <<  std::string(__func__) << std::endl;

        if(_record){
            sdp_record_free(_record);
            _record = nullptr;
        }

        sdp_list_free( _root_list, 0 );
        sdp_list_free( _access_proto_list, 0 );
        sdp_list_free( _proto_list, 0 );

        if( is_rfcomm_enabled() ){
            sdp_list_free( _rfcomm_list, 0 );
            sdp_data_free( _channel );
        }

        if( is_l2cap_enabled()){
            sdp_list_free( _l2cap_list, 0 );
            sdp_data_free( _psm );
        }

        if( is_att_enabled() ){
            sdp_list_free( _att_list, 0 );
        }
    }
};

/*
*
*
*/
using BleSevicePtr = std::shared_ptr<BleService>;

class BleServer {
public:
    BleServer() {}
    virtual ~BleServer() {}

   /*
   * Register server on SDP
   *
   */
   virtual bool sdp_register() {
        bdaddr_t baddr_any = {0,0,0,0,0,0};
        bdaddr_t baddr_local = {0, 0, 0, 0xff, 0xff, 0xff};
        sdp_session_t *session = nullptr;

        //Register all available services
        session = sdp_connect( &baddr_any, &baddr_local, SDP_RETRY_IF_BUSY );
        if( session == NULL ){
            std::cout <<  std::string(__func__) << " Session connection error. Err:" << std::to_string(errno) << std::endl;
            // Failed print errno
            return false;
        }

        for (auto it = _services.begin(); it != _services.end(); ++it) {
            std::cout <<  std::string(__func__) << " Register service: " << it->second->get_name() << std::endl;
            if(!it->second->is_registered()){
                int ret = sdp_record_register(session, it->second->get_record(), 0);
                if( ret == 0 ){
                    // Mark service as registered
                    _services[it->first]->set_registered(true);
                }
                else {
                    std::cout <<  std::string(__func__) << " Service registration error. Err:" << std::to_string(ret) << std::endl;
                }
            }
            else {
                // This service registered already
                std::cout <<  std::string(__func__) << " Registered already. Service: " << it->second->get_name() << std::endl;
            }
        }

        sdp_close( session );

       return true;
    }

   /*
   * Un-Register all servers from SDP
   *
   */
   virtual bool sdp_unregister() {
        bdaddr_t baddr_any = {0,0,0,0,0,0};
        bdaddr_t baddr_local = {0, 0, 0, 0xff, 0xff, 0xff};
        sdp_session_t *session = nullptr;

        //Register all available services
        session = sdp_connect( &baddr_any, &baddr_local, SDP_RETRY_IF_BUSY );
        if( session == NULL ){
            std::cout <<  std::string(__func__) << " Session connection error. Err:" << std::to_string(errno) << std::endl;
            // Failed print errno
            return false;
        }

        for (auto it = _services.begin(); it != _services.end(); ++it) {
            std::cout <<  std::string(__func__) << " Un-Register service: " << it->second->get_name() << std::endl;

            if(it->second->is_registered()){
                int ret = sdp_record_unregister(session, it->second->get_record());
                if( ret == 0 ){
                    // Mark service as registered
                    _services[it->first]->set_registered(false);
                }
                else {
                    std::cout <<  std::string(__func__) << " Service unregistration error. Err:" << std::to_string(ret) << std::endl;
                }
            }
            else {
                // This service registered already
                std::cout <<  std::string(__func__) << " Unregistered already. Service: " << it->second->get_name() << std::endl;
            }
        }

        sdp_close( session );
        return true;
    }

    /*
    * Add service to the list
    */
    void add_service(const BleSevicePtr& service){
        auto uuid = _services.find(service->uuid());
        if( uuid != _services.end()){
            _services.emplace( std::make_pair(service->uuid(), service) );
        }
    }

private:
    sdp_session_t *_session = nullptr;
    std::map<uint32_t, BleSevicePtr> _services;

};

}
}

#endif