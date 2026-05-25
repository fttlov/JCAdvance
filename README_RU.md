[![EN](Icon/en-US.png)](README.md)
[![RU](https://github.com/fttlov/JCAdvance/blob/main/Icon/ru-RU.png)](https://github.com/fttlov/JCAdvance/blob/master/README.RU.md)
← Выберите язык

## Что это:

Joy-con Advance - это эмулятор Xbox геймпада с расширенным функционалом для Joy-cons, Pro Controller, DualShock 4, and DualSense Edge, основанный на [DSAdvance](https://github.com/r57zone/DSAdvance) от r57zone. <br>

## Основные возможности:
- Назначайте кнопки вашего геймпада для эмуляции нажатия кнопок Xbox, клавиатуры и мыши <br>
- Режимы гироскопа: управление курсором мыши/виртуальным стиком; эмуляция руля для вождения, Aircraft <br>
- Горячие кнопки для смены режимов
- Настройки чувствительности гироскопа, мертвых зон стиков и триггеров, замена левого на правый <br>
- Поддержка вибрации
- Joy-con'ы объединяются в единый Xbox геймпад
- Функция Magic Wheel: используйте гироскоп и движение в различные стороны как дополнительные кнопки/действия
- Поддержка адаптивных триггеров
- Поддержка подключения двух геймпадов
- Внешние педали: подключите ваш руль/педали и используйте педали как аналоговые триггеры и не только

## В чём отличия:
В концепции использования Gyro Motion, user-friendly GUI, багфиксах и некоторых нововведениях. <br>
DSAdvance - это прежде всего эмулятор для двуручных контроллеров от Sony. В JCAdvance сделан фокус на Joy-con'ы и управление с помощью гироскопа. <br>
Убеждён, что в этом аспекте двуручные контроллеры и Joy-con'ы имеют различную концепцию применения. <br>
Двуручные: Gyro Motion используется для доводки при прицеливании, дополняя классический looking/aiming на стиках. <br>
Joy-con'ы - это свободные руки, и, как следствие, иные степени свободы, полноценное использование гироскопа для обзора/прицеливания в FPS TPS играх без использования стика.


## Что нового:
- Config.exe. Все основные настройки, опции гироскопа, привязка кнопок и горячих клавиш теперь в удобном GUI конфигураторе 
<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config1_ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config2_ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config3_ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config4-ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config5_ru.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config6_ru.png" width="150"></td>
  </tr>
</table>
- Теперь можно привязывать абсолютно все цифровые кнопки для эмуляции кнопок Xbox, клавиатуры и мыши в одном профиле <br>
- Менеджер профилей. Создавайте и управляйте профилями в одной вкладке конфигуратора <br>
- Полностью настраиваемые горячие клавиши для различных Gyro режимов с поддержкой комбинации кнопок (пример: R+HOME) <br>
- Добавлена опция Gyro Motion Space. Гибкая настройка поведения гироскопа для двуручных геймпадов и Joy-con (подробнее в техническом разделе) <br>
- Добавлена опция частоты опроса программы. Выше частота - больше плавности Gyro mouse <br>
- Добавлена опция чтения данных гироскопа с левого Joy-con <br>
- Добавлен EMA фильтр сглаживания движений <br>
- Добавлена опция нелинейности для стиков <br>
- (Экспериментально) Теперь поддерживаются почти все dinput рули/педали, не только Arduino (см. технический раздел) <br>
- Новое главное окно программы с текущим статусом настроек и горячих клавиш. Классическое меню по нажатию ALT-Z 
<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/Main1.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/Screenshot_Main2.png" width="150"></td>
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


## Возможные проблемы:
- Если вы столкнулись проблемой плохого коннекта или проблем с вибрацией при двух одновременно подключённых Joy-con'ах, попробуйте другой bluetooth адаптер. Известные беспроблемные адаптеры: ASUS USB-BT400 и более дешёвые аналоги на этом же чипе BCM20702 (есть и за 200 р.), некоторые bluetooth 4.0 от Ugreen.
- В данный момент  отсутствует доступ к геймпаду Sony, проверить их работу возможности нет. Код оригинальной программы для них не менялся, но всё может быть.
- Тесты для контроллеров Joy-con проводились на MobaPad M6s с упрощённой вибрацией, поэтому как работает HD вибрация и работает ли она вообще сказать затруднительно.
- При тестировании внешних педалей для работы в качестве триггеров использовался старый Logitech Wingman. Тесты прошли успешно, но выборка явно недостаточная. 

## Сборка
1. Скачать Source, распаковать
2. Скачать Visual Studio 17 [установить](https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/VS17_Install.png?token=GHSAT0AAAAAADYRJAGYXVW5TUCNT2QJLEOU2QML4CA) с этими компонентами <br>
3. Скачать Windows SDK 10.0.1776.x и [установить](https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/SDK_Install.png?token=GHSAT0AAAAAADYRJAGZXLP2JUHLXWREPIBM2QML43A) с этими компонентами <br>
Имеете SDK новее не забудьте сделать Retarget Solution
4. Откройте DSAdvance.vcxproj, выберите `Release` build type, `x86` или `x64` архитектуру и скомпелируйте
5. Для компиляции Config.ahk используйте Ahk2exe с base file: v2 U32  или U64 (зависит от разрядности JoyshockLibrary). Скрипт использует JoyShockLibrary.dll и иконки из папки `\Icon`.

## Editing
Для редактирования можно использовать VS Code с clangd. Соотв. файлы для совместимости добавлены

## Feedback
`fttlkov@gmail.com`

## Feedback
`fttlkov@gmail.com`
