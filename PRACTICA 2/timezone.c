#include <conio.h>
#include <stdio.h>

#include <windows.h>


int getTimeZone()
{
   TIME_ZONE_INFORMATION tziOld;
	DWORD dwRet;

	dwRet = GetTimeZoneInformation(&tziOld);

	 if(dwRet == TIME_ZONE_ID_STANDARD || dwRet == TIME_ZONE_ID_UNKNOWN)    
      tziOld.StandardBias/60;
   else if( dwRet == TIME_ZONE_ID_DAYLIGHT )
      return tziOld.DaylightBias/60;
   else
   {
      printf("GTZI failed (%d)\n", GetLastError());
      return 0;
   }

  

}