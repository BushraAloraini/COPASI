/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/utilities/CCopasiMessage.cpp,v $
   $Revision: 1.23 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/07/15 20:22:35 $
   End CVS Header */

// CCopasiMessage
//
// New Class based on pmutils read functionality
// (C) Stefan Hoops 2001

#include <string>
#include <iostream>

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#define  COPASI_TRACE_CONSTRUCTION

#include "copasi.h"
#include "CCopasiMessage.h"
#include "messages.h"
#include "CCopasiException.h"
#include "utility.h"

#define INITIALTEXTSIZE 1024

std::deque< CCopasiMessage > CCopasiMessage::mMessageDeque;

const CCopasiMessage & CCopasiMessage::peekFirstMessage()
{
  if (mMessageDeque.empty())
    CCopasiMessage(CCopasiMessage::RAW,
                   MCCopasiMessage + 1);

  return mMessageDeque.front();
}

const CCopasiMessage & CCopasiMessage::peekLastMessage()
{
  if (mMessageDeque.empty())
    CCopasiMessage(CCopasiMessage::RAW,
                   MCCopasiMessage + 1);

  return mMessageDeque.back();
}

CCopasiMessage CCopasiMessage::getFirstMessage()
{
  if (mMessageDeque.empty())
    CCopasiMessage(CCopasiMessage::RAW,
                   MCCopasiMessage + 1);

  CCopasiMessage Message(mMessageDeque.front());
  mMessageDeque.pop_front();

  return Message;
}

CCopasiMessage CCopasiMessage::getLastMessage()
{
  if (mMessageDeque.empty())
    CCopasiMessage(CCopasiMessage::RAW,
                   MCCopasiMessage + 1);

  CCopasiMessage Message(mMessageDeque.back());
  mMessageDeque.pop_back();

  return Message;
}

std::string CCopasiMessage::getAllMessageText(const bool & chronological)
{
  std::string Text = "";
  CCopasiMessage (*getMessage)() = chronological ? getFirstMessage : getLastMessage;

  while (!mMessageDeque.empty())
    {
      if (Text != "") Text += "\n";
      Text += getMessage().getText();
    }

  return Text;
}

void CCopasiMessage::clearDeque()
{
  mMessageDeque.clear();

  return;
}

CCopasiMessage::CCopasiMessage(void):
    mText(),
    mType(CCopasiMessage::RAW),
    mNumber(0)
{}

CCopasiMessage::CCopasiMessage(const CCopasiMessage & src):
    mText(src.mText),
    mType(src.mType),
    mNumber(src.mNumber)
{}

CCopasiMessage::CCopasiMessage(CCopasiMessage::Type type,
                               const char *format, ...)
{
  C_INT32 TextSize = INITIALTEXTSIZE;
  C_INT32 Printed = 0;

  va_list Arguments; // = NULL;
  va_start(Arguments, format);

  char *Text = NULL;
  Text = new char[TextSize + 1];

  Printed = vsnprintf(Text, TextSize + 1, format, Arguments);

  while (Printed < 0 || TextSize < Printed)
    {
      delete [] Text;

      (Printed < 0) ? TextSize *= 2 : TextSize = Printed;
      Text = new char[TextSize + 1];

      Printed = vsnprintf(Text, TextSize, format, Arguments);
    }

  mText = Text;
  mType = type;
  mNumber = 0;

  va_end(Arguments);

  handler();
}

CCopasiMessage::CCopasiMessage(CCopasiMessage::Type type,
                               unsigned C_INT32 number, ...)
{
  C_INT32 i = 0;

  C_INT32 TextSize = INITIALTEXTSIZE;
  C_INT32 Printed = 0;

  while (Messages[i].No != number && Messages[i].Text)
    i++;

  if (!Messages[i].Text)
    fatalError();

  va_list Arguments; // = NULL;
  va_start(Arguments, number);

  char *Text = NULL;
  Text = new char[TextSize + 1];

  Printed = vsnprintf(Text, TextSize, Messages[i].Text, Arguments);

  while (Printed < 0 || TextSize < Printed)
    {
      delete [] Text;

      (Printed < 0) ? TextSize *= 2 : TextSize = Printed;
      Text = new char[TextSize + 1];

      Printed = vsnprintf(Text, TextSize, Messages[i].Text, Arguments);
    }

  mText = Text;
  mType = type;
  mNumber = number;

  va_end(Arguments);

  handler();
}

void CCopasiMessage::handler(const bool & _throw)
{
  std::string Text = mText;

  switch (mType)
    {
    case RAW:
      mText = "";
      break;

    case TRACE:
      mText = ">TRACE ";
      mText += LocalTimeStamp();
      mText += "<\n";
      break;

    case WARNING:
      mText = ">WARNING ";
      mText += LocalTimeStamp();
      mText += "<\n";
      break;

    case ERROR:
      mText = ">ERROR ";
      mText += LocalTimeStamp();
      mText += "<\n";
      break;

    case EXCEPTION:
      mText = ">EXCEPTION ";
      mText += LocalTimeStamp();
      mText += "<\n";
      break;
    }

  mText += Text;

  if (mType != RAW) lineBreak();

  mMessageDeque.push_back(*this);

  if (mType == EXCEPTION)
    {
#ifdef COPASI_DEBUG
      DebugFile << mText << std::endl;
#endif // COPASI_DEBUG

      throw CCopasiException(*this);
    }
  else
    {
      std::cout << mText << std::endl;
    }
}

// overload assignment operator
CCopasiMessage &CCopasiMessage::operator=(const CCopasiMessage &RHS)
{
  mText = RHS.mText;
  mType = RHS.mType;
  mNumber = RHS.mNumber;

  return *this;
}
CCopasiMessage::~CCopasiMessage(void) {}
const std::string & CCopasiMessage::getText(void) const {return mText;}
const CCopasiMessage::Type & CCopasiMessage::getType(void) const {return mType;}
const unsigned C_INT32 & CCopasiMessage::getNumber(void) const {return mNumber;}

void CCopasiMessage::lineBreak()
{
  std::string Search("\n");
  std::string Replace("\n  ");
  std::string::size_type pos = 0;

  while (true)
    {
      pos = mText.find(Search, pos);

      if (pos == std::string::npos)
        break;

      mText.replace(pos, Search.length(), Replace);

      pos += Replace.length();
    }
}
