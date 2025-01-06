token=meme.token4
tnew $token
tset $token meme.token

apply=newmeme4
tnew $apply
tset $apply applynewmeme

airdrop=drop4;
tnew $airdrop
tset $airdrop airdropmeme


admin=ad

tpush $token init '["'$admin'","'$apply'"]' -p $token

tcli set account permission $token active --add-code
tcli set account permission $apply active --add-code
tcli set account permission $airdrop active --add-code

swap=hoot.swap2
spot=spot1
spot_apply=applyspot1
tpush $apply init '["'$admin'","'$airdrop'","'$swap'","'$spot'","'$spot_apply'", "'$token'"]' -p $apply
tpush $airdrop init '["'$admin'","'$apply'"]' -p $airdrop


# applymeme(const name& owner, 
#                      const asset&      coin,
#                      const string&     disc,
#                      const string&     icon_url, 
#                      const string&     urls,
#                      const uint64_t&   airdrop_ratio,
#                      const uint64_t&   destroy_ratio,      //转账手续费销毁
#                      const uint64_t&   transfer_ratio,
#                      const name&       fee_receiver, //转账手续费接收账户
#                      const bool&       airdrop_enable,
#                      const extended_symbol&  trade_symbol,
#                      const uint64_t&         init_price)
owner=ad
coin=4,MEME
disc=meme
icon_url=https://cdn.pixabay.com/photo
urls=https
airdrop_ratio=1000
destroy_ratio=500
transfer_ratio=300
fee_receiver=feerecv
airdrop_enable=true
# trade_symbol=4,USDT
init_price=10
# tnew $fee_receiver

tpush $apply applymeme '["'$owner'","1000000000.0000 '$coin'","'$disc'","'$icon_url'","'$urls'",'$airdrop_ratio','$destroy_ratio','$transfer_ratio',"'$feerecv'",true,["8,AMAX","amax.token"],'$init_price']' -p $owner 



