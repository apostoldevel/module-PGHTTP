[![ru](https://img.shields.io/badge/lang-ru-green.svg)](https://github.com/apostoldevel/module-PGHTTP/blob/master/README.ru-RU.md)

Postgres HTTP
-

**PGHTTP** - a module for [Apostol](https://github.com/apostoldevel/apostol).

Description
-
**PGHTTP** provides the ability to receive and process HTTP requests using the PL/pgSQL programming language.

Incoming requests
-
The module directs incoming HTTP `GET`, `POST`, `PATCH`, `PUT`, `DELETE` requests to the PostgreSQL database by calling the `http.get`, `http.post`, `http.patch`, `http.put`, `http.delete` functions, respectively, to process them.

Incoming requests are recorded in the `http.log` table.

Database module
-

PGHTTP is tightly coupled to the **`http`** module of [db-platform](https://github.com/apostoldevel/db-platform) (`db/sql/platform/http/`).

All incoming HTTP traffic is dispatched to PL/pgSQL handlers and logged in this module:

| Object | Purpose |
|--------|---------|
| `http.log` | Audit log of every incoming request (path, headers, params, body, runtime) |
| `http.write_to_log(...)` | Helper function called by each handler to record the request before processing |
| `http.get(path, headers, params)` | PL/pgSQL handler for `GET` requests |
| `http.post(path, headers, params, body)` | PL/pgSQL handler for `POST` requests |
| `http.patch(path, headers, params, body)` | PL/pgSQL handler for `PATCH` requests |
| `http.put(path, headers, params, body)` | PL/pgSQL handler for `PUT` requests |
| `http.delete(path, headers, params, body)` | PL/pgSQL handler for `DELETE` requests |

> **Note:** PGHTTP handles **incoming** HTTP requests dispatched into PL/pgSQL. For **outgoing** HTTP requests initiated from PL/pgSQL, see [PGFetch](https://github.com/apostoldevel/module-PGFetch) — both modules share the same `http` db-platform module.

Configuration
-

```ini
[module/PGHTTP]
enable=true
```

Database installation
-
Follow the instructions for installing PostgreSQL in the description of [Apostol](https://github.com/apostoldevel/apostol#postgresql).

Module installation
-

Follow the instructions for building and installing [Apostol](https://github.com/apostoldevel/apostol#build-and-installation).

General information
-

* Default endpoint URL: [http://localhost:8080/api/v1](http://localhost:8080/api/v1);
* All endpoints return either a `JSON object` or a `JSON array` depending on the number of records in the response. This behavior can be changed by adding the `?data_array=true` parameter to the request, in which case the response will be a `JSON array` regardless of the number of records.

* Endpoint URL format:
```url
http[s]://<hosthame>[:<port>]/<route1>/<route2>/.../<routeN>
```

## HTTP Status Codes
* HTTP `4XX` status codes are used for client-side errors - the problem is on the client side.
* HTTP `5XX` status codes are used for internal errors - the problem is on the server side. It is important **NOT** to consider this as a failure operation. The execution status is **UNKNOWN** and may be successful.

## Passing Parameters
* For `GET` endpoints, parameters should be sent as a `query string`.
* For others endpoints, some parameters can be sent as a `query string`, and some as a request body:
* The following content types are allowed when sending parameters as a request `body`:
    * `application/x-www-form-urlencoded` for `query string`;
    * `multipart/form-data` for `HTML forms`;
    * `application/json` for `JSON`.
* Parameters can be sent in any order.

Function Examples
-

To handle a `GET` request:
```sql
/**
* @param {text} path - Path
* @param {jsonb} headers - HTTP headers
* @param {jsonb} params - Query parameters
* @return {SETOF json}
**/
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

To handle a `POST` request:
```sql
/**
* @param {text} path - Path
* @param {jsonb} headers - HTTP headers
* @param {jsonb} params - Query parameters
* @param {jsonb} body - Request body
* @return {SETOF json}
**/
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
