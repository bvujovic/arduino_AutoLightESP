# AutoLightESP

Automatsko paljenje/gašenje pozadinskog (slabijeg) svetla u prostoriji.

Aparat pali svetlo kada dođe HIGH signal sa PIR-a, a nema dovoljno osvetljenja u prostoriji. ESP hostuje veb server na kojem je aplikacija kroz koju se mogu:
  1) podešavati neki parametri aparata: dužina trajanja svetla, niža i viša granica osvetljenja, vreme posle kojeg će se veb server i wifi ugasiti na ESP-u, ...
  2) pregledati parametri sa senzora (pre svega sa LDR-a).
WiFi i veb server se pale na taster koji je multipleksiran sa LDR-om na istoj žici.
