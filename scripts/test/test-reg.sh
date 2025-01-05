token=meme.token1
tnew $token
tset $token meme.token

apply=newmeme1
tnew $apply
tset $apply applynewmeme

airdrop=drop1;
tnew $airdrop


admin=ad

tpush $token init '["'$admin'","'$apply'","",""]' -p $token
