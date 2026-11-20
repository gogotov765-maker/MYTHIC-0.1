# MYTHIC kernel 0.1

Своё ядро x86 (32 бит) на C и ассемблере. Multiboot, VGA-текст, клавиатура PS/2, командная строка.

## Запуск на ПК
Нужны: gcc (с поддержкой -m32), binutils, make, QEMU.

    make run          # собрать и запустить в QEMU

Windows: проще всего через WSL (Ubuntu):
    sudo apt install build-essential gcc-multilib qemu-system-x86
    make run

## Сборка через GitHub Actions
Залей папку в репозиторий -> вкладка Actions -> Build MYTHIC -> скачай artifact `mythic`
(внутри mythic.iso и mythic.bin). Запуск ISO:

    qemu-system-i386 -cdrom mythic.iso

ISO можно также записать на флешку или подключить в VirtualBox / VMware.

## Команды
help, ver, clear, echo <текст>, color <HH>, reboot, halt

## Структура
- src/boot.s    - заголовок Multiboot, стек, вход в ядро
- src/kernel.c  - экран, клавиатура, оболочка
- src/linker.ld - раскладка памяти (ядро в 1 МБ)
- iso/          - конфиг GRUB для ISO
- Makefile, .github/workflows/build.yml

## Дальше
Прерывания (IDT/PIC/таймер) -> память (paging, куча) -> файловая система -> графика.
