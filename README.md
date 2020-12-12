# loginhelper
Allows automatic login+pin and makes pin input easier.

## Features
- Pin numbers are unrandomized
- Allow keyboard input on pin window
- (Optional) Auto login using environment variables or command line arguments with ingame-login patch or on private server
    
    This makes it possible to login straight to char select with just a .bat file (see example in .zip) and allows a more simple way of selecting accounts in launchers, including private server (CN175) launchers.
- (Optional) Auto pin using environment variables or command line arguments
    
    Separate from auto login, you can use this without ingame-login and with pretty much any launcher that allows custom cmd arguments or environment variables.
    
## Notes
Please prefer environment variables over cmd arguments whenever posibile, it's safer and supports all characters.

If you're using this plugin as a template, consider [using updated dependencies][0.0].

NCSoft EU/NA passwords are limited to 16 characters. If you're getting "wrong password" check if you have too many letters.

[0.0]: https://github.com/bnsmodpolice/bnspatch/tree/master/dependencies

## Download
<https://mega.nz/folder/4EUF2IhL#Ci1Y-sbbyw7nwwMGvHV2_w>

## Acknowledgements
- [microsoft/**Detours**][1.0] (MIT license)
- [microsoft/**GSL**][1.1] (MIT license)
- [processhacker/**phnt**][1.2] (CC-BY-4.0 license)
- [JustasMasiulis/**xorstr**][1.3] (Apache-2.0 license)
- [bnsmodpolice/**bnspatch**][1.4] (MIT License) ([searchers.h][1.5])
- [bnsmodpolice/**pluginloader**][1.6] (MIT License)

[1.0]: https://github.com/microsoft/Detours
[1.1]: https://github.com/microsoft/GSL
[1.2]: https://github.com/processhacker/phnt
[1.3]: https://github.com/JustasMasiulis/xorstr
[1.4]: https://github.com/bnsmodpolice/bnspatch
[1.5]: https://github.com/bnsmodpolice/bnspatch/blob/5d49740e4395bfb9bf6a484f74f7e3ef9ea37931/src/client/searchers.h
[1.6]: https://github.com/bnsmodpolice/pluginloader