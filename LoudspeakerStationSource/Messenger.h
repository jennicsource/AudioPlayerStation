
#include "Protocol.h"


int MessengerStationNumber = 0;

int Messenger_Values_Max = 16;


int Messenger_Value[16];

String Messenger_Path[16];
byte Messenger_Direction[16];

bool Messenger_Value_Received[16];
bool Messenger_Value_ToBeSentOut[16];
bool Messenger_Value_Injection[16];


void Messenger_SetValue(int Channel, int Value)
{
  if (Messenger_Value_Injection[Channel] == false)
  {
    Messenger_Value[Channel]       = Value;
    Messenger_Value_ToBeSentOut[Channel] = true;
  }
}


int Messenger_GetValue(int Channel)
{
  Messenger_Value_Received[Channel] = false;
  return Messenger_Value[Channel];
}



int Messenger_Init()
{
  int Messenger_Connected = Protocol_Init();

  for (int m = 0; m < Messenger_Values_Max; m++)
  {
    Messenger_Direction[m] = 0;
  }
 
  return Messenger_Connected;
}


int Messenger_GetChannelOfReceivedValue()
{
  int Result = -1;
  for (int m = 0; m < Messenger_Values_Max; m++)
  {
    if (Messenger_Value_Received[m] == true) 
    {
      Result = m;
      break;
    }
    
  }
  return Result;

}




void Messenger_ValueReceived(int MChannel, int Value)
{
  if (Messenger_Value_Injection[MChannel] == false)
  {
    Messenger_Value[MChannel]       = Value;
    Messenger_Value_Received[MChannel] = true;
  }

  //Serial.println("RECEIVED M:" +  String(MChannel) + ":" + String(Value));
}


void Messenger_SubscribeValue(String Subject, int MessageHandle)
{
  Protocol_SubscribeValue("AVS/Station/" + String(MessengerStationNumber) + Subject, MessageHandle);
} 


int Messenger_Start()
{
  
  for (int m = 0; m < Messenger_Values_Max; m++)
  {
    if (Messenger_Direction[m] == 2)
    {
      Messenger_SubscribeValue(Messenger_Path[m], m);
    }
  }

  for (int m = 0; m < Messenger_Values_Max; m++)
  {
    Messenger_Value_Received[m] =    false;
    Messenger_Value_ToBeSentOut[m] = false;
    Messenger_Value_Injection[m] = false;
  }


  Protocol_Start();

  return 0;
}


void Messenger_SetStationNumber(int Number)
{
  MessengerStationNumber = Number;
}


void Messenger_SetPathAndDirection(int Channel, String Path, byte Direction)
{
  Messenger_Path[Channel] = Path;
  Messenger_Direction[Channel] = Direction;
}




void Messenger_TransmitValue(int Channel)
{
  
  Protocol_SendValue("AVS/Station/" + String(MessengerStationNumber) + Messenger_Path[Channel], Messenger_Value[Channel] );

  Serial.println("AVS/Station/" + String(MessengerStationNumber) + Messenger_Path[Channel]);
}


void Messenger_SendValues()
{
  
  for (int m = 0; m < Messenger_Values_Max; m++ )
  {
    
    if (Messenger_Value_ToBeSentOut[m] == true)
    {
      Messenger_TransmitValue(m);
      
      Messenger_Value_ToBeSentOut[m] = false;
      
      delay(10);
    }
  }
 
}


void Messenger_Loop()
{
  Protocol_Loop();
}



// Eject Element Values
String Messenger_EjectValueDump()
{
  String Result = "";

  for (int m = 0; m < Messenger_Values_Max; m++)
  {
    if (Messenger_Value_Injection[m] == true) 
    {
      Result = Result + "*";
    }
    else
    {
      Result = Result + "-";
    }
    
    
    Result = Result + "M" + String(m) + " " + Messenger_Path[m] + " : " + Messenger_Value[m] + "\r\n";
  }
   
  return Result;
}

// ##T4:100%%

void Messenger_InjectReceiveValue(String InjectValueCommand)
{
  int pos = InjectValueCommand.indexOf(":");
  
  if (pos > 0)
  { 
    String Left = InjectValueCommand.substring(0, pos);
    String Right = InjectValueCommand.substring(pos + 1);

    //Serial.println(Left);
    //Serial.println(Right);

    int Channel = Left.toInt();

    if (Channel > -1 && Channel < Messenger_Values_Max)
    {
      if (Right.indexOf("*") > -1)
      {
        Messenger_Value_Injection[Channel] = false;
      }
      else
      {  
        int Value = Right.toInt();
            
        Messenger_Value[Channel] = Value;
        Messenger_Value_Injection[Channel] = true;
        Messenger_Value_Received[Channel] = true;

        
      }
    }

  }
  
}

void Messenger_InjectTransmitValue(String InjectValueCommand)
{
  int pos = InjectValueCommand.indexOf(":");
  
  if (pos > 0)
  { 
    String Left = InjectValueCommand.substring(0, pos);
    String Right = InjectValueCommand.substring(pos + 1);

    //Serial.println(Left);
    //Serial.println(Right);

    int Channel = Left.toInt();

    if (Channel > -1 && Channel < Messenger_Values_Max)
    {
      if (Right.indexOf("*") > -1)
      {
        Messenger_Value_Injection[Channel] = false;
      }
      else
      {  
        int Value = Right.toInt();
            
        Messenger_Value[Channel] = Value;
        Messenger_Value_Injection[Channel] = true;
        Messenger_Value_ToBeSentOut[Channel] = true;

        Messenger_SendValues();
      }
    }

  }
  
}


