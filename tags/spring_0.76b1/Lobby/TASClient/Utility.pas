{
  Usage: Call InitLib when program is started and DeInitLib when program gets closed. All other functions may be used only
  after library has been initialized.
}

unit Utility;

interface

uses
  Windows, Graphics, Classes, SyncObjs, Controls,Math;

type

  Float = Single;

  TStartPos =
  record
    x: Integer;
    y: Integer;
  end;

  { minwind and maxwind are defined as ints in unitsync.h, but some maps like "Core_Prime_Industrial_Area_01" use
    float values in their .smd files (e.g. "MinWind=0.2"). I use unitsync.dll now so that is not a problem anymore. }
  PMapInfo = ^TMapInfo;
  TMapInfo =
  packed record
    Description: string; // cast it as PChar
    TidalStrength: Integer;
    Gravity: Integer;
    MaxMetal: Float;
    ExtractorRadius: Integer;
    MinWind: Integer;
    MaxWind: Integer;

    // 0.61b1+
    Width: Integer;
    Height: Integer;
    PosCount: Integer;
    Positions: array[0..15] of TStartPos;
  end;

  PMinimapData = ^TMinimapData;
  TMinimapData = array[0..1024*1024-1] of Word; // raw bitmap data returned by GetMinimap function from UnitSync.dll

function InitLib: Boolean;
function ReInitLib: Boolean;
function ReInitLibWithDialog: Boolean; // uses InitWaitForm to (re)initialize library
procedure DeInitLib;
function AcquireSpringVersion: string;

procedure ReloadMapList(UpdateSplashScreen: Boolean); // will populate MapList and MapChecksums lists. If "UpdateSplashScreen" is True, it will update splash screen text too - use it only when loading map list at the beginning of the execution when splash screen form is still active!
function LoadMiniMap(MapName: string; bmp: TBitmap): Boolean;
function AcquireMapInfo(MapName: string): TMapInfo;
procedure ReloadModList;
function GetModHash(ModName: string): Integer;
procedure LoadMod(ModName: string);
procedure ReloadUnitList; // will populate UnitNames and UnitList lists
procedure ReloadSides; // populates SideList list and loads side images into SideImageList
procedure LoadLuaOptions(modOptions: Boolean; mapName: String);
procedure UnLoadLuaOptions(luaOptionList: TList);
function GetLuaAIList:TStrings;
function GetModValidMapList: TStringList;

var
  ModList: TStrings; // names of mods
  ModArchiveList: TStrings; // names of files in which mods are

  UnitNames: TStrings; // list of unit names
  UnitList: TStrings; // list of "code names" of units

  SideList: TStringList; // list of mod's sides, like "Arm", "Core", etc.
  SideImageList: TImageList; // images of sides. Also see SideList

  MapList: TStringList;
  MapChecksums: TStringList;

  LastInitSuccess: Boolean = True; // check this var to see if last call to InitLib/ReInitLib was successful!

implementation

uses
  SysUtils, StrUtils, Dialogs, Forms, Misc, MainUnit, InitWaitFormUnit,
  MapListFormUnit, SplashScreenUnit, BattleFormUnit, PreferencesFormUnit;


{ I assume bools are 1 byte in size (not 4) in unitsync.dll (from MSDN: "In Visual C++ 4.2,
  the Standard C++ header files contained a typedef that equated bool with int.
  In Visual C++ 5.0 and later, bool is implemented as a built-in type with a size
  of 1 byte. That means that for Visual C++ 4.2, a call of sizeof(bool) yields 4,
  while in Visual C++ 5.0 and later, the same call yields 1.") }

function Init(isServer: Boolean; id: Integer): Integer; stdcall; external 'UnitSync.dll' name 'Init';
procedure UnInit; stdcall; external 'UnitSync.dll' name 'UnInit';
{since 0.75} function GetSpringVersion: PChar; stdcall; external 'UnitSync.dll' name 'GetSpringVersion';
function ProcessUnits: Integer; stdcall; external 'UnitSync.dll' name 'ProcessUnits';
{since 0.61}function ProcessUnitsNoChecksum: Integer; stdcall; external 'UnitSync.dll' name 'ProcessUnitsNoChecksum';
function GetCurrentList: PChar; stdcall; external 'UnitSync.dll' name 'GetCurrentList';
procedure AddClient(id: Integer; unitList: PChar); stdcall; external 'UnitSync.dll' name 'AddClient';
procedure RemoveClient(id: Integer); stdcall; external 'UnitSync.dll' name 'RemoveClient';
function GetClientDiff(id: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetClientDiff';
procedure InstallClientDiff(diff: PChar); stdcall; external 'UnitSync.dll' name 'InstallClientDiff';
function GetUnitCount: Integer; stdcall; external 'UnitSync.dll' name 'GetUnitCount';
function GetUnitName(unit_: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetUnitName';
function GetFullUnitName(unit_: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetFullUnitName';
function IsUnitDisabled(unit_: Integer): Integer; stdcall; external 'UnitSync.dll' name 'IsUnitDisabled';
function IsUnitDisabledByClient(unit_: Integer; clientId: Integer): Integer; stdcall; external 'UnitSync.dll' name 'IsUnitDisabledByClient';
{------------------------------------------------------------------------------}
// {removed since 0.75} function InitArchiveScanner: Integer; stdcall; external 'UnitSync.dll' name 'InitArchiveScanner';
procedure AddArchive(name: PChar); stdcall; external 'UnitSync.dll' name 'AddArchive';
procedure AddAllArchives(root: PChar); stdcall; external 'UnitSync.dll' name 'AddAllArchives';
{since 0.61}function GetArchiveChecksum(arname: PChar): LongWord; stdcall; external 'UnitSync.dll' name 'GetArchiveChecksum';
{since 0.72}function GetPrimaryModChecksum(index: Integer): LongWord; stdcall; external 'UnitSync.dll' name 'GetPrimaryModChecksum';
function GetMapCount: Integer; stdcall; external 'UnitSync.dll' name 'GetMapCount';
function GetMapName(index: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetMapName';
function GetMapInfo(name: PChar; outInfo: PMapInfo): Integer; stdcall; external 'UnitSync.dll' name 'GetMapInfo';
// GetMinimap returns void*, I cast it as PMinimapData
function GetMinimap(filename: PChar; miplevel: Integer): PMinimapData; stdcall; external 'UnitSync.dll' name 'GetMinimap';
{------------------------------------------------------------------------------}
function GetPrimaryModCount: Integer; stdcall; external 'UnitSync.dll' name 'GetPrimaryModCount';
function GetPrimaryModName(index: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetPrimaryModName';
function GetPrimaryModArchive(index: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetPrimaryModArchive';
{since 0.61}function GetPrimaryModArchiveCount(index: Integer): Integer; stdcall; external 'UnitSync.dll' name 'GetPrimaryModArchiveCount';
{since 0.61}function GetPrimaryModArchiveList(arnr: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetPrimaryModArchiveList';
function GetPrimaryModIndex(name: PChar): Integer; stdcall; external 'UnitSync.dll' name 'GetPrimaryModIndex';
{since 0.63} function GetSideCount: Integer; stdcall; external 'UnitSync.dll' name 'GetSideCount';
{since 0.63} function GetSideName(side: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetSideName';
{since 0.71} function GetMapArchiveCount(mapName: PChar): Integer; stdcall; external 'UnitSync.dll' name 'GetMapArchiveCount';
{since 0.71} function GetMapArchiveName(index: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetMapArchiveName';
{------------------------------------------------------------------------------}
{since 0.68}function FindFilesArchive(archive, cur: Integer; nameBuf: PChar; var size: Integer): Integer; stdcall; external 'UnitSync.dll' name 'FindFilesArchive';
{since 0.68}function OpenArchive(name: PChar): Integer; stdcall; external 'UnitSync.dll' name 'OpenArchive';
{since 0.68}procedure CloseArchive(archive: Integer); stdcall; external 'UnitSync.dll' name 'CloseArchive';
{since 0.68}function OpenArchiveFile(archive: Integer; name: PChar): Integer; stdcall; external 'UnitSync.dll' name 'OpenArchiveFile';
{since 0.68}function ReadArchiveFile(archive: Integer; handle: Integer; buffer: Pointer; numBytes: Integer): Integer; stdcall; external 'UnitSync.dll' name 'ReadArchiveFile';
{since 0.68}procedure CloseArchiveFile(archive: Integer; handle: Integer); stdcall; external 'UnitSync.dll' name 'CloseArchiveFile';
{since 0.68}function SizeArchiveFile(archive: Integer; handle: Integer): Integer; stdcall; external 'UnitSync.dll' name 'SizeArchiveFile';
{------------------------------------------------------------------------------}
{since 0.68}function OpenFileVFS(const name: PChar): Integer; stdcall; external 'UnitSync.dll' name 'OpenFileVFS';
{since 0.68}procedure CloseFileVFS(handle: Integer); stdcall; external 'UnitSync.dll' name 'CloseFileVFS';
{since 0.68}procedure ReadFileVFS(handle: Integer; buf: Pointer; length: Integer); stdcall; external 'UnitSync.dll' name 'ReadFileVFS';
{since 0.68}function FileSizeVFS(handle: Integer): Integer; stdcall; external 'UnitSync.dll' name 'FileSizeVFS';
{since 0.68}function InitFindVFS(const pattern: PChar): Integer; stdcall; external 'UnitSync.dll' name 'InitFindVFS';
{since 0.68}function FindFilesVFS(handle: Integer; nameBuf: PChar; size: Integer): Integer; stdcall; external 'UnitSync.dll' name 'FindFilesVFS';
{------------------------------------------------------------------------------}
{since 0.76}function GetLuaAICount: Integer; stdcall; external 'UnitSync.dll' name 'GetLuaAICount';
{since 0.76}function GetLuaAIName(aiIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetLuaAIName';
{since 0.76}function GetLuaAIDesc(aiIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetLuaAIDesc';
{------------------------------------------------------------------------------}
{since 0.76}function GetMapOptionCount(mapName: PChar): Integer; stdcall; external 'UnitSync.dll' name 'GetMapOptionCount';
{since 0.76}function GetModOptionCount: Integer; stdcall; external 'UnitSync.dll' name 'GetModOptionCount';
{since 0.76}function GetOptionKey(optIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetOptionKey';
{since 0.76}function GetOptionName(optIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetOptionName';
{since 0.76}function GetOptionDesc(optIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetOptionDesc';
{since 0.76}function GetOptionType(optIndex: Integer): Integer; stdcall; external 'UnitSync.dll' name 'GetOptionType';
{since 0.76}function GetOptionBoolDef(optIndex: Integer): Boolean; stdcall; external 'UnitSync.dll' name 'GetOptionBoolDef';
{since 0.76}function GetOptionNumberDef(optIndex: Integer): Float; stdcall; external 'UnitSync.dll' name 'GetOptionNumberDef';
{since 0.76}function GetOptionNumberMin(optIndex: Integer): Float; stdcall; external 'UnitSync.dll' name 'GetOptionNumberMin';
{since 0.76}function GetOptionNumberMax(optIndex: Integer): Float; stdcall; external 'UnitSync.dll' name 'GetOptionNumberMax';
{since 0.76}function GetOptionNumberStep(optIndex: Integer): Float; stdcall; external 'UnitSync.dll' name 'GetOptionNumberStep';
{since 0.76}function GetOptionStringDef(optIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetOptionStringDef';
{since 0.76}function GetOptionStringMaxLen(optIndex: Integer): Integer; stdcall; external 'UnitSync.dll' name 'GetOptionStringMaxLen';
{since 0.76}function GetOptionListCount(optIndex: Integer): Integer; stdcall; external 'UnitSync.dll' name 'GetOptionListCount';
{since 0.76}function GetOptionListDef(optIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetOptionListDef';
{since 0.76}function GetOptionListItemName(optIndex: Integer; itemIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetOptionListItemName';
{since 0.76}function GetOptionListItemDesc(optIndex: Integer; itemIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetOptionListItemDesc';
{since 0.76}function GetOptionListItemKey(optIndex: Integer; itemIndex: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetOptionListItemKey';
{since 0.76}function GetModValidMapCount: integer; stdcall; external 'UnitSync.dll' name 'GetModValidMapCount';
{since 0.76}function GetModValidMap(index: Integer): PChar; stdcall; external 'UnitSync.dll' name 'GetModValidMap';


// some internal routines:
function RED_RGB565(c: Word): Word; forward;
function GREEN_RGB565(c: Word): Word; forward;
function BLUE_RGB565(c: Word): Word; forward;
function PACKRGB(r, g, b: Word): Word; forward;


{ ------------------------------------------------------------------------ }

function RED_RGB565(c: Word): Word;
begin
  Result := (c and $F800) shr 11;
end;

function GREEN_RGB565(c: Word): Word;
begin
  Result := (c and $07E0) shr 5;
end;

function BLUE_RGB565(c: Word): Word;
begin
  Result := c and $001F;
end;

function PACKRGB(r, g, b: Word): Word;
begin
  Result :=
  ((r shl 11) and $F800) or
  ((g shl 5) and $07E0) or
  (b and $001F);
end;

{ ------------------------------------------------------------------------ }

function InitLib: Boolean;
begin
  Result := Init(False, 0) = 1;
  LastInitSuccess := Result;
  if not Result then Exit;
end;

function ReInitLib: Boolean;
begin
  UnInit;
  Result := InitLib;
  if not Result then Exit;
  // reload mod list:
  ReloadModList;
end;

function ReInitLibWithDialog: Boolean;
var
  res: Integer;
begin
  InitWaitForm.ChangeCaption(MSG_REINITLIB);
  InitWaitForm.TakeAction := 3; // (re)initialize unitsync library
  InitWaitForm.ShowModal;
  if not LastInitSuccess then
  begin
    MessageDlg('Error initializing unit syncer!', mtError, [mbOK], 0);
    Application.Terminate;
  end;
end;

procedure DeInitLib;
begin
  UnInit;
end;

function AcquireSpringVersion: string;
begin
  Result := GetSpringVersion;
end;

{ ------------------------------------------------------------------------ }

// note: checksums are stored as hexadecimal numbers in text form
procedure ReloadMapList(UpdateSplashScreen: Boolean); // will populate MapList and MapChecksums lists
var
  i, j, c: Integer;
  MapCount: Integer;
  CheckSum : Integer;
  MapName: string;
  tmp: string;
  time: Cardinal;
begin
  if UpdateSplashScreen and (SplashScreenForm = nil) then UpdateSplashScreen := False; // just in case if caller forgot to set it to false

  MapList.Clear;
  MapCheckSums.Clear;
  MapListForm.ClearMapList;

  time := GetTickCount;
  MapCount := GetMapCount;

  if MapCount = 0 then
  begin
    MessageDlg('No maps found!', mtError, [mbOK], 0);
    //Application.Terminate;
    Exit;
  end;

  for i := 0 to MapCount-1 do
  begin
     MapName := GetMapName(i);

     // filter out maps with duplicate names:
     if MapList.IndexOf(MapName) <> -1 then Continue;

     // calculate checksum:
     CheckSum := 0;
     c := GetMapArchiveCount(PChar(MapName));
     for j := 0 to c-1 do
       Inc(CheckSum, GetArchiveCheckSum(GetMapArchiveName(j)));

     // check if map is "valid":
     if CheckSum = 0 then
     begin
       MainForm.AddMainLog('Skipping map file: ' + MapName + ' (bad checksum)', Colors.Error);
       Continue;
     end;
     if MapCheckSums.IndexOf(IntToHex(CheckSum, 8)) <> -1 then
     begin
       MainForm.AddMainLog('Skipping map file: ' + MapName + ' (duplicate checksum)', Colors.Error);
       Continue;
     end;

     // add map to the map list:
     if UpdateSplashScreen then SplashScreenForm.UpdateText('loading map (' + MapName + ')');
     MapList.Add(MapName);
     MapCheckSums.Add(IntToHex(CheckSum, 8));
     MapListForm.AddMap;
     TMapItem(MapListForm.Maps[MapListForm.Maps.Count-1]).LoadMapInfo;
     TMapItem(MapListForm.Maps[MapListForm.Maps.Count-1]).LoadMinimap(True);
//*** debug     MainForm.AddMainLog(MapName + ' ... ' + IntToStr(c) + ':' + MapCheckSums[MapCheckSums.Count-1], Colors.Info);
  end;
  MapListForm.SortedMaps.Assign(MapListForm.Maps);
  MapListForm.SortStylePopupMenu.Items[Preferences.MapSortStyle].OnClick(MapListForm.SortStylePopupMenu.Items[Preferences.MapSortStyle]); // this will also sort map list
  BattleForm.ChangeMapToNoMap(''); // we have to set CurrentMapIndex to -1 or else ChangeMap method may not change map later on if old map index collides with new one!
  BattleForm.PopulatePopupMenuMapList;
//***  showmessage('time taken: ' + inttostr(gettickcount - time) + ' ms');
end;

{ ------------------------------------------------------------------------ }

function LoadMiniMap(MapName: string; bmp: TBitmap): Boolean;
var
  mipsize: Integer;
  minimap: PMinimapData;
  x, y: Integer;
  P: PByteArray;
  color: Word;
begin
  Result := False;

  mipsize := 1024;

  minimap := GetMinimap(PChar(MapName), 0);
  if minimap = nil then
  begin
    MessageDlg('Loading '+MapName+' minimap failed !',mtError,[mbOk],0);
    Exit;
  end;

  bmp.PixelFormat := pf16bit;
  bmp.Width := 1024;
  bmp.Height := 1024;

  for y := 0 to bmp.Height -1 do
  begin
    P := bmp.ScanLine[y];

    for x := 0 to bmp.Width -1 do
    begin
      if  ((x+1) mod 4 = 0) and (x <> bmp.Width -1) then // vertical deinterlace
      begin
        color := Misc.RGB16BitsToColor((Misc.Color16BitsToR(minimap[y * mipsize + x-1])+Misc.Color16BitsToR(minimap[y * mipsize + x+1])) div 2,(Misc.Color16BitsToG(minimap[y * mipsize + x-1])+Misc.Color16BitsToG(minimap[y * mipsize + x+1])) div 2,(Misc.Color16BitsToB(minimap[y * mipsize + x-1])+Misc.Color16BitsToB(minimap[y * mipsize + x+1])) div 2);
        P[x*2+0] := Lo(color);
        P[x*2+1] := Hi(color);
      end
      else
      begin
        P[x*2+0] := Lo(minimap[y * mipsize + x]);
        P[x*2+1] := Hi(minimap[y * mipsize + x]);
      end;
    end;
  end;

  bmp.Modified := True; // force repaint

  Result := True;
end;

{ ------------------------------------------------------------------------ }

function AcquireMapInfo(MapName: string): TMapInfo;
var
  i: Integer;
begin
  SetLength(Result.Description, 256);
  GetMapInfo(PChar(MapName), @Result);
  Result.Width := Result.Width div 8;
  Result.Height := Result.Height div 8;
  for i := 1 to Length(Result.Description) do if Result.Description[i] = #0 then
  begin
    Result.Description := Copy(Result.Description, 1, i-1);
    Break;
  end;
end;

{ ------------------------------------------------------------------------ }

procedure ReloadModList; // will populate ModList and ModArchiveList lists
var
  ModCount: Integer;
  i: Integer;
begin
  ModList.Clear;
  ModArchiveList.Clear;

  ModCount := GetPrimaryModCount;
  for i := 0 to ModCount-1 do
  begin
    ModList.Add(GetPrimaryModName(i));
    ModArchiveList.Add(GetPrimaryModArchive(i));
  end;
end;

{ ------------------------------------------------------------------------ }

{ this is a fast routine, should be done in less than 1 ms (as far as I've tested it) }
function GetModHash(ModName: string): Integer;
var
  index: Integer;
  i: Integer;
begin
  Result := 0;
  index := ModList.IndexOf(ModName);
  if index = -1 then Exit;
  if Debug.Enabled then MainForm.AddMainLog('Hashing file: ' + ModList[index], Colors.MinorInfo);
  Result := GetPrimaryModChecksum(index)
end;

{ ------------------------------------------------------------------------ }

{ don't forget to reinit unitsync.dll each time you change mod (except for the first time)! }
procedure LoadMod(ModName: string);
begin
  AddAllArchives(PChar(ModName));
end;

{ ------------------------------------------------------------------------ }

procedure ReloadUnitList; // will populate UnitNames and UnitList lists
var
  i: Integer;
begin
  UnitNames.Clear;
  UnitList.Clear;

  // first process all units (this may take some time):
  while ProcessUnitsNoChecksum <> 0 do;

  for i := 0 to GetUnitCount-1 do
  begin
    UnitNames.Add(GetFullUnitName(i));
    UnitList.Add(GetUnitName(i));
  end;
end;

{ ------------------------------------------------------------------------ }

procedure ReloadSides; // populates SideList list and loads side images into SideImageList image list
var
  i: Integer;
  s: string;
  side: string;
  res: Integer;
  sl, sl2: TStringList;
  index, size, count: Integer;

  procedure LoadDefaultSideImage(Side: string);
  begin
    if UpperCase(Side) = 'ARM' then
      SideImageList.AddMasked(MainForm.DefaultArmImage.Picture.Bitmap, clWhite)
    else if UpperCase(Side) = 'CORE' then
      SideImageList.AddMasked(MainForm.DefaultCoreImage.Picture.Bitmap, clWhite)
    else
      SideImageList.AddMasked(MainForm.DefaultSideImage.Picture.Bitmap, clWhite); // none specific side
  end;

begin

  SideList.Clear;
  SideImageList.Clear;

  count := GetSideCount;
  for i := 0 to count-1 do
    SideList.Add(GetSideName(i));

  if count = 0 then // this should not happen - mod author forgot to define sides! 
  begin
    MessageDlg('This mod doesn''t have any sides defined. Using default sides (Arm/Core) ...', mtWarning, [mbOK], 0);
    SideList.Add('Arm');
    SideList.Add('Core');
  end;

  sl := TStringList.Create; // list of archive files
  sl2 := TStringList.Create; // list of archive file sides

  // get a list of all side images from mod archive:
  res := InitFindVFS('SidePics\*.bmp');
  SetLength(s, 255);
  res := FindFilesVFS(res, PChar(s), 255);
  if res = 0 then
  begin
    if Debug.Enabled then MainForm.AddMainLog('No side pictures found! Skipping ...', Colors.Error);
  end
  else
  begin
    while res <> 0 do
    begin
      for i := 1 to 255 do if s[i] = #0 then
      begin
        s := Copy(s, 1, i-1);
        Break;
      end;

      if Debug.Enabled then MainForm.AddMainLog('Side image found: ' + s, Colors.Info);
      sl.Add(s);
      sl2.Add(UpperCase(Copy(ExtractFileName(s), 1, Length(ExtractFileName(s))-4)));
      SetLength(s, 255);
      res := FindFilesVFS(res, PChar(s), 255);
    end;
  end;

  // load side images:
  for i := 0 to SideList.Count-1 do
  begin
    index := sl2.IndexOf(UpperCase(SideList[i]));

    // if we don't have a suitable side image, load the default one:
    if index = -1 then
      LoadDefaultSideImage(UpperCase(SideList[i]))
    else
    begin // let's load the side image from mod's archive file:
      res := OpenFileVFS(PChar(sl[index]));
      if res = 0 then
      begin
        MainForm.AddMainLog('Error: Unable to open archive file: ' + sl[index], Colors.Error);
        LoadDefaultSideImage(UpperCase(SideList[i]));
        Continue;
      end;
      size := FileSizeVFS(res);
      if size = 0 then
      begin
        MainForm.AddMainLog('Error: Unable to open archive file: ' + sl[index], Colors.Error);
        LoadDefaultSideImage(UpperCase(SideList[i]));
        CloseFileVFS(res);
        Continue;
      end;

      SetLength(s, size);
      ReadFileVFS(res, @s[1], size);
      try
        AddBitmapToImageList(SideImageList, s[1], size);
      except
        LoadDefaultSideImage(UpperCase(SideList[i]));
     end;
    end;
  end; // for

  sl.Free;
  sl2.Free;
end;

procedure LoadLuaOptions(modOptions: Boolean; mapName: String);
var
  i,j:integer;
  o: ^TLuaOption;
  nb: integer;
begin
  if modOptions then
  begin
    UnLoadLuaOptions(BattleForm.ModOptionsList);
    nb := GetModOptionCount;
  end
  else
  begin
    UnLoadLuaOptions(BattleForm.MapOptionsList);
    nb := GetMapOptionCount(PChar(mapName));
  end;

  for i:=0 to nb-1 do
  begin
    Case GetOptionType(i) of
      1: // bool
      begin
        New(o);
        o^ := TLuaOptionBool.Create;
        TLuaOptionBool(o^).DefaultValue := GetOptionBoolDef(i);
      end;
      2: // list
      begin
        New(o);
        o^ := TLuaOptionList.Create;
        with TLuaOptionList(o^) do
        begin
          DefaultValue := GetOptionListDef(i);
          for j:= 0 to GetOptionListCount(i)-1 do
          begin
            NameList.Add(GetOptionListItemName(i,j));
            DescriptionList.Add(GetOptionListItemDesc(i,j));
            KeyList.Add(GetOptionListItemKey(i,j))
          end;
        end;
      end;
      3: // number
      begin
        New(o);
        o^ := TLuaOptionNumber.Create;
        with TLuaOptionNumber(o^) do
        begin
          DefaultValue := GetOptionNumberDef(i);
          MinValue := GetOptionNumberMin(i);
          MaxValue := GetOptionNumberMax(i);
          StepValue := RoundTo(GetOptionNumberStep(i),-4);
        end;
      end;
      4: // string
      begin
        New(o);
        o^ := TLuaOptionString.Create;
        with TLuaOptionString(o^) do
        begin
          DefaultValue := GetOptionStringDef(i);
          MaxStringLength := GetOptionStringMaxLen(i);
        end;
      end;
    else
    begin
        New(o);
        o^ := TLuaOption.Create;
    end
    end;
    o^.Key := StrNew(GetOptionKey(i));
    o^.Name := StrNew(GetOptionName(i));
    o^.Description := StrNew(GetOptionDesc(i));
    if modOptions then
    begin
      o^.KeyPrefix := 'GAME\MODOPTIONS\';
      BattleForm.ModOptionsList.Add(o^);
    end
    else
    begin
      o^.KeyPrefix := 'GAME\MAPOPTIONS\';
      BattleForm.MapOptionsList.Add(o^);
    end;
  end;
end;
procedure UnLoadLuaOptions(luaOptionList: TList);
var
  i: Integer;
begin
  for i:=0 to luaOptionList.Count-1 do
      TLuaOption(luaOptionList[i]).Destroy;
  luaOptionList.Clear;
end;
function GetLuaAIList:TStrings;
var
  i:integer;
begin
  Result := TStringList.Create;
  for i:=0 to GetLuaAICount-1 do
    Result.Add('LuaAI:'+GetLuaAIName(i));
end;
function GetModValidMapList: TStringList;
var
  i:integer;
  return: TStringList;
begin
  return := TStringList.Create;
  for i:=0 to GetModValidMapCount-1 do
    return.Add(GetModValidMap(i));

  Result := return;
end;


{ ------------------------------------------------------------------------ }

initialization
  // side lists and unit lists gets loaded when needed and not in OnFormCreate
  // event (this information is accurate for 0.23 successor version at least)

  ModList := TStringList.Create;
  ModArchiveList := TStringList.Create;
  UnitList := TStringList.Create;
  UnitNames := TStringList.Create;
  SideList := TStringList.Create; // we will load side list when needed
  SideList.CaseSensitive := False;
  SideImageList := TImageList.CreateSize(16, 16);
  MapList := TStringList.Create;
  MapChecksums := TStringList.Create;

finalization

  if Assigned(ModList) then ModList.Free;
  if Assigned(ModArchiveList) then ModArchiveList.Free;
  if Assigned(UnitList) then UnitList.Free;
  if Assigned(UnitNames) then UnitNames.Free;
  if Assigned(SideList) then SideList.Free;
  if Assigned(SideImageList) then SideImageList.Free;
  if Assigned(MapList) then MapList.Free;
  if Assigned(MapChecksums) then MapChecksums.Free;

end.
