# Abtract
* This Yosemite contract is for non-native tokens.
* The Yosemite contract for the native token is [`yx.ntoken`](../../contracts/yx.ntoken/README.md), which the specification of the Yosemite token is explained.

# What You Can Do With This Contract
* An entity who owns the Yosemite account can create a token with `create` operation. YosemiteChain calls the entity the token issuer or the token depository.
   * When creating the token, the issuer can enable the setting of some options like account freezing or token freezing.
* The token depository issues and redeems the token.
* The token depository can freeze the token transfer.
* Any Yosemite accounts who own the token can transfer it each other.

# Management Actions

## setting fee for operations
* Transaction fee for operations is set by [yx.txfee](../../contracts/yx.txfee/)::settxfee operation.
```
cleos push action yx.txfee settxfee '{"operation":"tf.tcreate", "fee":"10000.0000 DKRW"}}' -p yosemite
cleos push action yx.txfee settxfee '{"operation":"tf.tissue", "fee":"100.0000 DKRW"}}' -p yosemite
cleos push action yx.txfee settxfee '{"operation":"tf.tredeem", "fee":"100.0000 DKRW"}}' -p yosemite
cleos push action yx.txfee settxfee '{"operation":"tf.ttransfer", "fee":"10.0000 DKRW"}}' -p yosemite
cleos push action yx.txfee settxfee '{"operation":"tf.tsetkyc", "fee":"5.0000 DKRW"}}' -p yosemite
cleos push action yx.txfee settxfee '{"operation":"tf.tsetopts", "fee":"5.0000 DKRW"}}' -p yosemite
cleos push action yx.txfee settxfee '{"operation":"tf.tfreezeac", "fee":"5.0000 DKRW"}}' -p yosemite
```

# Actions

## create
Create a (non-native) token with its symbol and precision

* The token creator naturally becomes the token depository.
* Enabling the setting of options cannot be done any more after creation, because the owners of the token should know about what options can be set and it must not be changed after they own.
```
cleos push action yx.token create '{"ysymbol":{"symbol":"4,BTC","issuer":"d2"},"can_set_options":0}' -p d2
```

### parameters of create
1. ysymbol : token symbol and its issuer
1. can_set_options : 16-bit flags of options
   * NONE : 0b0000000000000000 (0)
   * FREEZE_TOKEN_TRANSFER : 0b0000000000000001 (1)
   * FREEZE_ACCOUNT : 0b0000000000000010 (2)
   * SET_KYC_RULE : 0b0000000000000100 (4)

## issue
Issue the token to an account by the token depository

```
cleos push action yx.token issue '{"to":"user2","token":{"amount":"100000.0000 BTC","issuer":"d2"},"memo":"my memo"}' -p d2
```

### parameters of issue
1. to : the account who is transferred the token
1. token : the amount of token with the issuer(=token depository)
   * amount
   * issuer
1. memo : the additional data set by the action caller

### inline actions and notifications of issue
* Case 1. If the to account is different from the issuer, issue does the inline action for `transfer`.
```
#      yx.token <= yx.token::issue              {"to":"user2","token":{"amount":"100000.0000 BTC","issuer":"d2"},"memo":"my memo"}
#      yx.token <= yx.token::transfer           {"from":"d2","to":"user2","token":{"amount":"100000.0000 BTC","issuer":"d2"},"memo":"my memo"}
#            d2 <= yx.token::transfer           {"from":"d2","to":"user2","token":{"amount":"100000.0000 BTC","issuer":"d2"},"memo":"my memo"}
#         user2 <= yx.token::transfer           {"from":"d2","to":"user2","token":{"amount":"100000.0000 BTC","issuer":"d2"},"memo":"my memo"}
#     yx.ntoken <= yx.ntoken::payfee            {"payer":"d2","token":{"amount":"100.0000 DKRW","issuer":"d1"}}
#            d2 <= yx.ntoken::payfee            {"payer":"d2","token":{"amount":"100.0000 DKRW","issuer":"d1"}}
#      yx.txfee <= yx.ntoken::payfee            {"payer":"d2","token":{"amount":"100.0000 DKRW","issuer":"d1"}}
```

* Case 2. If the to account is the token depository itself, there is no `transfer` inline action.
```
#      yx.token <= yx.token::issue              {"to":"d2","token":{"amount":"10000.0000 BTC","issuer":"d2"},"memo":"my memo"}
#     yx.ntoken <= yx.ntoken::payfee            {"payer":"d2","token":{"amount":"100.0000 DKRW","issuer":"d1"}}
#            d2 <= yx.ntoken::payfee            {"payer":"d2","token":{"amount":"100.0000 DKRW","issuer":"d1"}}
#      yx.txfee <= yx.ntoken::payfee            {"payer":"d2","token":{"amount":"100.0000 DKRW","issuer":"d1"}}
```

## redeem
Redeem(burn) token from an account by the token depository

* At, first the account transfers the token to its depository.
```
cleos push action yx.token transfer '{"from":"user2","to":"d2","token":{"amount":"10000.0000 BTC","issuer":"d2"},"memo":"redeem by user2"}' -p user2
```
1. Then the token depository checks the transfer action is irreversible and calls redeem action.
```
cleos push action yx.token redeem '{"token":{"amount":"10000.0000 BTC","issuer":"d2"},"memo":"redeem for user2"}' -p d2
```

### parameters of redeem
1. token : the amount of token with the issuer(=token depository)
   * amount
   * issuer
1. memo : the additional data set by the action caller


## transfer
Transfer token

```
cleos push action yx.token transfer '{"from":"user2","to":"user3","token":{"amount":"10000.0000 BTC","issuer":"d2"},"memo":"my memo"}' -p user2
```

### parameters of transfer
1. from : account name to transfer from
1. to : account name to transfer to
1. token : the amount of token with the issuer(=token depository)
   * amount
   * issuer
1. memo : string less than or equal to 256 bytes

## wptransfer
Transfer token with the designated fee payer

```
cleos push action yx.token wptransfer '{"from":"user2","to":"user3","token":{"amount":"10000.0000 BTC","issuer":"d2"},"payer":"servprovider","memo":"my memo"}' -p user2
```

### parameters of wptransfer
1. from : account name to transfer from
1. to : account name to transfer to
1. token : the amount of token with the issuer(=token depository)
   * amount
   * issuer
1. payer : account name which pays transaction fee
1. memo : string less than or equal to 256 bytes


## setkycrule
Set the KYC vector for send or receive

* SET_KYC_RULE flag of can_set_options must be set at token creation time.
```
cleos push action yx.token setkycrule '{"ysymbol":{"symbol":"4,BTC","issuer":"d2"}, "type":0, "kyc":4}' -p d2
cleos push action yx.token setkycrule '{"ysymbol":{"symbol":"4,BTC","issuer":"d2"}, "type":1, "kyc":4}' -p d2
```

### parameters of setkycrule
1. ysymbol : token symbol and its issuer
1. type
   * 0 : transfer send
   * 1 : transfer receive
1. kyc : 16-bit flags from [yx.identity](../../contracts/yx.identity/)

## setoptions
overwrite or add token options

* The token depository can freeze token transfer or unfreeze it with TOKEN_OPTIONS_FREEZE_TOKEN_TRANSFER flag.
   * FREEZE_TOKEN_TRANSFER flag of can_set_options must be set at token creation time.

```
cleos push action yx.token setoptions '{"ysymbol":{"symbol":"4,BTC","issuer":"d2"}, "options":1, "overwrite":1}' -p d2
```

### parameters of setoptions
1. ysymbol : token symbol and its issuer
1. options : 16-bit flags
   * 0b0000000000000000 : TOKEN_OPTIONS_NONE
   * 0b0000000000000001 : TOKEN_OPTIONS_FREEZE_TOKEN_TRANSFER
1. overwrite
   * 1 : overwrite
   * 0 : add (don't overwrite)

## freezeacc
Freeze an account which has the token by its issuer

* FREEZE_ACCOUNT flag of can_set_options must be set at token creation time.
```
cleos push action yx.token freezeacc '{"ysymbol":{"symbol":"4,BTC","issuer":"d2"}, "accs":["user1","user2"], "freeze":1}' -p d2
cleos push action yx.token freezeacc '{"ysymbol":{"symbol":"4,BTC","issuer":"d2"}, "accs":["user2"], "freeze":0}' -p d2
```

### parameters of freezeacc
1. ysymbol : token symbol and its issuer
1. accs : this list of account names
1. freeze : freeze or unfreeze


# Tables

## tstats
Get the token statistics
```
cleos get table yx.token 4,BTC tstats
```

### results of tstats
```
{
  "rows": [{
      "issuer": "d2",
      "supply": "8000000000",
      "options": 0,
      "kyc_rule_types": [],
      "kyc_rule_flags": []
    }
  ],
  "more": false
}
```

## taccounts
Get all the token balances of the user
```
cleos get table yx.token user1 taccounts
```

### results of taccounts
1. id : This is just for internal managing purpose. 
1. yx_symbol_serialized : 128-bit integers which is combined with the EOS.IO symbol value and the issuer
   * | eosio::symbol value(64-bit integer) | issuer (64-bit integer) |
1. amount : balance of the account
```
{
  "rows": [{
      "id": 0,
      "yx_symbol_serialized": "0x00000000000080480654455354000000",
      "amount": "1000000000",
      "options": 0
    }
  ],
  "more": false
}
```