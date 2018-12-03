# MiFlo assets

De assets (afbeeldingen en geluidsfragmenten) voor op de SD card van de MiFlo.

Om deze te kunnen gebruiken, moeten deze geconverteerd worden. 
In deze readme staan de instructies voor linux/mac.

## Conversie van de afbeeldingen

Via het `convert` pakket. Bijvoorbeeld te installeren via apt (linux) of [brew](https://brew.sh/index_nl) (mac).

Zie [convert.sh](convert.sh) voor een bash script.

## Conversion van de audio

de [gameduino2 library](https://github.com/jamesbowman/gd2-lib) heeft de nodige conversietools aan boord om deze audio om te zetten in zogenaamde headers. 

### install gameduino library

```
sudo easy_install -U gameduino2
``` 

### generate headers with following code

```
gd3asset -3 -f all.gd2 -o all.h f_bol.wav f_uitst.wav g_cool.wav g_formid.wav g_machti.wav g_stieva.wav g_woew.wav m_flink2.wav m_spinne.wav f_dduim.wav f_wauw.wav g_dduim2.wav g_goed.wav g_muppet.wav g_super.wav g_woop.wav m_flink3.wav m_super.wav f_go.wav g_allez.wav g_demax.wav g_goedzo.wav g_prima.wav g_tsjing.wav g_zogoed.wav m_gewel.wav f_komaan.wav g_bol.wav g_fantas.wav g_hopla.wav g_scool.wav g_wiii.wav m_flink.wav m_goed.wav
gd3asset -3 -f minne.gd2 -o minne.h m_flink3.wav m_super.wav m_gewel.wav m_flink.wav m_goed.wav m_flink2.wav m_spinne.wav
gd3asset -3 -f flo.gd2 -o flo.h f_dduim.wav f_wauw.wav f_bol.wav f_uitst.wav f_go.wav f_komaan.wav
gd3asset -3 -f genflo.gd2 -o genflo.h f_dduim.wav f_wauw.wav f_bol.wav f_uitst.wav f_go.wav f_komaan.wav g_cool.wav g_formid.wav g_machti.wav g_stieva.wav g_woew.wav g_dduim2.wav g_goed.wav g_muppet.wav g_super.wav g_woop.wav g_allez.wav g_demax.wav g_goedzo.wav g_prima.wav g_tsjing.wav g_zogoed.wav g_bol.wav g_fantas.wav g_hopla.wav g_scool.wav g_wiii.wav
gd3asset -3 -f genminne.gd2 -o genminne.h g_cool.wav g_formid.wav g_machti.wav g_stieva.wav g_woew.wav g_dduim2.wav g_goed.wav g_muppet.wav g_super.wav g_woop.wav g_allez.wav g_demax.wav g_goedzo.wav g_prima.wav g_tsjing.wav g_zogoed.wav g_bol.wav g_fantas.wav g_hopla.wav g_scool.wav g_wiii.wav m_flink3.wav m_super.wav m_gewel.wav m_flink.wav m_goed.wav m_flink2.wav m_spinne.wav
```