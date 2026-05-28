#include "SD.h"
#include "SPI.h"


SPIClass SPI2(FSPI);

// --- Arduino SD Card File Handling Library ---
// https://docs.arduino.cc/libraries/sd/

// Global File objects for directory and data file access
File dir;
File datafile;



// =========================================
//  SD CARD INITIALIZATION
// =========================================
// Initializes SD card and SPI interface.
// Returns  1  on success,
//         -1  if SD mount fails,
//         -2  if no card is detected.
int SDCardFiles_Init()
{
  int Result = 1;
  
  // Prepare SD chip select pin
  pinMode(SD_CS, OUTPUT);      
  digitalWrite(SD_CS, HIGH);
  
  // Start SPI bus with defined pins
  SPI2.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);
 

  // Try to mount SD card
  if(!SD.begin())
  {
    Serial.println("Card Mount Failed");
    Result = -1;
	return Result;
  }


  // Check card type
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    Result = -2;
    return Result;
  }
  
 
  return Result;
}




// Array of filenames and number of filenames found with a given extension
String FileName[32];
uint8_t NumberOfFilesFound;

// =========================================
//  FILE LISTING BY EXTENSION
// =========================================
// Scans root folder for files with the specified extension (e.g. ".txt").
// Populates FileName[] array with up to 32 matching file names.
// Returns the number of files found.


int SDCardFiles_InitFileNames(String FileExtension)
{
  int intLoop = 0;  // Counts found files
  int found = 0;

  dir = SD.open("/"); // Open root directory

  File file;

  while (found == 0)
  {
    file = dir.openNextFile(); // Get next file in directory
       
    if (file && intLoop < 32) 
    {      
      Serial.println(String(file.name())); // Print filename to serial
      
      if (String(file.name()).endsWith(FileExtension)) // Check extension
      {
        FileName[intLoop] = String(file.name()); // Store filename
        intLoop = intLoop + 1;
      } 
    } 
    else 
    {
      found = 1; // No more files found
    }  

    file.close(); // Always close after use
  }
  
  dir.close(); // Close root directory

  NumberOfFilesFound = intLoop;
  return intLoop; // Number of files found
}

// =========================================
//  RETRIEVE FILE NAME BY INDEX
// =========================================
// Returns the filename at the given (0-based) index in FileName[].
String SDCardFiles_GetFilenameByNumber(uint8_t intNumber)
{
  String Result = "";
  if (intNumber < NumberOfFilesFound) Result = FileName[intNumber];
  return Result;
}

// =========================================
//  OPEN FILE FOR READING
// =========================================
// Opens the given file for reading. Returns 1 if successful, 0 if not.
int SDCardFiles_OpenFile(String Filename)
{
  int success = 0;
  datafile = SD.open(Filename);
  if (datafile) success = 1;
  return success;
}

// =========================================
//  GET FILE SIZE
// =========================================
// Returns size (in bytes) of opened file.
int32_t SDCardFiles_GetVolumeOfFile()
{
  return datafile.size();
}

// =========================================
//  CLOSE FILE
// =========================================
void SDCardFiles_CloseFile()
{
  datafile.close();
}

// =========================================
//  READ DATA INTO BUFFER
// =========================================
// Reads NumberOfBytesToRead bytes from open file into Buffer.
void SDCardFiles_ReadInBuffer(uint8_t* Buffer, uint16_t NumberOfBytesToRead)
{
  datafile.read(Buffer, NumberOfBytesToRead);
}

// =========================================
//  CHECK IF MORE DATA AVAILABLE
// =========================================
// Returns true if there is more data to read from the open file.
boolean SDCardFiles_DataLeftToRead()
{
  return datafile.available();
}
