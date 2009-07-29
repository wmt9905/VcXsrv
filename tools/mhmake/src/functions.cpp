/*  This file is part of mhmake.
 *
 *  Copyright (C) 2001-2009 Marc Haesen
 *
 *  Mhmake is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mhmake is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mhmake.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Rev$ */

#include "stdafx.h"

#include "util.h"
#include "mhmakefileparser.h"
#include "rule.h"

static const string s_QuoteString("\"");

funcdef mhmakefileparser::m_FunctionsDef[]= {
  {"filter",     &mhmakefileparser::f_filter}
 ,{"call",       &mhmakefileparser::f_call}
 ,{"subst",      &mhmakefileparser::f_subst}
 ,{"patsubst",   &mhmakefileparser::f_patsubst}
 ,{"concat",     &mhmakefileparser::f_concat}
 ,{"if",         &mhmakefileparser::f_if}
 ,{"findstring", &mhmakefileparser::f_findstring}
 ,{"firstword",  &mhmakefileparser::f_firstword}
 ,{"wildcard",   &mhmakefileparser::f_wildcard}
 ,{"basename",   &mhmakefileparser::f_basename}
 ,{"notdir",     &mhmakefileparser::f_notdir}
 ,{"dir",        &mhmakefileparser::f_dir}
 ,{"shell",      &mhmakefileparser::f_shell}
 ,{"relpath",    &mhmakefileparser::f_relpath}
 ,{"toupper",    &mhmakefileparser::f_toupper}
 ,{"tolower",    &mhmakefileparser::f_tolower}
 ,{"exist",      &mhmakefileparser::f_exist}
 ,{"filesindirs",&mhmakefileparser::f_filesindirs}
 ,{"fullname"   ,&mhmakefileparser::f_fullname}
 ,{"addprefix"  ,&mhmakefileparser::f_addprefix}
 ,{"addsuffix"  ,&mhmakefileparser::f_addsuffix}
 ,{"filter-out" ,&mhmakefileparser::f_filterout}
 ,{"word"       ,&mhmakefileparser::f_word}
 ,{"words"      ,&mhmakefileparser::f_words}
};

map<string,function_f> mhmakefileparser::m_Functions;

bool mhmakefileparser::m_FunctionsInitialised;

///////////////////////////////////////////////////////////////////////////////
void mhmakefileparser::InitFuncs(void)
{
  for (int i=0; i<sizeof(m_FunctionsDef)/sizeof(funcdef); i++)
    m_Functions[m_FunctionsDef[i].szFuncName]=m_FunctionsDef[i].pFunc;
}

///////////////////////////////////////////////////////////////////////////////
static string TrimString(const string &Input)
{
  unsigned Start=0;
  while (strchr(" \t",Input[Start])) Start++;
  if (Start>=Input.size())
    return g_EmptyString;
  unsigned Stop=Input.size()-1;
  while (strchr(" \t",Input[Stop])) Stop--;
  return Input.substr(Start,Stop-Start+1);
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_filter(const string & Arg) const
{
  int ipos=Arg.find(',');
  #ifdef _DEBUG
  if (ipos==string::npos) {
    cerr << "filter func should have 2 arguments: "<<Arg<<endl;
    throw 1;
  }
  #endif
  string Str=TrimString(Arg.substr(0,ipos));
  string List=Arg.substr(ipos+1);

  if (Str.empty())
    return Str;

  bool First=true;
  string Ret;
  char *pTok=strtok((char*)List.c_str()," \t");   // doing this is changing string, so this is very dangerous
  while (pTok)
  {
    string Item(pTok);
    if (PercentMatchList(Item,Str))
    {
      if (First)
      {
        Ret=Item;
        First=false;
      }
      else
      {
        Ret+=g_SpaceString;
        Ret+=Item;
      }
    }
    pTok=strtok(NULL," \t");
  }
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_filterout(const string & Arg) const
{
  int ipos=Arg.find(',');
  #ifdef _DEBUG
  if (ipos==string::npos) {
    cerr << "filter func should have 2 arguments: "<<Arg<<endl;
    throw 1;
  }
  #endif
  string Str=TrimString(Arg.substr(0,ipos));
  string List=Arg.substr(ipos+1);

  if (Str.empty())
    return Str;

  bool First=true;
  string Ret;
  char *pTok=strtok((char*)List.c_str()," \t");   // doing this is changing string, so this is very dangerous
  while (pTok)
  {
    string Item(pTok);
    if (!PercentMatchList(Item,Str))
    {
      if (First)
      {
        Ret=Item;
        First=false;
      }
      else
      {
        Ret+=g_SpaceString;
        Ret+=Item;
      }
    }
    pTok=strtok(NULL," \t");
  }
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_call(const string & Arg) const
{
  const char *pTmp=Arg.c_str();

  bool LastCharIsComma=Arg[Arg.length()-1]==',';

  string Func;
  pTmp=NextCharItem(pTmp,Func,',');
  map<string,string>::const_iterator pFunc=m_Variables.find(Func);
  #ifdef _DEBUG
  if (pFunc==m_Variables.end())
  {
    fprintf(stderr,"call to non existing function %s\n",Func.c_str());
    throw(1);
  }
  #endif
  Func=pFunc->second;
  int i=0;
  while (*pTmp || LastCharIsComma) {
    if (!*pTmp)
      LastCharIsComma=false; /* To stop the loop */
    string Repl;
    pTmp=NextCharItem(pTmp,Repl,',');
    i++;
    char Tmp[10];
    ::sprintf(Tmp,"$(%d)",i);
    int Len=::strlen(Tmp);
    int Pos=Func.find(Tmp);
    while (Func.npos!=Pos)
    {
      Func=Func.substr(0,Pos)+Repl+Func.substr(Pos+Len);
      Pos=Func.find(Tmp,Pos+Len);
    }
  }

  return ExpandExpression(Func);
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_subst(const string & Arg) const
{
  const char *pTmp=Arg.c_str();

  string FromString;
  pTmp=NextCharItem(pTmp,FromString,',');
  #ifdef _DEBUG
  if (!*pTmp) {
    cerr << "Wrong number of arguments in function subst" << endl;
    throw(1);
  }
  #endif

  string ToString;
  pTmp=NextCharItem(pTmp,ToString,',');
  string Text;
  NextCharItem(pTmp,Text,',');

  if (FromString.empty())
    return Text;

  string Ret;
  int Pos=Text.find(FromString);
  int PrevPos=0;
  while (Pos!=string::npos)
  {
    Ret+=Text.substr(PrevPos,Pos-PrevPos);
    Ret+=ToString;
    PrevPos=Pos+FromString.length();
    Pos=Text.find(FromString,PrevPos);
  }
  Ret+=Text.substr(PrevPos);

  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_patsubst(const string & Arg) const
{
  const char *pTmp=Arg.c_str();

  string FromString;
  pTmp=NextCharItem(pTmp,FromString,',');
  #ifdef _DEBUG
  if (!*pTmp) {
    cerr << "Wrong number of arguments in function subst" << endl;
    throw(1);
  }
  #endif

  string ToString;
  pTmp=NextCharItem(pTmp,ToString,',');
  string Text;
  NextCharItem(pTmp,Text,',');

  if (FromString.empty())
    return Text;

  return Substitute(Text,FromString,ToString);
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_concat(const string & Arg) const
{
  const char *pTmp=Arg.c_str();

  string JoinString;
  pTmp=NextCharItem(pTmp,JoinString,',');

  string List;
  pTmp=NextCharItem(pTmp,List,',');

  if (JoinString.empty() && List.empty())
  {
    /* assume as $(concat ,,items) construct */
    JoinString=",";
    pTmp=NextCharItem(pTmp,List,',');
  }

  bool First=true;
  string Ret;
  char *pTok=strtok((char*)List.c_str()," \t");   // doing this is changing string, so this is very dangerous
  while (pTok)
  {
    if (First)
    {
      First=false;
    }
    else
    {
      Ret+=JoinString;
    }
    Ret+=pTok;
    pTok=strtok(NULL," \t");
  }
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_if(const string & Arg) const
{
  const char *pTmp=Arg.c_str();

  string Cond;
  pTmp=NextCharItem(pTmp,Cond,',');
  string Ret;
  if (Cond.empty())
  {
    pTmp=NextCharItem(pTmp,Ret,',');
  }
  NextCharItem(pTmp,Ret,',');
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_findstring(const string & Arg) const
{
  const char *pTmp=Arg.c_str();
  string find;
  pTmp=NextCharItem(pTmp,find,',');
  string instr;
  NextCharItem(pTmp,instr,',');

  if (instr.find(find) != instr.npos)
    return find;
  else
    return g_EmptyString;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_firstword(const string & Arg) const
{
  string FirstWord;
  NextItem(Arg.c_str(),FirstWord);
  return FirstWord;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_wildcard(const string & Arg) const
{
  string FileSpec=TrimString(Arg);
  int LastSep=FileSpec.find_last_of(OSPATHSEP)+1;
  string Dir=FileSpec.substr(0,LastSep);
#ifdef WIN32
  struct _finddata_t FileInfo;
  long hFile=_findfirst(FileSpec.c_str(),&FileInfo);
  if (hFile==-1)
    return g_EmptyString;

  string Ret=g_EmptyString;

  /* We have to verify with percentmatch since the find functions *.ext also matches the functions *.xmlbrol */
  string CheckSpec=FileSpec.substr(LastSep);
  if (PercentMatch(FileInfo.name,CheckSpec,NULL,'*'))
  {
    Ret=Dir+FileInfo.name;
  }
  while (-1!=_findnext(hFile,&FileInfo))
  {
    if (PercentMatch(FileInfo.name,CheckSpec,NULL,'*'))
    {
      Ret+=g_SpaceString;
      Ret+=Dir;
      Ret+=FileInfo.name;
    }
  }
  _findclose(hFile);
#else
  glob_t Res;
  if (glob (FileSpec.c_str(), GLOB_ERR|GLOB_NOSORT|GLOB_MARK, NULL, &Res))
    return g_EmptyString;

  string Ret=g_EmptyString;
  string SepStr=g_EmptyString;
  for (int i=0; i<Res.gl_pathc; i++)
  {
    if (PercentMatch(Res.gl_pathv[i],FileSpec,NULL,'*'))
    {
      Ret+=SepStr;
      Ret+=Res.gl_pathv[i];
      SepStr=g_SpaceString;
    }
  }

  globfree(&Res);
#endif
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_exist(const string & Arg) const
{
  string File=TrimString(Arg);
  refptr<fileinfo> pFile=GetFileInfo(File);
  if (pFile->Exists())
  {
    return string("1");
  }
  else
    return g_EmptyString;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_filesindirs(const string & Arg) const
{
  const char *pTmp=Arg.c_str();

  string strFiles;
  pTmp=NextCharItem(pTmp,strFiles,',');
  #ifdef _DEBUG
  if (!*pTmp) {
    cerr << "Wrong number of arguments in function filesindirs" << endl;
    throw(1);
  }
  #endif
  string strDirs;
  NextCharItem(pTmp,strDirs,',');

  vector< refptr<fileinfo> > Dirs;
  SplitToItems(strDirs,Dirs);

  pTmp=strFiles.c_str();
  string Ret=g_EmptyString;
  bool first=true;
  while (*pTmp)
  {
    string File;
    refptr<fileinfo> pFile;
    pTmp=NextItem(pTmp,File);

    vector< refptr<fileinfo> >::iterator It=Dirs.begin();
    vector< refptr<fileinfo> >::iterator ItEnd=Dirs.end();
    while (It!=ItEnd)
    {
      pFile=GetFileInfo(File,*It++);
      if (pFile->Exists())
      {
        break;
      }
      pFile=NullFileInfo;
    }
    if (!pFile)
      continue;

    if (!first)
    {
      Ret+=g_SpaceString;
    }
    else
    {
      first=false;
    }
    Ret+=pFile->GetFullFileName();
  }

  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_fullname(const string & Arg) const
{
  string File=TrimString(Arg);
  refptr<fileinfo> pFile=GetFileInfo(File);
  return pFile->GetFullFileName();
}

///////////////////////////////////////////////////////////////////////////////
static string IterList(const string &List,string (*iterFunc)(const string &FileName,const string &Arg),const string &Arg=NullString)
{
  const char *pTmp=List.c_str();
  string Ret=g_EmptyString;
  bool first=true;
  while (*pTmp)
  {
    if (!first)
    {
      Ret+=g_SpaceString;
    }
    else
    {
      first=false;
    }
    string Item;
    pTmp=NextItem(pTmp,Item);
    Ret+=iterFunc(Item,Arg);
  }

  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
static string basename(const string &FileName,const string &)
{
  string Ret=FileName.substr(0,FileName.find_last_of('.'));
  if (FileName[0]=='"')
    Ret+=s_QuoteString;
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_basename(const string & FileNames) const
{
  return IterList(FileNames,basename);
}

///////////////////////////////////////////////////////////////////////////////
static string notdir(const string &FileName,const string &)
{
  int Pos=FileName.find_last_of(OSPATHSEP);
  if (Pos==string::npos)
  {
    return FileName;
  }
  else
  {
    string Ret=g_EmptyString;
    if (FileName[0]=='"')
      Ret+=s_QuoteString;
    Ret+=FileName.substr(Pos+1);
    return Ret;
  }

}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_notdir(const string & FileNames) const
{
  return IterList(FileNames,notdir);
}

///////////////////////////////////////////////////////////////////////////////
static string addprefix(const string &FileName,const string &Prefix)
{
  return Prefix+FileName;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_addprefix(const string & Arg) const
{
  const char *pTmp=Arg.c_str();
  string PreFix;
  pTmp=NextCharItem(pTmp,PreFix,',');
  #ifdef _DEBUG
  if (!*pTmp) {
    cerr << "Wrong number of arguments in function addprefix" << endl;
    throw(1);
  }
  #endif
  string FileNames;
  pTmp=NextCharItem(pTmp,FileNames,',');
  return IterList(FileNames,addprefix,PreFix);
}

///////////////////////////////////////////////////////////////////////////////
static string addsuffix(const string &FileName,const string &Suffix)
{
  return FileName+Suffix;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_addsuffix(const string & Arg) const
{
  const char *pTmp=Arg.c_str();
  string SufFix;
  pTmp=NextCharItem(pTmp,SufFix,',');
  #ifdef _DEBUG
  if (!*pTmp) {
    cerr << "Wrong number of arguments in function addsuffix" << endl;
    throw(1);
  }
  #endif
  string FileNames;
  pTmp=NextCharItem(pTmp,FileNames,',');
  return IterList(FileNames,addsuffix,SufFix);
}

///////////////////////////////////////////////////////////////////////////////
// Returns the n-th word number
string mhmakefileparser::f_word(const string & Arg) const
{
  const char *pTmp=Arg.c_str();

  string strNum;
  pTmp=NextCharItem(pTmp,strNum,',');

  int WordNbr=atoi(strNum.c_str());

  #ifdef _DEBUG
  if (!WordNbr)
  {
    if (!WordNbr) {
      cerr << "Expecting a number bigger then 0 for the word function"<<endl;
      throw(1);
    }
  }
  #endif

  int CurWord=0;
  while (*pTmp)
  {
    string Word;
    pTmp=NextItem(pTmp,Word);
    CurWord++;
    if (CurWord==WordNbr)
      return Word;
  }

  return g_EmptyString;
}

///////////////////////////////////////////////////////////////////////////////
// Returns the number of words
string mhmakefileparser::f_words(const string & Arg) const
{
  const char *pTmp=Arg.c_str();
  int NrWords=0;
  char szNumber[10];
  while (*pTmp)
  {
    string Word;
    pTmp=NextItem(pTmp,Word);
    NrWords++;
  }
  sprintf(szNumber,"%d",NrWords);

  return szNumber;
}

///////////////////////////////////////////////////////////////////////////////
static string dir(const string &FileName,const string &)
{
  int Pos=FileName.find_last_of(OSPATHSEP);
  if (Pos==string::npos)
  {
    return g_EmptyString;
  }
  else
  {
    string Ret=g_EmptyString;
    Ret+=FileName.substr(0,Pos+1);
    if (FileName[0]=='"')
      Ret+=s_QuoteString;
    return Ret;
  }

}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_dir(const string & FileNames) const
{
  return IterList(FileNames,dir);
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_shell(const string & Command) const
{
  string Output;

#ifdef _DEBUG
  if (g_PrintAdditionalInfo)
    cout << "shell: executing: Command '"<<Command<<"'"<<endl;
#endif

  ((mhmakefileparser*)this)->ExecuteCommand(string("@")+Command,&Output);  // Make sure that the command is not echoed
#ifdef _DEBUG
  if (g_PrintAdditionalInfo)
    cout << "shell returned '"<<Output<<"'"<<endl;
#endif
  return Output;
}

///////////////////////////////////////////////////////////////////////////////
static string relpath(const string &FileName,const string &)
{
  refptr<fileinfo> Path=GetFileInfo(FileName);
  const char *pCur=curdir::GetCurDir()->GetFullFileName().c_str();
  const char *pPath=Path->GetFullFileName().c_str();

  const char *pLast=pPath;
  while (*pCur==*pPath)
  {
    char Char=*pPath;
    if (!Char)
    {
      return "."; // Means that FileName is the same as the current directory
    }
    if (Char==OSPATHSEP)
      pLast=pPath+1;
    pCur++;
    pPath++;
  }
  if (*pPath==OSPATHSEP && !*pCur)
    pLast=pPath+1;
  string retPath;
  if (*pCur==OSPATHSEP) {
    bool first=true;
    pCur++;
    retPath="..";
    while (*pCur)
    {
      if (*pCur==OSPATHSEP)
        retPath+=OSPATHSEPSTR"..";
      pCur++;
    }
    if (!pPath)
      return retPath;
    else
      return retPath+OSPATHSEPSTR+pLast;
  }
  else
  {
    if (*pCur)
      retPath=".."OSPATHSEPSTR;
    while (*pCur)
    {
      if (*pCur==OSPATHSEP)
        retPath+=".."OSPATHSEPSTR;
      pCur++;
    }
    retPath+=pLast;
    return retPath;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Make a path name relative to the curren directory
string mhmakefileparser::f_relpath(const string & FileNames) const
{
  return IterList(FileNames,relpath);
}

///////////////////////////////////////////////////////////////////////////////
static string makeupper(const string &FileName,const string &)
{
  string Ret=FileName;
  string::const_iterator pSrc=FileName.begin();
  string::iterator pDest=Ret.begin();
  while (pSrc!=FileName.end())
  {
    *pDest++ = toupper(*pSrc++);
  }
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_toupper(const string & FileNames) const
{
  return IterList(FileNames,makeupper);
}

///////////////////////////////////////////////////////////////////////////////
static string makelower(const string &FileName,const string &)
{
  string Ret=FileName;
  string::const_iterator pSrc=FileName.begin();
  string::iterator pDest=Ret.begin();
  while (pSrc!=FileName.end())
  {
    *pDest++ = tolower(*pSrc++);
  }
  return Ret;
}

///////////////////////////////////////////////////////////////////////////////
string mhmakefileparser::f_tolower(const string & FileNames) const
{
  return IterList(FileNames,makelower);
}
