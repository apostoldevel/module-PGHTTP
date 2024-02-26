[![en](https://img.shields.io/badge/lang-en-green.svg)](https://github.com/apostoldevel/module-PGHTTP/blob/master/README.md)

Postgres Fetch
-
**PGHTTP** - модуль для [Апостол](https://github.com/apostoldevel/apostol).

Описание
-
**PGHTTP** предоставляет возможность принимать и обрабатывать HTTP-запросы на языке программирования PL/pgSQL.

Входящие запросы
-

Модуль направляет входящие HTTP `GET` и `POST` запросы в базу данных PostgreSQL вызывая для их обработки функции `http.get` и `http.post` соответственно.

Входящие запросы записываются в таблицу `http.log`.

Установка базы данных
-
Следуйте указаниям по установке PostgreSQL в описании [Апостол](https://github.com/apostoldevel/apostol#postgresql)

Установка модуля
-
Следуйте указаниям по сборке и установке [Апостол](https://github.com/apostoldevel/apostol#%D1%81%D0%B1%D0%BE%D1%80%D0%BA%D0%B0-%D0%B8-%D1%83%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0)

## Общая информация
* Базовая конечная точка (endpoint url): [http://localhost:8080/api/v1](http://localhost:8080/api/v1);
    * Модуль принимает только те запросы путь которых начинаются с `/api` (можно изменить в исходном коде).
* Все конечные точки возвращают: `JSON-объект` или `JSON-массив` в зависимости от количества записей в ответе. Изменить это поведение можно добавив в запрос параметр `?data_array=true` тогда ответ будет `JSON-массив` в независимости от количества записей;

### Формат endpoint url:
~~~
http[s]://<hosthame>[:<port>]/api/<route>
~~~

## HTTP коды возврата
* HTTP `4XX` коды возврата применимы для некорректных запросов - проблема на стороне клиента.
* HTTP `5XX` коды возврата используются для внутренних ошибок - проблема на стороне сервера. Важно **НЕ** рассматривать это как операцию сбоя. Статус выполнения **НЕИЗВЕСТЕН** и может быть успешным.

## Передача параметров
* Для `GET` конечных точек параметры должны быть отправлены в виде `строки запроса (query string)` .
* Для `POST` конечных точек, некоторые параметры могут быть отправлены в виде `строки запроса (query string)`, а некоторые в виде `тела запроса (request body)`:
* При отправке параметров в виде `тела запроса` допустимы следующие типы контента:
    * `application/x-www-form-urlencoded` для `query string`;
    * `multipart/form-data` для `HTML-форм`;
    * `application/json` для `JSON`.
* Параметры могут быть отправлены в любом порядке.

Парамерты функций
-
Для обработки `GET` запроса:
~~~sql
/**
 * Обрабатывает GET запрос.
 * @param {text} path - Путь
 * @param {jsonb} headers - HTTP заголовки
 * @param {jsonb} params - Параметры запроса
 * @return {SETOF json}
 */
CREATE OR REPLACE FUNCTION http.get (
  path      text,
  headers   jsonb,
  params    jsonb DEFAULT null
) RETURNS   SETOF json
~~~ 

Для обработки `POST` запроса:
~~~sql
/**
 * Обрабатывает POST запрос.
 * @param {text} path - Путь
 * @param {jsonb} headers - HTTP заголовки
 * @param {jsonb} params - Параметры запроса
 * @param {jsonb} body - Тело запроса
 * @return {SETOF json}
 */
CREATE OR REPLACE FUNCTION http.post (
  path      text,
  headers   jsonb,
  params    jsonb DEFAULT null,
  body      jsonb DEFAULT null
) RETURNS   SETOF json
~~~ 
