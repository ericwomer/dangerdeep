#!/bin/bash
for i in battleship_malaya carrier_bogue corvette destroyer_tribal large_freighter large_merchant medium_freighter medium_merchant medium_troopship small_merchant small_tanker submarine_IXc40 submarine_VIIc submarine_XXI ; do ./crosssection --nofullscreen $i.3ds > $i.cs 2> /dev/null;done
