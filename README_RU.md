[![EN](Icon/en.png)](https://github.com/fttlov/JCAdvance)
← En

## Что это:

Joy-con Advance - это эмулятор Xbox геймпада с расширенным функционалом для Joy-con, Pro Controller, DualShock 4 и DualSense Edge, основанный на DSAdvance от r57zone <br>

## Основные возможности:
- Назначайте кнопки вашего геймпада для эмуляции кнопок Xbox, клавиатуры и мыши <br>
- Режимы гироскопа: управление курсором мыши/виртуальным стиком; эмуляция руля для вождения, Aircraft <br>
- Горячие кнопки для смены режимов и профилей во время игры
- Настройки чувствительности гироскопа, мертвых зон стиков и триггеров, замена левого на правый <br>
- Поддержка вибрации
- Joy-con'ы объединяются в единый Xbox геймпад
- Функция Magic Wheel: используйте гироскоп и движение в различные стороны как дополнительные кнопки/действия
- Поддержка адаптивных триггеров
- Поддержка подключения двух геймпадов
- Внешние педали: подключите ваш руль/педали и используйте их как аналоговые триггеры и не только
- Минимальное потребление ресурсов. CPU usage 0.20% - 0.50%<br>

## В чём отличия:
- В концепции применения Gyro Motion
- user-friendly GUI
- багфиксах и новых функциях <br>

DSAdvance - это, прежде всего, эмулятор для двуручных контроллеров от Sony. С поддержкой Joy-con.<br>
JCAdvance не просто делает фокус на удобстве взаимодействия с Joy-con, но и даёт возможность гибкой настройки работы гироскопа для различных устройств. Убеждён, что в этом аспекте двуручные контроллеры и Joy-con'ы имеют различную концепцию применения: <br>
- Двуручные: Gyro Motion используется для доводки при прицеливании, дополняя классический looking/aiming на стиках <br>
- Joy-con'ы - это свободные руки, и, как следствие, иные степени свободы, полноценное использование гироскопа для обзора/прицеливания в FPS TPS играх без использования стика

## Что нового:
- Config.exe. Основные настройки, привязка кнопок и горячих клавиш, опции гироскопа теперь в удобном GUI конфигураторе 
<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config1_ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config2_ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config3_ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config4_ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config5_ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config6_ru.png" width="150"></td>
  </tr>
</table>

- Все цифровые кнопки геймпадов теперь доступны для эмуляции кнопок Xbox, клавиатуры и мыши в одном профиле <br>
- Автоматическая привязка с помощью кнопки "Bind" или вручную, через выпадающий список всех доступных кнопок <br>
- Менеджер профилей. Создавайте и управляйте профилями в одной вкладке конфигуратора <br>
- Полностью настраиваемые горячие клавиши для различных Gyro режимов с поддержкой комбинации кнопок (пример: R+HOME) <br>
- Добавлена опция Gyro Motion Space. Гибкая настройка поведения гироскопа для двуручных геймпадов и Joy-con (подробнее в техническом разделе) <br>
- Добавлена опция частоты опроса. Выше частота - больше плавности Gyro mouse <br>
- Добавлена опция чтения данных с гироскопа только для левого Joy-con <br>
- Добавлен EMA фильтр сглаживания движений <br>
- Добавлена опция нелинейного хода для стиков
- (Экспериментально) Теперь поддерживаются почти все dinput рули/педали, не только Arduino (см. технический раздел)
- Новое главное окно программы с текущим статусом настроек и горячих клавиш. Классическое меню по нажатию ALT-Z
 <table align="center">
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Main1_ru.png" width="150"></td>
  </tr>
</table>

## Необходимое ПО:
[ViGEm Bus Driver](https://github.com/nefarius/ViGEmBus) - Драйвер виртуального XBOX Gamepad от nefarius <br>
[Microsoft Visual C++ Redistributable 2017](https://learn.microsoft.com/en-us/answers/questions/4137965/download-link-for-microsoft-visual-c-2017-redistri) или новее

## Как пользоваться:
1. Скачиваем последний релиз [Releases](https://github.com/fttlov/JCAdvance/releases) 
2. Распаковываем архив в любую папку
3. Запускаем `JCAdvance.exe`, подключаем поддерживаемый контроллер - наслаждаемся!

## Важно!
Некоторые игры не будут работать корректно из-за проблемы двойного ввода - одновременной инициализации реального геймпада и виртуального.
Для решения проблемы рекомендуется использовать утилиту для скрытия геймпада от игр или всей системы [HidHide](https://github.com/nefarius/HidHide) от того же nefarius.
Подробную инструкцию по установке и настройке можно найти [здесь](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/).

<details>
  <summary><h2>Техническая информация</h2> (разверунуть)</summary>
  <br>
- Интерфейс и профили 
  
Введено новое 3-layer меню. 0-до подключения устройств, 1-после подключения, 2-классическое меню. Layer 2 слишком перегружен, но оставлен для cсовместимости, ифнормации о хоткеях и старом управлении профилями. Профили: в оригинальном коде было жесткое разделение на профили для Xbox (чтение .ini из папки XboxProfile) и Keyboaed/Mouse (KMProfile). Было невозможно в одном профиле эмулировать кнопки XBOX и KM. В текущей реализации основная папка XboxProfile с эмуляцей любых кнопок, менежмнет профилей через Config.exe. Однако, вы всё ещё можете переключаться с помощью горячих клавиш между профилями как внутри папки XboxProfile так и между папками, KMProfiles оставлена для совместимости. Профиль меняется только для текущей сессии.
  
- Частота опроса

теперь по умолчанию 125hz (sleepTimeout=8 в config.ini; 1 sec = 1000ms / 8) <br>
Из-за некоторых функциональных ограничений и багов в JoyShockLibrary автор DSAdvance использовал настройку sleepTimeout=15, что соответствует частоте опроса 66,67Hz,  что, очевидно,  недостаточно для плавной работы той же Gyro Mouse. <br>
Примером ограничений является функция Wheel, которая пропускала нажатие виртуальных кнопок при высокой частоте опроса. Код был переписан, создан таймер для надёжной регистрации нажатия кнопок даже при частоте 250hz. Кстати, даже при такой частоте опроса программа всё равно потребляет не более  ~ 0,60% CPU. <br>

- Исправлены баги и добавлены некоторые улучшения в JoyshockLibrary. Можно ознакомиться здесь: https://github.com/fttlov/JoyShockLibrary  <br>
- Благодаря этому стало возможно исправление странного бага с Gyro Stick, когда даже при плавном движении стик периодически отстреливал к центру на доли миллисекунд и возвращался обратно, а также появилась возможность нормально использовать важный параметр Gyro Motion Space <br>

- Gyro Motion Space:

Опция отвечает за то, как данные с гироскопа от движения рук будут обработаны и выданы в виде движения курсора/стика на экране. В оригинальном проекте опция была захардКОДена со значением 0, что сильно ограничивало возможности работы с гироскопом. <br>
Понять работу можно интуитивно поробовав разные значения. Но пусть будет и описание, без всяких жутких формул. <br>
Для двуручных геймпадов можно использовать все три значения 0,1,2. Мы остановимся на 0 и 2. Здесь значения влияют только на ось X, то есть какого рода движение нужно сделать, чтобы сместить курсор влево-вправо, а также под каким углом мы держим геймпад. Ось Y всегда работает одинаково. Используем пример со стандартным хватом, когда кнопки L R смотрят примерно под 45 от нас. Чтобы подвинуть курсор вправо-влево: <br>
0 - двигаем геймпадом как рулём в машине (наклон/Roll)<br>
2 - двигаем геймпадом как рычагами в танке (поворот/Yaw). Если опустить геймпад горизонтально, курсором снова нужно двигать как рулём. Но это не удобной хват, особенно для Gyro, поэтому не в счёт. <br>

Для Joy-con ситуация выглядит иначе, совсем другие возможности, но есть и свои ограничения, которые воздействуют на важные нам ось X и Y. У нас свободная рука, движение курсором осуществляется или за счёт поворота кисти (менее точно, но быстро) или всей рукой (предплечьем, более точно но медленно). На точное повторение  вектора движения руки курсором (двигаем руку вверх-влево - курсор идёт туда же) негативно влияют два фактора: а) угол поворота кисти (по и против часовой, ось Z roll, кнопки SL SR смотрят в пол или потолок) и б) положение геймпада (горизонтальное - кнопки R, ZR направлены на экран, вертикальное - смотрят в потолок). <br>
0 - угол поворота кисти всегда оказывает влияние при любом положении геймпада. То есть, чтобы провести курсор горизонтально влево, нужно вести кисть или всю руку влево не проворачивая кисть. <br>
1- Угол поворота кисти не важен (в пределах 180 градусов, то есть когда кнопки SL SR смотрят в пол или потолок), но важен хват. При более менее горизонтальном хвате (кнопки R ZR смотрят в экран) курсор будет чётко следовать вектору движения руки. Минус данного режима: при вертикальном хвате (R ZR смотрят в потолок) повороты кисти начинают управлять осью X. <br>
По описанию может сложиться впечатление, что играть так вообще невозможно, везде свои минусы. Но это не так, рука (мозг) быстро адаптируется, все режимы играбельны (кроме Joy-con 2). Тестируйте, выбирайте удобный для вас и вперёд.<br>

- EMA фильтр. Темпоральный фильтр сглаживания. Может сгладить даже трясущиеся руки алкаша. Но, как и все подобные фильтры берет часть информации из предыдущего кадра, а значит добавляет задержку ввода - инпут лаг. Чем больше берёт информации, тем больше задержка. При 60fps т значения такие: 25 добавляет ~2.7мс; 50 ~8мс; 75 ~24мс. При большем fps меньше задержка.<br>

- Исправлены возможные косяки с вибрацией joy-con. Ситуация  двоякая. Код тянется ещё с первого PC драйвера для Joy-con от 2018 года. Но вроде работает. У меня нет оригинальных joy-con, возможно есть какие-то проблемы с HD Rumble. <br>

- Улучшен код при подключении/отключении геймпадов. Особенно касается скорости подключения двух joy-con'ов<br>

- Исправлен баг зависание программы при одновременном отключении двух Joy-con'ов<br>

- Исправлен баг при котором: если подключить joy-con(1), отключить, подключить joy-con(2) - он не будет реагировать на ввод<br>

- Исправлен баг с не отображением заряда второго Joy-con'a<br>

- Внешние педали:

Насколько можно понять, автор DSADvance делал упор на проект педалей через Arduino, Хотя и заявлялось, что можно подключить dinput педали и они будут работать как триггеры, в коде был поиск dinput устройств, но работал он странно. <br>
Переписан код для реального поиска устройств в реестре, подхватывание имени, фильтр отсечения геймпадов (только нужны рули и педали). В Config.ini добавлена возможность чётко прописать имя устройства (из реестра или joy.cpl), а также смена осей, так как у разных рулей педали могут висеть на разных осях. Добавлено все 8 Windows осей для самостоятельно перебора. По умолчанию z и z-roll. <br>
Как подключить: подсоединяем руль/педали, в config.ini внизу ищем секцию [ExternalPedals], делаем DInput=1, сохраняем и запускаем JCAdvance. При старте будет опрашиваться dinput устройства. У вас будет пару секунд чтобы заметить какие устройства найдены и инициализированы. Увидели свой руль - почти всё говотово. Подключаем геймпад и проверяем работу в XinputTest или joy.cpl. Если триггеры не реагируют на нажатие педалей пробуем разные оси для Pedal1Axis Pedal2Axis в cofig.ini.
Если не работает, попробуйте прописать название своего устройства (cmd - joy.cpl вручную в Config.ini DeviceName = вместо AUTO.

- Проблемы отладки: <br>
В данный момент отсутствует доступ к геймпаду Sony и нормальный дебаггинг невозможен. Код для Sony не менялся, но всё может быть  <br>
Тесты контроллеров Joy-con проводились на MobaPad M6s с упрощённой вибрацией, поэтому как работает HD вибрация и работает ли она вообще сказать затруднительно <br>
В тестировании функции внешних педалей использовался старый руль Logitech Wingman. Тесты прошли успешно, но выборка явно недостаточная

</details>

### Возможные проблемы:
- Congfig.exe написан на AHK и он немного капризен к DPI в Windows. Если текст не влезает в окно или наклдывается друг на друга, временно уменьшите значение DPI или смените разрешение экрана
- Возможна ложно-позитивная реакция антивируса на Config.exe из-за вызовов библиотек. Его source открыт, но если вы параноик - немедленно удалите "вирус" и ковыряйте конфиги
- Если вы столкнулись проблемой плохого коннекта или проблем с вибрацией при двух одновременно подключённых Joy-con'ах, попробуйте другой bluetooth адаптер. Известные беспроблемные адаптеры: ASUS USB-BT400 и более дешёвые аналоги на этом же чипе BCM20702 (есть и за 200 р.), некоторые bluetooth 4.0 от Ugreen

### Список поддерживаемых контроллеров ограничен библиотекой Joyshocklibrary
И не будет расширяться до перехода на SDL в очень необозримом будущем.

### Credits
* [DSAdvance](https://github.com/r57zone/DSAdvance) - that was the starting point for me. r57zone has done a really great job and I thank him for that.
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation. Also uses some code from this library and [JibbSmart snippet](https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df) for aiming.
* [ViGEm](https://github.com/nefarius/ViGEmBus) for the ability to emulate various gamepads and [HidHide](https://github.com/nefarius/HidHide/) for hiding them.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* DS4Windows[[1]](https://github.com/Jays2Kings/DS4Windows)[[2]](https://github.com/Ryochan7/DS4Windows) for the battery level.
* [JoyCon-Driver](https://github.com/fossephate/JoyCon-Driver/blob/main/joycon-driver/include/Joycon.hpp) for Joy-Cons rumble.
* [Valkirie](https://github.com/Valkirie/JoyShockLibrary/commits/HDRumble) for adaptive triggers over Bluetooth.

<details>
  <summary><h3>Сборка</h3> (развернуть)</summary>

### Сборка
0. Если вы ничего не понимаете в программировании, как и я, чётко следуйте инструкции:
1. Скачать Source, распаковать
2. Скачать Visual Studio 17 [установить](https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/VS17_Install.png) с этими компонентами <br>
3. Скачать Windows SDK 10.0.1776.x и [установить](https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/SDK_Install.png) с этими компонентами <br>
Имеете SDK новее не забудьте сделать Retarget Solution
4. Откройте DSAdvance.vcxproj, выберите `Release` build type, `x86` или `x64` архитектуру и скомпелируйте
5. Для компиляции Config.ahk используйте Ahk2exe с base file: v2 U32  или U64. Скрипту нужны: JoyShockLibrary.dll в корне и иконки из папки `\Icon`.
JCadvance, Joyshocklibrary.dll и Config.exe должны быть оиднаковой разрядности.

### Редактирование
Для редактирования также можно использовать VS Code с clangd. Соотв. файлы для совместимости добавлены
</details>

## Поддержка проекта 🍺

Понравилась программа - купите автору пивка!

#### 🇷🇺  Россия/Беларусь 👉 **[CloudTips (СБП, TPay, карты МИР)](https://pay.cloudtips.ru/p/3ae0e7e5)** <table align="center">
<details>
  <summary>📷 QR code </summary>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/cloudtips.png" width="160" alt="CloudTips QR" />
</details>

#### 🌐 Worldwide 👉 **[Lava.top (Apple Pay / PayPal / Visa / Mastercard)](https://app.lava.top/4003151013?tabId=donate)** <sub> (без регистрации, email для чеков и истории переводов)</sub>


<details>
<summary><h4>🪙 Криптовалюта (Прямой перевод)</h4> </summary>

Пожалуйста, убедитесь, что вы отправляете перевод через **правильную сеть**, указанную внутри каждого спойлера!

<details>
<summary>🟢 <b>USDT (BSC / BEP-20) — Низкая комиссия</b></summary>
<br>
<ul>
  <li><b>Сеть:</b> BNB Smart Chain (BEP-20)</li>
  <li><b>Адрес:</b> <code>0x7bd7bb2a21d3489a6bce6de29d9e504eb6bb1429</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/BSC%20(BEP20)%200x7bd7bb2a21d3489a6bce6de29d9e504eb6bb1429.png" width="160" alt="USDT BEP-20 QR" />
<br><br>
</details>

<details>
<summary>🟢 <b>USDT (Tron / TRC-20) — Классический вариант</b></summary>
<br>
<ul>
  <li><b>Сеть:</b> Tron (TRC-20)</li>
  <li><b>Address:</b> <code>TXAdZL5Y4FqhUdZP5TeShMyXPk9hBWh27o</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Tron%20(TRC20)%20TXAdZL5Y4FqhUdZP5TeShMyXPk9hBWh27o.png" width="160" alt="USDT TRC-20 QR" />
<br><br>
</details>

<details>
<summary>🔵 <b>TON (Toncoin) — Мгновенно и без комиссии</b></summary>
<br>
<ul>
  <li><b>Сеть:</b> TON Chain</li>
  <li><b>Адрес:</b> <code>UQC0uPYhCF5R3OZKC_HKsNi84oLtVvXBneI8fKVwhF2Ykcro</code> (👉 <b><a href="https://tonkeeper.app/transfer/UQC0uPYhCF5R3OZKC_HKsNi84oLtVvXBneI8fKVwhF2Ykcro">Открыть в кошельке</a></b>)</li>
  <li><b>Важно:</b> Memo / Tag указывать не нужно!</li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/TON%20(TON)%20UQC0uPYhCF5R3OZKC_HKsNi84oLtVvXBneI8fKVwhF2Ykcro.png" width="160" alt="TON QR" />
<br><br>
</details>

<details>
<summary>🪙 <b>LTC (Litecoin) — Минимальная комиссия сети</b></summary>
<br>
<ul>
  <li><b>Сеть:</b> Litecoin (LTC)</li>
  <li><b>Адрес:</b> <code>Lb3GnY7u8aKYsFQi7nY4gi5QeWb9Y8QDeR</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/LTC%20(LTC)%20Lb3GnY7u8aKYsFQi7nY4gi5QeWb9Y8QDeR.png" width="160" alt="LTC QR" />
<br><br>
</details>

<details>
<summary>🔶 <b>BTC (Bitcoin)</b></summary>
<br>
<ul>
  <li><b>Сеть:</b> Bitcoin</li>
  <li><b>Адрес:</b> <code>1MVqQdFdf8WCGyyZP6nqCE314nZj7mDGYR</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/BTC%20(BTC)%201MVqQdFdf8WCGyyZP6nqCE314nZj7mDGYR.png" width="160" alt="BTC QR" />
<br><br>
</details>

<details>
<summary>🔷 <b>ETH (Ethereum / ERC-20)</b></summary>
<br>
<ul>
  <li><b>Сеть:</b> Ethereum (ERC-20)</li>
  <li><b>Адрес:</b> <code>0x7bd7bb2a21d3489a6bce6de29d9e504eb6bb1429</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/ETH%20(ERC20)%200x7bd7bb2a21d3489a6bce6de29d9e504eb6bb1429.png" width="160" alt="ETH QR" />
<br><br>
</details>

</details>

### Обратная связь
`fttlkov@gmail.com`
