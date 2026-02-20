[![en](https://img.shields.io/badge/lang-en-green.svg)](https://github.com/apostoldevel/module-PGHTTP/blob/master/README.md)

Postgres HTTP
-
**PGHTTP** - модуль для [Апостол](https://github.com/apostoldevel/apostol).

Описание
-
**PGHTTP** предоставляет возможность принимать и обрабатывать HTTP-запросы на языке программирования PL/pgSQL.

Входящие запросы
-

Модуль направляет входящие HTTP методы `GET`, `POST`, `PATCH`, `PUT`, `DELETE` в базу данных PostgreSQL вызывая для их обработки соответствующие функции `http.get`, `http.post`, `http.patch`, `http.put`, `http.delete`.

Входящие запросы записываются в таблицу `http.log`.

Модуль базы данных
-

PGHTTP тесно связан с модулем **`http`** платформы [db-platform](https://github.com/apostoldevel/db-platform) (`db/sql/platform/http/`).

Весь входящий HTTP-трафик диспетчеризуется в PL/pgSQL-обработчики и журналируется в этом модуле:

| Объект | Назначение |
|--------|------------|
| `http.log` | Журнал аудита каждого входящего запроса (путь, заголовки, параметры, тело, время выполнения) |
| `http.write_to_log(...)` | Вспомогательная функция, вызываемая каждым обработчиком для записи запроса перед обработкой |
| `http.get(path, headers, params)` | PL/pgSQL-обработчик `GET`-запросов |
| `http.post(path, headers, params, body)` | PL/pgSQL-обработчик `POST`-запросов |
| `http.patch(path, headers, params, body)` | PL/pgSQL-обработчик `PATCH`-запросов |
| `http.put(path, headers, params, body)` | PL/pgSQL-обработчик `PUT`-запросов |
| `http.delete(path, headers, params, body)` | PL/pgSQL-обработчик `DELETE`-запросов |

> **Примечание:** PGHTTP обрабатывает **входящие** HTTP-запросы, диспетчеризуя их в PL/pgSQL. Для **исходящих** HTTP-запросов, инициируемых из PL/pgSQL, используйте [PGFetch](https://github.com/apostoldevel/module-PGFetch) — оба модуля разделяют один и тот же модуль `http` платформы db-platform.

Настройка
-

```ini
[module/PGHTTP]
enable=true
```

Установка базы данных
-
Следуйте указаниям по установке PostgreSQL в описании [Апостол](https://github.com/apostoldevel/apostol#postgresql)

Установка модуля
-
Следуйте указаниям по сборке и установке [Апостол](https://github.com/apostoldevel/apostol#%D1%81%D0%B1%D0%BE%D1%80%D0%BA%D0%B0-%D0%B8-%D1%83%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0)

## Общая информация
* Конечная точка по умолчанию (endpoint url): [http://localhost:8080/api/v1](http://localhost:8080/api/v1);
* Все конечные точки возвращают: `JSON-объект` или `JSON-массив` в зависимости от количества записей в ответе. Изменить это поведение можно добавив в запрос параметр `?data_array=true` тогда ответ будет `JSON-массив` в независимости от количества записей;

### Формат endpoint url:
```url
http[s]://<hosthame>[:<port>]/<route1>/<route2>/.../<routeN>
```

## HTTP коды возврата
* HTTP `4XX` коды возврата применимы для некорректных запросов - проблема на стороне клиента.
* HTTP `5XX` коды возврата используются для внутренних ошибок - проблема на стороне сервера. Важно **НЕ** рассматривать это как операцию сбоя. Статус выполнения **НЕИЗВЕСТЕН** и может быть успешным.

## Передача параметров
* Для `GET` методов параметры должны быть отправлены в виде `строки запроса (query string)` .
* Для остальных методов, некоторые параметры могут быть отправлены в виде `строки запроса (query string)`, а некоторые в виде `тела запроса (request body)`:
* При отправке параметров в виде `тела запроса` допустимы следующие типы контента:
    * `application/x-www-form-urlencoded` для `query string`;
    * `multipart/form-data` для `HTML-форм`;
    * `application/json` для `JSON`.
* Параметры могут быть отправлены в любом порядке.

Примеры функций
-

Для обработки `GET` запроса:
```sql
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
AS $$
DECLARE
  r         record;

  nId       bigint;

  cBegin    timestamptz;

  vMessage  text;
  vContext  text;
BEGIN
  nId := http.write_to_log(path, headers, params);

  IF split_part(path, '/', 3) != 'v1' THEN
    RAISE EXCEPTION 'Invalid API version.';
  END IF;

  cBegin := clock_timestamp();

  FOR r IN SELECT * FROM jsonb_each(headers)
  LOOP
    -- parse headers here
  END LOOP;

  CASE split_part(path, '/', 4)
  WHEN 'ping' THEN

    RETURN NEXT json_build_object('code', 200, 'message', 'OK');

  WHEN 'time' THEN

    RETURN NEXT json_build_object('serverTime', trunc(extract(EPOCH FROM Now())));

  WHEN 'headers' THEN

    RETURN NEXT coalesce(headers, jsonb_build_object());

  WHEN 'params' THEN

    RETURN NEXT coalesce(params, jsonb_build_object());

  WHEN 'log' THEN

    FOR r IN SELECT * FROM http.log ORDER BY id DESC
    LOOP
      RETURN NEXT row_to_json(r);
    END LOOP;

  ELSE

    RETURN NEXT json_build_object('error', json_build_object('code', 404, 'message', format('Path "%s" not found.', path)));

  END CASE;

  UPDATE http.log SET runtime = age(clock_timestamp(), cBegin) WHERE id = nId;

  RETURN;
EXCEPTION
WHEN others THEN
  GET STACKED DIAGNOSTICS vMessage = MESSAGE_TEXT, vContext = PG_EXCEPTION_CONTEXT;

  PERFORM http.write_to_log(path, headers, params, null, 'GET', vMessage, vContext);

  RETURN NEXT json_build_object('error', json_build_object('code', 400, 'message', vMessage));

  RETURN;
END;
$$ LANGUAGE plpgsql
  SECURITY DEFINER
  SET search_path = http, pg_temp;
```

Для обработки `POST` запроса:
```sql
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
AS $$
DECLARE
  r         record;

  nId       bigint;

  cBegin    timestamptz;

  vMessage  text;
  vContext  text;
BEGIN
  nId := http.write_to_log(path, headers, params, body, 'POST');

  IF split_part(path, '/', 3) != 'v1' THEN
    RAISE EXCEPTION 'Invalid API version.';
  END IF;

  FOR r IN SELECT * FROM jsonb_each(headers)
  LOOP
    -- parse headers here
  END LOOP;

  cBegin := clock_timestamp();

  CASE split_part(path, '/', 4)
  WHEN 'ping' THEN

    RETURN NEXT json_build_object('code', 200, 'message', 'OK');

  WHEN 'time' THEN

    RETURN NEXT json_build_object('serverTime', trunc(extract(EPOCH FROM Now())));

  WHEN 'headers' THEN

    RETURN NEXT coalesce(headers, jsonb_build_object());

  WHEN 'params' THEN

    RETURN NEXT coalesce(params, jsonb_build_object());

  WHEN 'body' THEN

    RETURN NEXT coalesce(body, jsonb_build_object());

  ELSE

    RAISE EXCEPTION 'Path "%" not found.', path;

  END CASE;

  UPDATE http.log SET runtime = age(clock_timestamp(), cBegin) WHERE id = nId;

  RETURN;
EXCEPTION
WHEN others THEN
  GET STACKED DIAGNOSTICS vMessage = MESSAGE_TEXT, vContext = PG_EXCEPTION_CONTEXT;

  PERFORM http.write_to_log(path, headers, params, body, 'POST', vMessage, vContext);

  RETURN NEXT json_build_object('error', json_build_object('code', 400, 'message', vMessage));

  RETURN;
END;
$$ LANGUAGE plpgsql
  SECURITY DEFINER
  SET search_path = http, pg_temp;
```
