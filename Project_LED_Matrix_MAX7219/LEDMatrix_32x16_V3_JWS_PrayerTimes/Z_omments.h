// =======================================================================

// for ESP-01 module
//#define DIN_PIN 2 // D4
//#define CS_PIN  3 // D9/RX
//#define CLK_PIN 0 // D3

// for NodeMCU 1.0/D1 mini
//#define DIN_PIN 15  // D8
//#define CS_PIN  13  // D7
//#define CLK_PIN 12  // D6

// =======================================================================
// https://en.wikipedia.org/wiki/Summer_Time_in_Europe
// from
// Sunday (31 − ((((5 × y) ÷ 4) + 4) mod 7)) March at 01:00 UTC
// to
// Sunday (31 − ((((5 × y) ÷ 4) + 1) mod 7)) October at 01:00 UTC

//int checkSummerTime()
//{
//  if(month>3 && month<10) return 1;
//  if(month==3 && day>=31-(((5*year/4)+4)%7) ) return 1;
//  if(month==10 && day<31-(((5*year/4)+1)%7) ) return 1;
//  return 0;
//}
// =======================================================================
