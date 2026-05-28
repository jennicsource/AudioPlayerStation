#define  FIELD_MAX  16

#define  FIELD_TYPE_TEXT   1


#define  FONT_2_X   15
#define  FONT_2_Y   25

// a struct that holds all parameters of a display field
typedef struct {
  int page;   // the display page the field shows up
  int x;     // coordinates
  int y;
  int w;     // width
  int h;     // height
  
  char text[32];  // a char array containing the text
  
  byte fieldtype;
  byte textsize;
 
} field_t;

field_t fields[FIELD_MAX];  // array of a struct, containing all fields data including the text the display is showing in that field

int FieldCount;
int CurrentPage;


// /helper functions to stack fields horizontally or vertically
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



// getting the text content of a field
String FieldGetText(int index)
{
  String ResultString(fields[index].text);
  return ResultString;
}

// set the text content of a field
void FieldSetText(int index, String Text)
{
  if (index > -1 && index < FIELD_MAX)
  {
    int slength = Text.length();  // cut to 30 chars max
    if (slength > 30) slength = 30;
    for (int i = 0; i < slength; i++)   // put the string in the char array of a field, containing the text
    {
      fields[index].text[i] = Text[i];
    }
    fields[index].text[slength] = 0;
  }

}


// set the parameters of a field. you must do that at the beginning for each of the fields you want to use
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



// draw the field on the display
void fielddraw(int index)
{
  // find the coordinates to let the text shine up in the center of the field
  int textw = FONT_2_X * FieldGetText(index).length();
  int texth = FONT_2_Y;
  int offsetx = (fields[index].w  - textw) / 2;
  int offsety = (fields[index].h  - texth) / 2;
  
  // this is display dependent. puts the actual text on the display
  oled.setTextSize( fields[index].textsize ); 
  oled.setTextColor(SH110X_WHITE);
  oled.setCursor( fields[index].x + offsetx, fields[index].y + offsety );
  oled.println( FieldGetText(index) );

}



// draw all the fields which are elements of the current display page
void PageDraw()
{
  for (int index = 0; index < FieldCount; index++)
  {
    if (fields[index].page == CurrentPage || fields[index].page == 255)
    {
      fielddraw(index);
    }
  }
  // this is display dependent. for this OLED the function must be called after all the text was printed in all the different display fields
  oled.display();     
}

// switch to another display page
void PageChange(int NewPage)
{
  CurrentPage = NewPage; 
  oled.clearDisplay();    // this is display dependent. here the OLED is cleared
  PageDraw();             // draw all the fields which are elements of the current display page
};



// not used in this application
void FieldDraw(int index)     // draws the field if it is part of the current display page
{
  if (index > -1 && index < FieldCount)
  {
    if (fields[index].page == CurrentPage || fields[index].page == 255)
    {
      fielddraw(index);
    }
  }
}

// not used in this application
// buffers text in the Field, before it can be drawn. when the text has changed in that field, the field is drawn once new, to prevent flickering
void FieldWrite(int index, String Text)
{
  if (index > -1 && index < FieldCount)
  {
    if (fields[index].page == CurrentPage || fields[index].page == 255)  // we investigate the field if it is part of the current display page
    {
      String LastText = FieldGetText(index);  // get the text which is buffered at the moment
      if (Text != LastText)                   // if we have new text to display, we draw the field once new
      {
        FieldSetText(index, Text);
        fielddraw(index);
      }
    }
  }
}






