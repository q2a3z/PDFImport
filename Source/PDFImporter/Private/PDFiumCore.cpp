#include "PDFiumCore.h"
#include "PDF.h"

#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/EngineVersionComparison.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"

#include "IImageWrapperModule.h"
#include "IImageWrapper.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"

THIRD_PARTY_INCLUDES_START
// C++ Includes.
#include <string>
#include <fstream>

// PDFium Includes.
#include "fpdf_formfill.h"
#include "fpdf_text.h"
#include "fpdf_edit.h"
#include "fpdf_save.h"
THIRD_PARTY_INCLUDES_END

// Global library initialization checker.
static inline bool bIsLibInitialized = false;

// Global char converter pool for ASCII to CHAR.
static inline TMap<uint32_t, FString> Global_ASCII_to_Char;

// Global char converter pool for CHAR to ASCII.
static inline TMap<FString, uint32_t> Global_Char_To_ASCII;

void DefineCharCodes()
{
	Global_ASCII_to_Char.Add(10, "\n");
	Global_ASCII_to_Char.Add(32, " ");
	Global_ASCII_to_Char.Add(33, "!");
	Global_ASCII_to_Char.Add(34, "\"");
	Global_ASCII_to_Char.Add(35, "#");
	Global_ASCII_to_Char.Add(36, "$");
	Global_ASCII_to_Char.Add(37, "%");
	Global_ASCII_to_Char.Add(38, "&");
	Global_ASCII_to_Char.Add(39, "'");
	Global_ASCII_to_Char.Add(40, "(");
	Global_ASCII_to_Char.Add(41, ")");
	Global_ASCII_to_Char.Add(42, "*");
	Global_ASCII_to_Char.Add(43, "+");
	Global_ASCII_to_Char.Add(44, ",");
	Global_ASCII_to_Char.Add(45, "-");
	Global_ASCII_to_Char.Add(46, ".");
	Global_ASCII_to_Char.Add(47, "/");
	Global_ASCII_to_Char.Add(48, "0");
	Global_ASCII_to_Char.Add(49, "1");
	Global_ASCII_to_Char.Add(50, "2");
	Global_ASCII_to_Char.Add(51, "3");
	Global_ASCII_to_Char.Add(52, "4");
	Global_ASCII_to_Char.Add(53, "5");
	Global_ASCII_to_Char.Add(54, "6");
	Global_ASCII_to_Char.Add(55, "7");
	Global_ASCII_to_Char.Add(56, "8");
	Global_ASCII_to_Char.Add(57, "9");
	Global_ASCII_to_Char.Add(58, ":");
	Global_ASCII_to_Char.Add(59, ";");
	Global_ASCII_to_Char.Add(60, "<");
	Global_ASCII_to_Char.Add(61, "=");
	Global_ASCII_to_Char.Add(62, ">");
	Global_ASCII_to_Char.Add(63, "?");
	Global_ASCII_to_Char.Add(64, "@");
	Global_ASCII_to_Char.Add(65, "A");
	Global_ASCII_to_Char.Add(66, "B");
	Global_ASCII_to_Char.Add(67, "C");
	Global_ASCII_to_Char.Add(68, "D");
	Global_ASCII_to_Char.Add(69, "E");
	Global_ASCII_to_Char.Add(70, "F");
	Global_ASCII_to_Char.Add(71, "G");
	Global_ASCII_to_Char.Add(72, "H");
	Global_ASCII_to_Char.Add(73, "I");
	Global_ASCII_to_Char.Add(74, "J");
	Global_ASCII_to_Char.Add(75, "K");
	Global_ASCII_to_Char.Add(76, "L");
	Global_ASCII_to_Char.Add(77, "M");
	Global_ASCII_to_Char.Add(78, "N");
	Global_ASCII_to_Char.Add(79, "O");
	Global_ASCII_to_Char.Add(80, "P");
	Global_ASCII_to_Char.Add(81, "Q");
	Global_ASCII_to_Char.Add(82, "R");
	Global_ASCII_to_Char.Add(83, "S");
	Global_ASCII_to_Char.Add(84, "T");
	Global_ASCII_to_Char.Add(85, "U");
	Global_ASCII_to_Char.Add(86, "V");
	Global_ASCII_to_Char.Add(87, "W");
	Global_ASCII_to_Char.Add(88, "X");
	Global_ASCII_to_Char.Add(89, "Y");
	Global_ASCII_to_Char.Add(90, "Z");
	Global_ASCII_to_Char.Add(91, "[");
	Global_ASCII_to_Char.Add(92, "\\");
	Global_ASCII_to_Char.Add(93, "]");
	Global_ASCII_to_Char.Add(94, "^");
	Global_ASCII_to_Char.Add(95, "_");
	Global_ASCII_to_Char.Add(96, "`");
	Global_ASCII_to_Char.Add(97, "a");
	Global_ASCII_to_Char.Add(98, "b");
	Global_ASCII_to_Char.Add(99, "c");
	Global_ASCII_to_Char.Add(100, "d");
	Global_ASCII_to_Char.Add(101, "e");
	Global_ASCII_to_Char.Add(102, "f");
	Global_ASCII_to_Char.Add(103, "g");
	Global_ASCII_to_Char.Add(104, "h");
	Global_ASCII_to_Char.Add(105, "i");
	Global_ASCII_to_Char.Add(106, "j");
	Global_ASCII_to_Char.Add(107, "k");
	Global_ASCII_to_Char.Add(108, "l");
	Global_ASCII_to_Char.Add(109, "m");
	Global_ASCII_to_Char.Add(110, "n");
	Global_ASCII_to_Char.Add(111, "o");
	Global_ASCII_to_Char.Add(112, "p");
	Global_ASCII_to_Char.Add(113, "q");
	Global_ASCII_to_Char.Add(114, "r");
	Global_ASCII_to_Char.Add(115, "s");
	Global_ASCII_to_Char.Add(116, "t");
	Global_ASCII_to_Char.Add(117, "u");
	Global_ASCII_to_Char.Add(118, "v");
	Global_ASCII_to_Char.Add(119, "w");
	Global_ASCII_to_Char.Add(120, "x");
	Global_ASCII_to_Char.Add(121, "y");
	Global_ASCII_to_Char.Add(122, "z");
	Global_ASCII_to_Char.Add(123, "{");
	Global_ASCII_to_Char.Add(124, "|");
	Global_ASCII_to_Char.Add(125, "}");
	Global_ASCII_to_Char.Add(126, "~");
	Global_ASCII_to_Char.Add(128, "€");
	Global_ASCII_to_Char.Add(130, "‚");
	Global_ASCII_to_Char.Add(131, "ƒ");
	Global_ASCII_to_Char.Add(132, "„");
	Global_ASCII_to_Char.Add(133, "…");
	Global_ASCII_to_Char.Add(134, "†");
	Global_ASCII_to_Char.Add(135, "‡");
	Global_ASCII_to_Char.Add(136, "ˆ");
	Global_ASCII_to_Char.Add(137, "‰");
	Global_ASCII_to_Char.Add(138, "Š");
	Global_ASCII_to_Char.Add(139, "‹");
	Global_ASCII_to_Char.Add(140, "Œ");
	Global_ASCII_to_Char.Add(145, "‘");
	Global_ASCII_to_Char.Add(146, "’");
	Global_ASCII_to_Char.Add(147, "“");
	Global_ASCII_to_Char.Add(148, "”");
	Global_ASCII_to_Char.Add(149, "•");
	Global_ASCII_to_Char.Add(150, "–");
	Global_ASCII_to_Char.Add(151, "—");
	Global_ASCII_to_Char.Add(152, "˜");
	Global_ASCII_to_Char.Add(153, "™");
	Global_ASCII_to_Char.Add(154, "š");
	Global_ASCII_to_Char.Add(155, "›");
	Global_ASCII_to_Char.Add(156, "œ");
	Global_ASCII_to_Char.Add(159, "Ÿ");
	Global_ASCII_to_Char.Add(161, "¡");
	Global_ASCII_to_Char.Add(162, "¢");
	Global_ASCII_to_Char.Add(163, "£");
	Global_ASCII_to_Char.Add(164, "¤");
	Global_ASCII_to_Char.Add(165, "¥");
	Global_ASCII_to_Char.Add(166, "¦");
	Global_ASCII_to_Char.Add(167, "§");
	Global_ASCII_to_Char.Add(168, "¨");
	Global_ASCII_to_Char.Add(169, "©");
	Global_ASCII_to_Char.Add(170, "ª");
	Global_ASCII_to_Char.Add(171, "«");
	Global_ASCII_to_Char.Add(172, "¬");
	Global_ASCII_to_Char.Add(174, "®");
	Global_ASCII_to_Char.Add(175, "¯");
	Global_ASCII_to_Char.Add(176, "°");
	Global_ASCII_to_Char.Add(177, "±");
	Global_ASCII_to_Char.Add(178, "²");
	Global_ASCII_to_Char.Add(179, "³");
	Global_ASCII_to_Char.Add(180, "´");
	Global_ASCII_to_Char.Add(181, "µ");
	Global_ASCII_to_Char.Add(182, "¶");
	Global_ASCII_to_Char.Add(183, "·");
	Global_ASCII_to_Char.Add(184, "¸");
	Global_ASCII_to_Char.Add(185, "¹");
	Global_ASCII_to_Char.Add(186, "º");
	Global_ASCII_to_Char.Add(187, "»");
	Global_ASCII_to_Char.Add(188, "¼");
	Global_ASCII_to_Char.Add(189, "½");
	Global_ASCII_to_Char.Add(190, "¾");
	Global_ASCII_to_Char.Add(191, "¿");
	Global_ASCII_to_Char.Add(192, "À");
	Global_ASCII_to_Char.Add(193, "Á");
	Global_ASCII_to_Char.Add(194, "Â");
	Global_ASCII_to_Char.Add(195, "Ã");
	Global_ASCII_to_Char.Add(196, "Ä");
	Global_ASCII_to_Char.Add(197, "Å");
	Global_ASCII_to_Char.Add(198, "Æ");
	Global_ASCII_to_Char.Add(199, "Ç");
	Global_ASCII_to_Char.Add(200, "È");
	Global_ASCII_to_Char.Add(201, "É");
	Global_ASCII_to_Char.Add(202, "Ê");
	Global_ASCII_to_Char.Add(203, "Ë");
	Global_ASCII_to_Char.Add(204, "Ì");
	Global_ASCII_to_Char.Add(205, "Í");
	Global_ASCII_to_Char.Add(206, "Î");
	Global_ASCII_to_Char.Add(207, "Ï");
	Global_ASCII_to_Char.Add(208, "Ğ");
	Global_ASCII_to_Char.Add(209, "Ñ");
	Global_ASCII_to_Char.Add(210, "Ò");
	Global_ASCII_to_Char.Add(211, "Ó");
	Global_ASCII_to_Char.Add(212, "Ô");
	Global_ASCII_to_Char.Add(213, "Õ");
	Global_ASCII_to_Char.Add(214, "Ö");
	Global_ASCII_to_Char.Add(215, "×");
	Global_ASCII_to_Char.Add(216, "Ø");
	Global_ASCII_to_Char.Add(217, "Ù");
	Global_ASCII_to_Char.Add(218, "Ú");
	Global_ASCII_to_Char.Add(219, "Û");
	Global_ASCII_to_Char.Add(220, "Ü");
	Global_ASCII_to_Char.Add(221, "İ");
	Global_ASCII_to_Char.Add(222, "Ş");
	Global_ASCII_to_Char.Add(223, "ß");
	Global_ASCII_to_Char.Add(224, "à");
	Global_ASCII_to_Char.Add(225, "á");
	Global_ASCII_to_Char.Add(226, "â");
	Global_ASCII_to_Char.Add(227, "ã");
	Global_ASCII_to_Char.Add(228, "ä");
	Global_ASCII_to_Char.Add(229, "å");
	Global_ASCII_to_Char.Add(230, "æ");
	Global_ASCII_to_Char.Add(231, "ç");
	Global_ASCII_to_Char.Add(232, "è");
	Global_ASCII_to_Char.Add(233, "é");
	Global_ASCII_to_Char.Add(234, "ê");
	Global_ASCII_to_Char.Add(235, "ë");
	Global_ASCII_to_Char.Add(236, "ì");
	Global_ASCII_to_Char.Add(237, "í");
	Global_ASCII_to_Char.Add(238, "î");
	Global_ASCII_to_Char.Add(239, "ï");
	Global_ASCII_to_Char.Add(240, "ğ");
	Global_ASCII_to_Char.Add(241, "ñ");
	Global_ASCII_to_Char.Add(242, "ò");
	Global_ASCII_to_Char.Add(243, "ó");
	Global_ASCII_to_Char.Add(244, "ô");
	Global_ASCII_to_Char.Add(245, "õ");
	Global_ASCII_to_Char.Add(246, "ö");
	Global_ASCII_to_Char.Add(247, "÷");
	Global_ASCII_to_Char.Add(248, "ø");
	Global_ASCII_to_Char.Add(249, "ù");
	Global_ASCII_to_Char.Add(250, "ú");
	Global_ASCII_to_Char.Add(251, "û");
	Global_ASCII_to_Char.Add(252, "ü");
	Global_ASCII_to_Char.Add(253, "ı");
	Global_ASCII_to_Char.Add(254, "ş");
	Global_ASCII_to_Char.Add(255, "ÿ");

	Global_Char_To_ASCII.Add("\n", 10);
	Global_Char_To_ASCII.Add(" ", 32);
	Global_Char_To_ASCII.Add("!", 33);
	Global_Char_To_ASCII.Add("\"",34);
	Global_Char_To_ASCII.Add("#", 35);
	Global_Char_To_ASCII.Add("$", 36);
	Global_Char_To_ASCII.Add("%", 37);
	Global_Char_To_ASCII.Add("&", 38);
	Global_Char_To_ASCII.Add("'", 39);
	Global_Char_To_ASCII.Add("(", 40);
	Global_Char_To_ASCII.Add(")", 41);
	Global_Char_To_ASCII.Add("*", 42);
	Global_Char_To_ASCII.Add("+", 43);
	Global_Char_To_ASCII.Add(",", 44);
	Global_Char_To_ASCII.Add("-", 45);
	Global_Char_To_ASCII.Add(".", 46);
	Global_Char_To_ASCII.Add("/", 47);
	Global_Char_To_ASCII.Add("0", 48);
	Global_Char_To_ASCII.Add("1", 49);
	Global_Char_To_ASCII.Add("2", 50);
	Global_Char_To_ASCII.Add("3", 51);
	Global_Char_To_ASCII.Add("4", 52);
	Global_Char_To_ASCII.Add("5", 53);
	Global_Char_To_ASCII.Add("6", 54);
	Global_Char_To_ASCII.Add("7", 55);
	Global_Char_To_ASCII.Add("8", 56);
	Global_Char_To_ASCII.Add("9", 57);
	Global_Char_To_ASCII.Add(":", 58);
	Global_Char_To_ASCII.Add(";", 59);
	Global_Char_To_ASCII.Add("<", 60);
	Global_Char_To_ASCII.Add("=", 61);
	Global_Char_To_ASCII.Add(">", 62);
	Global_Char_To_ASCII.Add("?", 63);
	Global_Char_To_ASCII.Add("@", 64);
	Global_Char_To_ASCII.Add("A", 65);
	Global_Char_To_ASCII.Add("B", 66);
	Global_Char_To_ASCII.Add("C", 67);
	Global_Char_To_ASCII.Add("D", 68);
	Global_Char_To_ASCII.Add("E", 69);
	Global_Char_To_ASCII.Add("F", 70);
	Global_Char_To_ASCII.Add("G", 71);
	Global_Char_To_ASCII.Add("H", 72);
	Global_Char_To_ASCII.Add("I", 73);
	Global_Char_To_ASCII.Add("J", 74);
	Global_Char_To_ASCII.Add("K", 75);
	Global_Char_To_ASCII.Add("L", 76);
	Global_Char_To_ASCII.Add("M", 77);
	Global_Char_To_ASCII.Add("N", 78);
	Global_Char_To_ASCII.Add("O", 79);
	Global_Char_To_ASCII.Add("P", 80);
	Global_Char_To_ASCII.Add("Q", 81);
	Global_Char_To_ASCII.Add("R", 82);
	Global_Char_To_ASCII.Add("S", 83);
	Global_Char_To_ASCII.Add("T", 84);
	Global_Char_To_ASCII.Add("U", 85);
	Global_Char_To_ASCII.Add("V", 86);
	Global_Char_To_ASCII.Add("W", 87);
	Global_Char_To_ASCII.Add("X", 88);
	Global_Char_To_ASCII.Add("Y", 89);
	Global_Char_To_ASCII.Add("Z", 90);
	Global_Char_To_ASCII.Add("[", 91);
	Global_Char_To_ASCII.Add("\\",92);
	Global_Char_To_ASCII.Add("]", 93);
	Global_Char_To_ASCII.Add("^", 94);
	Global_Char_To_ASCII.Add("_", 95);
	Global_Char_To_ASCII.Add("`", 96);
	Global_Char_To_ASCII.Add("a", 97);
	Global_Char_To_ASCII.Add("b", 98);
	Global_Char_To_ASCII.Add("c", 99);
	Global_Char_To_ASCII.Add("d", 100);
	Global_Char_To_ASCII.Add("e", 101);
	Global_Char_To_ASCII.Add("f", 102);
	Global_Char_To_ASCII.Add("g", 103);
	Global_Char_To_ASCII.Add("h", 104);
	Global_Char_To_ASCII.Add("i", 105);
	Global_Char_To_ASCII.Add("j", 106);
	Global_Char_To_ASCII.Add("k", 107);
	Global_Char_To_ASCII.Add("l", 108);
	Global_Char_To_ASCII.Add("m", 109);
	Global_Char_To_ASCII.Add("n", 110);
	Global_Char_To_ASCII.Add("o", 111);
	Global_Char_To_ASCII.Add("p", 112);
	Global_Char_To_ASCII.Add("q", 113);
	Global_Char_To_ASCII.Add("r", 114);
	Global_Char_To_ASCII.Add("s", 115);
	Global_Char_To_ASCII.Add("t", 116);
	Global_Char_To_ASCII.Add("u", 117);
	Global_Char_To_ASCII.Add("v", 118);
	Global_Char_To_ASCII.Add("w", 119);
	Global_Char_To_ASCII.Add("x", 120);
	Global_Char_To_ASCII.Add("y", 121);
	Global_Char_To_ASCII.Add("z", 122);
	Global_Char_To_ASCII.Add("{", 123);
	Global_Char_To_ASCII.Add("|", 124);
	Global_Char_To_ASCII.Add("}", 125);
	Global_Char_To_ASCII.Add("~", 126);
	Global_Char_To_ASCII.Add("€", 128);
	Global_Char_To_ASCII.Add("‚", 130);
	Global_Char_To_ASCII.Add("ƒ", 131);
	Global_Char_To_ASCII.Add("„", 132);
	Global_Char_To_ASCII.Add("…", 133);
	Global_Char_To_ASCII.Add("†", 134);
	Global_Char_To_ASCII.Add("‡", 135);
	Global_Char_To_ASCII.Add("ˆ", 136);
	Global_Char_To_ASCII.Add("‰", 137);
	Global_Char_To_ASCII.Add("Š", 138);
	Global_Char_To_ASCII.Add("‹", 139);
	Global_Char_To_ASCII.Add("Œ", 140);
	Global_Char_To_ASCII.Add("‘", 145);
	Global_Char_To_ASCII.Add("’", 146);
	Global_Char_To_ASCII.Add("“", 147);
	Global_Char_To_ASCII.Add("”", 148);
	Global_Char_To_ASCII.Add("•", 149);
	Global_Char_To_ASCII.Add("–", 150);
	Global_Char_To_ASCII.Add("—", 151);
	Global_Char_To_ASCII.Add("˜", 152);
	Global_Char_To_ASCII.Add("™", 153);
	Global_Char_To_ASCII.Add("š", 154);
	Global_Char_To_ASCII.Add("›", 155);
	Global_Char_To_ASCII.Add("œ", 156);
	Global_Char_To_ASCII.Add("Ÿ", 159);
	Global_Char_To_ASCII.Add("¡", 161);
	Global_Char_To_ASCII.Add("¢", 162);
	Global_Char_To_ASCII.Add("£", 163);
	Global_Char_To_ASCII.Add("¤", 164);
	Global_Char_To_ASCII.Add("¥", 165);
	Global_Char_To_ASCII.Add("¦", 166);
	Global_Char_To_ASCII.Add("§", 167);
	Global_Char_To_ASCII.Add("¨", 168);
	Global_Char_To_ASCII.Add("©", 169);
	Global_Char_To_ASCII.Add("ª", 170);
	Global_Char_To_ASCII.Add("«", 171);
	Global_Char_To_ASCII.Add("¬", 172);
	Global_Char_To_ASCII.Add("®", 174);
	Global_Char_To_ASCII.Add("¯", 175);
	Global_Char_To_ASCII.Add("°", 176);
	Global_Char_To_ASCII.Add("±", 177);
	Global_Char_To_ASCII.Add("²", 178);
	Global_Char_To_ASCII.Add("³", 179);
	Global_Char_To_ASCII.Add("´", 180);
	Global_Char_To_ASCII.Add("µ", 181);
	Global_Char_To_ASCII.Add("¶", 182);
	Global_Char_To_ASCII.Add("·", 183);
	Global_Char_To_ASCII.Add("¸", 184);
	Global_Char_To_ASCII.Add("¹", 185);
	Global_Char_To_ASCII.Add("º", 186);
	Global_Char_To_ASCII.Add("»", 187);
	Global_Char_To_ASCII.Add("¼", 188);
	Global_Char_To_ASCII.Add("½", 189);
	Global_Char_To_ASCII.Add("¾", 190);
	Global_Char_To_ASCII.Add("¿", 191);
	Global_Char_To_ASCII.Add("À", 192);
	Global_Char_To_ASCII.Add("Á", 193);
	Global_Char_To_ASCII.Add("Â", 194);
	Global_Char_To_ASCII.Add("Ã", 195);
	Global_Char_To_ASCII.Add("Ä", 196);
	Global_Char_To_ASCII.Add("Å", 197);
	Global_Char_To_ASCII.Add("Æ", 198);
	Global_Char_To_ASCII.Add("Ç", 199);
	Global_Char_To_ASCII.Add("È", 200);
	Global_Char_To_ASCII.Add("É", 201);
	Global_Char_To_ASCII.Add("Ê", 202);
	Global_Char_To_ASCII.Add("Ë", 203);
	Global_Char_To_ASCII.Add("Ì", 204);
	Global_Char_To_ASCII.Add("Í", 205);
	Global_Char_To_ASCII.Add("Î", 206);
	Global_Char_To_ASCII.Add("Ï", 207);
	Global_Char_To_ASCII.Add("Ğ", 208);
	Global_Char_To_ASCII.Add("Ñ", 209);
	Global_Char_To_ASCII.Add("Ò", 210);
	Global_Char_To_ASCII.Add("Ó", 211);
	Global_Char_To_ASCII.Add("Ô", 212);
	Global_Char_To_ASCII.Add("Õ", 213);
	Global_Char_To_ASCII.Add("Ö", 214);
	Global_Char_To_ASCII.Add("×", 215);
	Global_Char_To_ASCII.Add("Ø", 216);
	Global_Char_To_ASCII.Add("Ù", 217);
	Global_Char_To_ASCII.Add("Ú", 218);
	Global_Char_To_ASCII.Add("Û", 219);
	Global_Char_To_ASCII.Add("Ü", 220);
	Global_Char_To_ASCII.Add("İ", 221);
	Global_Char_To_ASCII.Add("Ş", 222);
	Global_Char_To_ASCII.Add("ß", 223);
	Global_Char_To_ASCII.Add("à", 224);
	Global_Char_To_ASCII.Add("á", 225);
	Global_Char_To_ASCII.Add("â", 226);
	Global_Char_To_ASCII.Add("ã", 227);
	Global_Char_To_ASCII.Add("ä", 228);
	Global_Char_To_ASCII.Add("å", 229);
	Global_Char_To_ASCII.Add("æ", 230);
	Global_Char_To_ASCII.Add("ç", 231);
	Global_Char_To_ASCII.Add("è", 232);
	Global_Char_To_ASCII.Add("é", 233);
	Global_Char_To_ASCII.Add("ê", 234);
	Global_Char_To_ASCII.Add("ë", 235);
	Global_Char_To_ASCII.Add("ì", 236);
	Global_Char_To_ASCII.Add("í", 237);
	Global_Char_To_ASCII.Add("î", 238);
	Global_Char_To_ASCII.Add("ï", 239);
	Global_Char_To_ASCII.Add("ğ", 240);
	Global_Char_To_ASCII.Add("ñ", 241);
	Global_Char_To_ASCII.Add("ò", 242);
	Global_Char_To_ASCII.Add("ó", 243);
	Global_Char_To_ASCII.Add("ô", 244);
	Global_Char_To_ASCII.Add("õ", 245);
	Global_Char_To_ASCII.Add("ö", 246);
	Global_Char_To_ASCII.Add("÷", 247);
	Global_Char_To_ASCII.Add("ø", 248);
	Global_Char_To_ASCII.Add("ù", 249);
	Global_Char_To_ASCII.Add("ú", 250);
	Global_Char_To_ASCII.Add("û", 251);
	Global_Char_To_ASCII.Add("ü", 252);
	Global_Char_To_ASCII.Add("ı", 253);
	Global_Char_To_ASCII.Add("ş", 254);
	Global_Char_To_ASCII.Add("ÿ", 255);
}

const FString FPDFiumCore::PagesDirectoryPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("PDFImporter"))->GetBaseDir(), TEXT("Content")));

FPDFiumCore::FPDFiumCore()
{
	// dllファイルのパスを取得
	FString PDFiumDllPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("PDFImporter"), TEXT("ThirdParty")));
#ifdef _WIN64
	PDFiumDllPath = FPaths::Combine(PDFiumDllPath, TEXT("Win64"));
#elif _WIN32
	PDFiumDllPath = FPaths::Combine(PDFiumDllPath, TEXT("Win32"));
#endif
	PDFiumDllPath = FPaths::Combine(PDFiumDllPath, TEXT("pdfium.dll"));

	// モジュールをロード
	PDFiumModule = FPlatformProcess::GetDllHandle(*PDFiumDllPath);
	if (PDFiumModule == nullptr)
	{
		UE_LOG(PDFImporter, Fatal, TEXT("Failed to load PDFium module"));
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	UE_LOG(PDFImporter, Log, TEXT("PDFium dll loaded"));
}

FPDFiumCore::~FPDFiumCore()
{
	FPlatformProcess::FreeDllHandle(PDFiumModule);
	UE_LOG(PDFImporter, Log, TEXT("PDFium dll unloaded"));
}

bool FPDFiumCore::PDFiumLib_Init()
{
	if (bIsLibInitialized == true)
	{
		UE_LOG(PDFImporter, Log, TEXT("PDFium Library already initialized"));
		return false;
	}
	
	FPDF_LIBRARY_CONFIG config;
	FMemory::Memset(&config, 0, sizeof(config));

	config.version = 2;
	config.m_pUserFontPaths = NULL;
	config.m_pIsolate = NULL;
	config.m_v8EmbedderSlot = 0;
	FPDF_InitLibraryWithConfig(&config);

	bIsLibInitialized = true;

	DefineCharCodes();
	UE_LOG(PDFImporter, Log, TEXT("PDFium Library successfully initialized."));

	return true;
}

bool FPDFiumCore::PDFiumLib_Close()
{
	if (bIsLibInitialized == false)
	{
		UE_LOG(PDFImporter, Log, TEXT("PDFium Library already closed."));

		return false;
	}
	
	bIsLibInitialized = false;
	FPDF_DestroyLibrary();

	UE_LOG(PDFImporter, Log, TEXT("PDFium Library successfully closed."));

	return true;
}

bool FPDFiumCore::PDFiumLib_State()
{
	return bIsLibInitialized;
}


UPDF* FPDFiumCore::ConvertPdfToPdfAsset(const FString& InputPath, int Dpi, int FirstPage, int LastPage, bool bIsImportIntoEditor)
{
	IFileManager& FileManager = IFileManager::Get();
	
	// PDFがあるか確認
	if (!FileManager.FileExists(*InputPath))
	{
		UE_LOG(PDFImporter, Error, TEXT("File not found : %s"), *InputPath);
		return nullptr;
	}
	// 作業用のディレクトリを作成
	FString TempDirPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ConvertTemp"));
	TempDirPath = FPaths::ConvertRelativePathToFull(TempDirPath);
	if (FileManager.DirectoryExists(*TempDirPath))
	{
		FileManager.DeleteDirectory(*TempDirPath);
	}
	FileManager.MakeDirectory(*TempDirPath);
	UE_LOG(PDFImporter, Log, TEXT("A working directory has been created (%s)"), *TempDirPath);

	// PDFからjpg画像を作成
	FString OutputPath = FPaths::Combine(TempDirPath, FPaths::GetBaseFilename(InputPath) + TEXT("%010d.jpg"));
	TArray<UTexture2D*> Buffer; 
	UPDF* PDFAsset = nullptr;

	if (ConvertPdfToJpeg(InputPath, OutputPath, Dpi, FirstPage, LastPage))
	{
		// 画像のファイルパスを取得
		TArray<FString> PageNames;
		IFileManager::Get().FindFiles(PageNames, *TempDirPath, L"jpg");
		
		// 作成したjpg画像を読み込む
		UTexture2D* TextureTemp;
		for (const FString& PageName : PageNames)
		{
			bool bResult = false;
			if (bIsImportIntoEditor)
			{
#if WITH_EDITORONLY_DATA
				//bResult = CreateTextureAssetFromFile(FPaths::Combine(TempDirPath, PageName), TextureTemp);
#endif
			}
			else
			{
				bResult = LoadTexture2DFromFile(FPaths::Combine(TempDirPath, PageName), TextureTemp);
			}
			
			if (bResult)
			{
				Buffer.Add(TextureTemp);
			}
		}

		// PDFアセットを作成
		PDFAsset = NewObject<UPDF>();

		if (FirstPage <= 0 || LastPage <= 0 || FirstPage > LastPage)
		{
			FirstPage = 1;
			LastPage = Buffer.Num();
		}

		PDFAsset->PageRange = FPageRange(FirstPage, LastPage);
		PDFAsset->Dpi = Dpi;
		PDFAsset->Pages = Buffer;
	}

	// 作業ディレクトリを削除
	if (FileManager.DirectoryExists(*TempDirPath))
	{
		FileManager.DeleteDirectory(*TempDirPath, true, true);
		UE_LOG(PDFImporter, Log, TEXT("Successfully deleted working directory (%s)"), *TempDirPath);
	}

	return PDFAsset;
}

bool FPDFiumCore::ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage)
{
	if (bIsLibInitialized == false)
	{
		return false;
	}

	if (InputPath.IsEmpty() == true)
	{
		return false;
	}
	double Sampling = 1;
	FColor BG_Color;
	FString Path = InputPath;

	//if (UGameplayStatics::GetPlatformName() == "Windows")
	//{
	//	FPaths::MakePlatformFilename(Path);
	//}

	if (FPaths::FileExists(Path) == false)
	{
		return false;
	}
	
	if (!(FirstPage > 0 && LastPage > 0 && FirstPage <= LastPage))
	{
		FirstPage = 1;
		LastPage = INT_MAX;
	}
	FPDF_DOCUMENT pdf_doc = FPDF_LoadDocument(TCHAR_TO_ANSI(*InputPath),nullptr);
	for (int32 PageIndex = 0; PageIndex < FPDF_GetPageCount(pdf_doc); PageIndex++)
	{		
		FPDF_PAGE PDF_Page = FPDF_LoadPage(pdf_doc, PageIndex);
		double_t PDF_Page_Width = FPDF_GetPageWidth(PDF_Page);
		double_t PDF_Page_Height = FPDF_GetPageHeight(PDF_Page);

		FVector2D EachResolution = FVector2D(PDF_Page_Width, PDF_Page_Height);
		void* FirstScan = malloc(static_cast<SIZE_T>(PDF_Page_Width * PDF_Page_Height * Sampling * Sampling * 4));
		FPDF_BITMAP PDF_Bitmap = FPDFBitmap_CreateEx((int)(PDF_Page_Width * Sampling), (int)(PDF_Page_Height * Sampling), FPDFBitmap_BGRA, FirstScan, (int)(PDF_Page_Width * Sampling * 4));
		FPDFBitmap_FillRect(PDF_Bitmap, 0, 0, (PDF_Page_Width * Sampling), (PDF_Page_Height * Sampling), BG_Color.ToPackedARGB());
		
		FPDF_FORMHANDLE Form_Handle;
		FMemory::Memset(&Form_Handle, 0, sizeof(Form_Handle));
		FPDF_FFLDraw(Form_Handle, PDF_Bitmap, PDF_Page, 0, 0, (int)(PDF_Page_Width * Sampling), (int)(PDF_Page_Height * Sampling), 0, 0);
		
		FPDFBitmap_Destroy(PDF_Bitmap);
		FPDF_ClosePage(PDF_Page);
	}
	FPDF_CloseDocument(pdf_doc);

	//if (Out_Pages.Num() > 0)
	//{
	//	return true;
	//}

	//else
	//{
		return false;
	//}
}

bool FPDFiumCore::LoadTexture2DFromFile(const FString& FilePath, class UTexture2D*& LoadedTexture)
{
	// 画像データを読み込む
	TArray<uint8> RawFileData;
	if (FFileHelper::LoadFileToArray(RawFileData, *FilePath) &&
		ImageWrapper.IsValid() && 
		ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num())
		)
	{
		// 非圧縮の画像データを取得
#if UE_VERSION_OLDER_THAN(4, 25, 0)
		const TArray<uint8>* UncompressedRawData = nullptr;
#else
		TArray<uint8> UncompressedRawData;
#endif		
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedRawData))
		{
			// Texture2Dを作成
			UTexture2D* NewTexture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
			if (!NewTexture)
			{
				return false;
			}

			// ピクセルデータをテクスチャに書き込む
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			void* TextureData = NewTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
#if UE_VERSION_OLDER_THAN(4, 25, 0)
			FMemory::Memcpy(TextureData, UncompressedRawData->GetData(), UncompressedRawData->Num());
#else
			FMemory::Memcpy(TextureData, UncompressedRawData.GetData(), UncompressedRawData.Num());
#endif	
			NewTexture->PlatformData->Mips[0].BulkData.Unlock();
			NewTexture->UpdateResource();
#else
			void* TextureData = NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedRawData.GetData(), UncompressedRawData.Num());
			NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
			NewTexture->UpdateResource();
#endif					

			LoadedTexture = NewTexture;

			return true;
		}
	}
	return false;
}

bool LoadTextFromFile(const FString& FilePath, TArray<FString>& Out_Texts)
{
	if (bIsLibInitialized == false)
	{
		return false;
	}

	if (FilePath.IsEmpty() == true)
	{
		return false;
	}
	FPDF_DOCUMENT pdf_doc = FPDF_LoadDocument(TCHAR_TO_ANSI(*FilePath),nullptr);

	for (int32 PageIndex = 0; PageIndex < FPDF_GetPageCount(pdf_doc); PageIndex++)
	{
		FPDF_PAGE PDF_Page = FPDF_LoadPage(pdf_doc, PageIndex);
		FPDF_TEXTPAGE PDF_TextPage = FPDFText_LoadPage(PDF_Page);

		int CharCount = FPDFText_CountChars(PDF_TextPage);
		unsigned short* CharBuffer = (unsigned short*)malloc(static_cast<size_t>(CharCount) * 4);
		FPDFText_GetText(PDF_TextPage, 0, CharCount, CharBuffer);

		FString PageText;
		PageText.AppendChars((WIDECHAR*)CharBuffer, CharCount);
		Out_Texts.Add(PageText);

		FPDFText_ClosePage(PDF_TextPage);
		FPDF_ClosePage(PDF_Page);
		free(CharBuffer);
	}
	FPDF_CloseDocument(pdf_doc);

	return true;
}

