#include "yx.system.hpp"

#include <yosemitelib/native_token.hpp>
#include <yosemitelib/system_accounts.hpp>
#include <yosemitelib/system_depository.hpp>
#include <yosemitelib/transaction_fee.hpp>

#include <eosiolib/dispatcher.hpp>

namespace yosemitesys {

    using yosemite::sys_depository_info;
    using yosemite::yx_asset;

    void system_contract::regsysdepo( const account_name depository, const std::string& url, uint16_t location ) {

        eosio_assert( url.size() < 512, "url too long" );
        require_auth( depository );

        auto depo = _sys_depositories.find( depository );

        if ( depo != _sys_depositories.end() ) {
            _sys_depositories.modify( depo, depository, [&]( sys_depository_info& info ){
                info.is_authorized = false;
                info.url           = url;
                info.location      = location;
            });
        } else {
            _sys_depositories.emplace( depository, [&]( sys_depository_info& info ){
                info.owner         = depository;
                info.is_authorized = false;
                info.url           = url;
                info.location      = location;
            });
        }

        // pay transaction fee if not signed by system contract owner
        if (!has_auth(_self)) {
            yosemite::native_token::charge_transaction_fee(depository, YOSEMITE_TX_FEE_OP_NAME_SYSTEM_REG_SYS_DEPO);
        }
    }

    void system_contract::authsysdepo( const account_name depository ) {
        require_auth( _self );

        auto depo = _sys_depositories.find( depository );

        eosio_assert( depo != _sys_depositories.end(), "not found registered system depository" );
        eosio_assert( !depo->is_authorized, "system depository is already authorized" );

        _sys_depositories.modify( depo, 0, [&]( sys_depository_info& info ){
            info.is_authorized = true;
        });
    }

    void system_contract::rmvsysdepo( const account_name depository ) {
        require_auth( _self );

        auto depo = _sys_depositories.find( depository );

        eosio_assert( depo != _sys_depositories.end(), "not found registered system depository" );
        eosio_assert( depo->is_authorized, "system depository is already unauthorized" );

        _sys_depositories.modify( depo, 0, [&]( sys_depository_info& info ){
            info.is_authorized = false;
        });
    }

} //namespace yosemitesys
