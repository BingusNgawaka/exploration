# PicoBoy
Had an old pico and a [1"44in LCD Display from Waveshare](https://www.waveshare.com/wiki/Pico-LCD-1.44?srsltid=AfmBOorHh7DxfAOohGTAJg0tjV6X2brcfTwUVk3tDCImlKbAE45uEVh_) so decided to use the manufacturers dodgy SDK to make a gameboy like thing.

# Build
Connect Pico via MicroUSB
```
mkdir build
cd build
cmake ..
sudo make
```
Note the sudo I know it's dodgy but you can look at the code it's literally just so that picotool can force the pico into BOOTSEL mode to flash the uf2 file.
If you don't want to sudo a random make file (I don't blame you) run the make file normally then run:
```
sudo ./../picotool/build/picotool load -x main.uf2 -f
```
as picotool is the only thing that needs sudo because it does some wizadry with media devices and USB ports
