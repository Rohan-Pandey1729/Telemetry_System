#include "DataLogger.h"

DataLogger::DataLogger(uint32_t bnoPeriod, uint32_t bmePeriod, uint32_t gpsPeriod) 
{

  timerBNO = new Timer(bnoPeriod);
  timerBME = new Timer(bmePeriod);
  timerGPS = new Timer(gpsPeriod);
  pinMode(chipSelect, OUTPUT);
}


bool DataLogger::initialize()
{

  if (!SD.begin(chipSelect)) 
  {
    Serial.println("Card failed, or not present. Terminating");
    return false;
  }
  Serial.println("Card initialized.");

  if (!newDataFile()) {
    Serial.println("Failed to open data file");
    return false;
  }
    
  return true;
}


/*
* Closes old file, and generates a new file, opens that file, and adds headers.
*/
bool DataLogger::newDataFile() {
    fileName = getFileName();
    const char* filePath = fileName.c_str();
    Serial.println(filePath);
    Serial.println(fileName);
    File dataFile = SD.open(filePath, FILE_WRITE);
    // if (!newDataFile) {
    //     Serial.println("Failed to open data file");
    //     return false;
    // }

  if (!dataFile) {
    Serial.println("Hello!");
    return false;
  }

    addDataHeaders(dataFile);
    Serial.print("New file created and opened -> name: ");
    Serial.println(fileName);
    dataFile.close();
    return true;
}

void DataLogger::addDataHeaders(File dataFile)
{
  dataFile.println("SystemMillis,BMETemperature,BMEPressure,BMEApproximateAlt,BNO Quat W, BNO Quat X,BNO Quat Y,BNO Quat Z,BNO Gyro X,BNO Gyro Y,BNO Gyro Z,BNO Accel X,BNO Accel Y,BNO Accel Z,BNO_Grav_X,BNO_Grav_Y,BNO_Grav_Z,BNO CALIBRATION sys,BNOGyro,BNOAccel,BNOMagnitude,ALGOPos1,ALGOPos2,GPSHour,GPSmin,GPSsec,GPSmilli,GPSday,GPSmonth,GPSyear,GPSFix,GPSfixquality_3d,GPSLat,GPSlong,GPSSpeed,GPSAngle,GPSAlt,GPSSatNum,GPSLongitude,GPSLatitude,GPSAltitude,GPSSpeed,GPSAngle");
}


/*
* Checks for files that exist in numerical order, creating a file name after the last existing one is found. Called when the datalogger class is created
* Format: datalog_n (n is a number, zero-indexed)
*/
String DataLogger::getFileName() 
{

  String file = "datalog";

  int i = 0;

  while (SD.exists((file + i + ".csv").c_str())) {
    i++;
  }

  return file + i + ".csv";
  
}


// SD OUTPUT METHODS:

void DataLogger::logDataCSV()
{

  File dataFile = SD.open(fileName.c_str(), FILE_WRITE);

  bool bmeReady = (timerBME->check());
  bool bnoReady = (timerBNO->check());
  bool gpsReady = updateGPS();

  if ((bmeReady || bnoReady || gpsReady) && dataFile)
  {
    dataFile.print(millis());
    dataFile.print(",");

    if (bmeReady)
    {
      timerBME->reset();
      logBME(dataFile);
      
    }
    else 
      logBMESpacer(dataFile);

    if (bnoReady)
    {
      timerBNO->reset();
      logBNO(dataFile);
      logAlgo(dataFile);
    }
    else {
      logBNOSpacer(dataFile);
      logAlgoSpacer(dataFile);
    }
      

    if (gpsReady)
    {
      timerGPS->reset();
      logGPS(dataFile);
    }
    else {
      logGPSSpacer(dataFile);
    }

    dataFile.println();
  }

    dataFile.close();
  
}

void DataLogger::logBME(File dataFile)
{

  float pressure = getPressure();
  float aproxAlt = getSeaLevelAlt();
  float temp = getTemperature();

  dataFile.print(temp);
  dataFile.print(",");

  dataFile.print(pressure);
  dataFile.print(",");

  dataFile.print(aproxAlt);
  dataFile.print(",");

}

void DataLogger::logBMESpacer(File dataFile)
{
  dataFile.print(",");
  dataFile.print(",");
  dataFile.print(",");
}

void DataLogger::logBNO(File dataFile)
{

  imu::Quaternion quat = getQuaternion();
  imu::Vector<3> gyros = getGyroscpe();
  imu::Vector<3> accele = getAccelermometer();
  imu::Vector<3> grav = getGravity();
  updateCalibration();

  dataFile.print(quat.w(), 4);
  dataFile.print(",");
  dataFile.print(quat.x(), 4);
  dataFile.print(",");
  dataFile.print(quat.y(), 4);
  dataFile.print(",");
  dataFile.print(quat.z(), 4);
  dataFile.print(",");
  
  dataFile.print(gyros.x());
  dataFile.print(",");
  dataFile.print(gyros.y());
  dataFile.print(",");
  dataFile.print(gyros.z());
  dataFile.print(",");

  dataFile.print(accele.x());
  dataFile.print(",");
  dataFile.print(accele.y());
  dataFile.print(",");
  dataFile.print(accele.z());
  dataFile.print(",");

  dataFile.print(grav.x());
  dataFile.print(",");
  dataFile.print(grav.y());
  dataFile.print(",");
  dataFile.print(grav.z());
  dataFile.print(",");
  // uint8_t systemCal, gyro, accel, mag = 0;
  dataFile.print(systemCal, DEC);
  dataFile.print(",");
  dataFile.print(gyro, DEC);
  dataFile.print(",");
  dataFile.print(accel, DEC);
  dataFile.print(",");
  dataFile.print(mag, DEC);
  dataFile.print(",");
}

void DataLogger::logBNOSpacer(File dataFile)
{
  for (int i = 0; i < 17; i++)
  {
    dataFile.print(",");
  }

}

void DataLogger::logAlgo(File dataFile) 
{
  float posTemp[2];
  float *pos = getPosAlgo(posTemp);
      
  dataFile.print(pos[0]);
  dataFile.print(",");
  dataFile.print(pos[1]);
  dataFile.print(",");
}

void DataLogger::logAlgoSpacer(File dataFile)
{
  dataFile.print(",");
  dataFile.print(",");
}

void DataLogger::logGPS(File dataFile)
{

  int timeArr[4];
  int *gpsTime = getGPSTime(timeArr);
  for (int i = 3; i > -1; i--)
  {
    dataFile.print(gpsTime[i]);
    dataFile.print(",");
  }

  int dateArr[3];
  int *gpsDate = getGPSDate(dateArr);
  for (int i = 0; i < 3; i++)
  {
    dataFile.print(gpsDate[i]);
    dataFile.print(",");
  }

  dataFile.print((int)getGPSFix());
  dataFile.print(",");
  dataFile.print((int)getGPSFixquality_3d());
  dataFile.print(",");

  if (getGPSFix()) {
      dataFile.print(getLongitude(), 6);
      dataFile.print(",");
      dataFile.print(getLatitude(), 6);
      dataFile.print(",");
      dataFile.print(getAltitude());
      dataFile.print(",");
      dataFile.print(getSpeed());
      dataFile.print(",");
      dataFile.print(getAngle());
      dataFile.print(",");
    }
    else { // Spacer for missing data when there is no fix
      dataFile.print(",");
      dataFile.print(",");
      dataFile.print(",");
      dataFile.print(",");
      dataFile.print(",");
      dataFile.print(",");
      dataFile.print(",");
      dataFile.print(",");
    }
  
}

void DataLogger::logGPSSpacer(File dataFile)
{  
  for (int i = 0; i < 14; i++)
  {
    dataFile.print(",");
  }

}

/*

Create 25hz

bite to length converter to check if we're within the bandwidth

Basically want to eliminiate strings from the transmission package, and have a specific order that has a beginnign and and ned of the packet that you can read

Call Lora transmission every time the 5Hz transmission is done, and then call the Datalogger on the other one

*/