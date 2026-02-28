#pragma once

#ifdef WITH_POSTGRESQL

#include "apostol/application.hpp"
#include "apostol/http.hpp"
#include "apostol/apostol_module.hpp"
#include "apostol/pg.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace apostol
{

// ─── PGHTTP ─────────────────────────────────────────────────────────────────
//
// Worker module that routes incoming HTTP requests to PostgreSQL functions:
//   GET    → http.get(path, headers, params)
//   POST   → http.post(path, headers, params, body)
//   PUT    → http.put(path, headers, params, body)
//   PATCH  → http.patch(path, headers, params, body)
//   DELETE → http.delete(path, headers, params, body)
//
// Uses deferred responses: sets resp.set_deferred(true) and sends the
// response asynchronously via connection_ctx when PG completes.
//
// Mirrors v1 CPGHTTP from src/modules/Workers/PGHTTP/.
//
class PGHTTP final : public ApostolModule
{
public:
    /// Self-configures from app.module_config("PGHTTP"):
    ///   "endpoints" → URL patterns (default ["/api/*"])
    explicit PGHTTP(Application& app);

    std::string_view name() const override { return "PGHTTP"; }
    bool enabled() const override { return enabled_; }
    bool check_location(const HttpRequest& req) const override;
    void heartbeat(std::chrono::system_clock::time_point) override {}

protected:
    void init_methods() override;

private:
    void do_get(const HttpRequest& req, HttpResponse& resp);
    void do_post(const HttpRequest& req, HttpResponse& resp);
    void do_put(const HttpRequest& req, HttpResponse& resp);
    void do_patch(const HttpRequest& req, HttpResponse& resp);
    void do_delete(const HttpRequest& req, HttpResponse& resp);

    /// Common dispatch: build SQL, execute async, set deferred.
    void pq_dispatch(const HttpRequest& req, HttpResponse& resp,
                     std::string_view pg_func, bool has_body);

    PgPool&                   pool_;
    std::vector<std::string>  endpoints_;
    bool                      enabled_;
};

} // namespace apostol

#endif // WITH_POSTGRESQL
