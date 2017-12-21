Секретный блокнот
=================================
В данном проекте реализованы следующие возможности:

1. Симметричное шифрование: SERPENT(CBC - Cipher Block Chaining)
2. Асимметричное шифрование на эллиптических кривых: BIGN - 45 СТБ(Документация по 45 стандарту в Doc/)
3. Хэш-функция: BeltHASH - 31 СТБ
4. Функция зашиты ключа: BeltKWR - 31 СТБ
5. Аунтефикация по паролю
6. Добавлен срок годности сеансового ключа( 5 сообщений)


Реализация
----------------------

1. Реализован HTTPServer (HTTPServer.py) на Python3.6
2. Реализован HTTPCLient (HTTPCLient.py) на Python3.6
3. Реализован SERPENT (CryptoPart/serpent.c, CryptoPart/serpent.h) на С
4. Реализован BeltHASH & BeltKWR (CryptoPart/belt.h, CryptoPart/belt.c)
5. Реализован BIGN (CryptoPart/ tzi_bign.c)
6. В CryptoPart/wrap.c находится обёртка для использования C-функций в Pyhton)
7. В Crypto.py принимающая обертка для подтягивания C-функций
8. Store/ - хранилище файлов.
9. lib.so - собранная библиотека для вызова wrap.c из Python
10. Пользователи = {user : user, alex: qwe, ivan: 123}

Использование
------------------------
1. Для старта сервера: > python3.6 HTTPServer.py
2. Для старта клиента: > pyhton3.6 HTTPClient.py
3. Если есть желание!!!(не обязательно) 
   можно пересобрать библиотеку( > gcc -c -fPIC *.c *.h
				 > gcc -shared -Wl,-soname,lib.so -o lib.so  *.o ) в CryptoPart/	

Схема
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
![alt text](https://github.com/MD-Levitan/SecureNotepad/blob/master/Diagram.png)

