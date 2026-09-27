#pragma once
#include <stddef.h>
#include <string.h>
struct LxDate { int year,month,day; };
inline bool parseLxDate(const char *text,LxDate &date) {
  const size_t n=strlen(text);
  if ((n!=8 && n!=10) || text[2]!='/' || text[5]!='/') return false;
  for (size_t i=0;i<n;++i)
    if (i!=2 && i!=5 && (text[i]<'0' || text[i]>'9')) return false;
  date.month=(text[0]-'0')*10+text[1]-'0';
  date.day=(text[3]-'0')*10+text[4]-'0';
  date.year=(text[6]-'0')*10+text[7]-'0';
  if (n==10) date.year=date.year*100+(text[8]-'0')*10+text[9]-'0';
  else date.year+=date.year>20?2000:2100; // Preserve upstream two-digit pivot.
  if (date.year<2000 || date.year>2300 || date.month<1 || date.month>12) return false;
  const int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  const bool leap=date.year%4==0 && (date.year%100!=0 || date.year%400==0);
  return date.day>=1 && date.day<=days[date.month-1]+(date.month==2 && leap);
}
