token=meme.token11
apply=newmeme11
airdrop=drop11
# tnew $airdrop
# tnew $apply
# tnew $token
tset $token meme.token
tset $apply applynewmeme
tset $airdrop airdropmeme

admin=ad

tpush $token init '["'$admin'","'$apply'"]' -p $token

tcli set account permission $token active --add-code
tcli set account permission $apply active --add-code
tcli set account permission $airdrop active --add-code

swap=pool
spot=spot1
spot_apply=dexapply1
tpush $apply init '["'$admin'","'$airdrop'","'$swap'","'$spot_apply'", "'$token'"]' -p $apply
tpush $airdrop init '["'$admin'","'$apply'"]' -p $airdrop
tpush $swap settkbanks '[["amax.token","amax.mtoken","mdao.token","cnyg.token","'$token'"]]' -p tech
tpush $swap addlpcreator '["'$apply'"]' -p tech
owner=ad
coin=HMEM
disc=meme
icon_url=https://cdn.pixabay.com/photo
urls=https
airdrop_enable=true
airdrop_ratio=1000
fee_ratio=500
transfer_ratio=300
swap_sell_fee_receiver=feerecv

tpush $apply applymeme '["'$owner'","1000000000.0000 '$coin'",["100.00000000 AMAX","amax.token"], 
                        "'$disc'","'$icon_url'","'$urls'", "white_paper", '$airdrop_enable', 
                        '$airdrop_ratio' , '$fee_ratio', '$transfer_ratio',
                        "'$swap_sell_fee_receiver'"]' -p $owner 
# tpush $apply clearmeme '["4,'$coin'"]' -p $apply 

 tcli system delegatebw amax $apply '0.1 AMAX' '0.1 AMAX'


 tcli system delegatebw amax $token '0.1 AMAX' '0.1 AMAX'


tpush amax.token transfer '{"from": "ad", "to": "'$apply'", "quantity": "100.00000000 AMAX", "memo": "meme:4,'$coin'"}' -p ad
