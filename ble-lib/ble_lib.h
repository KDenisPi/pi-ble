/*
 * ble_lib.h
 *
 * BLE library. Wrapper over Gattlib
 *
 *  Created on: Sep 19, 2018
 *      Author: Denis Kudia
 */

#ifndef BLE_LIB_H
#define BLE_LIB_H

#include <iostream>
#include <string>
#include <cstdio>
#include <vector>

namespace pi_ble {
namespace ble_lib {

#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

class BleLib {
public:
    /*
    *
    */
    BleLib(const std::string& src_id = "") : _src_id(src_id) {
        bdaddr_t id;
        if(_src_id.empty())
            _dev_id = hci_get_route(NULL);
        else{
            str2ba(_src_id.c_str(), &id);
            _dev_id = hci_get_route(&id);
        }
    }

    /*
    * Check device id
    */
   bool is_device_id(){
       return (_dev_id >= 0);
   }

    /*
    *
    */
    virtual ~BleLib() {
    }

    /*
    * Find partner ID by name
    */
    const std::string find_partner(const std::string parner_name){
        std::string id;
        int len = 8, flags = IREQ_CACHE_FLUSH, max_rsp = 255;
        char addr[19] = { 0 };
        char name[248] = { 0 };

        if(!is_device_id() && parner_name.empty()){
            return id;
        }

        int sock = hci_open_dev( _dev_id );
        if ( sock < 0) {
            //TODO: Print error
            return id;
        }

        inquiry_info *ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

        /*
        * Request list of availavle points
        */
        int num_rsp = hci_inquiry(_dev_id, len, max_rsp, NULL, &ii, flags);
        if( num_rsp >= 0 ){
            std::cout <<  "Detected " << num_rsp << std::endl;

            for (int i = 0; i < num_rsp; i++){
                ba2str(&(ii+i)->bdaddr, addr);
                memset(name, 0, sizeof(name));

                if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) >= 0){
                    std::cout <<  "Addr " << addr << " Name " << name << std::endl;
                    if(parner_name.compare(name) == 0){
                        id = addr;
                        break;
                    }
                }
                else{
                    std::cout <<  "Addr " << addr << std::endl;
                }
            }
        }
        else{
            //TODO: Print error
        }

        free( ii );
        close( sock );

        return id;
    }

    /*
    * Print service class information for each service in the list
    */
    const std::vector<std::string> get_service_class_info(const sdp_list_t *list)
    {
        char buffer[MAX_LEN_SERVICECLASS_UUID_STR + 2*MAX_LEN_UUID_STR];
        char ServiceClassUUID_str[MAX_LEN_SERVICECLASS_UUID_STR];
        char UUID_str[MAX_LEN_UUID_STR];
        std::vector<std::string> result;

    	for (; list; list = list->next){
            uuid_t *uuid = (uuid_t *)list->data;

            int res = sdp_uuid2strn(uuid, UUID_str, MAX_LEN_UUID_STR);
            res = sdp_svclass_uuid2strn(uuid, ServiceClassUUID_str, MAX_LEN_SERVICECLASS_UUID_STR);
            if (res == 0) //uuid->type != SDP_UUID128)
                std::sprintf(buffer, "  \"%s\" (0x%s)", ServiceClassUUID_str, UUID_str);
            else
                std::sprintf(buffer, "  UUID 128: %s", UUID_str);

            result.push_back(std::string(buffer));
        }

        return result;
    }

    /*
    * Print attribute information fir each attribute in the list
    */
    void print_attribute_info(const sdp_list_t *list){
    	for (; list; list = list->next){
            sdp_data_t *d = (sdp_data_t*)list->data;
            printf("attrib ID: 0x%x ",d->attrId);

            switch( d->dtd ) {
                case SDP_UUID16:
                case SDP_UUID32:
                case SDP_UUID128:
                    printf("UUID type: 0x%x\n",d->dtd);
                    break;
                case SDP_UINT8:
                    printf("UINT8 type: 0x%x value: [0x%x]\n",d->dtd, d->val.uint8);
                    break;
                case SDP_UINT16:
                    printf("UINT16 type: 0x%x value: [0x%x]\n",d->dtd, d->val.uint16);
                    break;
                case SDP_UINT32:
                    printf("UINT32 type: 0x%x value: [0x%x]\n",d->dtd, d->val.uint32);
                    break;
                case SDP_TEXT_STR8:
                case SDP_TEXT_STR16:
                case SDP_TEXT_STR32:
                    {
                        char buffer[128];
                        if ( strlen(d->val.str) < sizeof(buffer)) {
				            strcpy(buffer, d->val.str);
                            printf("TEXT type: 0x%x [%s]\n",d->dtd, buffer);
                        }
                        else
                            printf("TEXT type: 0x%x\n",d->dtd);
                    }
                    break;
                case SDP_URL_STR8:
                case SDP_URL_STR16:
                case SDP_URL_STR32:
                    {
                        char buffer[128];
                        if ( strlen(d->val.str) < sizeof(buffer)) {
				            strcpy(buffer, d->val.str);
                            printf("URL type: 0x%x [%s]\n",d->dtd, buffer);
                        }
                        else
                            printf("URL type: 0x%x [too long %ld]\n",d->dtd, strlen(d->val.str));
                    }
                    break;
                default:
                    printf("type: 0x%x\n",d->dtd);
            }
        }
    }

    /*
    * Ger Service name
    */
    inline const std::string get_service_name(const sdp_record_t *rec){
        return get_service_attribut(rec, SDP_ATTR_SVCNAME_PRIMARY);
    }

    /*
    * Ger Service description
    */
    inline const std::string get_service_description(const sdp_record_t *rec){
        return get_service_attribut(rec, SDP_ATTR_SVCDESC_PRIMARY);
    }

    /*
    * Ger Service provider name
    */
    inline const std::string get_service_provider_name(const sdp_record_t *rec){
        return get_service_attribut(rec, SDP_ATTR_PROVNAME_PRIMARY);
    }

    /*
    * Ger Service documentation URL
    */
    inline const std::string get_service_doc_url(const sdp_record_t *rec){
        return get_service_attribut(rec, SDP_ATTR_DOC_URL);
    }

    /*
    * Ger Service icon URL
    */
    inline const std::string get_service_icon_url(const sdp_record_t *rec){
        return get_service_attribut(rec, SDP_ATTR_ICON_URL);
    }

    /*
    * Detect service parameters using SDP service
    *
    * TODO Service parameters will be added lated
    */
   const bool detect_service_over_sdp(const std::string id, const uint16_t* services, int services_count){
        uuid_t svc_uuid;
        bdaddr_t target, baddr_any = {0,0,0,0,0,0};
        sdp_list_t *response_list = NULL, *attrid_list = NULL;
        sdp_list_t *search_list = NULL;

        if( str2ba( id.c_str(), &target ) < 0 ){
            //invalid address
            return false;
        }

        // connect to the SDP server running on the remote machine
        sdp_session_t *session = sdp_connect( &baddr_any, &target, SDP_RETRY_IF_BUSY );
        if(session == NULL){
            //TODO: error code
            return false;
        }

        if(services_count > 0){
            for(int i = 0; i < services_count; i++){
                // specify the UUID of the application we're searching for
                uint16_t class16 = services[i] & 0xffff;
                sdp_uuid16_create(&svc_uuid, class16);
                search_list = sdp_list_append( search_list, &svc_uuid );
            }
        }
        else{
            sdp_uuid16_create(&svc_uuid, PUBLIC_BROWSE_GROUP);
            search_list = sdp_list_append( search_list, &svc_uuid );
        }

        // specify that we want a list of all the matching applications' attributes
        uint32_t range = 0x0000ffff;
        attrid_list = sdp_list_append( NULL, &range );

        // get a list of service records that have UUID 0xabcd
        int ret = sdp_service_search_attr_req( session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
        if( ret >= 0 ){
            sdp_list_t *r = response_list;

            // go through each of the service records
            for (; r; r = r->next ) {
                sdp_record_t *rec = (sdp_record_t*) r->data;
                sdp_list_t *attr_list = rec->attrlist;
                sdp_list_t *service_list;
                sdp_list_t *proto_list;

                std::cout << "**********************************" << std::endl << "Service Name: " << get_service_name(rec) << std::endl;
                std::cout << "Service Description: " << get_service_description(rec) << std::endl;
                std::cout << "Service Provider: " << get_service_provider_name(rec) << std::endl;

                std::cout << std::endl << "Attributes List" << std::endl;
                print_attribute_info(attr_list);

                if (sdp_get_service_classes(rec, &service_list) == 0) {
                    std::vector<std::string> svc_classes = get_service_class_info(service_list);
                    sdp_list_free(service_list, 0);

                    std::cout << std::endl << "Service Class ID List" << std::endl;
                    print_vector(svc_classes);
                }

                // get a list of the protocol sequences
                if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {
                    sdp_list_t *p = proto_list;
                    // go through each protocol sequence
                    for( ; p ; p = p->next ) {
                        sdp_list_t *pds = (sdp_list_t*)p->data;

                        // go through each protocol list of the protocol sequence
                        for( ; pds ; pds = pds->next ) {

                            // check the protocol attributes
                            sdp_data_t *d = (sdp_data_t*)pds->data;
                            int proto = 0;
                            for( ; d; d = d->next ) {

                                if(d->attrId > 0)
                                    printf("attr ID: 0x%x\n",d->attrId);

                                switch( d->dtd ) {
                                    case SDP_UUID16:
                                    case SDP_UUID32:
                                    case SDP_UUID128:
                                        proto = sdp_uuid_to_proto( &d->val.uuid );
                                        if(proto == RFCOMM_UUID)
                                            printf("\nprotocol: RFCOMM 0x%x\n",proto);
                                        else if( proto == ATT_UUID)
                                            printf("\nprotocol: ATT 0x%x\n",proto);
                                        else if( proto == L2CAP_UUID)
                                            printf("\nprotocol: L2CAP 0x%x\n",proto);
                                        else if( proto == AVCTP_UUID)
                                            printf("\nprotocol: AVCTP 0x%x\n",proto);
                                        else
                                            printf("\nprotocol: 0x%x\n",proto);
                                        break;
                                    case SDP_UINT8:
                                            printf("parameter UINT8 value: 0x%x\n",d->val.int8);
                                        break;
                                    case SDP_UINT16:
                                            printf("parameter UINT16 value: 0x%x\n",d->val.int16);
                                        break;
                                    default:
                                        printf("parameter type: 0x%x\n",d->dtd);
                                }
                            }

                            if( proto == L2CAP_UUID || proto == RFCOMM_UUID){
                                int port = sdp_get_proto_port(p, proto);
                                if( proto == RFCOMM_UUID ) {
                                    printf("rfcomm channel: %d\n",port);
                                }
                                else if( proto == L2CAP_UUID ) {
                                    printf("l2cap Protocol Service Multiplexers (PSM): %d\n",port);
                                }
                            }
                        }
                        sdp_list_free( (sdp_list_t*)p->data, 0 );
                    }
                    sdp_list_free( proto_list, 0 );
                }

                printf("found service record 0x%x\n\n", rec->handle);
                sdp_record_free( rec );
            }

        }
        else{ //error ret -1

        }

        sdp_close(session);

        return true;
   }

private:
    std::string _src_id; //use NULL if empty
    int _dev_id; //device ID
    int _fd; //Socket file descriptor

    /*
    *
    */
    void print_vector(const std::vector<std::string> vctr){
        for(std::string item : vctr) {
            std::cout << item << std::endl;
        }
    }

    /*
    *
    */
    const std::string get_service_attribut(const sdp_record_t *rec, uint16_t attr){
        std::string result;
        char buffer[64];

        switch(attr){
            case SDP_ATTR_SVCNAME_PRIMARY:
                if(sdp_get_service_name(rec, buffer, sizeof(buffer)) == 0)
                    result = buffer;
                break;
            case SDP_ATTR_SVCDESC_PRIMARY:
                if(sdp_get_service_desc(rec, buffer, sizeof(buffer)) == 0)
                    result = buffer;
                break;
            case SDP_ATTR_PROVNAME_PRIMARY:
                if(sdp_get_provider_name(rec, buffer, sizeof(buffer)) == 0)
                    result = buffer;
                break;
            case SDP_ATTR_DOC_URL:
                if(sdp_get_doc_url(rec, buffer, sizeof(buffer)) == 0)
                    result = buffer;
                break;
            case SDP_ATTR_CLNT_EXEC_URL:
                if(sdp_get_clnt_exec_url(rec, buffer, sizeof(buffer)) == 0)
                    result = buffer;
                break;
            case SDP_ATTR_ICON_URL:
                if(sdp_get_icon_url(rec, buffer, sizeof(buffer)) == 0)
                    result = buffer;
                break;
        }

        return result;
    }

};

} //namespace ble_lib
}//namespace pi_ble

#endif
