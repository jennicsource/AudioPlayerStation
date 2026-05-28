#define  FIELD_MAX  16

#define  FIELD_TYPE_TEXT   1


#define  FONT_2_X   15
#define  FONT_2_Y   25


typedef struct {
  int page;
  int x;
  int y;
  int w;
  int h;
  
  char text[32];
  
  byte fieldtype;
  byte textsize;
 
} field_t;

field_t fields[FIELD_MAX];

int FieldCount;
int CurrentPage;



int Down_Margin = 0;
int Down_RootControl = -1;
int Down_y = 0;


int Right_Margin = 0;
int Right_RootControl = -1;
int Right_x = 0;





void MarginSet(int RightMargin, int DownMargin)
{
  Down_Margin = DownMargin;
  Right_Margin = RightMargin;
}


int RowAs(int index)
{
  return fields[index].y;
}

int ColAs(int index)
{
  return fields[index].x;
}



int Down(int index)
{
  if (index != Down_RootControl)
  {
      Down_y = fields[index].y + fields[index].h + Down_Margin;
      Down_RootControl = index;
  }
  else
  {
      Down_y = Down_y + fields[index].h + Down_Margin;
  }
  return Down_y;
}


int Right(int index)
{
  if (index != Right_RootControl)
  {
      Right_x = fields[index].x + fields[index].w + Right_Margin;
      Right_RootControl = index;
  }
  else
  {
      Right_x = Right_x + fields[index].w + Right_Margin;
  }
  return Right_x;
}



int ChildX(int index, int count, int child)
{
  int x;
  if (count > 0)
  {
    x = fields[index].x + fields[index].w * (child - 1) / count;
  }
  else
  {
    x = fields[index].x;
  }
  return x;
}


int TouchLastX = -1;
int TouchLastY = -1;

String FieldGetText(int index)
{
  String ResultString(fields[index].text);
  return ResultString;
}


void FieldSetText(int index, String Text)
{
  if (index > -1 && index < FIELD_MAX)
  {
    int slength = Text.length();
    if (slength > 30) slength = 30;
    for (int i = 0; i < slength; i++)
    {
      fields[index].text[i] = Text[i];
    }
    fields[index].text[slength] = 0;
  }

}



void FieldSet(int index, int page, int x, int y, int w, int h, String text, int fieldtype, int textsize)
{
  if (index > -1 && index < FIELD_MAX)
  {
    fields[index].page = page;
    fields[index].x = x;
    fields[index].y = y;
    fields[index].w = w;
    fields[index].h = h;

    fields[index].fieldtype = fieldtype;
    fields[index].textsize = textsize;
    
    FieldSetText(index, text);
    
    if (index > (FieldCount - 1))
    {
      FieldCount = index + 1;
    }
  }
}




void fielddraw(int index)
{
  
  int textw = FONT_2_X * FieldGetText(index).length();
  int texth = FONT_2_Y;
  int offsetx = (fields[index].w  - textw) / 2;
  int offsety = (fields[index].h  - texth) / 2;
  
  oled.setTextSize( fields[index].textsize ); 
  oled.setTextColor(SH110X_WHITE);
  oled.setCursor( fields[index].x + offsetx, fields[index].y + offsety );
  oled.println( FieldGetText(index) );

}




void PageDraw()
{
  for (int index = 0; index < FieldCount; index++)
  {
    if (fields[index].page == CurrentPage || fields[index].page == 255)
    {
      fielddraw(index);
    }
  }

  oled.display();     
}


void PageChange(int NewPage)
{
  CurrentPage = NewPage; 
  oled.clearDisplay();
  PageDraw();
};


void FieldDraw(int index)
{
  if (index > -1 && index < FieldCount)
  {
    if (fields[index].page == CurrentPage || fields[index].page == 255)
    {
      fielddraw(index);
    }
  }
}



void FieldWrite(int index, String Text)
{
  if (index > -1 && index < FieldCount)
  {
    
    if (fields[index].page == CurrentPage || fields[index].page == 255)
    {
      String LastText = FieldGetText(index);
      if (Text != LastText)
      {
        FieldSetText(index, Text);
        fielddraw(index);
      }
    }
  }
}






