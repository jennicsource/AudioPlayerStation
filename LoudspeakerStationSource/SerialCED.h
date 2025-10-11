

byte NodeNumber = 0;
byte Mode = 0;

int MessageChannel = 0;
int MessageValue = 0;



int SerialCED_Init()
{
  int Success = 1;

  Mode = 0;

  
  return Success;
}



long LastOutputSerial = 0;
int PeriodOutputSerial = 200;

long LastSensorSerial = 0;
int PeriodSensorSerial = 300;


char myData[64];
String ReceivedString;

int i = 0;


String TrimToContent(String Text)
{
  String Result = Text;

  int pos = Result.lastIndexOf("#");

  Result = Result.substring(pos + 2); 

  pos = Result.indexOf("%");

  Result = Result.substring(0, pos);

  return Result;

}


byte SendValueDump = 1;

void SerialCED_Loop()
{
  
  if (SendValueDump == 1)
  {

    if ((millis() - LastOutputSerial) > PeriodOutputSerial)   // each second
    {
      LastOutputSerial = millis();

      Serial.println("##O" + Output_EjectVirtualDisplay() + "%%");
    }

    if ((millis() - LastSensorSerial) > PeriodSensorSerial)   // each second
    {
      LastSensorSerial = millis();

      Serial.println("##M" + Messenger_EjectValueDump() + "%%");
      delay(5);
    }
  
  }

  

  if (Serial.available() > 0) {
    // Lies das eingehende Byte:
    byte x = Serial.read();

    if(x != '%')  //check if % character has arrived; if so, stop receiving further characters
    {
        myData[i] = x;   //received character is not a % character; save it
        i++;             //adjust array index
    }
    else
    {
        myData[i] = '\0';  //insert null character at the end of the received string
        ReceivedString = myData;  
        
        if (ReceivedString.indexOf("#I") > 0)
        { 
          Input_InjectButtonPress(ReceivedString);
        }

        if (ReceivedString.indexOf("#T") > 0)
        {         
          Messenger_InjectTransmitValue(TrimToContent(ReceivedString));
        }

        if (ReceivedString.indexOf("#R") > 0)
        {         
          Messenger_InjectReceiveValue(TrimToContent(ReceivedString));
        }

        i = 0; 
    }

  }



  /*
  if (Serial.available() > 0) {
    // Lies das eingehende Byte:
    byte incomingByte = Serial.read();

    int  NewMode = Mode;

    if (incomingByte == 45)  //  -
    {
      NewMode = 0;
      
      Serial.println("Command ?");
    }
    
    if (incomingByte == 42)  //  *
    {
      NewMode = 1;
      Serial.println("Nodenumber (a...z) ?");
    }
    
    if (Mode == 1 && incomingByte > 96 && incomingByte < 123)  //  a ... z
    {
      NodeNumber = incomingByte - 97;
      SerialCED_DumpElem();
    }

    if (incomingByte == 77)  //  M
    {
      NewMode = 4;
      Serial.println("Message Channel ?");
    }

    if (Mode == 4 && incomingByte > 47 && incomingByte < 58)  //  0 ... 9
    {
      MessageChannel = incomingByte - 48;
      NewMode = 5;
      Serial.println("Message Value ?");
    }

    if (Mode == 5 && incomingByte > 47 && incomingByte < 58)  //  0 ... 9
    {
      MessageValue = (incomingByte - 48) * 10;
      Messenger_SetValue(MessageChannel, MessageValue);
      Messenger_SendValues();

      Serial.println("Message Sent.");
    }
   
    if (incomingByte == 79)  //  O
    {
      //NewMode = 4;
      Serial.println(Display0.GetVirtualDisplay());
    }


    Mode = NewMode;
  }
  */

}




