# C++ API Server Template

A minimal, production-leaning template for building a REST API server in C++ on top of [Drogon](https://github.com/drogonframework/drogon). Use it as a starting point — replace the example endpoints with your own and add domain code under `app/`.

## What's included

- **Drogon HTTP server** with a versioned controller (`api::v1`) and three example endpoints under `/api/v1/`: `/health`, `/ready`, `POST /items`.
- **YAML configuration** loaded via `yaml-cpp`, with DB credentials sourced from environment variables.
- **PostgreSQL** support through Drogon's ORM, plus a non-blocking `db::ping` coroutine. `/ready` is a C++20 coroutine handler, so the DB check never blocks an IO thread.
- **Reflect-cpp** + a generic `convertAndValidate<T>()` helper, with a `drogon::fromRequest<T>` specialization that auto-binds JSON bodies to typed structs.
- **Global exception handler** that turns any `ControllerError` (or unhandled `std::exception`) into a structured `FailedResponse` JSON envelope with the right HTTP status — including errors thrown during JSON parsing, before the controller body even runs, and from coroutine handlers. Unexpected errors are logged in full but return only a generic message and a trace ID, so internals never leak to clients.
- **Structured request/response logging** via Drogon advices (`preRouting`, `postHandling`, `beginning`).
- **Outbound HTTP client** (`ApiClient`) for calling upstream services: retries timeouts, network failures and 5xx with exponential backoff + jitter. Fire-and-forget (`put`/`get`) or awaitable (`co_await putCoro(...)`/`getCoro(...)`) when you need the response. Only idempotent methods are exposed, so retries can't create duplicates.
- **Custom `ControllerError`** exception type that maps cleanly to JSON error responses.
- **Generic `ThreadPool`** built on `std::jthread`: tasks run in parallel, a throwing task doesn't kill its worker, and shutdown drains the queue.
- **Graceful shutdown** on SIGINT/SIGTERM via Drogon's built-in handlers (stops accepting, finishes in-flight requests).
- **Unit tests** (GoogleTest) for the thread pool, exception handler and request binding, run in CI as a plain build, under ASan + UBSan, and under TSan.
- **Multi-stage Dockerfile** + `docker-compose.yaml` with a Postgres service.
- **GitHub Actions**: tests (3 sanitizer configurations) and the Docker image on every push/PR; Doxygen API docs published to GitHub Pages.
- **VS Code** launch + tasks config for one-key build/debug.

## Layout

```
app/
  include/                          # public headers (mirrors src/)
  src/
    controllers/api/                # HTTP handlers (one method per file)
    controllers/models/request/     # request DTOs (reflect-cpp aggregates)
    controllers/models/response/    # response envelopes
    controllers/serialization/      # JSON ↔ struct conversion + fromRequest hooks
    core/                           # Config, advices, thread pool, exception handler
    custom_exceptions/              # ControllerError
    db/                             # DB ping helper
    utils/                          # ApiClient, time
    main.cpp
tests/                              # GoogleTest unit tests
CMakeLists.txt
Dockerfile
docker-compose.yaml
config.yaml                         # service + Drogon config
.env.example                        # copy to .env and fill in
```

## Run with Docker

```bash
cp .env.example .env       # then edit credentials
docker compose up --build
```

Try the endpoints:

```bash
curl http://localhost:5555/api/v1/health
curl http://localhost:5555/api/v1/ready
curl -X POST http://localhost:5555/api/v1/items \
     -H 'Content-Type: application/json' \
     -d '{"name":"Widget","quantity":3,"trace_id":"req-001"}'
```

| Endpoint            | Method | Purpose                                                 |
|---------------------|--------|---------------------------------------------------------|
| `/api/v1/health`    | GET    | Liveness probe — always 200.                            |
| `/api/v1/ready`     | GET    | Readiness probe — pings Postgres; 503 if it's down.     |
| `/api/v1/items`     | POST   | Example handler with auto-bound JSON body + validation. |

## Run the tests

In Docker (same as CI):

```bash
docker build --target test .                                  # plain
docker build --target test --build-arg SANITIZER=address .    # ASan + UBSan
docker build --target test --build-arg SANITIZER=thread .     # TSan
```

Locally, with the dependencies and GoogleTest installed:

```bash
cmake -S . -B build -DBUILD_TESTS=ON -DSANITIZER=address   # SANITIZER is optional
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## API docs

`doxygen Doxyfile` writes HTML to `docs/html/` (not committed). CI publishes it to GitHub Pages on every push to `main`.

## Dependencies

The Dockerfile builds these from source:

- Drogon `v1.9.11`
- yaml-cpp (pinned commit)
- reflect-cpp `v0.22.0`

## Adding an endpoint

The `POST /items` example shows the full pattern. To add your own:

1. Define the request DTO under `app/include/controllers/models/request/<YourRequest>.h` as a plain aggregate struct — reflect-cpp picks up fields automatically.
2. In `app/include/controllers/serialization/RequestConvert.h`, include your header and add a `drogon::fromRequest<YourRequest>` specialization (see the `CreateItemRequest` block). This is what lets Drogon auto-bind the JSON body to your struct as a controller method argument.
3. Declare the handler on `api::v1` in `app/include/controllers/api/V1.h` and register it with `METHOD_ADD`.
4. Implement it in a new `.cc` file under `app/src/controllers/api/`. Throw `ControllerError` for any failure — the global exception handler in `core/ExceptionHandler.cc` converts it to a `FailedResponse` with the right HTTP status. No try/catch in controllers needed.

## License

[MIT](LICENSE)
