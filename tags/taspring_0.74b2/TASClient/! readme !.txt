UPDATED: 13 May 2006

*** In order to compile the sources, you'll have to install some free 3rd-party components: ***

- TjanTracker (Included with these sources - just install janTracker.pas)
- Virtual TreeView component: http://www.lischke-online.de/VirtualTreeview/
- TWSocket (ICS): http://www.overbyte.be/frame_index.html?redirTo=/ssl.html
- GraphicEx: http://www.soft-gems.net/Graphics.php
- RichEditURL (Included with these sources - just install RichEditURL.pas)
- TExImage (Included with these sources - just install ImageEx.pas)
- JVCL (http://homepages.borland.com/jedi/jvcl/) - download JVCL+JCL bundle

Note: All components that are included with the sources are located
in /LobbyComponents folder and are by default installed in "Spring lobby"
component tab. To change that, modify 'register' method in .pas files.

To remove any of these custom components from Delphi, follow these steps:
1) go to Component->Configure Palette and find 'Spring lobby', then
   hide the component you wish to remove and finally delete the 'Spring lobby'
   tab (if you removed all components from it, else leave it).
2) go to Component->Configure Packages->Borland user components
   and click Edit. Remove desired components from the package and hit "Compile".
3) remove path to component(s) from search path by going to 
   Tools->Environment options->Library->Library path
4) note that Delphi stores component tab location for each component in registry,
   so if you'll try moving the component to another component tab later on, you'll
   have to first delete any references to that component in registry:
   HKEY_CURRENT_USER\Software\Borland\Delphi\7.0\Palette
   (or, you can also move it manually by righclicking on component palette and
   choosing "properties" and then move/rename the component tab there)

To enable theme support, you'll have to download several components
(you won't be able to compile the program withouth them). It is 
important that you download exactly the same version of these
components as listed here, since some of them are not compatible with 
newer versions:

- Toolbar2000 2.1.8 (http://www.jrsoftware.org/tb2k.php, download from http://files.jrsoftware.org/tb2k/)
- Patch for TB2K 2.1.8 (http://club.telepolis.com/silverpointdev/sptbxlib/tbxpatch218.zip)
- TBX 2.1 Beta 1 (http://www.g32.org/tbx/index.html)
- TntWare Delphi Unicode Controls 2.2.8 or above (http://www.tntware.com/delphicontrols/unicode/)
- SpTBXLib 1.8.1 (http://club.telepolis.com/silverpointdev/sptbxlib/index.htm)
- Additional TBX themes (http://www.rmklever.com/zipfiles/TBXThemes21.zip)

To install these components, you should use Silverpoint MultiInstaller
(http://club.telepolis.com/silverpointdev/multiinstaller/index.htm),
instructions on how to install them are on the same page.

*** List of 3rd-party libraries included (no need to install or download): ***

- GpTimeZone (http://17slon.com/gp/gp/gptimezone.htm)
- ESBDates (http://www.softpile.com/Development/Source_Code/Review_08509_index.html)
- GpIFF (http://17slon.com/gp/gp/gpiff.htm)

*** Important notes: ***

- I use Delphi 7 to compile the program (it compiles with any newer Delphi as well, haven't
  tested it with older ones though)

- You should disable all compiler warnings or simply ignore them (in Delphi 7, go to
  Tools->Debugger options->Language Exceptions->Stop on Delphi Exceptions (there should be
  no tick here - it should be disabled)).

*** Credits for various graphics: ***

- Neuralize (most buttons, client status symbols and battle icons)
- RuneCrafter (splash screen and other stuff) 
- Maelstrom (NAT warning icon, "bot mode" icon)
- IceXuick (map grade icons, some popup menu icons)
- Some icons are taken from the TBX components (demo app)