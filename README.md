# 🧮 BigInt Calculator – Aritmetika s libovolnou přesností

Tato aplikace byla vytvořena jako semestrální práce v rámci předmětu Programování v jazyce C (KIV/PC) na Fakultě aplikovaných věd ZČU. Jedná se o interpret aritmetických výrazů pracující s celočíselnými hodnotami o teoreticky neomezené velikosti.

## ✨ Hlavní funkcionalita

* 🔢 **Podpora číselných soustav:** Program nativně zpracovává vstupy v desítkové, binární (prefix `0b`) a hexadecimální (prefix `0x`) soustavě.
* 📝 **Zpracování výrazů:** Korektní vyhodnocování infixových výrazů s ohledem na prioritu operátorů a správné párování závorek.
* ⚡ **Vysoký výkon:** Implementované algoritmy umožňují efektivní výpočty i pro velmi vysoké řády – například faktoriál 1000! je vypočten v čase pod 100 ms.
* 🖥️ **Režimy provozu:** Aplikace podporuje interaktivní příkazovou řádku i dávkové zpracování dat ze souboru řádek po řádku.

## 🛠️ Podporované operace a příkazy

Aplikace implementuje širokou škálu matematických operací:
* ➕ **Základní aritmetika:** Sčítání, odčítání, násobení, celočíselné dělení a modulo.
* 📈 **Pokročilé funkce:** Umocňování, faktoriál a unární minus.

Pro ovládání prostředí jsou k dispozici řídicí příkazy:
* `dec`, `bin`, `hex` – Nastavení soustavy pro výpis výsledků.
* `out` – Zobrazení aktuálního nastavení interpretu.
* `quit` – Korektní ukončení programu.

## 🧠 Technická realizace

### 🏗️ Reprezentace dat
Pro vnitřní uložení čísel BigInt bylo zvoleno dynamické pole 32bitových slov (`uint32_t`) v kombinaci s odděleným znaménkem. Tato binární reprezentace byla upřednostněna před desítkovou z důvodu efektivnějšího využití systémových prostředků a možnosti využít nativní 64bitové mezivýpočty pro zpracování přenosu (carry).

### 🔍 Syntaktická analýza
Převod vstupního infixového řetězce na proveditelnou formu zajišťuje **Shunting-yard algoritmus**. Výsledná postfixová notace (reverzní polská notace) je následně vyhodnocována pomocí zásobníku operandů.



### 🛡️ Správa paměti a stabilita
Důraz byl kladen na striktní správu dynamické paměti. Veškeré alokace jsou prováděny podle aktuální potřeby a následně uvolňovány, což bylo verifikováno nástrojem Valgrind jako "leak-free". Program splňuje standard C99 a je plně přenositelný mezi systémy Linux a Windows.

## 🚀 Sestavení projektu

* 🐧 **Linux / Unix:** `make`
* 🪟 **Windows (MinGW):** `mingw32-make -f Makefile.win`
