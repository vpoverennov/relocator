Relocator
=========

Relocator allows you to launch games in fixed mode(fake fullscreen), that is window is in windowed mode but occupies whole screen. It means you can alt-tab to any program instantly and game will not minimize. It was created for World Of Tanks but may work for other games if they can run in window.

Tested with: 
* World Of Tanks
* Lineage II Interlude
* Unreal Tournament 2004
* Warcraft III
* World of Warplanes

Installation and settings
-------------------------

The case of World Of Tanks. To launch WoT in fixed mode you need to

* [Download relocator](https://github.com/vpoverennov/relocator/releases/tag/original)
* Copy it into WoT folder (for example `G:\games\wot\`)
* Change link setting so it looks like this
`G:\games\wot\rl.exe WorldOfTanks`
* Set up WoT to run in window mode (alt+enter when in fullscreen)

Additional info
---------------

Warcraft III
------------

requires `-window` argument, so path will look like

```rl.exe war3.exe -window```


Building
--------

The [original release](https://github.com/vpoverennov/relocator/releases/tag/original) of relocator was built with mingw using [scons](https://www.scons.org/).

For newer releases using [zig](https://ziglang.org/) is recommended

Download zig (at least 0.15.2) from https://ziglang.org/learn/getting-started/

Run
```shell
zig build --release
```
This will create `zig-out/bin/rl.exe` - move it wherever you want and use


Have Fun!



Релокатор
=========

Релокатор позволяет запускать игры в fixed mode(fake fullscreen), то есть в оконном режиме но на весь экран. При этом используется все пространство экрана и сохраняются все плюсы оконного режима. Это значит вы можете мгновенно alt-tab'ать в другое окно и игра не будет сворачиваться. Он был создан для запуска World Of Tanks в этом режиме, но может сработать и для других игр, умеющих оконный режим.

Проверено с:
* World Of Tanks
* Lineage II Interlude
* Unreal Tournament 2004
* Warcraft III
* World of Warplanes

Установка и настройка
---------------------

На примере игры World Of Tanks. Чтобы запустить WoT в фиксед моде необходимо:

* [скачать релокатор](https://github.com/vpoverennov/relocator/releases/tag/original)
* Скопировать его в папку WoT (предположим это `G:\games\wot\`)
* Изменить в свойствах ярлыка путь на примерно такой (скрин ниже)
`G:\games\wot\rl.exe WorldOfTanks`
* Настроить танки на работу в оконном режиме (alt+enter когда в полноэкранном)

Дополнительная информация
-------------------------

Warcraft III
------------

требует аргумент `-window`, поэтому путь будет выглядеть как

```rl.exe war3.exe -window```
