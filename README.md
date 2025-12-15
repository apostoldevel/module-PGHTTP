[![ru](https://img.shields.io/badge/lang-ru-green.svg)](https://github.com/apostoldevel/module-PGHTTP/blob/master/README.ru-RU.md)

Postgres HTTP
-

**PGHTTP** - a module for [Apostol](https://github.com/apostoldevel/apostol).

Description
-
**PGHTTP** provides the ability to receive and process HTTP requests using the PL/pgSQL programming language.

Incoming requests
-
The module directs incoming HTTP `GET` and `POST` requests to the PostgreSQL database by calling the `http.get` and `http.post` functions, respectively, to process them.

Incoming requests are recorded in the `http.log` table.

Database installation
-
Follow the instructions for installing PostgreSQL in the description of [Apostol](https://github.com/apostoldevel/apostol#postgresql).

Module installation
-

Follow the instructions for building and installing [Apostol](https://github.com/apostoldevel/apostol#%D1%81%D0%B1%D0%BE%D1%80%D0%BA%D0%B0-%D0%B8-%D1%83%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0).

General information
-

* Base endpoint URL: [http://localhost:8080/api/v1](http://localhost:8080/api/v1);
    * The module only accepts requests whose path starts with `/api` (this can be changed in the source code).
* All endpoints return either a `JSON object` or a `JSON array` depending on the number of records in the response. This behavior can be changed by adding the `?data_array=true` parameter to the request, in which case the response will be a `JSON array` regardless of the number of records.

* Endpoint URL format:
~~~
http[s]://<hosthame>[:<port>]/api/<route>
~~~

## HTTP Status Codes
* HTTP `4XX` status codes are used for client-side errors - the problem is on the client side.
* HTTP `5XX` status codes are used for internal errors - the problem is on the server side. It is important **NOT** to consider this as a failure operation. The execution status is **UNKNOWN** and may be successful.

## Passing Parameters
* For `GET` endpoints, parameters should be sent as a `query string`.
* For `POST` endpoints, some parameters can be sent as a `query string`, and some as a request body:
* The following content types are allowed when sending parameters as a request `body`:
    * `application/x-www-form-urlencoded` for `query string`;
    * `multipart/form-data` for `HTML forms`;
    * `application/json` for `JSON`.
* Parameters can be sent in any order.

Function Parameters
-

To handle a `GET` request:
~~~sql
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
~~~

To handle a `POST` request:
~~~sql
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
~~~
