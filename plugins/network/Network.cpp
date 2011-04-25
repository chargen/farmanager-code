#include <initguid.h>
#include "Network.hpp"
#include "version.hpp"

//-----------------------------------------------------------------------------
#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}

//-----------------------------------------------------------------------------
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
  InitializeNetFunction();

  HANDLE hPlugin=new NetBrowser;
  if (hPlugin==NULL)
    return(INVALID_HANDLE_VALUE);
  NetBrowser *Browser=(NetBrowser *)hPlugin;

  if(OpenFrom==OPEN_COMMANDLINE)
  {
    TCHAR Path[MAX_PATH] = _T("\\\\");

    int I=0;
    TCHAR *cmd=(TCHAR *)Item;
    TCHAR *p=_tcschr(cmd, _T(':'));
    if (!p || !*p)
    {
      delete Browser;
      return INVALID_HANDLE_VALUE;
    }
    *p++ = _T('\0');
    bool netg;
    if (!lstrcmpi(cmd, _T("netg")))
      netg = true;
    else if (!lstrcmpi(cmd, _T("net")))
      netg = false;
    else
    {
      delete Browser;
      return INVALID_HANDLE_VALUE;
    }
    cmd = p;
    if(lstrlen(FSF.Trim(cmd)))
    {
      if (cmd [0] == _T('/'))
        cmd [0] = _T('\\');
      if (cmd [1] == _T('/'))
        cmd [1] = _T('\\');
      if (!netg && !Opt.NavigateToDomains)
      {
        if(cmd[0] == _T('\\') && cmd[1] != _T('\\'))
          I=1;
        else if(cmd[0] != _T('\\') && cmd[1] != _T('\\'))
          I=2;
      }
      lstrcpy(Path+I, cmd);

      FSF.Unquote(Path);
      // Expanding environment variables.
      {
          TCHAR PathCopy[MAX_PATH];
          lstrcpy(PathCopy, Path);
          ExpandEnvironmentStrings(PathCopy, Path, ARRAYSIZE(Path));
      }
      Browser->SetOpenFromCommandLine (Path);
    }
  }
  /* The line below is an UNDOCUMENTED and UNSUPPORTED EXPERIMENTAL
      mechanism supported ONLY in FAR 1.70 beta 3. It will NOT be supported
      in later versions. Please DON'T use it in your plugins. */
  else if (OpenFrom == 7)
  {
    if (!Browser->SetOpenFromFilePanel ((TCHAR *) Item))
    {
      // we don't support upwards browsing from NetWare shares -
      // it doesn't work correctly
      delete Browser;
      return INVALID_HANDLE_VALUE;
    }
  }
  else {
    if (IsFirstRun && Opt.LocalNetwork)
      Browser->GotoLocalNetwork();
  }
  IsFirstRun = FALSE;

  TCHAR szCurrDir[MAX_PATH];
  if (GetCurrentDirectory(ARRAYSIZE(szCurrDir), szCurrDir))
  {
    if (*szCurrDir == _T('\\') && GetSystemDirectory(szCurrDir, ARRAYSIZE(szCurrDir)))
    {
      szCurrDir[2] = _T('\0');
      SetCurrentDirectory(szCurrDir);
    }
  }
  return(hPlugin);
}

//-----------------------------------------------------------------------------
void WINAPI ClosePluginW(HANDLE hPlugin)
{
  delete (NetBrowser *)hPlugin;
}

//-----------------------------------------------------------------------------
int WINAPI GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->GetFindData(pPanelItem,pItemsNumber,OpMode));
}

//-----------------------------------------------------------------------------
void WINAPI FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  Browser->FreeFindData(PanelItem,ItemsNumber);
}

//-----------------------------------------------------------------------------
void WINAPI GetOpenPluginInfoW(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  Browser->GetOpenPluginInfo(Info);
}

//-----------------------------------------------------------------------------
int WINAPI SetDirectoryW(HANDLE hPlugin,const TCHAR *Dir,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->SetDirectory(Dir,OpMode));
}

//-----------------------------------------------------------------------------
int WINAPI DeleteFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                                 int ItemsNumber,int OpMode)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->DeleteFiles(PanelItem,ItemsNumber,OpMode));
}

//-----------------------------------------------------------------------------
int WINAPI ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return(Browser->ProcessKey(Key,ControlState));
}

//-----------------------------------------------------------------------------
int WINAPI ProcessEventW(HANDLE hPlugin,int Event,void *Param)
{
  NetBrowser *Browser=(NetBrowser *)hPlugin;
  return Browser->ProcessEvent (Event, Param);
}

//-----------------------------------------------------------------------------
