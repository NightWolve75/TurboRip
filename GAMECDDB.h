#pragma pack(1)

#define USCODE " (U)"
#define JPCODE " (J)"
#define RGSIZE 5

PSTR CDTYPE[] = {
	"NEC PC-FX CD-ROM",        // 0
	"NEC PC Engine CD-ROM",    // 1
	"NEC TurboGrafx-16 CD-ROM" // 2
};

struct TOC_INFO {
	unsigned char CDType;
	char *CDTitle;
	unsigned long CDDBID;
};

TOC_INFO TOC_INFO_LIST[] = {
{
	1, // NEC PC Engine CD-ROM
	"J Thunder",
	0x0108C613
	},{
	1, // NEC PC Engine CD-ROM
	"Mamono Hunter Youko - Tooki Yobigoe",
	0x01094813
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Pyramid Plunder",
	0x02000D01
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Hawiian Island Girls",
	0x02000F01
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"The Local Girls of Hawaii",
	0x02001201
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Bikini Girls",
	0x02003B01
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"The Addams Family",
	0x02004C01
	},{
	0, // NEC PC-FX CD-ROM
	"Nnyuu [GANHE001B]  - Disc 2",
	0x02007E01
	},{
	1, // NEC PC Engine CD-ROM
	"Startling Odyssey",
	0x0209CA15
	},{
	1, // NEC PC Engine CD-ROM
	"Fire Pro Jyoshi - Dome Choujyo Taisen",
	0x020ABF14
	},{
	1, // NEC PC Engine CD-ROM
	"Exile - Toki no Hasama he",
	0x020B5327
	},{
	1, // NEC PC Engine CD-ROM
	"IQ Panic",
	0x020D534F
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Sherlock Holmes Consulting Detective",
	0x020F3201
	},{
	1, // NEC PC Engine CD-ROM
	"Sherlock Holmes no Tantei Kouza",
	0x020F3601
	},{
	1, // NEC PC Engine CD-ROM
	"Sherlock Holmes no Tantei Kouza II",
	0x020F8301
	},{
	1, // NEC PC Engine CD-ROM
	"Advanced Variable Geo",
	0x030AE825
	},{
	1, // NEC PC Engine CD-ROM
	"Nemurenumori no Chiisana Ohanashi",
	0x030DE945
	},{
	1, // NEC PC Engine CD-ROM
	"Dungeon Explorer II",
	0x03106322
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Loom",
	0x04064317
	},{
	1, // NEC PC Engine CD-ROM
	"Flash Hiders Taikenban",
	0x05057316
	},{
	1, // NEC PC Engine CD-ROM
	"Dungeon Master - Theron's Quest",
	0x05094813
	},{
	0, // NEC PC-FX CD-ROM
	"Tonari no Princess Rolfee [FXNHE741]",
	0x05113102
	},{
	0, // NEC PC-FX CD-ROM
	"Anime Freak FX Vol.2 [FXNHE513]",
	0x060FF602
	},{
	0, // NEC PC-FX CD-ROM
	"Anime Freak FX Vol.5 [FXNHE738] - Disc 1",
	0x070C0902
	},{
	0, // NEC PC-FX CD-ROM
	"Ginga Ojousama Densetsu Yuna FX - Kanashimi no Selene [FXHUD506]",
	0x070FB602
	},{
	1, // NEC PC Engine CD-ROM
	"Urusei Yatsura - Stay with You - Bonus CD Audio",
	0x07100613
	},{
	1, // NEC PC Engine CD-ROM
	"The Davis Cup Tennis",
	0x08061615
	},{
	1, // NEC PC Engine CD-ROM
	"Nekketsu Legend Baseball",
	0x0807CB14
	},{
	0, // NEC PC-FX CD-ROM
	"Return to Zork [FXNHE505]",
	0x0808A802
	},{
	0, // NEC PC-FX CD-ROM
	"All Japan Female Pro Wrestle - Queen of Queens [FXNHE503] - Disc A",
	0x080D3C02
	},{
	0, // NEC PC-FX CD-ROM
	"Cutey Honey FX [FXNHE511]",
	0x080EC102
	},{
	0, // NEC PC-FX CD-ROM
	"Miraculum - The Last Revelation [FXNHE617]",
	0x08107102
	},{
	0, // NEC PC-FX CD-ROM
	"All Japan Female Pro Wrestle - Queen of Queens [FXNHE503] - Disc B",
	0x08110102
	},{
	0, // NEC PC-FX CD-ROM
	"Zoku Hatukoi Monogatari [FXNHE629] - Disc A",
	0x08114C02
	},{
	0, // NEC PC-FX CD-ROM
	"Zoku Hatukoi Monogatari [FXNHE629] - Disc B",
	0x08115802
	},{
	1, // NEC PC Engine CD-ROM
	"Cyber City OEDO 808",
	0x090D0358
	},{
	0, // NEC PC-FX CD-ROM
	"Fire Woman Matoi-Gumi [FXNHE634]",
	0x090D6C02
	},{
	1, // NEC PC Engine CD-ROM
	"Garou Densetsu II - Aratanaru Tatakai",
	0x090D9014
	},{
	0, // NEC PC-FX CD-ROM
	"Anime Freak FX Vol.2 - Sample Disc [No Serial]",
	0x09102202
	},{
	0, // NEC PC-FX CD-ROM
	"Anime Freak FX Vol.1 [FXNHE510]",
	0x0910D702
	},{
	0, // NEC PC-FX CD-ROM
	"Kokuu Hyouryuu Nirgends [FXNHE625]",
	0x09113202
	},{
	1, // NEC PC Engine CD-ROM
	"Bomberman '94 Taikenban",
	0x0B002F02
	},{
	0, // NEC PC-FX CD-ROM
	"Tenchi Muyo FX [FXNIC601] - Disc B",
	0x0B03DB02
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Hyper Catalog Duo-RX - Disc A",
	0x0B0B6B23
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Hyper Catalog VI - Disc A",
	0x0B0B6B23
	},{
	0, // NEC PC-FX CD-ROM
	"Anime Freak FX Vol.5 [FXNHE738] - Disc 2",
	0x0B0DF302
	},{
	0, // NEC PC-FX CD-ROM
	"Anime Freak FX Vol.6 [FXNHE845] - Disc 2",
	0x0B0FF802
	},{
	0, // NEC PC-FX CD-ROM
	"Anime Freak FX Vol.4 [FXNHE636]",
	0x0B104902
	},{
	1, // NEC PC Engine CD-ROM
	"Slime World",
	0x0C062614
	},{
	1, // NEC PC Engine CD-ROM
	"Galaxy Fraulein Yuna - HuVideo CD",
	0x0C0A9A02
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Sherlock Holmes Consulting Detective Volume II",
	0x0C0F9602
	},{
	0, // NEC PC-FX CD-ROM
	"Anime Freak FX Vol.3 [FXNHE621]",
	0x0C10CA02
	},{
	1, // NEC PC Engine CD-ROM
	"De Ja",
	0x0C113202
	},{
	1, // NEC PC Engine CD-ROM
	"Ultrabox IV Go",
	0x0D091324
	},{
	1, // NEC PC Engine CD-ROM
	"Valis III - The Fantasm Soldier",
	0x0D0CA648
	},{
	0, // NEC PC-FX CD-ROM
	"Anime Freak FX Vol.6 [FXNHE845] - Disc 1",
	0x0D0F2002
	},{
	1, // NEC PC Engine CD-ROM
	"Steam Heart's",
	0x0D103F24
	},{
	0, // NEC PC-FX CD-ROM
	"Last Imperial Prince [FXNHE635] - Disc B",
	0x0D109B16
	},{
	1, // NEC PC Engine CD-ROM
	"Pastel Lime",
	0x0E045414
	},{
	1, // NEC PC Engine CD-ROM
	"Super Darius II",
	0x0E0C3A13
	},{
	1, // NEC PC Engine CD-ROM
	"Dragon Knight II",
	0x0E0EEE24
	},{
	0, // NEC PC-FX CD-ROM
	"Tenchi Muyo FX [FXNIC601] - Disc A",
	0x0E0FB902
	},{
	1, // NEC PC Engine CD-ROM
	"Deden no Den",
	0x0F003502
	},{
	1, // NEC PC Engine CD-ROM
	"Psychic Detective Series Vol. III - Aya - Auto Demo",
	0x0F009302
	},{
	1, // NEC PC Engine CD-ROM
	"Dragon Knight III",
	0x100D6E13
	},{
	1, // NEC PC Engine CD-ROM
	"Loom",
	0x11064817
	},{
	0, // NEC PC-FX CD-ROM
	"Blue Breaker [FXNHE622]",
	0x12106702
	},{
	0, // NEC PC-FX CD-ROM
	"Blue Breaker [FXNHE622]",
	0x12106702
	},{
	1, // NEC PC Engine CD-ROM
	"Fiend Hunter",
	0x130BC016
	},{
	1, // NEC PC Engine CD-ROM
	"Super Albatross",
	0x130C093B
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Valis II",
	0x140AC637
	},{
	1, // NEC PC Engine CD-ROM
	"Spriggan Mark II - Re Terraform Project",
	0x140E2625
	},{
	0, // NEC PC-FX CD-ROM
	"Der Langrisser FX [FXNHE618]",
	0x14113303
	},{
	1, // NEC PC Engine CD-ROM
	"Emerald Dragon Taikenban",
	0x150AA114
	},{
	1, // NEC PC Engine CD-ROM
	"Road Spirits",
	0x150ADF15
	},{
	1, // NEC PC Engine CD-ROM
	"Mine Sweeper",
	0x16096818
	},{
	0, // NEC PC-FX CD-ROM
	"Blue Chicago Blues [FXNHE512] - Disc A",
	0x160C9E03
	},{
	1, // NEC PC Engine CD-ROM
	"Last Armageddon {BRCD0001-7-0904-R1D}",
	0x170AB313
	},{
	1, // NEC PC Engine CD-ROM
	"Kidou Keisatsu Patlabor - Chapter of Griffon",
	0x170E2F29
	},{
	1, // NEC PC Engine CD-ROM
	"Tekipaki Working Love",
	0x170EB014
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Beyond Shadowgate",
	0x180A5924
	},{
	1, // NEC PC Engine CD-ROM
	"Gate of Thunder",
	0x180B2C12
	},{
	1, // NEC PC Engine CD-ROM
	"Zan - Kagerou no Toki",
	0x180BBB52
	},{
	1, // NEC PC Engine CD-ROM
	"Aurora Quest - Otaku no Seiza",
	0x190B5D14
	},{
	1, // NEC PC Engine CD-ROM
	"Akumajou Dracula X - Chi no Rondo",
	0x190B8616
	},{
	1, // NEC PC Engine CD-ROM
	"Princess Maker I - Drama CD Audio",
	0x1A03DD03
	},{
	1, // NEC PC Engine CD-ROM
	"Wizardry V - Heart of the Maelstrom",
	0x1A08F25D
	},{
	1, // NEC PC Engine CD-ROM
	"Sotsugyou II - Neo Generation",
	0x1A0BB313
	},{
	1, // NEC PC Engine CD-ROM
	"Tecmo World Cup Super Soccer",
	0x1B08C515
	},{
	1, // NEC PC Engine CD-ROM
	"Mahjong on the Beach",
	0x1C077913
	},{
	1, // NEC PC Engine CD-ROM
	"Nekketsu Koushinkyoku - Soreyuke Daiundoukai",
	0x1C0A2B36
	},{
	1, // NEC PC Engine CD-ROM
	"Princess Maker I",
	0x1C104034
	},{
	1, // NEC PC Engine CD-ROM
	"CD Battle Hikari no Yuushatachi",
	0x1E01C204
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Meteor Blaster DX",
	0x1E111315
	},{
	1, // NEC PC Engine CD-ROM
	"Monster Maker - Yami no Ryuukishi",
	0x1F112B23
	},{
	1, // NEC PC Engine CD-ROM
	"Downtown Nekketsu Monogatari",
	0x2008CF17
	},{
	1, // NEC PC Engine CD-ROM
	"Quiz de Gakuensai",
	0x200CA34C
	},{
	1, // NEC PC Engine CD-ROM
	"CD Mahjong Bishoujo Chuushinha",
	0x2105D014
	},{
	1, // NEC PC Engine CD-ROM
	"Ranma ½",
	0x210E2537
	},{
	1, // NEC PC Engine CD-ROM
	"Basted",
	0x220D9A27
	},{
	0, // NEC PC-FX CD-ROM
	"First Kiss Monogatari [FXHNX801]",
	0x2210ED04
	},{
	1, // NEC PC Engine CD-ROM
	"Shin Onryou Senki",
	0x23052A04
	},{
	1, // NEC PC Engine CD-ROM
	"Ranma ½ Toraware no Hanayome",
	0x2305E725
	},{
	1, // NEC PC Engine CD-ROM
	"Dragon Half",
	0x2308E216
	},{
	1, // NEC PC Engine CD-ROM
	"Cosmic Fantasy - Bouken Shounen Yuu",
	0x23092C63
	},{
	1, // NEC PC Engine CD-ROM
	"Kisou Louga",
	0x230CA516
	},{
	1, // NEC PC Engine CD-ROM
	"AV Tanjou",
	0x24076946
	},{
	0, // NEC PC-FX CD-ROM
	"Mahjong Gokuu Tenjiku [FXNHE402]",
	0x240DC829
	},{
	1, // NEC PC Engine CD-ROM
	"Super Schwartzschild II",
	0x240DE116
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Lords of Thunder",
	0x250CDB17
	},{
	0, // NEC PC-FX CD-ROM
	"Blue Chicago Blues [FXNHE512] - Disc B",
	0x2510CB04
	},{
	0, // NEC PC-FX CD-ROM
	"Dragon Knight IV [FXNAV603]",
	0x270B5B04
	},{
	0, // NEC PC-FX CD-ROM
	"Farland Story FX [FXNHE628]",
	0x270CA005
	},{
	1, // NEC PC Engine CD-ROM
	"Sexy Idol Mahjong",
	0x2807B115
	},{
	1, // NEC PC Engine CD-ROM
	"Solid Force",
	0x280E7915
	},{
	1, // NEC PC Engine CD-ROM
	"Magical Saurus Tour",
	0x29078A04
	},{
	1, // NEC PC Engine CD-ROM
	"Quiz Marugoto the World",
	0x2B06C316
	},{
	1, // NEC PC Engine CD-ROM
	"Mystic Formula",
	0x2B0AD32C
	},{
	1, // NEC PC Engine CD-ROM
	"Garou Densetsu Special",
	0x2B10AB17
	},{
	1, // NEC PC Engine CD-ROM
	"Hyper Dyne SideArms Special",
	0x2C0AEF17
	},{
	1, // NEC PC Engine CD-ROM
	"Pomping World",
	0x2D05E018
	},{
	1, // NEC PC Engine CD-ROM
	"Cal III",
	0x2D089E05
	},{
	1, // NEC PC Engine CD-ROM
	"CD Pachisuro Bishoujo Gambler",
	0x2E05002C
	},{
	0, // NEC PC-FX CD-ROM
	"Pia Carrot he Youkoso [FXNKT701]",
	0x2E106105
	},{
	0, // NEC PC-FX CD-ROM
	"Cocktail Pack - Pia Carrot he Youkoso [FXNKT702]",
	0x2E106105
	},{
	0, // NEC PC-FX CD-ROM
	"Kishin Douji Zenki FX [FXHUD505]",
	0x2E112414
	},{
	1, // NEC PC Engine CD-ROM
	"Last Armageddon {BRCD0001-6-0625-R3D}",
	0x2F0AB513
	},{
	1, // NEC PC Engine CD-ROM
	"Last Armageddon {BRCD0001-6-0625-R1F}",
	0x2F0AB513
	},{
	1, // NEC PC Engine CD-ROM
	"Ryuuko no Ken",
	0x2F0AC320
	},{
	1, // NEC PC Engine CD-ROM
	"Ryuuko no Ken - Sample Disc",
	0x2F0AC320
	},{
	1, // NEC PC Engine CD-ROM
	"Garou Densetsu II - Aratanaru Tatakai - Sample Disc",
	0x2F0DEC15
	},{
	1, // NEC PC Engine CD-ROM
	"Himitsu no Hanazono",
	0x30033B04
	},{
	1, // NEC PC Engine CD-ROM
	"Wrestling Angels - Double Impact",
	0x300A5305
	},{
	1, // NEC PC Engine CD-ROM
	"Human Sports Festival",
	0x3109F117
	},{
	0, // NEC PC-FX CD-ROM
	"Megami Paradise II [FXNHE623]",
	0x310FFB04
	},{
	1, // NEC PC Engine CD-ROM
	"Baby Jo",
	0x32080F05
	},{
	1, // NEC PC Engine CD-ROM
	"Jantei Monogatari",
	0x3302ED05
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Magical Dinosaur Tour",
	0x33077C04
	},{
	1, // NEC PC Engine CD-ROM
	"Ai Chou Aniki",
	0x330BA515
	},{
	1, // NEC PC Engine CD-ROM
	"Virgin Dream",
	0x34040B05
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Dragon Slayer - The Legend of Heroes",
	0x340EA116
	},{
	1, // NEC PC Engine CD-ROM
	"Super Real Mahjong P II & III Custom",
	0x350B7E4B
	},{
	0, // NEC PC-FX CD-ROM
	"Tyoushin Heiki Zeroigar [FXNHE624]",
	0x350E0405
	},{
	1, // NEC PC Engine CD-ROM
	"Double Dragon II - The Revenge",
	0x360B851A
	},{
	1, // NEC PC Engine CD-ROM
	"Psychic Storm",
	0x360BC519
	},{
	1, // NEC PC Engine CD-ROM
	"Super Real Mahjong P IV Custom",
	0x38087227
	},{
	1, // NEC PC Engine CD-ROM
	"Asuka 120% Maxima Burning Fest",
	0x3809EE19
	},{
	1, // NEC PC Engine CD-ROM
	"Nobunaga no Yabou - Bushou Fuuunroku",
	0x380AC648
	},{
	1, // NEC PC Engine CD-ROM
	"Flash Hiders",
	0x380DCA2C
	},{
	1, // NEC PC Engine CD-ROM
	"Princess Maker II",
	0x38110516
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Buster Bros.",
	0x3905E318
	},{
	1, // NEC PC Engine CD-ROM
	"Top O Nerae! GunBuster Volume I",
	0x39073005
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Super Air Zonk",
	0x390C6D18
	},{
	1, // NEC PC Engine CD-ROM
	"CD Denjin Rockabilly Tengoku",
	0x390C6D18
	},{
	1, // NEC PC Engine CD-ROM
	"Angels II - Holy Night",
	0x3A02D405
	},{
	1, // NEC PC Engine CD-ROM
	"Yawara! II",
	0x3A08FD06
	},{
	1, // NEC PC Engine CD-ROM
	"Fray CD Xak Gaiden",
	0x3A0C5638
	},{
	1, // NEC PC Engine CD-ROM
	"Dragon Slayer - Eiyuu Densetsu",
	0x3A0E9C16
	},{
	1, // NEC PC Engine CD-ROM
	"A. III",
	0x3B00D106
	},{
	1, // NEC PC Engine CD-ROM
	"Seiryuu Densetsu Monbit",
	0x3B042105
	},{
	1, // NEC PC Engine CD-ROM
	"Bishoujo Senshi Sailor Moon",
	0x3B0C2618
	},{
	1, // NEC PC Engine CD-ROM
	"Cosmic Fantasy IV - Ginga Shounen Densetsu Gekitou Hen",
	0x3B0E2A3B
	},{
	1, // NEC PC Engine CD-ROM
	"Mad Stalker - Full Metal Force - Sample Disc",
	0x3C01FB06
	},{
	1, // NEC PC Engine CD-ROM
	"Bakuretsu Hunter",
	0x3C025405
	},{
	1, // NEC PC Engine CD-ROM
	"Develo Starter Kit - Basic",
	0x3C042606
	},{
	1, // NEC PC Engine CD-ROM
	"Urusei Yatsura - Stay with You",
	0x3C0B3905
	},{
	1, // NEC PC Engine CD-ROM
	"Kaze Kiri - Ninja Action",
	0x3C0C6918
	},{
	1, // NEC PC Engine CD-ROM
	"Winds of Thunder",
	0x3C0CDD17
	},{
	0, // NEC PC-FX CD-ROM
	"Minimum Nanonic [FXNHE631]",
	0x3D0D0505
	},{
	1, // NEC PC Engine CD-ROM
	"Dennou Tenshi - Digital Angel",
	0x3E0E1805
	},{
	1, // NEC PC Engine CD-ROM
	"Super Cd-Rom² Rpg Sampler",
	0x3F0A0A19
	},{
	0, // NEC PC-FX CD-ROM
	"Battle Heat [FXHUD401]",
	0x3F103D16
	},{
	1, // NEC PC Engine CD-ROM
	"Sugoroku '92 Nari Tore Nariagari Trendy",
	0x40025207
	},{
	1, // NEC PC Engine CD-ROM
	"Master of Monsters",
	0x40032E05
	},{
	1, // NEC PC Engine CD-ROM
	"Jantei Monogatari II - Uchuu Tantei Divan Kanketsu Hen",
	0x40052005
	},{
	1, // NEC PC Engine CD-ROM
	"Forgotten Worlds",
	0x410CB419
	},{
	1, // NEC PC Engine CD-ROM
	"Adventure Quiz - Capcom World & Hatena no Daibouken",
	0x4301F007
	},{
	1, // NEC PC Engine CD-ROM
	"Lodoss Tousenki - Record of Lodoss War",
	0x4303FE06
	},{
	1, // NEC PC Engine CD-ROM
	"Lodoss Tousenki - Record of Lodoss War Taikenban",
	0x4303FE06
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Lords of the Rising Sun",
	0x430A8E2A
	},{
	1, // NEC PC Engine CD-ROM
	"Langrisser - Hikari no Matsuei",
	0x430C6626
	},{
	1, // NEC PC Engine CD-ROM
	"Jantei Monogatari II - Uchuu Tantei Divan Shutsudou Hen",
	0x44054205
	},{
	1, // NEC PC Engine CD-ROM
	"J. League Tremendous Soccer '94",
	0x440C8F1A
	},{
	1, // NEC PC Engine CD-ROM
	"Ranma ½ Datou, Ganso Musabetsu Kakutou-Ryuu!",
	0x440E1A48
	},{
	1, // NEC PC Engine CD-ROM
	"Gambler Jikochuushinha - Mahjong Puzzle Collection",
	0x45081716
	},{
	1, // NEC PC Engine CD-ROM
	"Pachiokun III - Pachisuro & Pachinko",
	0x450A1616
	},{
	0, // NEC PC-FX CD-ROM
	"Chip Can Kick! [FXNHE626]",
	0x450E5117
	},{
	1, // NEC PC Engine CD-ROM
	"Slot Gambler",
	0x46071115
	},{
	1, // NEC PC Engine CD-ROM
	"Police Connection",
	0x46085118
	},{
	1, // NEC PC Engine CD-ROM
	"Cosmic Fantasy IV - Ginga Shounen Densetsu Totsunyuu Hen",
	0x460E4549
	},{
	1, // NEC PC Engine CD-ROM
	"Ruin - Kami no Isan",
	0x490BD717
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"3 in 1 DUO Demo CD",
	0x4A0BDE1A
	},{
	1, // NEC PC Engine CD-ROM
	"Tengai Makyo - Ziria [Hibaihin]",
	0x4B083106
	},{
	1, // NEC PC Engine CD-ROM
	"Tengai Makyo - Ziria",
	0x4B083106
	},{
	1, // NEC PC Engine CD-ROM
	"Dekoboko Densetsu - Hashire Wagamanma",
	0x4C095D1A
	},{
	1, // NEC PC Engine CD-ROM
	"Valis II - The Fantasm Soldier",
	0x4C0B3738
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"4 in 1 Super CD",
	0x4C0BCC17
	},{
	1, // NEC PC Engine CD-ROM
	"Star Parodia",
	0x4C0E8118
	},{
	1, // NEC PC Engine CD-ROM
	"Cal II",
	0x4D07AB07
	},{
	1, // NEC PC Engine CD-ROM
	"Space Invaders - The Original Game",
	0x4E02B507
	},{
	1, // NEC PC Engine CD-ROM
	"Browning",
	0x4E096F1B
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Last Alert",
	0x4E0B7F3C
	},{
	1, // NEC PC Engine CD-ROM
	"Tengai Makyo - Fuun Kabuki Den Shutsugeki no Sho",
	0x50086808
	},{
	1, // NEC PC Engine CD-ROM
	"Kagamai no Kuni no Legend",
	0x500AF507
	},{
	1, // NEC PC Engine CD-ROM
	"Image Fight II",
	0x500B2B19
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Godzilla",
	0x51093618
	},{
	1, // NEC PC Engine CD-ROM
	"Black Hole Assault",
	0x510BE418
	},{
	1, // NEC PC Engine CD-ROM
	"Hatsukoi Monogatari",
	0x510D6006
	},{
	1, // NEC PC Engine CD-ROM
	"Quiz Avenue",
	0x52045507
	},{
	1, // NEC PC Engine CD-ROM
	"Bakuden Unbalance Zone",
	0x5206EE26
	},{
	1, // NEC PC Engine CD-ROM
	"Yawara!",
	0x52085607
	},{
	1, // NEC PC Engine CD-ROM
	"Dragon Slayer - Eiyuu Densetsu II",
	0x520F8A18
	},{
	0, // NEC PC-FX CD-ROM
	"Ruruli Ra Rura [FXNHE627]",
	0x5310C815
	},{
	1, // NEC PC Engine CD-ROM
	"Red Alert",
	0x540B9F3A
	},{
	1, // NEC PC Engine CD-ROM
	"Quiz no Hoshi",
	0x540BEF2C
	},{
	1, // NEC PC Engine CD-ROM
	"Develo Magazine Volume I",
	0x5509602B
	},{
	1, // NEC PC Engine CD-ROM
	"Doukyuusei",
	0x550C1F07
	},{
	1, // NEC PC Engine CD-ROM
	"Develo Starter Kit - Assembler",
	0x56043D08
	},{
	1, // NEC PC Engine CD-ROM
	"3 x 3 Eyes - Sanjiyan Hensei",
	0x560BCF07
	},{
	1, // NEC PC Engine CD-ROM
	"Mirai Shonen Conan",
	0x560C2839
	},{
	1, // NEC PC Engine CD-ROM
	"Tadaima Yuusha Boshuuchuu - Sample Disc",
	0x5703A109
	},{
	1, // NEC PC Engine CD-ROM
	"Hihou Densetsu Chris no Bouken",
	0x570BB91B
	},{
	1, // NEC PC Engine CD-ROM
	"Faerie Dust Story - Meikyuu no Elfeene",
	0x570D3A60
	},{
	1, // NEC PC Engine CD-ROM
	"Galaxy Fraulein Yuna II - Eien no Princess",
	0x57106017
	},{
	1, // NEC PC Engine CD-ROM
	"Builderland",
	0x58047A08
	},{
	1, // NEC PC Engine CD-ROM
	"Tanjou Debut",
	0x58112A27
	},{
	1, // NEC PC Engine CD-ROM
	"Quiz Avenue II",
	0x59063C08
	},{
	1, // NEC PC Engine CD-ROM
	"Blood Gear",
	0x5906FD2D
	},{
	1, // NEC PC Engine CD-ROM
	"Laser Soft Visual Collection Volume I - Cosmic Fantasy Visual Shuu",
	0x590DD05D
	},{
	1, // NEC PC Engine CD-ROM
	"Tokimeki Memorial {HRKM70414-1FAAT}",
	0x590DDB08
	},{
	1, // NEC PC Engine CD-ROM
	"Tokimeki Memorial {HRKM70217-4FAAT}",
	0x590DDB08
	},{
	1, // NEC PC Engine CD-ROM
	"Tokimeki Memorial {HRKM70701-2FAAT}",
	0x590DDB08
	},{
	1, // NEC PC Engine CD-ROM
	"Tokimeki Memorial {HRKM71014-3FABT}",
	0x590DDB08
	},{
	1, // NEC PC Engine CD-ROM
	"Tokimeki Memorial {HRKM70414-1FABT}",
	0x590DDB08
	},{
	1, // NEC PC Engine CD-ROM
	"Tokimeki Memorial {HRKM71014-3FAAT}",
	0x590DDB08
	},{
	1, // NEC PC Engine CD-ROM
	"Genocide",
	0x5A0AFC18
	},{
	1, // NEC PC Engine CD-ROM
	"Nobunaga no Yabou Zenkokuban",
	0x5B055C09
	},{
	1, // NEC PC Engine CD-ROM
	"Death Bringer - The Knight of Darkness",
	0x5C05791C
	},{
	1, // NEC PC Engine CD-ROM
	"Bazaru Degozaru no Game Degozaru",
	0x5C091B1C
	},{
	1, // NEC PC Engine CD-ROM
	"Juuouki",
	0x5D023509
	},{
	1, // NEC PC Engine CD-ROM
	"Hellfire S - The Another Story",
	0x5D0B1818
	},{
	1, // NEC PC Engine CD-ROM
	"World Heroes II",
	0x5D0C3029
	},{
	1, // NEC PC Engine CD-ROM
	"World Heroes II - Sample Disc",
	0x5D0C3029
	},{
	1, // NEC PC Engine CD-ROM
	"Godzilla - Bakutou Retsuden",
	0x5E094E18
	},{
	1, // NEC PC Engine CD-ROM
	"Ultrabox III Go",
	0x5E0B4E5C
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Forgotten Worlds",
	0x5E0CA619
	},{
	0, // NEC PC-FX CD-ROM
	"Sotsugyou II - Neo Generation FX [FXNHE401]",
	0x5E0CAD07
	},{
	1, // NEC PC Engine CD-ROM
	"Super Daisenryaku",
	0x5F02AA09
	},{
	1, // NEC PC Engine CD-ROM
	"Quiz Avenue III",
	0x5F07E708
	},{
	1, // NEC PC Engine CD-ROM
	"Lemmings",
	0x5F0A0E19
	},{
	1, // NEC PC Engine CD-ROM
	"Super Mahjong Taikai",
	0x5F0CB43D
	},{
	1, // NEC PC Engine CD-ROM
	"Shougi Database Kiyuu",
	0x6005B508
	},{
	0, // NEC PC-FX CD-ROM
	"Akazukin Cha Cha [FXNHE630]",
	0x600B7906
	},{
	1, // NEC PC Engine CD-ROM
	"Psychic Detective Series Vol. IV - Orgel",
	0x62057E08
	},{
	1, // NEC PC Engine CD-ROM
	"Sol Moonarge",
	0x620B9317
	},{
	1, // NEC PC Engine CD-ROM
	"Daisenpuu Custom",
	0x6408C609
	},{
	1, // NEC PC Engine CD-ROM
	"Summer Carnival '93 - Nexzr Special",
	0x640B3017
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Hyper Catalog (CD-Rom Capsule)",
	0x6507DA1A
	},{
	1, // NEC PC Engine CD-ROM
	"Seirei Senshi Spriggan",
	0x650D341B
	},{
	1, // NEC PC Engine CD-ROM
	"Lord of Wars",
	0x66066F0A
	},{
	1, // NEC PC Engine CD-ROM
	"Sexy Idol Mahjong Yakyuuken no Uta",
	0x66085D1A
	},{
	1, // NEC PC Engine CD-ROM
	"Mashou Denki La Valeur",
	0x660AE718
	},{
	1, // NEC PC Engine CD-ROM
	"Chiki Chiki Boys",
	0x660C261A
	},{
	1, // NEC PC Engine CD-ROM
	"Tengai Makyo II - Manji Maru",
	0x660EC919
	},{
	1, // NEC PC Engine CD-ROM
	"Sexy Idol Mahjong Fashion Monogatari",
	0x67092B1E
	},{
	1, // NEC PC Engine CD-ROM
	"Martial Champion",
	0x6709E251
	},{
	1, // NEC PC Engine CD-ROM
	"Go! Go! Birdie Chance",
	0x670E4308
	},{
	1, // NEC PC Engine CD-ROM
	"Quiz Marugoto the World II - Time Machine ni Onegai!",
	0x680A012A
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Implode",
	0x6906A70A
	},{
	1, // NEC PC Engine CD-ROM
	"Inoue Mami - Kono Hoshi ni Tatta Hitori no Kimi",
	0x6909A909
	},{
	1, // NEC PC Engine CD-ROM
	"Nexzr",
	0x690AE517
	},{
	1, // NEC PC Engine CD-ROM
	"Cobra II - Densetsu no Otoko",
	0x6A094109
	},{
	1, // NEC PC Engine CD-ROM
	"Sol Bianca",
	0x6B0CBB63
	},{
	1, // NEC PC Engine CD-ROM
	"Psychic Detective Series Vol. III - Aya",
	0x6C056C09
	},{
	1, // NEC PC Engine CD-ROM
	"Chou Aniki",
	0x6C064D1C
	},{
	1, // NEC PC Engine CD-ROM
	"Travelers! Densetsu wo ButtobaSe",
	0x6D075308
	},{
	1, // NEC PC Engine CD-ROM
	"Kaze no Densetsu Xanadu II",
	0x6D0DFD30
	},{
	1, // NEC PC Engine CD-ROM
	"God Panic - Shijyou Saikyo Gundan",
	0x6E0C8919
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Fighting Street",
	0x6F09781A
	},{
	1, // NEC PC Engine CD-ROM
	"Fighting Street",
	0x6F09781A
	},{
	1, // NEC PC Engine CD-ROM
	"Valis IV - The Fantasm Soldier",
	0x6F0D1E5E
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Fantasy Star Soldier",
	0x6F0E7C18
	},{
	1, // NEC PC Engine CD-ROM
	"Nishimura Kyoutarou Mystery - Hokutosei no Onna",
	0x70040117
	},{
	1, // NEC PC Engine CD-ROM
	"The TV Show",
	0x700C9E1C
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Hyper Catalog IV",
	0x700F6340
	},{
	1, // NEC PC Engine CD-ROM
	"Hawk F-123",
	0x7108A91C
	},{
	1, // NEC PC Engine CD-ROM
	"Pachiokun Warau Uchuu",
	0x7109751B
	},{
	1, // NEC PC Engine CD-ROM
	"Lady Phantom",
	0x710D102C
	},{
	1, // NEC PC Engine CD-ROM
	"Cardangels",
	0x710E272A
	},{
	1, // NEC PC Engine CD-ROM
	"Hu PGA Tour Power Golf 2 - Golfer",
	0x7206BC0A
	},{
	1, // NEC PC Engine CD-ROM
	"Nekketsu Koukou Dodgeballbu Soccer Hen",
	0x720C8A38
	},{
	1, // NEC PC Engine CD-ROM
	"Pachiokun Maboroshi no Densetsu",
	0x730CAA2F
	},{
	1, // NEC PC Engine CD-ROM
	"Motteke Tamago",
	0x7408B209
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Hyper Catalog Duo-RX - Disc B",
	0x740AB42D
	},{
	1, // NEC PC Engine CD-ROM
	"Vasteel II",
	0x740BAE1A
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Riot Zone",
	0x740BF318
	},{
	1, // NEC PC Engine CD-ROM
	"Crest of Wolf",
	0x740BF818
	},{
	1, // NEC PC Engine CD-ROM
	"Private Eyedol",
	0x750EA919
	},{
	1, // NEC PC Engine CD-ROM
	"Fushigi no Umi no Nadia",
	0x760C1209
	},{
	1, // NEC PC Engine CD-ROM
	"Janshin Densetsu - Quest of Jongmaster",
	0x760DC21C
	},{
	1, // NEC PC Engine CD-ROM
	"Ultrabox VI Go",
	0x760E851B
	},{
	1, // NEC PC Engine CD-ROM
	"Shinsetsu Shiawase Usagi F - Yuujiyou Yorimo Aiyoku",
	0x77034C0C
	},{
	1, // NEC PC Engine CD-ROM
	"1552 Tenka Tairan",
	0x770E191C
	},{
	1, // NEC PC Engine CD-ROM
	"Galaxy Fraulein Yuna {HRH310827-3FAFT}",
	0x780AD20A
	},{
	1, // NEC PC Engine CD-ROM
	"Galaxy Fraulein Yuna {HRH310827-3FABT}",
	0x780AD20A
	},{
	1, // NEC PC Engine CD-ROM
	"Galaxy Fraulein Yuna {HRH310827-3FAAT}",
	0x780AD20A
	},{
	1, // NEC PC Engine CD-ROM
	"Gain Ground SX",
	0x7905AC0A
	},{
	1, // NEC PC Engine CD-ROM
	"Strider Hiryuu",
	0x790ABF1E
	},{
	1, // NEC PC Engine CD-ROM
	"F1 Circus Special - Pole to Win",
	0x790D333F
	},{
	1, // NEC PC Engine CD-ROM
	"Bonanza Bros.",
	0x7A05E517
	},{
	1, // NEC PC Engine CD-ROM
	"R-Type Complete CD",
	0x7A0ABF2E
	},{
	1, // NEC PC Engine CD-ROM
	"Fausseté Amour",
	0x7A0BD82E
	},{
	1, // NEC PC Engine CD-ROM
	"Gambler Jikochuushinha",
	0x7B03480A
	},{
	1, // NEC PC Engine CD-ROM
	"Mahou no Shoujo Silky Lip [Ver 3.1]",
	0x7B04EF08
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Ys III - Wanderers from Ys",
	0x7B0FDA19
	},{
	1, // NEC PC Engine CD-ROM
	"Might and Magic",
	0x7C06530B
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Fan Special CD-Rom Volume I",
	0x7C072B09
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume IV",
	0x7C08FB0B
	},{
	1, // NEC PC Engine CD-ROM
	"Mugen Senshi Valis - Legend of a Fantasm Soldier",
	0x7C0C1E1D
	},{
	1, // NEC PC Engine CD-ROM
	"Kuusou Kagaku Sekai Gulliver Boy",
	0x7C11310A
	},{
	1, // NEC PC Engine CD-ROM
	"Championship Rally",
	0x7D07090A
	},{
	1, // NEC PC Engine CD-ROM
	"Shanghai III - Dragon's Eye",
	0x7D07560C
	},{
	1, // NEC PC Engine CD-ROM
	"Arunamu no Kiba - Juuzoku Juuni Shinto Densetsu",
	0x7D0DFA2A
	},{
	1, // NEC PC Engine CD-ROM
	"Hyaku Monogatari - Hontou ni Atta Kowai Hanashi",
	0x7D108E1C
	},{
	0, // NEC PC-FX CD-ROM
	"Nnyuu [GANHE001A]  - Disc 1",
	0x7E04C80A
	},{
	1, // NEC PC Engine CD-ROM
	"Shadow of the Beast",
	0x7E0A900A
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"John Madden Duo CD Football",
	0x7F06260C
	},{
	1, // NEC PC Engine CD-ROM
	"CD Hanafuda Bishoujo Fan Club",
	0x7F07461D
	},{
	1, // NEC PC Engine CD-ROM
	"Mamono Hunter Youko - Makai Kara no Tenkousei",
	0x7F08BA1A
	},{
	1, // NEC PC Engine CD-ROM
	"Bunilla Syndrome Mahjong",
	0x7F0DD84D
	},{
	1, // NEC PC Engine CD-ROM
	"Princess Minerva",
	0x800AE11B
	},{
	1, // NEC PC Engine CD-ROM
	"Zero 4 Champ II",
	0x800C4B2B
	},{
	1, // NEC PC Engine CD-ROM
	"Ys III - Wanderers from Ys",
	0x800FD119
	},{
	1, // NEC PC Engine CD-ROM
	"Exile II - Janen no Jishou",
	0x810C6C1C
	},{
	1, // NEC PC Engine CD-ROM
	"Metal Angel",
	0x8209D40B
	},{
	1, // NEC PC Engine CD-ROM
	"Snatcher",
	0x820E0418
	},{
	1, // NEC PC Engine CD-ROM
	"Top O Nerae! GunBuster Volume II",
	0x830B552A
	},{
	1, // NEC PC Engine CD-ROM
	"Gradius II - Gofer no Yabou",
	0x830B851C
	},{
	1, // NEC PC Engine CD-ROM
	"Tadaima Yuusha Boshuuchuu",
	0x8406AA0A
	},{
	0, // NEC PC-FX CD-ROM
	"Shanghai - The Great Wall [FXNHE507]",
	0x8406C71D
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume I",
	0x8408290B
	},{
	1, // NEC PC Engine CD-ROM
	"Jack Nicklaus World Tour Golf",
	0x8504850B
	},{
	1, // NEC PC Engine CD-ROM
	"Mahjong Sword - Princess Quest Gaiden",
	0x850E3346
	},{
	1, // NEC PC Engine CD-ROM
	"Lodoss Tousenki II - Record of Lodoss War II {HCD4066}",
	0x8606B80B
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume II",
	0x8608770B
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"It Came from the Desert",
	0x860C111A
	},{
	1, // NEC PC Engine CD-ROM
	"Shinsetsu Shiawase Usagi II",
	0x8704940C
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume III",
	0x87087E0B
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Shadow of the Beast",
	0x870ABF0A
	},{
	0, // NEC PC-FX CD-ROM
	"Deep Blue Fleet [FXNHE504]",
	0x8A0BFB0B
	},{
	1, // NEC PC Engine CD-ROM
	"Bishoujo Senshi Sailor Moon Collection",
	0x8B064F0E
	},{
	1, // NEC PC Engine CD-ROM
	"Seisenshi Denshou - Jantaku no Kishi",
	0x8B0DD21A
	},{
	1, // NEC PC Engine CD-ROM
	"Burai II - Yami Koutei no Gyakushuu",
	0x8C0EEA2D
	},{
	1, // NEC PC Engine CD-ROM
	"Megami Paradise",
	0x8D0E972B
	},{
	1, // NEC PC Engine CD-ROM
	"Space Fantasy Zone",
	0x8E03FD0A
	},{
	1, // NEC PC Engine CD-ROM
	"Babel",
	0x900B9552
	},{
	1, // NEC PC Engine CD-ROM
	"Chou Eiyuu Densetsu - Dynastic Hero",
	0x920C221C
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"The Dynastic Hero",
	0x920C291C
	},{
	1, // NEC PC Engine CD-ROM
	"Metal Angel II",
	0x920CCC0D
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume V",
	0x93072E0C
	},{
	1, // NEC PC Engine CD-ROM
	"J. B. Harold Murder Club",
	0x930A5E0B
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"J. B. Harold Murder Club",
	0x930A5E0B
	},{
	1, // NEC PC Engine CD-ROM
	"F1 Team Simulation Project F",
	0x9406D51F
	},{
	1, // NEC PC Engine CD-ROM
	"Dragon Knight & Grafitti",
	0x940B180C
	},{
	1, // NEC PC Engine CD-ROM
	"High Grenadier",
	0x9604450C
	},{
	1, // NEC PC Engine CD-ROM
	"Kakutou Haou Densetsu Algunos",
	0x9705360D
	},{
	1, // NEC PC Engine CD-ROM
	"Yamamura Misa Suspense - Kinsen Ka Kyo E Zara Satsu Jin Ji Ken",
	0x980BF71B
	},{
	1, // NEC PC Engine CD-ROM
	"Zero Wing",
	0x9A09F51C
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Mysterious Song {FCD0001}",
	0x9B0CD21D
	},{
	1, // NEC PC Engine CD-ROM
	"Shin Megami Tensei",
	0x9C035810
	},{
	1, // NEC PC Engine CD-ROM
	"Shiawase Usagi II - Toraware Usagi Sailor Z",
	0x9C05B40F
	},{
	1, // NEC PC Engine CD-ROM
	"Sotsugyou - Graduation",
	0x9C0DF01C
	},{
	1, // NEC PC Engine CD-ROM
	"Ys Book I & II",
	0x9C10FA2B
	},{
	1, // NEC PC Engine CD-ROM
	"Lodoss Tousenki II - Record of Lodoss War II {HMD-003}",
	0x9D05D60B
	},{
	1, // NEC PC Engine CD-ROM
	"Moonlight Lady",
	0x9D094830
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume V - Maku no Uchi",
	0x9D09D70D
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Exile II - Wicked Phenomenon",
	0x9D0BDF1C
	},{
	1, // NEC PC Engine CD-ROM
	"Jim Power - In Mutant Planet",
	0x9D0C6D0C
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Ys Book I & II",
	0x9D11022B
	},{
	1, // NEC PC Engine CD-ROM
	"Rom Rom Stadium",
	0x9E042B10
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Valis III",
	0x9E0B703F
	},{
	1, // NEC PC Engine CD-ROM
	"Ys IV - The Dawn of Ys {HCD3051-5-1116-R1P}",
	0x9E112420
	},{
	1, // NEC PC Engine CD-ROM
	"Bishoujo Jyanshi Idol Pai",
	0x9F0CA257
	},{
	1, // NEC PC Engine CD-ROM
	"The Kick Boxing",
	0xA0024D0E
	},{
	1, // NEC PC Engine CD-ROM
	"Gotzendiener",
	0xA0036D0D
	},{
	1, // NEC PC Engine CD-ROM
	"Formation Soccer '95 - Della Serie A",
	0xA009491F
	},{
	1, // NEC PC Engine CD-ROM
	"KO Seiki Beast - Gaia Fukkatsu Kanketsu Hen",
	0xA00D9D1B
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Jack Nicklaus Turbo Golf",
	0xA2049B0B
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Insanity",
	0xA304ED0D
	},{
	1, // NEC PC Engine CD-ROM
	"Ultrabox Sohkan Go",
	0xA3071B1D
	},{
	1, // NEC PC Engine CD-ROM
	"The Pro Yakyuu Super",
	0xA30A381D
	},{
	1, // NEC PC Engine CD-ROM
	"Bakushou Yoshimoto no Shinkigeki",
	0xA30CA61E
	},{
	1, // NEC PC Engine CD-ROM
	"Super Raiden",
	0xA409690F
	},{
	1, // NEC PC Engine CD-ROM
	"Kawa no Nushizuri - Shizenha",
	0xA40B331E
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Cotton - Fantastic Night Dreams",
	0xA40C331B
	},{
	1, // NEC PC Engine CD-ROM
	"Battlefield '94 in Tokyo Dome",
	0xA40D771F
	},{
	1, // NEC PC Engine CD-ROM
	"Daisenryaku II - Campaign Version",
	0xA505090D
	},{
	1, // NEC PC Engine CD-ROM
	"The Atlas - Renaissance Voyager",
	0xA505FD0E
	},{
	1, // NEC PC Engine CD-ROM
	"Mahou no Shoujo Silky Lip [Ver 5.0]",
	0xA508DF0D
	},{
	1, // NEC PC Engine CD-ROM
	"Mahou no Shoujo Silky Lip [Ver 5.0] alt",
	0xA508DF0D
	},{
	1, // NEC PC Engine CD-ROM
	"Shinsetsu Shiawase Usagi",
	0xA605BB0D
	},{
	1, // NEC PC Engine CD-ROM
	"Farjius no Jakoutei - Neo Metal Fantasy",
	0xA60C031E
	},{
	1, // NEC PC Engine CD-ROM
	"L - Dis",
	0xA60C8930
	},{
	1, // NEC PC Engine CD-ROM
	"Magicoal",
	0xA90CA71E
	},{
	1, // NEC PC Engine CD-ROM
	"Vasteel",
	0xA90DDD20
	},{
	1, // NEC PC Engine CD-ROM
	"Mitsubachi Gakuen",
	0xAA0BB70E
	},{
	1, // NEC PC Engine CD-ROM
	"Tenchi Muyou! Ryououki",
	0xAA0F9F0D
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Hyper Catalog II",
	0xAB09BD23
	},{
	1, // NEC PC Engine CD-ROM
	"Cotton - Fantastic Night Dreams",
	0xAB0C301B
	},{
	1, // NEC PC Engine CD-ROM
	"Xak I & II",
	0xAB0C811D
	},{
	1, // NEC PC Engine CD-ROM
	"Faceball Taikenban",
	0xAC02760F
	},{
	1, // NEC PC Engine CD-ROM
	"Cobra - Kokuryuuou no Densetsu",
	0xAC05960C
	},{
	1, // NEC PC Engine CD-ROM
	"Ultrabox V Go",
	0xAD09451F
	},{
	1, // NEC PC Engine CD-ROM
	"The Pro Yakyuu Super '94",
	0xAE08A01E
	},{
	1, // NEC PC Engine CD-ROM
	"Populous - The Promised Lands",
	0xAF075C0E
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Splash Lake",
	0xAF076C0E
	},{
	1, // NEC PC Engine CD-ROM
	"Star Mobile",
	0xAF0A700E
	},{
	1, // NEC PC Engine CD-ROM
	"Sangokushi III",
	0xAF0CF61E
	},{
	1, // NEC PC Engine CD-ROM
	"Yu Yu Hakusho",
	0xAF0D2D1D
	},{
	1, // NEC PC Engine CD-ROM
	"Yami no Ketsuzoku Harukanaru Kioku",
	0xAF0E520D
	},{
	1, // NEC PC Engine CD-ROM
	"Quiz Tonosama no Yabou",
	0xB009CB24
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume III - Yappashi Band",
	0xB20A420D
	},{
	1, // NEC PC Engine CD-ROM
	"Mahjong Lemon Angel",
	0xB20ADB53
	},{
	1, // NEC PC Engine CD-ROM
	"Renny Blaster",
	0xB20CBB1D
	},{
	1, // NEC PC Engine CD-ROM
	"Bomberman Panic Bomber",
	0xB20D4E1F
	},{
	1, // NEC PC Engine CD-ROM
	"Mateki Densetsu Astralius",
	0xB30B182F
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Cosmic Fantasy II",
	0xB30D3562
	},{
	1, // NEC PC Engine CD-ROM
	"Terraforming",
	0xB408F20E
	},{
	1, // NEC PC Engine CD-ROM
	"Madou Monogatari I - Honou no Sotsuenji",
	0xB40D6620
	},{
	1, // NEC PC Engine CD-ROM
	"Laplace no Ma",
	0xB506DF0E
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume II - Nattoku Idol",
	0xB60AA50D
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Hyper Catalog VI - Disc B",
	0xB70BD42F
	},{
	1, // NEC PC Engine CD-ROM
	"Tengai Makyo - Kabuki Itouryodan",
	0xB70D911D
	},{
	1, // NEC PC Engine CD-ROM
	"Sword Master",
	0xB70DB11F
	},{
	1, // NEC PC Engine CD-ROM
	"Sotsugyou Shashin - Miki",
	0xB80B6F2E
	},{
	1, // NEC PC Engine CD-ROM
	"Seiya Monogatari - Anearth Fantasy Stories Taikenban",
	0xBA099920
	},{
	1, // NEC PC Engine CD-ROM
	"Hi-Leg Fantasy",
	0xBB08D80D
	},{
	1, // NEC PC Engine CD-ROM
	"Cosmic Fantasy II - Bouken Shounen Ban",
	0xBB0D3462
	},{
	1, // NEC PC Engine CD-ROM
	"Cosmic Fantasy III - Bouken Shounen Rei",
	0xBB0E4360
	},{
	1, // NEC PC Engine CD-ROM
	"Snatcher - Pilot Disk",
	0xBC0AD922
	},{
	1, // NEC PC Engine CD-ROM
	"Laplace no Ma - Preview Disk",
	0xBD06E00E
	},{
	1, // NEC PC Engine CD-ROM
	"CD Bishoujo Pachinko Kyuuma Yon Shimai",
	0xBE06B122
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Syd Mead's Terra Forming",
	0xBE08ED0E
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume I - Suteki ni Standard",
	0xBE0AB50D
	},{
	1, // NEC PC Engine CD-ROM
	"Travel Epuru",
	0xBE0BE049
	},{
	1, // NEC PC Engine CD-ROM
	"Wizardry I & II",
	0xBF07F40F
	},{
	1, // NEC PC Engine CD-ROM
	"Burai - Hachigyoku no Yuushi Densetsu",
	0xBF0E5247
	},{
	1, // NEC PC Engine CD-ROM
	"Ane-San",
	0xBF0EC11D
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Might and Magic III - Isles of Terra",
	0xC006B710
	},{
	1, // NEC PC Engine CD-ROM
	"Might and Magic III - Isles of Terra",
	0xC006B710
	},{
	1, // NEC PC Engine CD-ROM
	"Splash Lake",
	0xC007710E
	},{
	1, // NEC PC Engine CD-ROM
	"Eiyuu Sangokushi",
	0xC007780F
	},{
	1, // NEC PC Engine CD-ROM
	"Linda ³ {HLHLF HE230908-2 V9V4}",
	0xC00FDC1B
	},{
	1, // NEC PC Engine CD-ROM
	"Uchuu Senkan Yamato",
	0xC1086210
	},{
	1, // NEC PC Engine CD-ROM
	"Rom² Karaoke Volume IV - Choito Otona!",
	0xC10B240D
	},{
	1, // NEC PC Engine CD-ROM
	"Super Real Mahjong Special",
	0xC10C7046
	},{
	1, // NEC PC Engine CD-ROM
	"Laser Soft Visual Collection Volume II - Valis Visual Shuu",
	0xC10D0250
	},{
	1, // NEC PC Engine CD-ROM
	"Mad Stalker - Full Metal Force",
	0xC10D9B22
	},{
	1, // NEC PC Engine CD-ROM
	"Brandish",
	0xC20A8120
	},{
	1, // NEC PC Engine CD-ROM
	"Kisou Louga II - The Ends of Shangrila",
	0xC20B4B20
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Sim Earth - The Living Planet",
	0xC308E223
	},{
	1, // NEC PC Engine CD-ROM
	"Emerald Dragon",
	0xC411261F
	},{
	1, // NEC PC Engine CD-ROM
	"Tenshi no Uta",
	0xC50B1E21
	},{
	1, // NEC PC Engine CD-ROM
	"Download II",
	0xC6074610
	},{
	1, // NEC PC Engine CD-ROM
	"Macross Eien no Love Song",
	0xC60D2C2E
	},{
	1, // NEC PC Engine CD-ROM
	"Puyo Puyo CD Tsuu",
	0xC70DF051
	},{
	1, // NEC PC Engine CD-ROM
	"Horror Story",
	0xC80C6310
	},{
	1, // NEC PC Engine CD-ROM
	"Neo Nectaris",
	0xCA10041F
	},{
	1, // NEC PC Engine CD-ROM
	"Alshark",
	0xCB0A460F
	},{
	1, // NEC PC Engine CD-ROM
	"Hyper Wars",
	0xCB0F3210
	},{
	1, // NEC PC Engine CD-ROM
	"Eikan ha Kimini - Koukou Yakyuu Zenkoku Taikai",
	0xCC07191F
	},{
	1, // NEC PC Engine CD-ROM
	"Linda ³",
	0xCC10401C
	},{
	1, // NEC PC Engine CD-ROM
	"Faceball",
	0xCD06EB10
	},{
	1, // NEC PC Engine CD-ROM
	"Monster Lair - Wonderboy III",
	0xCD098010
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Monster Lair",
	0xCD098010
	},{
	1, // NEC PC Engine CD-ROM
	"Summer Carnival '92 - Alzadick",
	0xCF063311
	},{
	1, // NEC PC Engine CD-ROM
	"Macross 2036",
	0xCF0CFF23
	},{
	1, // NEC PC Engine CD-ROM
	"Quiz Caravan Cult Q",
	0xD0044022
	},{
	1, // NEC PC Engine CD-ROM
	"Gulclight TDF 2",
	0xD00C780F
	},{
	1, // NEC PC Engine CD-ROM
	"GS Mikami",
	0xD1056112
	},{
	1, // NEC PC Engine CD-ROM
	"Doraemon Nobita no Dorabian Night",
	0xD2032D25
	},{
	1, // NEC PC Engine CD-ROM
	"Tenchi wo Kurau",
	0xD2053E14
	},{
	1, // NEC PC Engine CD-ROM
	"Sim Earth - The Living Planet",
	0xD2081823
	},{
	1, // NEC PC Engine CD-ROM
	"Golden Axe",
	0xD20B5347
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Hyper Catalog V",
	0xD3089B35
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Dungeon Master - Theron's Quest {TGXCD1041}",
	0xD4094113
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Dungeon Master - Theron's Quest",
	0xD4094113
	},{
	1, // NEC PC Engine CD-ROM
	"Wizardry III & IV",
	0xD40ADA10
	},{
	1, // NEC PC Engine CD-ROM
	"Iga Ninden Gaiou",
	0xD40D9224
	},{
	1, // NEC PC Engine CD-ROM
	"The Manhole",
	0xD50BDC24
	},{
	1, // NEC PC Engine CD-ROM
	"Shangai II",
	0xD803880D
	},{
	1, // NEC PC Engine CD-ROM
	"Shiawase Usagi - Nureta Bishoujo Ichi Hajimete na no ni",
	0xD805C40F
	},{
	1, // NEC PC Engine CD-ROM
	"Sengoku Kantou Sankokushi",
	0xD80AAC4B
	},{
	1, // NEC PC Engine CD-ROM
	"Gensou Tairiku Auleria",
	0xD80E0920
	},{
	0, // NEC PC-FX CD-ROM
	"Team Innocent [FXHUD402]",
	0xD810E90E
	},{
	1, // NEC PC Engine CD-ROM
	"Galaxy Deka Gayvan",
	0xD9087E11
	},{
	1, // NEC PC Engine CD-ROM
	"Dragonball Z - Idainaru Son Gokuu Densetsu",
	0xDA07A90F
	},{
	1, // NEC PC Engine CD-ROM
	"Rayxanber II",
	0xDA090110
	},{
	1, // NEC PC Engine CD-ROM
	"Where in the World is Carmen Sandiego",
	0xDA094B22
	},{
	1, // NEC PC Engine CD-ROM
	"Ys IV - The Dawn of Ys {HCD3051-4-1108-R1F}",
	0xDA111B20
	},{
	1, // NEC PC Engine CD-ROM
	"Sangokushi Eiketsu - Tenka ni Nozomu",
	0xDC09AA4A
	},{
	1, // NEC PC Engine CD-ROM
	"Tenshi no Uta II - Datenshi no Uta",
	0xDD0BAA22
	},{
	1, // NEC PC Engine CD-ROM
	"Taiheiki",
	0xDE0AF625
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Bonk III - Bonk's Big Adventure",
	0xDE0DFE11
	},{
	1, // NEC PC Engine CD-ROM
	"Motoroader MC",
	0xDF086512
	},{
	0, // NEC PC-FX CD-ROM
	"Lunatic Dawn [FXNHE509]",
	0xDF0DB51F
	},{
	1, // NEC PC Engine CD-ROM
	"Color Wars",
	0xE0051910
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Exile",
	0xE00B2026
	},{
	1, // NEC PC Engine CD-ROM
	"Super Real Mahjong P V Custom",
	0xE00B3243
	},{
	1, // NEC PC Engine CD-ROM
	"Efera & Jiliora - The Emblem from Darkness",
	0xE10C4111
	},{
	1, // NEC PC Engine CD-ROM
	"Bikkuriman Daijikai",
	0xE206D649
	},{
	1, // NEC PC Engine CD-ROM
	"Rayxanber III",
	0xE20A7512
	},{
	1, // NEC PC Engine CD-ROM
	"Pop'n Magic",
	0xE20B3A23
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Vasteel",
	0xE20CFB20
	},{
	1, // NEC PC Engine CD-ROM
	"Kaze no Densetsu Xanadu",
	0xE20DEC11
	},{
	1, // NEC PC Engine CD-ROM
	"Kaze no Densetsu Xanadu - Sample Disc",
	0xE20DEC11
	},{
	1, // NEC PC Engine CD-ROM
	"Star Breaker",
	0xE3090011
	},{
	1, // NEC PC Engine CD-ROM
	"Kaizou Choujin Shubibinman III - Ikai no Princess",
	0xE40DB135
	},{
	1, // NEC PC Engine CD-ROM
	"Seiya Monogatari - Anearth Fantasy Stories [Debug Version]",
	0xE40F5D20
	},{
	1, // NEC PC Engine CD-ROM
	"Sorcerian",
	0xE50BFB33
	},{
	1, // NEC PC Engine CD-ROM
	"Rainbow Islands",
	0xE50C7C20
	},{
	1, // NEC PC Engine CD-ROM
	"Kiaidan 00",
	0xE70D0A25
	},{
	1, // NEC PC Engine CD-ROM
	"Shin Sangokushi - Tenka ha Ware ni",
	0xE70DE145
	},{
	1, // NEC PC Engine CD-ROM
	"Xak III - The Eternal Recurrence",
	0xE80C9723
	},{
	1, // NEC PC Engine CD-ROM
	"Magical Fantasy Adventure - Popful Mail",
	0xE80CC421
	},{
	1, // NEC PC Engine CD-ROM
	"Startling Odyssey II",
	0xE8110F34
	},{
	1, // NEC PC Engine CD-ROM
	"Ginga Fukei Densetsu Sapphire",
	0xE9090811
	},{
	1, // NEC PC Engine CD-ROM
	"Ginga Fukei Densetsu Sapphire [Bootleg]",
	0xE9090A11
	},{
	1, // NEC PC Engine CD-ROM
	"Seiya Monogatari - Anearth Fantasy Stories",
	0xEA0F9620
	},{
	1, // NEC PC Engine CD-ROM
	"Mahjong Clinic Special",
	0xEC083C10
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Final Zone II",
	0xED0AB237
	},{
	0, // NEC PC-FX CD-ROM
	"Boundary Gate Daughter of Kingdom [FXNHE620]",
	0xEE0A805B
	},{
	1, // NEC PC Engine CD-ROM
	"Prince of Persia",
	0xEF0A0A13
	},{
	1, // NEC PC Engine CD-ROM
	"The Pro Yakyuu",
	0xF0043015
	},{
	1, // NEC PC Engine CD-ROM
	"Ultrabox II Go",
	0xF00CC122
	},{
	1, // NEC PC Engine CD-ROM
	"Super Schwartzschild",
	0xF10C9E13
	},{
	1, // NEC PC Engine CD-ROM
	"Rising Sun",
	0xF20B0335
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Shape Shifter",
	0xF20E6562
	},{
	1, // NEC PC Engine CD-ROM
	"No.Ri.Ko",
	0xF305EB62
	},{
	1, // NEC PC Engine CD-ROM
	"Avenger",
	0xF3084427
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Dungeon Explorer II",
	0xF3107522
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Prince of Persia",
	0xF40A0513
	},{
	1, // NEC PC Engine CD-ROM
	"Ganchouhishi - Aoki Ookami to Shiroki Mejika",
	0xF40D9A10
	},{
	1, // NEC PC Engine CD-ROM
	"Paradion - Auto Crusher Palladium",
	0xF508DF12
	},{
	1, // NEC PC Engine CD-ROM
	"Puyo Puyo CD",
	0xF509C325
	},{
	1, // NEC PC Engine CD-ROM
	"Shapeshifter - Makai Eiyuu Den",
	0xF50E6562
	},{
	1, // NEC PC Engine CD-ROM
	"Metamor Jupiter",
	0xF6073B25
	},{
	1, // NEC PC Engine CD-ROM
	"Sylphia",
	0xF70D7D12
	},{
	1, // NEC PC Engine CD-ROM
	"Crazy Hospital - Fushigi no Kuni no Tenshi",
	0xF8049311
	},{
	1, // NEC PC Engine CD-ROM
	"Jantei Monogatari III - Saver Angels",
	0xF8061412
	},{
	1, // NEC PC Engine CD-ROM
	"PCEngine Hyper Catalog III",
	0xF8081D25
	},{
	1, // NEC PC Engine CD-ROM
	"Final Zone II",
	0xFA0ABD37
	},{
	2, // NEC TurboGrafx-16 CD-ROM
	"Camp California",
	0xFA0F1338
	},{
	1, // NEC PC Engine CD-ROM
	"Tengai Makyo - Kabuki Den",
	0xFA106322
	},{
	1, // NEC PC Engine CD-ROM
	"Dead of the Brain I & II",
	0xFB0ADE22
	},{
	1, // NEC PC Engine CD-ROM
	"Legion",
	0xFC07AB15
	},{
	1, // NEC PC Engine CD-ROM
	"Super Darius",
	0xFC0A2313
	}
};

#define CD_TITLE_COUNT 557
#define qCD_TITLE_COUNT "557"

#pragma pack()
