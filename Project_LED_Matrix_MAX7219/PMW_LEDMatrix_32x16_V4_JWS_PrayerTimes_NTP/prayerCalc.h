#include <PrayerTimes.h>

double times[sizeof(TimeName)/sizeof(char*)];

int ihti = 2;
float lati = -7.06119;
float longi = 110.44424;
uint8_t zonawaktu = utcOffset;

void JadwalSholat() {  
  set_calc_method(Karachi);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(AngleBased);
  set_fajr_angle(20);
  set_isha_angle(18);
  
  get_prayer_times(NTPyear, NTPmonth, NTPday, lati, longi, zonawaktu, times);
}
