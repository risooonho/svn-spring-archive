<?php
/*                                                                                                                                                                                                                                 
===Fail algab...===
 <nowiki>
*/

# NOTE: To turn off "Current Events" in the sidebar,
# set "currentevents" => "-"
/*
 </nowiki>
===Nimeruumid===
 <nowiki>
*/
# The names of the namespaces can be set here, but the numbers
# are magical, so don't change or move them!  The Namespace class
# encapsulates some of the magic-ness.
#
$wgNamespaceNamesEt = array(
        NS_MEDIA            => 'Meedia',
        NS_SPECIAL          => 'Eri',
        NS_MAIN             => '',
        NS_TALK             => 'Arutelu',
        NS_USER             => 'Kasutaja',
        NS_USER_TALK        => 'Kasutaja_arutelu',
        NS_PROJECT          => $wgMetaNamespace,
        NS_PROJECT_TALK     => $wgMetaNamespace . '_arutelu',
        NS_IMAGE            => 'Pilt',
        NS_IMAGE_TALK       => 'Pildi_arutelu',
        NS_MEDIAWIKI        => 'MediaWiki',
        NS_MEDIAWIKI_TALK   => 'MediaWiki_arutelu',
        NS_TEMPLATE         => 'Mall',
        NS_TEMPLATE_TALK    => 'Malli_arutelu',
        NS_HELP             => 'Juhend',
        NS_HELP_TALK        => 'Juhendi_arutelu',
        NS_CATEGORY         => 'Kategooria',
        NS_CATEGORY_TALK    => 'Kategooria_arutelu'
) + $wgNamespaceNamesEn;
/*
 </nowiki>
===Nahad===
 <nowiki>
*/
/* private */ $wgSkinNamesEt = array(
	'standard' => "Standard",
	'nostalgia' => "Nostalgia",
	'cologneblue' => "Kölni sinine",
	'smarty' => "Paddington",
	'montparnasse' => "Montparnasse",
	'davinci' => "DaVinci",
	'mono' => "Mono",
	'monobook' => "MonoBook",
	 "myskin" => "Mu oma nahk" 
);
/*
 </nowiki>

===Kiirriba===

 <nowiki>
*/
/* private */ $wgQuickbarSettingsEt = array(
	"Ei_ole", "Püsivalt_vasakul", "Püsivalt paremal", "Ujuvalt vasakul"
);
/*
 </nowiki>
===Raamatupoodide nimekiri===
 <nowiki>
*/
#Lisasin eestimaised poed, aga võõramaiseid ei julenud kustutada.


/* private */ $wgBookstoreListEt = array(
	"Apollo" => "http://www.apollo.ee/search.php?keyword=$1&search=OTSI",
	"minu Raamat" => "http://www.raamat.ee/advanced_search_result.php?keywords=$1",
	"Raamatukoi" => "http://www.raamatukoi.ee/cgi-bin/index?valik=otsing&paring=$1",
	"AddALL" => "http://www.addall.com/New/Partner.cgi?query=$1&type=ISBN",
	"PriceSCAN" => "http://www.pricescan.com/books/bookDetail.asp?isbn=$1",
	"Barnes & Noble" => "http://shop.barnesandnoble.com/bookSearch/isbnInquiry.asp?isbn=$1",
	"Amazon.com" => "http://www.amazon.com/exec/obidos/ISBN=$1"
);


/*
 </nowiki>
===Võtmesõnad===
 <nowiki>
*/
#Jätsin tõlkimata -- minu elu see ei muuda kui need ka ainult inglise keeles on. 
#Aga mis mul selle vastu võiks olla, kui eestikeelsed sünonüümid olemas.


# Note to translators:
#   Please include the English words as synonyms.  This allows people
#   from other wikis to contribute more easily.
#
#
/* private */ $wgMagicWordsEt = array(
#   ID                                 CASE  SYNONYMS
    MAG_REDIRECT             => array( 0,    '#redirect', "#suuna"    ),
    MAG_NOTOC                => array( 0,    '__NOTOC__'              ),
    MAG_FORCETOC             => array( 0,    '__FORCETOC__'           ),
    MAG_TOC                  => array( 0,    '__TOC__'                ),
    MAG_NOEDITSECTION        => array( 0,    '__NOEDITSECTION__'      ),
    MAG_START                => array( 0,    '__START__'              ),
    MAG_CURRENTMONTH         => array( 1,    'CURRENTMONTH'           ),
    MAG_CURRENTMONTHNAME     => array( 1,    'CURRENTMONTHNAME'       ),
    MAG_CURRENTDAY           => array( 1,    'CURRENTDAY'             ),
    MAG_CURRENTDAYNAME       => array( 1,    'CURRENTDAYNAME'         ),
    MAG_CURRENTYEAR          => array( 1,    'CURRENTYEAR'            ),
    MAG_CURRENTTIME          => array( 1,    'CURRENTTIME'            ),
    MAG_NUMBEROFARTICLES     => array( 1,    'NUMBEROFARTICLES'       ),
    MAG_CURRENTMONTHNAMEGEN  => array( 1,    'CURRENTMONTHNAMEGEN'    ),
        MAG_PAGENAME             => array( 1,    'PAGENAME'               ),
        MAG_PAGENAMEE                    => array( 1,    'PAGENAMEE'              ),
        MAG_NAMESPACE            => array( 1,    'NAMESPACE'              ),
        MAG_MSG                  => array( 0,    'MSG:'                   ),
        MAG_SUBST                => array( 0,    'SUBST:'                 ),
    MAG_MSGNW                => array( 0,    'MSGNW:'                 ),
        MAG_END                  => array( 0,    '__END__'                ),
    MAG_IMG_THUMBNAIL        => array( 1,    'thumbnail', 'thumb'     ),
    MAG_IMG_RIGHT            => array( 1,    'right'                  ),
    MAG_IMG_LEFT             => array( 1,    'left'                   ),
    MAG_IMG_NONE             => array( 1,    'none'                   ),
    MAG_IMG_WIDTH            => array( 1,    '$1px'                   ),
    MAG_IMG_CENTER           => array( 1,    'center', 'centre'       ),
    MAG_IMG_FRAMED           => array( 1,    'framed', 'enframed', 'frame' ),
    MAG_INT                  => array( 0,    'INT:'                   ),
    MAG_SITENAME             => array( 1,    'SITENAME'               ),
    MAG_NS                   => array( 0,    'NS:'                    ),
        MAG_LOCALURL             => array( 0,    'LOCALURL:'              ),
        MAG_LOCALURLE            => array( 0,    'LOCALURLE:'             ),
        MAG_SERVER               => array( 0,    'SERVER'                 ),
        MAG_GRAMMAR              => array( 0,    'GRAMMAR:'               )
);


/*

 </nowiki>
===Erileheküljed===
 <nowiki>
*/
#Language.php seda jada (enam?) ei sisalda, alltoodud võtmesõnad määratakse
#ainult jadas $wgAllMessages. Vajab veel selgitamist, kas vaadeldavat jada on tarvis.


# All special pages have to be listed here: a description of ""
# will make them not show up on the "Special Pages" page, which
# is the right thing for some of them (such as the "targeted" ones).
#

/* private */ $wgValidSpecialPagesEt = array(
	"Userlogin"		=> "",
	"Userlogout"	=> "",
	"Preferences"	=> "Minu eelistuste seadmine",
	"Watchlist"		=> "Minu jälgimisloend",
	"Recentchanges" => "Viimati muudetud artiklid",
	"Upload"		=> "Pildifailide üleslaadimine",
	"Imagelist"		=> "Piltide loend",
	"Listusers"		=> "Kasutajad",
	"Statistics"	=> "Saidi statistika",
	"Randompage"	=> "Juhuslik artikkel",

	"Lonelypages"	=> "üksildased artiklid",
	"Unusedimages"	=> "Kasutamata pildid",
	"Popularpages"	=> "Kõige loetavamad artiklid",
	"Wantedpages"	=> "Kõige oodatumad artiklid",
	"Shortpages"	=> "Lühikesed artiklid",
	"Longpages"		=> "Pikad artiklid",
	"Newpages"		=> "Uued artiklid",
#	"Intl"		=> "Keeltevahelised lingid",
	"Allpages"		=> "Kõik artiklid pealkirja järgi",

	"Ipblocklist"	=> "Blokeeritud IP-aadressid",
	"Maintenance" => "Hoolduslehekülg",
	"Specialpages"  => "",
	"Contributions" => "",
	"Emailuser"		=> "",
	"Whatlinkshere" => "",
	"Recentchangeslinked" => "",
	"Movepage"		=> "",
	"Booksources"	=> "Raamatud",
	"Export"		=> "Eksport XML-vormingusse",
	"Version"		=> "Versioon",
);
/*
 </nowiki>
===Administraatori erilehed===
 <nowiki> 
*/
/* private */ $wgSysopSpecialPagesEt = array(
	"Blockip"		=> "Blokeeri IP-aadress",
	"Asksql"		=> "Otsi andmebaasist",
	"Undelete"		=> "Taasta kustutatud leheküljed"
);

/*
 </nowiki>
===Arendaja erilehed===
 <nowiki>
*/
/* private */ $wgDeveloperSpecialPagesEt = array(
	"Lockdb"		=> "Võta andmebaas kirjutuskaitse alla",
	"Unlockdb"		=> "Taasta andmebaasi kirjutuspääs",
);

/*
 </nowiki>
===Süsteemi teated===
 <nowiki>
*/
/* private */ $wgAllMessagesEt = array(

'special_version_prefix' => '',
'special_version_postfix' => '',

/*
 </nowiki>
====Linkimisseaded====
 <nowiki>
*/
"tog-hover"   => "Näita tekstimulli siselinkide peale",
"tog-underline" => "Lingid alla kriipsutada",
"tog-highlightbroken" => "Vorminda lingirikked<a href=\"\" class=\"new\">nii</a> (alternatiiv: nii<a href=\"\" class=\"internal\">?</a>).",
"tog-justify" => "Lõikude rööpjoondus",
"tog-hideminor" => "Peida pisiparandused viimastes muudatustes",
"tog-usenewrc" => "Laiendatud viimased muudatused (mitte kõikide brauserite puhul)",
"tog-numberheadings" => "Pealkirjade automaatnummerdus",
"tog-showtoolbar" => "Redigeerimise tööriistariba näitamine",
"tog-rememberpassword" => "Parooli meeldejätmine tulevasteks seanssideks",
"tog-editwidth" => "Redaktoriboksil on täislaius",
"tog-editondblclick" => "Artiklite redigeerimine topeltklõpsu peale (JavaScript)",
"tog-watchdefault" => "Jälgi uusi ja muudetud artikleid",
"tog-minordefault" => "Märgi kõik parandused vaikimisi pisiparandusteks",
"tog-previewontop" => "Näita eelvaadet redaktoriboksi ees, mitte järel",



/*
 </nowiki>
====Kuud, päevad====
 <nowiki>
*/

# Dates
'sunday' => 'pühapäev',
'monday' => 'esmaspäev',
'tuesday' => 'teisipäev',
'wednesday' => 'kolmapäev',
'thursday' => 'neljapäev',
'friday' => 'reede',
'saturday' => 'laupäev',
'january' => 'jaanuar',
'february' => 'veebruar',
'march' => 'märts',
'april' => 'aprill',
'may_long' => 'mai',
'june' => 'juuni',
'july' => 'juuli',
'august' => 'august',
'september' => 'september',
'october' => 'oktoober',
'november' => 'november',
'december' => 'detsember',
'jan' => 'jaan',
'feb' => 'veebr',
'mar' => 'märts',
'apr' => 'apr',
'may' => 'mai',
'jun' => 'juuni',
'jul' => 'juuli',
'aug' => 'aug',
'sep' => 'sept',
'oct' => 'okt',
'nov' => 'nov',
'dec' => 'dets',





/*
 </nowiki>
====Bits of text used by many pages====
 <nowiki>
*/
# Bits of text used by many pages:
#
'categories' => 'Kategooriad',
'category' => 'kategooria',
'category_header' => 'Selles kategoorias on "$1" artiklit',
'subcategories' => 'Alamkategooriad',


"linktrail"		=> "/^([a-z]+)(.*)\$/sD",
"mainpage"		=> "Esileht",
"mainpagetext"	=> "Wiki tarkvara installeeritud.",
"mainpagedocfooter" => "Juhiste saamiseks kasutamise ning konfigureerimise kohta vaata palun inglisekeelset [http://meta.wikipedia.org/wiki/MediaWiki_i18n dokumentatsiooni liidese kohaldamisest]
ning [http://meta.wikipedia.org/wiki/MediaWiki_User%27s_Guide kasutusjuhendit].",
'portal'                =>  'Kogukonnavärav', # Kirjutajate portaal?
'portal-url'            => '{{ns:4}}:Kogukonnavärav',
"about"			=> "Tiitelandmed",
"aboutsite" => "{{SITENAME}} tiitelandmed",
"aboutpage"		=> "{{ns:4}}:Tiitelandmed",
"article"		=> "Sisu",  # või "Artikkel" nagu praegu Vikipeedias?
"help"			=> "Juhend", # Vikipeedias "Spikker"
"helppage"		=> "{{ns:12}}:Juhend",
"wikititlesuffix" => "{{SITENAME}}",
"bugreports"	=> "Teated programmivigadest",
"bugreportspage" => "{{ns:4}}:Teated_programmivigadest",
'sitesupport'   => 'Annetused', # Set a URL in $wgSiteSupportPage in LocalSettings.php
"faq"			=> "KKK",
"faqpage"		=> "{{ns:4}}:KKK",
"edithelp"		=> "Redigeerimisjuhend",
"newwindow"             => "(avaneb uues aknas)",
"edithelppage"	=> "{{ns:12}}:Kuidas_artiklit_redigeerida",
"cancel"		=> "Tühista",
"qbfind"		=> "Otsi",
"qbbrowse"		=> "Sirvi",
"qbedit"		=> "Redigeeri",
"qbpageoptions" => "Lehekülje suvandid",  // en: this page
"qbpageinfo"	=> "Lehekülje andmed",    // en: context
"qbmyoptions"	=> "Minu suvandid",       // en: my pages
"qbspecialpages" => "Erileheküljed",
'moredotdotdot' => 'Veel...',
"mypage"		=> "Minu lehekülg",
"mytalk"		=> "Minu arutelu",
'anontalk'              => 'Arutelu selle IP jaoks',
"currentevents" => "Jooksvad sündmused",
"navigation"	=> "Navigeerimine",
"errorpagetitle" => "Viga",
'disclaimers' => 'Hoiatused',
"disclaimerpage"   => "{{ns:4}}:Üldised_hoiatused", # lihtsalt "Hoiatused"?
"returnto"		=> "Naase $1 juurde",
"tagline"	=> "Allikas: {{SITENAME}}",
"whatlinkshere"	=> "Siia viitavad artiklid",
"help"			=> "Juhend",
"search"		=> "Otsi",
"go"		=> "Mine",
"history"		=> "Artikli ajalugu",
'history_short' => 'Ajalugu',
'info_short'    => 'Info',
"printableversion" => "Prinditav versioon",
"editthispage"	=> "Redigeeri seda artiklit",
'edit' => 'Redigeeri',

"delete" => "Kustuta",
"deletethispage" => "Kustuta see artikkel",
"undelete_short" => "Taasta $1 muudatust",
"protect" => "Kaitse",
"protectthispage" => "Kaitse seda artiklit",
"unprotect" => "Ära kaitse",
"unprotectthispage" => "Ära kaitse seda artiklit",
"newpage" => "Uus artikkel",
"talkpage"		=> "Selle artikli arutelu",
'specialpage' => 'Erilehekülg',
'personaltools' => 'Personaalsed tööriistad',
'postcomment'   => 'Lisa kommentaar',
'addsection'   => '+',
"articlepage"	=> "Artiklilehekülg",
"subjectpage"	=> "Teema", # For compatibility
'talk' => 'Arutelu',
'toolbox' => 'Tööriistakast',
"userpage" => "Kasutajalehekülg",
"wikipediapage" => "Metalehekülg",
"imagepage" => 	"Pildilehekülg",
"viewtalkpage" => "Arutelulehekülg",
"otherlanguages" => "Teised keeled",
"redirectedfrom" => "(Ümber suunatud artiklist $1)",
"lastmodified"	=> "Viimati muudetud $1.",
"viewcount"		=> "Seda lehekülge on külastatud $1 korda.",
"gnunote" => "Kogu tekst on kasutatav litsentsi <a class=internal href='$wgScriptPath/GNU_FDL'>GNU Free Documentation License</a> tingimustel.", # aegunud, võib vist eemaldada, asendada järgmisega:
"copyright" => "Kogu tekst on kasutatav litsentsi <a class=internal href='$wgScriptPath/GNU_FDL'>GNU Vaba Dokumentatsiooni Litsentsi</a> (GFDL) tingimustel.",
'poweredby'     => "{{SITENAME}} kasutab avatud koodiga wiki mootorit [http://www.mediawiki.org/ MediaWiki].",
"printsubtitle" => "(Allikas: {{SERVER}})",
"protectedpage" => "Kaitstud artikkel",
"administrators" => "{{ns:4}}:Administraatorid",
"sysoptitle"	=> "Nõutav süsteemi operaatori staatus",
"sysoptext"	=> "Seda toimingut saavad sooritada ainult süsteemi operaatori staatusega kasutajad. Vaata $1.",
"developertitle" => "Nõutav süsteemiarendaja staatus",
"developertext"	=> "Seda toimingut saavad sooritada ainult süsteemiarendaja staatusega kasutajad.
Vaata $1.",
'bureaucrattitle'       => 'Nõutav bürokraadi staatus',
"bureaucrattext"      => "Seda toimingut saavad sooritada ainult \"bürokraadi\" staatusega kasutajad. Vaata $1.",
"nbytes"		=> "$1 baiti",
"go"			=> "Mine",
"ok"			=> "OK",
"sitetitle"		=> "{{SITENAME}}",
'pagetitle'             => "$1 - {{SITENAME}}",
"sitesubtitle"	=> "Vaba entsüklopeedia", # sõltub saidist (FIXME)
"retrievedfrom" => "Välja otsitud andmebaasist \"$1\"", # parandaks sõnastust?
"newmessages" => "Teile on uusi sõnumeid ($1).",
"newmessageslink" => "Vaata sõnumeid.",
'editsection'=>'redigeeri',
'toc' => 'Sisukord',
'showtoc' => 'näita',
'hidetoc' => 'peida',
'thisisdeleted' => "Vaata või taasta $1?", # View or restore... 
'restorelink' => "Kustutatud muudatuste arv: $1",
'feedlinks' => 'Sööde:', # See sõna ei meeldi, aga paremat ei tea.
'sitenotice'    => '', # the equivalent to wgSiteNotice

/*
 </nowiki>
====Short words for each namespace...====
 <nowiki>
*/

# Short words for each namespace, by default used in the 'article' tab in monobook
'nstab-main' => 'Artikkel',
'nstab-user' => 'Kasutaja',
'nstab-media' => 'Meedia',
'nstab-special' => 'Eri',
'nstab-wp' => 'Tiitelandmed', # about
'nstab-image' => 'Pilt',
'nstab-mediawiki' => 'Sõnum', # Message
'nstab-template' => 'Mall',
'nstab-help' => 'Juhend',
'nstab-category' => 'Kategooria',

/*
 </nowiki>
====Main script and global functions====
 <nowiki>
*/
# Main script and global functions
#
"nosuchaction"	=> "Sellist toimingut pole.",
"nosuchactiontext" => "Wiki ei tunne sellele aadressile vastavat toimingut.",
"nosuchspecialpage" => "Sellist erilehekülge pole.",
"nospecialpagetext" => "Wiki ei tunne sellist erilehekülge.",

/*
 </nowiki>
====General errors====
 <nowiki>
*/

# General errors
#
"error"			=> "Viga",
"databaseerror" => "Andmebaasi viga",
"dberrortext"	=> "Andmebaasipäringus oli süntaksiviga.
Otsingupäring oli ebakorrektne (vaata $5) või on tarkvaras viga.
Viimane andmebaasipäring oli:
<blockquote><tt>$1</tt></blockquote>
ja see kutsuti funktsioonist \"<tt>$2</tt>\".
MySQL andis vea \"<tt>$3: $4</tt>\".",
"dberrortextcl" => "Andmebaasipäringus oli süntaksiviga.
Viimane andmebaasipäring oli:
\"$1\"
ja see kutsuti funktsioonist \"$2\".
MySQL andis vea \"$3: $4\".\n",
"noconnect"		=> "Vabandame! Wikil on tehnilisi probleeme ning ta ei saa andmebaasiserveriga $1 ühendust",
"nodb"			=> "Andmebaasi $1 ei õnnestunud kätte saada",
'cachederror'           => 'Järgnev lehekülg on puhverdatud koopia soovitud leheküljest ja ei pruugi seega olla värskeim.',
"readonly"		=> "Andmebaas on hetkel kirjutuskaitse all", # 'Database locked', võimalik et siiski "Andmebaas kaitse alla"
"enterlockreason" => "Sisesta lukustamise põhjus ning juurdepääsu taastamise ligikaudne aeg",
"readonlytext"	=> "Andmebaas on praegu kirjutuskaitse all, tõenäoliselt andmebaasi rutiinseks hoolduseks, mille lõppedes normaalne olukord taastub.
Administraator, kes selle kaitse alla võttis, andis järgmise selgituse:
<p>$1",
"missingarticle" => "Andmebaas ei leidnud lehekülje \"$1\" teksti, kuigi see oleks pidanud olema leitav. 

<p>Tavaliselt on selle põhjuseks vananenud sisuerinevuste- või ajaloolink leheküljele, mis on kustutatud. 

<p>Kui ei ole tegemist sellise juhtumiga, siis võib olla tegemist tarkvaraveaga. Palun teatage sellest administraatorile, märkides ära kasutatud aadressi.",
"internalerror" => "Sisemine viga",
"filecopyerror" => "Ei saanud faili \"$1\" failiks \"$2\" kopeerida.",
"filerenameerror" => "Ei saanud faili \"$1\" failiks \"$2\" ümber nimetada.",
"filedeleteerror" => "Faili nimega \"$1\" ei ole võimalik kustutada.",
"filenotfound"	=> "Faili nimega \"$1\" ei leitud.",
"unexpected"	=> "Ootamatu väärtus: \"$1\"=\"$2\".",
"formerror"		=> "Viga: vormi ei saanud salvestada",	
"badarticleerror" => "Seda toimingut ei saa sellel leheküljel sooritada.",
"cannotdelete"	=> "Seda lehekülge või pilti ei ole võimalik kustutada. (Võib-olla keegi teine juba kustutas selle.)",
"badtitle"		=> "Vigane pealkiri",
"badtitletext"	=> "Küsitud artiklipealkiri oli kas vigane, tühi või siis
valesti viidatud keelte- või wikidevaheline pealkiri.",
"perfdisabled" => "Vabandage! See funktsioon ajutiselt ei tööta, sest ta aeglustab andmebaasi kasutamist võimatuseni. Sellepärast täiustatakse vastavat programmi lähitulevikus. Võib-olla teete seda Teie!",
'perfdisabledsub' => "Siin on salvestatud koopia $1-st:", # obsolete?
'perfcached' => 'Järgnevad andmed on puhverdatud ja ei pruugi olla kõige värskemad:',
'wrong_wfQuery_params' => "Valed parameeterid funktsioonile wfQuery()<br />
Funktsioon: $1<br />
Päring: $2
",
'viewsource' => 'Vaata lähteteksti',
'protectedtext' => "See lehekülg on lukustatud, et muudatusi vältida. Selleks võib olla
mitmesuguseid põhjusi, vaata palun artiklit
[[{{ns:4}}:Lukustatud lehekülg]].
Sa saad aga vaadata ja kopeerida selle lehekülje lähteteksti -- ",
'seriousxhtmlerrors' => 'tidy leidis, et xhtml-märgenduses on tõsiseid vigu.',



/*
 </nowiki>
====Login and logout pages====
 <nowiki>
*/

# Login and logout pages
#
"logouttitle"	=> "Väljalogimine",
"logouttext"	=> "Te olete välja loginud.
Võite kasutada süsteemi anonüümselt, aga ka sama või mõne teise kasutajana uuesti sisse logida.\n",
     # rookisin Vikipeedia välja, {{SITENAME}} oleks õige, aga vajab ümbersõnastamist.

"welcomecreation" => "<h2>Tere tulemast, $1!</h2><p>Teie konto on loodud. Ärge unustage seada oma eelistusi.",

"loginpagetitle" => "Sisselogimine",
"yourname"		=> "Teie kasutajanimi",
"yourpassword"	=> "Teie parool",
"yourpasswordagain" => "Sisestage parool uuesti",
"newusersonly"	=> " (ainult uued kasutajad)",
"remembermypassword" => "Parooli meeldejätmine tulevasteks seanssideks.",
"loginproblem"	=> "<b>Sisselogimine ei õnnestunud.</b><br />Proovige uuesti!",
"alreadyloggedin" => "<font color=red><b>Kasutaja $1, Te olete juba sisse loginud!</b></font><br />\n",

"areyounew"		=> "Kui olete siin uustulnuk ja tahate saada kasutajakontot, siis 
sisestage kasutajanimi, seejärel parool (kaks korda).
E-posti aadress ei ole kohustuslik; kui Te kaotate oma parooli, võite lasta selle saata Teie poolt antud aadressil.<br />\n", # aegunud -- kustutada.

"login"			=> "Logi sisse",
'loginprompt'           => "{{SITENAME}} võimaldab sisselogimist vaid siis kui küpsised on lubatud.",
"userlogin"		=> "Logi sisse",
"logout"		=> "Logi välja",
"userlogout"	=> "Logi välja",
"createaccount"	=> "Loo uus konto",
'createaccountmail'     => 'meili teel',
"badretype"		=> "Sisestatud paroolid ei lange kokku.",
"userexists"	=> "Sisestatud kasutajanimi on juba kasutusel. Valige uus nimi.",
"youremail"		=> "Teie e-posti aadress*",
"yournick"		=> "Teie hüüdnimi (allakirjutamiseks)",
"emailforlost"	=> "* Meiliaadressi sisestamine ei ole kohustuslik. Kuid see aitab inimestel Teiega veebisaidi kaudu ühendust võtta, ilma et Te peaksite neile oma meiliaadressi avaldama, ning samuti on sellest kasu, kui unustate parooli. <br /><br />Teie päris nime, kui otsustate selle avaldada, kasutatakse Teie tööpanuse seostamiseks Teiega.",
'prefs-help-userdata' => '* <strong>Tegelik nimi</strong> (pole kohustuslik): kui otsustate selle avaldada, kasutatakse seda Teie tööpanuse seostamiseks Teiega.<br />
* <strong>E-post</strong> (pole kohustuslik): Võimaldab inimestel Teiega veebisaidi kaudu ühendust võtta, ilma et Te peaksite neile oma meiliaadressi avaldama, samuti on sellest kasu, kui unustate parooli.',

"loginerror"	=> "Viga sisselogimisel",
'nocookiesnew'  => "Kasutajakonto loodi, aga sa ei ole sisse logitud, sest {{SITENAME}} kasutab kasutajate tuvastamisel küpsiseid. Sinu brauseris on küpsised keelatud. Palun sea küpsised lubatuks ja logi siis oma vastse kasutajanime ning parooliga sisse.",
"nocookieslogin"      => "{{SITENAME}} kasutab kasutajate tuvastamisel küpsiseid. Sinu brauseris on küpsised keelatud. Palun sea küpsised lubatuks ja proovi siis uuesti.",
"noname"		=> "Sa ei sisestanud kasutajanime lubataval kujul.",
"loginsuccesstitle" => "Sisselogimine õnnestus",
"loginsuccess"	=> "Te olete sisse loginud. Teie kasutajanimi on \"$1\".",
"nosuchuser"	=> "Kasutajat nimega \"$1\" ei ole olemas. Kontrollige kirjapilti või kasutage alljärgnevat vormi uue kasutajakonto loomiseks.",
"wrongpassword"	=> "Vale parool. Proovige uuesti.",
"mailmypassword" => "Saada mulle meili teel uus parool",
"passwordremindertitle" => "{{SITENAME}} - unustatud salasõna",
"passwordremindertext" => "Keegi (tõenäoliselt Teie, IP-aadressilt $1),
palus, et me saadaksime Teile uue parooli süsteemi sisselogimiseks.
Kasutaja \"$2\" parool on nüüd \"$3\".
Võiksid sisse logida ja selle ajutise parooli ära muuta. <BR \>

Sinu {{SITENAME}}.",
"noemail"		=> "Kasutaja \"$1\" meiliaadressi meil kahjuks pole.",
"passwordsent"	=> "Uus parool on saadetud kasutaja \"$1\" registreeritud meiliaadressil.
Pärast parooli saamist logige palun sisse.",
'loginend'              => ' ',
'mailerror' => "Viga kirja saatmisel: $1",
'acct_creation_throttle_hit' => 'Vabandame, aga te olete loonud juba $1 kontot. Rohkem te ei saa.',

/*
 </nowiki>
====Edit page toolbar====
 <nowiki>
*/

# Edit page toolbar
'bold_sample'=>'Rasvane kiri',
'bold_tip'=>'Rasvane kiri',
'italic_sample'=>'Kursiiv',
'italic_tip'=>'Kursiiv',
'link_sample'=>'Lingitav pealkiri',
'link_tip'=>'Siselink',
'extlink_sample'=>'http://www.välislink.com Lingi nimi',
'extlink_tip'=>'Välislink (ära unusta prefiksit http://)',
'headline_sample'=>'Pealkiri',
'headline_tip'=>'Teise taseme pealkiri',
'math_sample'=>'Sisesta valem siia',
'math_tip'=>'Matemaatiline tekst (LaTeX)',
'nowiki_sample'=>'Sisesta formaatimata tekst',
'nowiki_tip'=>'Ignoreeri viki vormindust',
'image_sample'=>'Näidis.jpg',
'image_tip'=>'Pilt',
'media_sample'=>'Näidis.mp3',
'media_tip'=>'Meediafail',
'sig_tip'=>'Sinu allkiri koos ajatempliga',
'hr_tip'=>'Horisontaaljoon',
'infobox'=>'Näidisteksti saamiseks vajuta nupule',
# alert box shown in browsers where text selection does not work, test e.g. with konqueror
'infobox_alert'=>"Sisesta palun tekst, mida tahad formaatida.\\n See ilmub kopeerimiseks ja kleepimiseks
infokasti.\\nNäide:\\n$1\\nmuutub selliseks:\\n$2",

/*
 </nowiki>
====Edit pages====
 <nowiki>
*/

# Edit pages
#
"summary"		=> "Resümee",
'subject'               => 'Pealkiri',
"minoredit"		=> "See on pisiparandus",
"watchthis"		=> "Jälgi seda artiklit",
"savearticle"	=> "Salvesta",
"preview"		=> "Vaata",
"showpreview"	=> "Näita eelvaadet",
"blockedtitle"	=> "Kasutaja on blokeeritud",
"blockedtext"	=> "Teie kasutajanime või IP-aadressi blokeeris $1.
Tema põhjendus on järgmine:<br />''$2''<p>Küsimuse arutamiseks võite pöörduda $1 või mõne teise
[[{{ns:4}}:administraatorid|administraatori]] poole.

Pange tähele, et Te ei saa sellele kasutajale teadet saata, kui Te pole registreerinud oma [[Eri:Eelistused|eelistuste lehel]] kehtivat e-posti aadressi.

Teie IP on $3. Lisage see aadress kõigile järelpärimistele, mida kavatsete teha.",

'whitelistedittitle' => 'Toimetamiseks on vaja sisse logida',
'whitelistedittext' => 'Lehekülgede toimetamiseks peate [[Eri:Userlogin|sisse logima]].',
'whitelistreadtitle' => 'Lugemiseks peate olema sisse logitud',
'whitelistreadtext' => 'Lehekülgede lugemiseks peate [[Eri:Userlogin|sisse logima]].',
'whitelistacctitle' => 'Teil pole õigust kasutajakontot luua',
'whitelistacctext' => 'Et selles Vikis kontosid luua, peate olema [[Eri:Userlogin|sisse logitud]] ja omama vastavaid õigusi.',

'loginreqtitle' => 'Vajalik on sisselogimine',
'loginreqtext'  => 'Lehekülgede vaatamiseks peate [[Eri:Userlogin|sisse logima]].',
'accmailtitle' => 'Saadeti parool.',
'accmailtext' => "Kasutaja '$1' parool saadeti aadressile $2.",

"newarticle"	=> "(Uus)",
"newarticletext" =>
"Seda lehekülge veel ei ole.
Lehekülje loomiseks hakake kirjutama all olevasse boksi 
(lisainfo saamiseks vaadake [[{{ns:4}}:Juhend|juhendit]]).
Kui sattusite siia kogemata, klõpsake lihtsalt brauseri ''back''-nupule.",
"anontalkpagetext" => "---- ''See on arutelulehekülg anonüümse kasutaja kohta, kes ei ole loonud kontot või ei kasuta seda. Sellepärast tuleb meil kasutaja identifitseerimiseks kasutada tema [[IP-aadress]]i. See IP-aadress võib olla mitmele kasutajale ühine. Kui olete anonüümne kasutaja ning leiate, et kommentaarid sellel leheküljel ei ole mõeldud Teile, siis palun [[{{ns:4}}:Kasutaja sisselogimine|looge konto või logige sisse]], et edaspidi arusaamatusi vältida.'' ",
"noarticletext" => "(See lehekülg on praegu tühi)",
'clearyourcache' => "'''Märkus:''' Pärast salvestamist pead sa muudatuste nägemiseks oma brauseri puhvri tühjendama: '''Mozilla:''' vajuta ''reload''(või ''ctrl-r''), '''IE / Opera:''' ''ctrl-f5'', '''Safari:''' ''cmd-r'', '''Konqueror''' ''ctrl-r''.",
'usercssjsyoucanpreview' => "<strong>Vihje:</strong> Kasuta nuppu 'Näita eelvaadet' oma uue css/js testimiseks enne salvestamist.",
'usercsspreview' => "'''Ärge unustage, et seda versiooni teie isiklikust stiililehest pole veel salvestatud!'''",
'userjspreview' => "'''Ärge unustage, et see versioon teie isiklikust javascriptist on alles salvestamata!'''",

"updated"		=> "(Värskendatud)",
"note"			=> "<strong>Meeldetuletus:</strong> ",
"previewnote"	=> "Ärge unustage, et see versioon ei ole veel salvestatud!",
"previewconflict" => "See eelvaade näitab, kuidas ülemises toimetuskastis olev tekst hakkab välja nägema, kui otsustate salvestada.", ## redaktoriboks?
"editing"		=> "Redigeerimisel on $1",
"editconflict"	=> "Redigeerimiskonflikt: $1",
"explainconflict" => "Keegi teine on muutnud seda lehekülge pärast seda, kui Teie seda redigeerima hakkasite.
Ülemine toimetuskast sisaldab teksti viimast versiooni.
Teie muudatused on alumises kastis.
Teil tuleb need viimasesse versiooni üle viia.
Kui Te klõpsate nupule
 \"Salvesta\", siis salvestub <b>ainult</b> ülemises toimetuskastis olev tekst.\n<p>",
"yourtext"		=> "Teie tekst",
"storedversion" => "Salvestatud redaktsioon",
"editingold"	=> "<strong>ETTEVAATUST! Te redigeerite praegu selle lehekülje vana redaktsiooni.
Kui Te selle salvestate, siis lähevad kõik vahepealsed muudatused kaduma.</strong>\n",
"yourdiff"		=> "Erinevused",
"copyrightwarning" => "Pidage silmas, et kõik kaastööd loetakse avaldatuks vastavalt GNU Vaba Dokumentatsiooni Litsentsile
(Üksikasjad on leheküljel $1).
Kui Te ei soovi, et Teie poolt kirjutatut halastamatult redigeeritakse ja omal äranägemisel kasutatakse, siis ärge seda siia salvestage.<br />
Te kinnitate ka, et kirjutasite selle ise või võtsite selle kopeerimiskitsenduseta allikast. 
<strong>ÄRGE SAATKE AUTORIÕIGUSTEGA KAITSTUD MATERJALI ILMA LOATA!</strong>", # Vikipeedia võtsin välja, {{SITENAME}} paigutada kuidagi?
"longpagewarning" => "HOIATUS: Selle lehekülje pikkus ületab $1 kilobaiti. Mõne brauseri puhul valmistab raskusi juba 32-le kilobaidile läheneva pikkusega lehekülgede redigeerimine. Palun kaaluge selle lehekülje sisu jaotamist lühemate lehekülgede vahel.",
"readonlywarning" => "HOIATUS: Andmebaas on lukustatud hooldustöödeks, nii et praegu ei saa parandusi salvestada. Võite teksti alal hoida tekstifailina ning salvestada hiljem.",
"protectedpagewarning" => "HOIATUS:  See lehekülg on lukustatud, nii et seda saavad redigeerida ainult süsteemi operaatori õigustega kasutajad. Järgige juhtnööre leheküljel 
<a href='$wgScriptPath/{{ns:4}}:Juhtnöörid_kaitstud_lehekülje_kohta'>
</a>.",

/*
 </nowiki>
====History pages====
 <nowiki>
*/
# History pages
#
"revhistory"	=> "Redigeerimislugu",
"nohistory"		=> "Sellel leheküljel ei ole eelmisi redaktsioone.",
"revnotfound"	=> "Redaktsiooni ei leitud",
"revnotfoundtext" => "Teie poolt päritud vana redaktsiooni ei leitud.
Palun kontrollige aadressi, millel Te seda lehekülge leida püüdsite.\n",
"loadhist"		=> "Lehekülje ajaloo laadimine",
"currentrev"	=> "Viimane redaktsioon",
"revisionasof"	=> "Redaktsioon $1",
"cur"			=> "viim",
"next"			=> "järg",
"last"			=> "eel",
"orig"			=> "orig",
"histlegend"	=> "Märgi versioonid, mida tahad võrrelda ja vajuta võrdlemisnupule.
Legend: (viim) = erinevused võrreldes viimase redaktsiooniga,
(eel) = erinevused võrreldes eelmise redaktsiooniga, P = pisimuudatus",
'history_copyright'    => '-',
/*
 </nowiki>
====Diffs====
 <nowiki>
*/
# Diffs
#
"difference"	=> "(Erinevused redaktsioonide vahel)",
"loadingrev"	=> "Redaktsiooni laadimine erinevustelehekülje jaoks",
"lineno"		=> "Rida $1:",
"editcurrent"	=> "Redigeeri selle lehekülje viimast redaktsiooni",
'selectnewerversionfordiff' => 'Vali võrdlemiseks uuem versioon',
'selectolderversionfordiff' => 'Vali võrdlemiseks vanem versioon',
'compareselectedversions' => 'Võrdle valitud versioone',
/*
 </nowiki>
====Search results====
 <nowiki>
*/
# Search results
#
"searchresults" => "Otsingu tulemid",
"searchhelppage" => "{{ns:4}}:Otsing",
"searchingwikipedia" => "{{ns:4}} &ndash; Otsimine",
"searchresulttext" => "Lisainfot otsimise kohta vaata $1.",
"searchquery"	=> "Päring \"$1\"",
"badquery"		=> "Vigane päring",
"badquerytext"	=> "Teie päringut ei saanud menetleda.
Tõenäoliselt püüdsite otsida vähem kui kolme-tähelist sõna.
Selline otsing ei ole praegu veel võimalik. Võib ka olla,
et päring oli vigane, nt. \"koer and and kass\" ei ole lubatav.
Palun proovige teistsugust päringut.",
"matchtotals"	=> "Otsitud sõna \"$1\" leidub $2 artikli pealkirjas
ning $3 artikli tekstis.",
"nogomatch" => "Täpselt sellise pealkirjaga lehekülge ei ole, proovime täistekstotsingut.",
"titlematches"	=> "Tabamused artiklipealkirjades",
"notitlematches" => "Artiklipealkirjades tabamusi ei ole",
"textmatches"	=> "Tabamused artiklitekstides",
"notextmatches"	=> "Artiklitekstides tabamusi ei ole",
"prevn"			=> "eelmised $1",
"nextn"			=> "järgmised $1",
"viewprevnext"	=> "Näita ($1) ($2) ($3).",
"showingresults" => "Allpool näitame <b>$1</b> tulemit alates tulemist #<b>$2</b>.",
"nonefound"		=> "<strong>Märkus</strong>: otsingute ebaõnnestumise sagedaseks põhjuseks on asjaolu,
et väga sageli esinevaid sõnu ei võta süsteem otsimisel arvesse. Teine põhjus võib olla
mitme otsingusõna kasutamine (tulemusena ilmuvad ainult leheküljed, mis sisaldavad kõiki otsingusõnu).",
"powersearch" => "Otsi",
"powersearchtext" => "
Otsing nimeruumidest :<br />
$1<br />
$2 Loetle ümbersuunamisi &nbsp; Otsi $3 $9",

/*
 </nowiki>
====Preferences page====
 <nowiki>
*/

# Preferences page
#
"preferences"	=> "Teie eelistused",
"prefsnologin" => "Te ei ole sisse loginud",
"prefsnologintext"	=> "Et oma eelistusi seada, <a href=\"" .
 "{{localurle:Special:Userlogin}}\">tuleb Teil</a>
sisse logida.",
"prefslogintext" => "Te olete sisse loginud kasutajanimega \"$1\".
Teie sisemine ID-number on $2.",
"prefsreset"	=> "Teie eelistused on arvutimälu järgi taastatud.",
"qbsettings"	=> "Kiirriba sätted", 
"changepassword" => "Muuda parool",
"skin"			=> "Nahk",
"math"			=> "Valemite näitamine",
"math_failure"		=> "Arusaamatu süntaks",
"math_unknown_error"	=> "Tundmatu viga",
"math_unknown_function"	=> "Tundmatu funktsioon ",
"math_lexing_error"	=> "Väljalugemisviga",
"math_syntax_error"	=> "Süntaksiviga",
"saveprefs"		=> "Salvesta eelistused",
"resetprefs"	=> "Lähtesta eelistused",
"oldpassword"	=> "Vana parool",
"newpassword"	=> "Uus parool",
"retypenew"		=> "Sisestage uus parool uuesti",
"textboxsize"	=> "Redaktoriboksi suurus",
"rows"			=> "Ridade arv",
"columns"		=> "Veergude arv",
"searchresultshead" => "Otsingutulemite sätted",
"resultsperpage" => "Tulemeid leheküljel",
"contextlines"	=> "Ridu tulemis",
"contextchars"	=> "Konteksti pikkus real",
"stubthreshold" => "Nupu näitamise lävi",
"recentchangescount" => "Pealkirjade arv viimastes muudatustes",
"savedprefs"	=> "Teie eelistused on salvestatud.",
"timezonetext"	=> "Kohaliku aja ja serveri aja (maailmaaja) vahe tundides.",
"localtime"	=> "Kohalik aeg",
"timezoneoffset" => "Ajavahe",
"emailflag"		=> "Blokeeri e-kirjad teistelt kasutajatelt",

/*
 </nowiki>
====Recent changes====
 <nowiki>
*/

# Recent changes
#
"changes" => "muudatused",
"recentchanges" => "Viimased muudatused",
"recentchangestext" => "Jälgige sellel leheküljel viimaseid muudatusi.",
"rcloaderr"		=> "Viimaste muudatuste laadimine",
"rcnote"		=> "Allpool on esitatud viimased <strong>$1</strong> muudatust viimase <strong>$2</strong> päeva jooksul.",
"rcnotefrom"	=> "Allpool on esitatud muudatused alates <b>$2</b> (näidatakse kuni <b>$1</b> muudatust).",
"rclistfrom"	=> "Näita muudatusi alates $1",
# "rclinks"		=> "Näita viimast $1 muudatust viimase $2 tunni / viimase $3 päeva jooksul",
"rclinks"		=> "Näita viimast $1 muudatust viimase $2 päeva jooksul.",
"rchide"		=> "in $4 form; $1 pisiparandust; $2 sekundaarset nimeruumi; $3 multiple edits.", ##MIS SEE ON?
"diff"			=> "erin",
"hist"			=> "ajal",
"hide"			=> "peida",
"show"			=> "näita",
"tableform"		=> "tabel",
"listform"		=> "list",
"nchanges"		=> "$1 muudatust",
"minoreditletter" => "P",
"newpageletter" => "U",

/*
 </nowiki>
====Upload====
 <nowiki>
*/
# Upload
#
"upload"		=> "Faili üleslaadimine",
"uploadbtn"		=> "Üleslaadimine",
"uploadlink"	=> "Piltide üleslaadimine",
"reupload"		=> "Uuesti üleslaadimine",
"reuploaddesc"	=> "Tagasi üleslaadimise vormi juurde.",
"uploadnologin" => "sisse logimata",
"uploadnologintext"	=> "Kui Te soovite faile üles laadida, peate <a href=\"" .
  "{{localurle:Special:Userlogin}}\">sisse logima</a>.",
"uploadfile"	=> "Faili üleslaadimine",
"uploaderror"	=> "Viga üleslaadimisel",
"uploadtext"	=> "<strong>STOPP!</strong> Enne kui sooritad üleslaadimise,
peaksid tagama, et see järgib siinset [[{{ns:4}}:Image_use_policy|piltide kasutamise korda]].

Et näha või leida eelnevalt üleslaetud pilte,
mine vaata [[Special:Imagelist|piltide nimekirja]].
Üleslaadimised ning kustutamised logitakse [[Special:Log/upload|üleslaadimise logis]].

Järgneva vormi abil saad laadida üles uusi pilte
oma artiklite illustreerimiseks.
Enamikul brauseritest, näed nuppu \"Browse...\", mis viib sind
sinu operatsioonisüsteemi standardsesse failiavamisaknasse.
Faili valimisel sisestatakse selle faili nimi tekstiväljale
nupu kõrval.
Samuti pead märgistama kastikese, kinnitades sellega,
et sa ei riku seda faili üleslaadides kellegi autoriõigusi.
Üleslaadimise lõpuleviimiseks vajuta nupule \"Üleslaadimine\".
See võib võtta pisut aega, eriti kui teil on aeglane internetiühendus.

Eelistatud formaatideks on fotode puhul JPEG , joonistuste
ja ikoonilaadsete piltide puhul PNG, helide jaoks aga OGG.
Nimeta oma failid palun nõnda, et nad kirjeldaksid arusaadaval moel faili sisu, see aitab segadusi vältida.
Pildi lisamiseks artiklile, kasuta linki kujul:
<b><nowiki>[[image:pilt.jpg]]</nowiki></b> või <b><nowiki>[[image:pilt.png|alt. tekst]]</nowiki></b>.
Helifaili puhul: <b><nowiki>[[media:fail.ogg]]</nowiki></b>.

Pane tähele, et nagu ka ülejäänud siinsete lehekülgede puhul,
võivad teised sinu poolt laetud faile saidi huvides
muuta või kustutada ning juhul kui sa süsteemi kuritarvitad
võidakse sinu ligipääs sulgeda.",
"uploadlog"		=> "üleslaadimise logi",
"uploadlogpage" => "Upload_log",
"uploadlogpagetext" => "Järgnev on nimekiri viimastest üleslaadimistest.
Kellaajad on märgitud serveri ajaarvamise järele (UTC).
<ul>
</ul>
",
"filename"		=> "Faili nimi",
"filedesc"		=> "Lühikirjeldus",
"affirmation"	=> "Kinnitan, et selle faili autoriõiguste valdaja
on nõus seda litsenseerima tingimuste alusel, mille esitab $1.",
"copyrightpage" => "{{ns:4}}:Autoriõigused",
"copyrightpagename" => "{{SITENAME}} ja autoriõigused",
"uploadedfiles"	=> "Üleslaetud failid",
"noaffirmation" => "Te peate kinnitama, et Teie poolt üleslaetavad failid
ei riku kellegi autoriõigusi.",
"ignorewarning"	=> "Ignoreeri hoiatust ja salvesta sellest hoolimata.",
"minlength"		=> "Pildi nimi peab sisaldama vähemalt kolme tähte.",
"badfilename"	=> "Pildi nimi on muudetud. Uus nimi on \"$1\".",
"badfiletype"	=> "\".$1\" ei ole soovitatav formaat.",
"largefile"		=> "Soovitame mitte saata faile, mille suurus ületab 100 kilobaiti.",
"successfulupload" => "Üleslaadimine õnnestus",
"fileuploaded"	=> "Fail nimega \"$1\" õnnestus üles laadida.
Palun järgi seda linki: ($2). See viib su kirjelduslehele, et saaksid esita
asjassepuutuvad andmed faili kohta: kust on ta pärit, millal
ja kelle poolt ta loodi, jne.",
"uploadwarning" => "Hoiatus üleslaadimise asjus",
"savefile"		=> "Salvesta fail",
"uploadedimage" => "laadisin üles \"$1\"",

/*
 </nowiki>
====Image list====
 <nowiki>
*/
# Image list
#
"imagelist"		=> "Piltide loend",
"imagelisttext"	=> "Piltide arv järgnevas loendis: $1. Sorteeritud $2.", # $2 -- nt. "kuupäeva järgi".
"getimagelist"	=> "hangin piltide nimekirja",
"ilshowmatch"	=> "Näita kõiki pilte, mille nimi klapib",
"ilsubmit"		=> "Otsi",
"showlast"		=> "Näita viimast $1 pilti sorteerituna $2.", # $2 nt. "nime järgi"
"all"			=> "kõik",
"byname"		=> "nime järgi",
"bydate"		=> "kuupäeva järgi",
"bysize"		=> "suuruse järgi",
"imgdelete"		=> "del",
"imgdesc"		=> "kirj",
"imglegend"		=> "Legend: (kirj) = näita/redigeeri pildi kirjeldust.",
"imghistory"	=> "Pildi ajalugu",
"revertimg"		=> "taas",
"deleteimg"		=> "del",
"deleteimgcompletely"		=> "del",
"imghistlegend" => "Legend: (viim) = see on pildi viimane versioon, (del) = kustuta
see vana versioon, (taas) = taasta see vana versioon.
<br /><i>Klõpsa kuupäevale, et näha tookord laetud pilti.</i>.",
"imagelinks"	=> "Pildilingid",
"linkstoimage"	=> "Sellele pildile viitavad järgmised leheküljed:",
"nolinkstoimage" => "Selle pildile ei viita ükski lehekülg.",

/*
 </nowiki>
====Statistics====
 <nowiki>
*/
# Statistics
#
"statistics"	=> "Statistika",
"sitestats"		=> "Saidi statistika",
"userstats"		=> "Kasutaja statistika",
"sitestatstext" => "Lehekülgede koguarv andmebaasis: <b>$1</b>.

See arv hõlmab ka arutelulehekülgi, abiartikleid Vikipeedia kohta, väga lühikesi lehekülgi (nuppe), ümbersuunamislehekülgi ning muid lehekülgi, millel tõenäoliselt ei ole entsüklopeediaartikleid. Ilma neid arvestamata on Vikipeedias praegu <b>$2</b> lehekülge, mida võib pidada artikliteks.

Alates uuele programmile üleminekust 18. detsembril 2003 on lehekülgi vaadatud kokku <b>$3</b> korda ja redigeeritud kokku <b>$4</b> korda. Seega on lehekülje kohta tehtud <b>$5</b> parandust ja iga paranduse kohta tuleb <b>$5</b> vaatamist.", # viimase lausepoole võiks kohalikes seadetes eemaldada,
  # sest see kipub mingil põhjusel olema null, tõenäoliselt praegu külastusi lihtsalt kokku ei loeta.
  # enamasti on arvud.
  # Võiks veel ainsust silmas pidades ühtteist ümber sõnastada, aga see esineb tõesti üliharva.
"userstatstext" => "Registreeritud kasutajate arv: <b>$1</b>.
Administraatori staatuses kasutajaid: <b>$2</b> (vt $3).",
/*
 </nowiki>
====Maintenance Page====
 <nowiki>
*/
# Maintenance Page
#
"maintenance"		=> "Hoolduslehekülg",
"maintnancepagetext"	=> "This page includes several handy tools for everyday maintenance. Some of these functions tend to stress the database, so please do not hit reload after every item you fixed ;-)",
"maintenancebacklink"	=> "Tagasi hooldusleheküljele",
"disambiguations"	=> "Disambiguation pages",
"disambiguationspage"	=> "Wikipedia:Links_to_disambiguating_pages",
"disambiguationstext"	=> "The following articles link to a <i>disambiguation page</i>. They should link to the appropriate topic instead.<br />A page is treated as dismbiguation if it is linked from $1.<br />Links from other namespaces are <i>not</i> listed here.",
"doubleredirects"	=> "Double Redirects",
"doubleredirectstext"	=> "<b>Attention:</b> This list may contain false positives. That usually means there is additional text with links below the first #REDIRECT.<br />\nEach row contains links to the first and second redirect, as well as the first line of the second redirect text, usually giving the \"real\" taget article, which the first redirect should point to.",
"brokenredirects"	=> "Broken Redirects",
"brokenredirectstext"	=> "The following redirects link to a non-existing article.",
"selflinks"		=> "Iseendale viitavad leheküljed",
"selflinkstext"		=> "Järgmised leheküljed sisaldavad viita iseendale, mis ei ole soovitatav.",
"mispeelings"           => "Pages with misspellings",
"mispeelingstext"               => "The following pages contain a common misspelling, which are listed on $1. The correct spelling might be given (like this).",
"mispeelingspage"       => "List of common misspellings",
"missinglanguagelinks"  => "Missing Language Links",
"missinglanguagelinksbutton"    => "Find missing language links for",
"missinglanguagelinkstext"      => "These articles do <i>not</i> link to their counterpart in $1. Redirects and subpages are <i>not</i> shown.",

/*
 </nowiki>
====Miscellaneous special pages====
 <nowiki>
*/
# Miscellaneous special pages
#
"orphans"		=> "Üksildased artiklid",
"lonelypages"	=> "Üksildased artiklid",
"unusedimages"	=> "Kasutamata pildid",
"popularpages"	=> "Populaarsed leheküljed",
"nviews"		=> "Külastuste arv: $1",
"wantedpages"	=> "Kõige oodatumad artiklid",
"nlinks"		=> "Linkide arv: $1",
"allpages"		=> "Kõik artiklid",
"randompage"	=> "Juhuslik artikkel",
"shortpages"	=> "Lühikesed artiklid",
"longpages"		=> "Pikad artiklid",
"listusers"		=> "Kasutajad",
"specialpages"	=> "Erileheküljed",
"spheading"		=> "Erileheküljed",
/* aegunud
"sysopspheading" => "Administraatori erileheküljed",
"developerspheading" => "Arendaja erileheküljed",
*/
"protectpage"	=> "Kaitse lehekülge",
"recentchangeslinked" => "Seotud muudatused",
"rclsub"		=> "(lehekülgedel, millele \"$1\" viitab)", #
"debug"			=> "Silu",
"newpages"		=> "Uued leheküljed",
'ancientpages'          => 'Vanimad leheküljed',
"intl"		=> "Keeltevahelised lingid",
'move' => 'Teisalda',
"movethispage"	=> "Teisalda lehekülg",
"unusedimagestext" => "<p>Pange palun tähele, et teised
veebisaidid, nagu nt. rahvusvahelised Vikipeediad, võivad
linkida lehekülgedele otselinginga ja seega võivad
siin esitatud pildid olla ikkagi aktiivses kasutuses.",
"booksources"	=> "Raamatud",
"booksourcetext" => "All on esitatud linkide loend teistesse
saitidesse, mis müüvad uusi ja kasutatud raamatuid ning võivad
omada lisainfot otsitavate raamatute kohta.
{{ns:4}} ei ole nende ettevõtmistega seotud ja seda nimekirja
ei tohiks konstrueerida reklaami tegemiseks.",
"alphaindexline" => "$1 to $2",

/*
 </nowiki>
====Email this user====
 <nowiki>
*/
# Email this user
#
"mailnologin"	=> "No send address",
"mailnologintext" => "You must be [[Special:Userlogin|logged in]]
and have a valid e-mail address in your [[Special:Preferences|preferences]]
to send e-mail to other users.",
"emailuser"		=> "E-mail this user",
"emailpage"		=> "E-mail user",
"emailpagetext"	=> "If this user has entered a valid e-mail address in
his or her user preferences, the form below will send a single message.
The e-mail address you entered in your user preferences will appear
as the \"From\" address of the mail, so the recipient will be able
to reply.",
"noemailtitle"	=> "No e-mail address",
"noemailtext"	=> "This user has not specified a valid e-mail address,
or has chosen not to receive e-mail from other users.",
"emailfrom"		=> "From",
"emailto"		=> "To",
"emailsubject"	=> "Subject",
"emailmessage"	=> "Sõnum",
"emailsend"		=> "Saada",
"emailsent"		=> "E-post saadetud",
"emailsenttext" => "Teie sõnum on saadetud.",

/*
 </nowiki>
====Watchlist====
 <nowiki>
*/
# Watchlist
#
"watchlist"		=> "Minu jälgimisloend",
"watchlistsub"	=> "(kasutajale nimega \"$1\")",
"nowatchlist"	=> "Teie jälgimisloend on tühi.",
"watchnologin"	=> "Ei ole sisse loginud",
"watchnologintext"	=> "Jälgimisloendi muutmiseks peate [[Special:Userlogin|sisse logima]].",
"addedwatch"	=> "Lisatud jälgimisloendile",
"addedwatchtext" => "Lehekülg \"$1\" on lisatud Teie [[Special:Watchlist|jälgimisloendile]].
Edasised muudatused sellel lehel ja sellega seotud aruteluküljel reastatakse siin
ning [[Special:Recentchanges||viimaste muudatuste lehel]] tuuakse ta esile
<b>rasvase</b> kirja abil.</p>
<p>Kui tahad seda lehte hiljem jälgimisloendist eemaldada, klõpsa päisenupule \"Lõpeta jälgimine\".",
"removedwatch"	=> "Jälgimisloendist eemaldatud",
"removedwatchtext" => "Lehekülg pealkirjaga \"$1\" on Teie jälgimisloendist eemaldatud.",
'watch' => 'Jälgi',
"watchthispage"	=> "Jälgi seda lehekülge",
"unwatchthispage" => "Lõpeta jälgimine",
"notanarticle"	=> "Pole artikkel",

/*
 </nowiki>
====Delete/protect/revert====
 <nowiki>
*/
# Delete/protect/revert
#
"deletepage"	=> "Kustuta lehekülg",
"confirm"		=> "Kinnita",
"confirmdelete" => "Kinnita kustutamine",
"deletesub"		=> "(Kustutan lehekülje \"$1\")",
"confirmdeletetext" => "Sa oled andmebaasist jäädavalt kustutamas lehte või pilti
koos kogu tema ajalooga. Palud kinnita, et sa tahad seda tõepoolest teha, et
sa mõistad tagajärgi ja et sinu tegevus on kooskõlas siinse
[[{{ns:4}}:Policy|sisekorraga]].", # Wikipedia:Policy tuleks ka tõlkida
"confirmcheck"	=> "Jah, tahan tõesti kustutada.",
"actioncomplete" => "Toiming sooritatud",
"deletedtext"	=> "\"$1\" on kustutatud.
Viimaste kustutuste loendit näed siit: $2.",
"deletedarticle" => "\"$1\" kustutatud",
"dellogpage"	=> "Kustutatud_leheküljed",
"dellogpagetext" => "Allpool on esitatud nimekiri viimastest kustutamistest.
Kõik toodud kellaajad järgivad serveriaega (UTC).
<ul>
</ul>
",
"deletionlog"	=> "Kustutatud leheküljed",
"reverted"		=> "Pöörduti tagasi varasemale versioonile",
"deletecomment"	=> "Kustutamise põhjus",
"imagereverted" => "Varasemale versioonile tagasipöördumine õnnestus.",
"rollback"		=> "Pöördu varasemale versioonile",
"rollbacklink"	=> "taasta varasem versioon",
"cantrollback"	=> "Ei saa muudatusi tagasi pöörata; viimane kaastööline on artikli ainus autor.",
"revertpage"	=> "Pöörduti tagasi viimasele muudatusele, mille tegi $1",

/*
 </nowiki>
====Undelete====
 <nowiki>
*/
# Undelete
"undelete" => "Taasta kustutatud lehekülg",
"undeletepage" => "Kustutatud lehekülgede vaatamine ja taastamine",
"undeletepagetext" => "Järgnevad leheküljed on kustutatud, kuis arhiivis
veel olemas, neid saab taastada. Arhiivi sisu vistatakse aegajalt üle parda.",
"undeletearticle" => "Taasta kustutatud artikkel",
"undeleterevisions" => "Arhiveeritud versioone on $1.",
"undeletehistory" => "Kui taastate lehekülje, taastuvad kõik versioonid artikli
ajaloona. Kui vahepeal on loodud uus samanimeline lehekülg, ilmuvad taastatud
versioonid varasema ajaloona. Kehtivat versiooni automaatselt välja ei vahetata.",
"undeleterevision" => "Kustutatud versioon seisuga $1",
"undeletebtn" => "Taasta!",
"undeletedarticle" => "\"$1\" taastatud",
"undeletedtext"   => "Artikkel [[$1]] on taastatud.
Viimaste kustutamiste ja taastamiste logi on esitatud lehel [[{{ns:4}}:Kustutatud_leheküljed]].",

/*
 </nowiki>
====Contributions====
 <nowiki>
*/
# Contributions
#
"contributions"	=> "Kasutaja kaastööd",
"mycontris" => "Minu kaastöö",
"contribsub"	=> "Kasutaja \"$1\" jaoks",
"nocontribs"	=> "Antud kriteeriumile vastavaid muudatusi ei leidnud.",
"ucnote"		=> "Esitatakse selle kasutaja tehtud viimased <b>$1</b> muudatust viimase <b>$2</b> päeva jooksul.",
"uclinks"		=> "Näita viimast $1 muudatust; viimase $2 päeva jooksul.",
"uctop"		=> " (üles)" ,

/*
 </nowiki>
====What links here====
 <nowiki>
*/
# What links here
#
"whatlinkshere"	=> "Viidad siia",
"notargettitle" => "Puudub sihtlehekülg",
"notargettext"	=> "Sa ei ole esitanud sihtlehekülge ega kasutajat, kelle kallal seda operatsiooni toime panna.",
"linklistsub"	=> "(Linkide loend)",
"linkshere"		=> "Siia viitavad järgmised leheküljed:",
"nolinkshere"	=> "Siia ei viita ükski lehekülg.",
"isredirect"	=> "ümbersuunamislehekülg",

/*
 </nowiki>
====Block/unblock IP====
 <nowiki>
*/
# Block/unblock IP
#
"blockip"		=> "Blokeeri IP-aadress",
"blockiptext"	=> "Use the form below to block write access
from a specific IP address.
This should be done only only to prevent vandalism, and in
accordance with [[Wikipedia:Policy|Wikipedia policy]].
Fill in a specific reason below (for example, citing particular
pages that were vandalized).",
"ipaddress"		=> "IP-aadress",
"ipbreason"		=> "Põhjus",
"ipbsubmit"		=> "Blokeeri see aadress",
"badipaddress"	=> "The IP address is badly formed.",
"noblockreason" => "You must supply a reason for the block.",
"blockipsuccesssub" => "Block succeeded",
"blockipsuccesstext" => "IP-aadress \"$1\" on blokeeritud.
<br />See [[Special:Ipblocklist|IP block list]] to review blocks.",
"unblockip"		=> "Unblock IP address",
"unblockiptext"	=> "Use the form below to restore write access
to a previously blocked IP address.",
"ipusubmit"		=> "Unblock this address",
"ipusuccess"	=> "IP address \"$1\" unblocked",
"ipblocklist"	=> "Blokeeritud IP-aadresside loend",
"blocklistline"	=> "$1, $2 blocked $3",
"blocklink"		=> "blokeeri",
"unblocklink"	=> "unblock",
"contribslink"	=> "contribs",

/*
 </nowiki>
====Developer tools====
 <nowiki>
*/
# Developer tools
#
"lockdb"		=> "Lukusta andmebaas",
"unlockdb"		=> "Tee andmebaas lukust lahti",
"lockdbtext"	=> "Locking the database will suspend the ability of all
users to edit pages, change their preferences, edit their watchlists, and
other things requiring changes in the database.
Please confirm that this is what you intend to do, and that you will
unlock the database when your maintenance is done.",
"unlockdbtext"	=> "Unlocking the database will restore the ability of all
users to edit pages, change their preferences, edit their watchlists, and
other things requiring changes in the database.
Please confirm that this is what you intend to do.",
"lockconfirm"	=> "Yes, I really want to lock the database.",
"unlockconfirm"	=> "Yes, I really want to unlock the database.",
"lockbtn"		=> "Võta andmebaas kirjutuskaitse alla",
"unlockbtn"		=> "Taasta andmebaasi kirjutuspääs",
"locknoconfirm" => "You did not check the confirmation box.",
"lockdbsuccesssub" => "Andmebaas kirjutuskaitse all",
"unlockdbsuccesssub" => "Kirjutuspääs taastatud",
"lockdbsuccesstext" => "Andmebaas on nüüd kirjutuskaitse all.
<br />Kui Teie hooldustöö on läbi, ärge unustage kirjutuspääsu taastada!",
"unlockdbsuccesstext" => "Andmebaasi kirjutuspääs on taastatud.",

/*
 </nowiki>
====SQL query====
 <nowiki>
*/
# SQL query
#
"asksql"		=> "SQL query",
"asksqltext"	=> "Use the form below to make a direct query of the
Wikipedia database.
Use single quotes ('like this') to delimit string literals.
This can often add considerable load to the server, so please use
this function sparingly.",
"sqlquery"		=> "Enter query",
"querybtn"		=> "Submit query",
"selectonly"	=> "Queries other than \"SELECT\" are restricted to
Wikipedia developers.",
"querysuccessful" => "Query successful",

/*
 </nowiki>
====Move page====
 <nowiki>
*/
# Move page
#
"movepage"		=> "Teisalda artikkel",
"movepagetext"	=> "Using the form below will rename a page, moving all
of its history to the new name.
The old title will become a redirect page to the new title.
Links to the old page title will not be changed; be sure to
[[Special:Maintenance|check]] for double or broken redirects.
You are responsible for making sure that links continue to
point where they are supposed to go.

Note that the page will '''not''' be moved if there is already
a page at the new title, unless it is empty or a redirect and has no
past edit history. This means that you can rename a page back to where
it was just renamed from if you make a mistake, and you cannot overwrite
an existing page.

<b>ETTEVAATUST!</b>
Võimalik, et olete tegemas ootamatut ning drastilist muudatust väga loetavasse artiklisse;
enne muudatuse tegemist mõelge palun järele, milised võivad olla selle tagajärjed.",
"movepagetalktext" => "Koos artiklileheküljega teisaldatakse automaatselt ka arutelulehekülg, '''välja arvatud juhtudel, kui:'''
*liigutate lehekülge ühest nimeruumist teise,
*uue nime all on juba olemas mittetühi arutelulehekülg või
*jätate alumise kastikese märgistamata.

Neil juhtudel teisaldage arutelulehekülg soovi korral eraldi või ühendage ta omal käel uue aruteluleheküljega.",
"movearticle"	=> "Teisalda artiklilehekülg",
"movenologin"	=> "Te ei ole sisse loginud",
"movenologintext" => "Et lehekülge teisaldada, peate registreeruma
kasutajaks ja [[Special:Userlogin|sisse logima]]",
"newtitle"		=> "Uue pealkirja alla",
"movepagebtn"	=> "Teisalda artikkel",
"pagemovedsub"	=> "Artikkel on teisaldatud",
"pagemovedtext" => "Artikkel \"[[$1]]\" on teisaldatud pealkirja \"[[$2]]\" alla.",
"articleexists" => "Selle nimega artikkel on juba olemas või pole valitud nimi lubatav. Palun valige uus nimi.",
"talkexists"	=> "Artikkel on teisaldatud, kuid arutelulehekülge ei saanud teisaldada, sest uue nime all on arutelulehekülg juba olemas. Palun ühendage aruteluleheküljed ise.",
"movedto"		=> "Teisaldatud pealkirja alla:",
"movetalk"		=> "Teisalda ka \"arutelu\", kui saab.",
"talkpagemoved" => "Ka vastav arutelulehekülg on teisaldatud.",
"talkpagenotmoved" => "Vastav arutelulehekülg jäi teisaldamata.",
/*
 </nowiki>
====Matemaatilise teksti seadete valikud====
  <nowiki>
*/
 #Math
   'mw_math_png' => "Alati PNG",
   'mw_math_simple' => "Kui väga lihtne, siis HTML, muidu PNG",
   'mw_math_html' => "Võimaluse korral HTML, muidu PNG",
   'mw_math_source' => "Säilitada TeX (tekstibrauserite puhul)",
   'mw_math_modern' => "Soovitatav moodsate brauserite puhul",
   'mw_math_mathml' => 'MathML',

);

/*
 </nowiki>

===Ja pisut programmikoodi===
Eesti keele jaoks vaja kohaldada funktsioon ''date()''.
 <nowiki>
*/

require_once( "LanguageUtf8.php" );

class LanguageEt extends LanguageUtf8 {


	function getBookstoreList () {
		global $wgBookstoreListEt ;
		return $wgBookstoreListEt ;
	}

	function getNamespaces() {
		global $wgNamespaceNamesEt;
		return $wgNamespaceNamesEt;
	}

	function getNsText( $index ) {
		global $wgNamespaceNamesEt;
		return $wgNamespaceNamesEt[$index];
	}

	function getNsIndex( $text ) {
		global $wgNamespaceNamesEt;

		foreach ( $wgNamespaceNamesEt as $i => $n ) {
			if ( 0 == strcasecmp( $n, $text ) ) { return $i; }
		}
		return false;
	}

	function getQuickbarSettings() {
		global $wgQuickbarSettingsEt;
		return $wgQuickbarSettingsEt;
	}

	function getSkinNames() {
		global $wgSkinNamesEt;
		return $wgSkinNamesEt;
	}

	function getValidSpecialPages()
	{
		global $wgValidSpecialPagesEt;
		return $wgValidSpecialPagesEt;
	}

	function getSysopSpecialPages()
	{
		global $wgSysopSpecialPagesEt;
		return $wgSysopSpecialPagesEt;
	}

	function getDeveloperSpecialPages()
	{
		global $wgDeveloperSpecialPagesEt;
		return $wgDeveloperSpecialPagesEt;
	}

	function getMessage( $key ) {
		global $wgAllMessagesEt;
		if( isset( $wgAllMessagesEt[$key] ) ) {
			return $wgAllMessagesEt[$key];
		} else {
			return Language::getMessage( $key );
		}
	}

}
/*
 </nowiki>
*/
?>