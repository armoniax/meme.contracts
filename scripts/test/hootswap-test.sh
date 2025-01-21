swap='pool'
farm='farm'
liqd='liqd'
tech='tech'
tnew $swap
tnew $farm
tnew $liqd
tnew $tech


tset $swap hoot.swap
tset $farm hoot.farm
tset $liqd hoot.token

tcli set account permission $swap active --add-code
tcli set account permission $farm active --add-code
tcli set account permission $liqd active --add-code

tpush $swap setadmin '["'$tech'"]' -p $swap
tpush $swap setliqbank '["'$liqd'"]' -p $tech

tpush $swap setcollector '["'$tech'", "'$tech'"]' -p $tech


tpush $swap settkbanks '[["amax.token","amax.mtoken","mdao.token","cnyg.token"]]' -p $tech
tpush $swap addlpcreator '["'$tech'"]' -p $tech


tpush $farm setadmin '["'$tech'"]' -p $farm

tpush $swap setfglobal '["amax.devfund",10, 10 ,"0.0500 APL"]'  -p $tech