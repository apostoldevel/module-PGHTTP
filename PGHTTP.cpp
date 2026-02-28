#ifdef WITH_POSTGRESQL

#include "PGHTTP.hpp"

#include "apostol/http.hpp"
#include "apostol/http_utils.hpp"
#include "apostol/pg.hpp"
#include "apostol/pg_exec.hpp"
#include "apostol/pg_utils.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

namespace apostol
{

// ─── Construction ───────────────────────────────────────────────────────────

PGHTTP::PGHTTP(Application& app)
    : pool_(app.db_pool())
    , enabled_(true)
{
    if (auto* cfg = app.module_config("PGHTTP")) {
        if (cfg->contains("endpoints") && (*cfg)["endpoints"].is_array())
            for (auto& e : (*cfg)["endpoints"])
                if (e.is_string())
                    endpoints_.push_back(e.get<std::string>());
    }
    if (endpoints_.empty())
        endpoints_.push_back("/api/*");
}

// ─── check_location ─────────────────────────────────────────────────────────

bool PGHTTP::check_location(const HttpRequest& req) const
{
    return match_path(req.path, endpoints_);
}

// ─── init_methods ───────────────────────────────────────────────────────────

void PGHTTP::init_methods()
{
    add_method("GET",    [this](auto& req, auto& resp) { do_get(req, resp); });
    add_method("POST",   [this](auto& req, auto& resp) { do_post(req, resp); });
    add_method("PUT",    [this](auto& req, auto& resp) { do_put(req, resp); });
    add_method("PATCH",  [this](auto& req, auto& resp) { do_patch(req, resp); });
    add_method("DELETE", [this](auto& req, auto& resp) { do_delete(req, resp); });
}

// ─── Method handlers ────────────────────────────────────────────────────────

void PGHTTP::do_get(const HttpRequest& req, HttpResponse& resp)
{
    pq_dispatch(req, resp, "http.get", false);
}

void PGHTTP::do_post(const HttpRequest& req, HttpResponse& resp)
{
    pq_dispatch(req, resp, "http.post", true);
}

void PGHTTP::do_put(const HttpRequest& req, HttpResponse& resp)
{
    pq_dispatch(req, resp, "http.put", true);
}

void PGHTTP::do_patch(const HttpRequest& req, HttpResponse& resp)
{
    pq_dispatch(req, resp, "http.patch", true);
}

void PGHTTP::do_delete(const HttpRequest& req, HttpResponse& resp)
{
    pq_dispatch(req, resp, "http.delete", true);
}

// ─── pq_dispatch ────────────────────────────────────────────────────────────

void PGHTTP::pq_dispatch(const HttpRequest& req, HttpResponse& resp,
                          std::string_view pg_func, bool has_body)
{
    auto path_q    = pq_quote_literal(req.path);
    auto headers_j = headers_to_json(req.headers);
    auto params_j  = params_to_json(req.params);

    std::string sql;

    if (has_body) {
        // Determine body content: JSON passthrough or form-to-json conversion
        std::string body_json;
        auto ct = req.content_type();
        std::transform(ct.begin(), ct.end(), ct.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (req.body.empty()) {
            body_json = "null";
        } else if (ct.find("application/json") != std::string::npos) {
            body_json = pq_quote_literal(req.body);
        } else {
            body_json = pq_quote_literal(form_to_json(req.body));
        }

        sql = fmt::format("SELECT * FROM {}({}, {}::jsonb, {}::jsonb, {}::jsonb)",
                          pg_func, path_q, pq_quote_literal(headers_j),
                          pq_quote_literal(params_j), body_json);
    } else {
        sql = fmt::format("SELECT * FROM {}({}, {}::jsonb, {}::jsonb)",
                          pg_func, path_q, pq_quote_literal(headers_j),
                          pq_quote_literal(params_j));
    }

    exec_sql(pool_, req, resp, std::move(sql),
        [](std::shared_ptr<HttpConnection> conn, std::vector<PgResult> results) {
            HttpResponse r;
            r.set_header("Content-Type", "application/json");

            if (results.empty() || !results[0].ok()) {
                std::string err_msg = results.empty()
                    ? "no result"
                    : (results[0].error_message() ? results[0].error_message() : "unknown error");
                reply_error(r, HttpStatus::internal_server_error, err_msg);
                conn->send_response(r);
                return;
            }

            const auto& res = results[0];

            if (res.rows() == 0 || res.columns() == 0) {
                r.set_status(HttpStatus::no_content);
                conn->send_response(r);
                return;
            }

            // The PG function returns the result in column 0
            const char* val = res.value(0, 0);
            std::string body = val ? val : "null";

            // Check for application-level error in the response JSON
            std::string error_message;
            int error_code = check_pg_error(body, error_message);

            if (error_code != 0) {
                r.set_status(error_code_to_status(error_code))
                 .set_body(body, "application/json");
            } else {
                r.set_status(HttpStatus::ok)
                 .set_body(body, "application/json");
            }

            conn->send_response(r);
        });
}

} // namespace apostol

#endif // WITH_POSTGRESQL
