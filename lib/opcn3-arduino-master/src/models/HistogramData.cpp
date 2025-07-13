#include "HistogramData.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
float Sizeboundries[25] ={0.35,0.46,0.66,1.0,1.3,1.7,2.3,3.0,4.0,5.2,6.5,8.0,10.0,12.0,14.0,16.0,18.0,20.0,22.0,25.0,28.0,31.0,34.0,37.0,40.0};

float HistogramData::getTempC()
{
    return -45 + 175 * (temperature / (pow(2, 16) - 1));
}
float HistogramData::getTempF()
{
    return -49 + 347 * (temperature / (pow(2, 16) - 1));
}
float HistogramData::getHumidity()
{
    return 100 * (humidity / (pow(2, 16) - 1));
}



std::string floatToStringReplaceDot(float value) {
       std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << value;
    std::string str = ss.str();
    
    std::replace(str.begin(), str.end(), '.', 'o');
    return str;
}


String HistogramData::toString() {

    // info = "-----histogram----- ";
    String info = "opc_validity ";
    info += valid;
    info += " ";
    //info += "Bin Counts\n";
    for (int i = 0; i < 24; i++) {
        info += "opc_h";
        String bin = floatToStringReplaceDot(Sizeboundries[i]).c_str();
        info += bin;
        info += " ";
        info += binCounts[i];
        info += " ";
    }
    info += " ";
    //info += "------------------------------------------------- ";
    info += "opc_timebin1 ";
    info += bin1TimeToCross;
    info += " ";
    info += "opc_timebin3 ";
    info += bin3TimeToCross;
    info += " ";
    info += "opc_timebin5 ";
    info += bin5TimeToCross;
    info += " ";
    info += "opc_timebin7 ";
    info += bin7TimeToCross;
    info += " ";

   // info += "------------------------------------------------- ";
    info += "opc_sampling_period ";
    info += samplingPeriod;
    info += " ";
   // info += "------------------------------------------------- ";
    info += "opc_sample_flow_rate ";
    info += sampleFlowRate;
    info += " ";
   // info += "------------------------------------------------- ";
    info += "opc_temp_raw ";
    info += temperature;
    info += " ";
   // info += "------------------------------------------------- ";
    info += "opc_humid_raw ";
    info += humidity;
    info += " ";
  //  info += "------------------------------------------------- ";
    info += "opc_pm1 ";
    info += pm1;
    info += " ";
   // info += "------------------------------------------------- ";
    info += "opc_pm25 ";
    info += pm2_5;
    info += " ";
   // info += "------------------------------------------------- ";
    info += "opc_pm10 ";
    info += pm10;
    info += " ";
    
  //  info += "------------------------------------------------- ";

    // info += "MSLNS ";

    // info += rejectCountGlitch;
    // info += " ";
    // info += rejectCountLongTOF;
    // info += " ";
    // info += rejectCountRatio;
    // info += " ";
    // info += rejectCountOutOfRange;
    // info += " ";
    // info += fanRevCount;
    // info += " ";
    // info += laserStatus;
    // info += " ";
    info += "opc_checksum ";
    info += checkSum;
    info += "\n\r";

    return info;
}