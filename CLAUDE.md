# cgmembers-frame - Common Good PHP member site

The live Common Good member site, admin panel, and transaction engine. Drupal-7-derived PHP, evolving since ~2010. Handles authentication, accounts, transactions, grants, notifications, and admin tooling. The SvelteKit companion (`cgpay`) uses this app as the source of truth for identity and business logic.

## What this repo is

- **`cgmembers/`** - the app (Drupal 7 install with the `rcredits` module carrying most Common Good logic)
- **`cgmembers/rcredits/`** - Common Good's own module, where 95% of engineering work happens
- **`config/deploy.rb`, `Capfile`, `Gemfile`** - Capistrano deploy config
- **`db/migrations/`** - Phinx migrations (schema changes)
- **`lib/`, `vendor/`** - Composer dependencies
- **`unused/`, `_unused-tools (graveyard)`** - retired code; do not restore without asking William

## Tech stack

- PHP 7.4+ on Apache/nginx
- MariaDB / MySQL 5.7+
- Drupal 7 core (heavily customized; `hook_menu`, `db_query`, `drupal_form_submit`, etc.)
- Gherkin-for-Drupal for Behat-style feature tests
- Phinx for migrations
- Capistrano for deploys
- Git submodule: `GherkinForDrupal` (SSH-aliased origin per environment)

## Directory map

```
cgmembers/
  index.php                    Drupal 7 entry point
  includes/                    Drupal core
  modules/                     contrib modules
  rcredits/                    ** Common Good's module - where you work **
    boot.inc                   bootstrap glue
    bootstrap.inc              CLI bootstrap (menu rebuild etc.)
    cg-*.inc                   top-level subsystems (see Namespaces below)
    defs.inc                   constants (permission bits, coflags, config values)
    admin/                     admin-only forms + tools
    api/                       JSON/HTTP API handlers used by clients
    classes/                   PHP classes (Acct, etc.)
    forms/                     Drupal form callbacks (formFoo, formFoo_validate, formFoo_submit)
      cgpay*.inc               server-to-server endpoints for the SvelteKit app
      cgpaysso.inc             /cgpay-sso - create session for verified uid
      cgpaylookup.inc          /cgpay-lookup - identifier -> uid
      cgpaywhoami.inc          /cgpay-whoami - session -> user identity
      cgpaygrants.inc          /cgpay-grants - grant CRUD from node
    rweb/                      user-facing web pages (dashboard, history, etc.)
      rweb-subs.inc            shared web utilities
      features/                Gherkin tests (.feature files)
    rsmart/                    "smart" pos client (older mobile-first flow)
    rcron/                     scheduled jobs
    rvote/                     voting features
    admin/                     admin console
db/
  migrations/                  Phinx migrations (dated filenames)
config/
  deploy.rb                    Capistrano main config
  deploy/*.rb                  per-stage overrides (test, dev, staging, demo, beta, main)
```

## Namespaces + shorthand

Every `cg-*.inc` file declares a namespace. In callers, they're imported with a short alias by convention:

```php
use CG as r;                // core "credits" logic
use CG\Db as db;           // database helpers
use CG\Util as u;          // utility functions
use CG\Backend as be;      // backend / tx engine
use CG\Web as w;           // web-form helpers
use CG\Risk as k;          // risk analysis
use CG\Txs as x;           // transactions
use CG\QR as qr;           // QR codes
```

Reading `r\loginString($id)` or `db\get('uid', 'sessions', ...)` or `u\fmtAmt($n)` is idiomatic. Match this style in new code.

## Key patterns

- **Drupal form callbacks:** `formFoo($form, &$sta, $args = '')` builds, `formFoo_validate` validates, `formFoo_submit` commits. Registered in `cg-menu.inc`.
- **Menu registry (`cg-menu.inc`):**
  ```php
  'path/goes/here' => ['norm|sub|dft|call', 'Title', 'FormNameOrArgs', 'perm bits', 'funcName']
  ```
  `'call'` = server-to-server (JSON), no form UI. New routes need a menu rebuild after deploy.
- **String translation:** `t('literal string with %sub', 'sub', $value)` for static strings, `tr('%foo', 'foo', $value)` for runtime string interpolation. Use `t()` when the whole string is known at code-write time; use `tr()` for dynamic substitution at runtime.
- **DB access:**
  - Reads: `db\get('col', 'table', 'where=:val', ['val' => ...])` for scalars, `db\row(...)` for rows, `db\col(...)` for columns.
  - Writes: `db\insert('table', $assocArray)`, `db\update('table', $assocArray, $where)`. NEVER inline `db_query` for writes if a helper exists.
- **Acct class:** `r\acct($uid)` returns an `Acct` instance with magic accessors (`$a->fullName`, `$a->bestName`, `$a->co`, `$a->sponsored`, `$a->can(B_ADMIN)`). Read `classes/acct.class` for available fields.
- **Permission bits:** defined in `defs.inc` as `B_OK`, `B_ADMIN`, `B_CO`, etc. `$a->can(B_ADMIN)` for boolean check.
- **Coflags:** company account subflags (`CO_SPONSORED`, `CO_OUROWN`, etc. in `defs.inc`).
- **`exitJust(X_CODE)` / `exitJson($data, X_OK)`** - terminate a JSON endpoint with a status code (see `defs.inc` for `X_*`).

## Server-to-server endpoints (cgpay integration)

All POST-only, all require `X-CG-Internal-Token: <CGPAY_SSO_SECRET>` header, all use `hash_equals` for the compare (fail-closed on empty). Registered as `'call'` type in `cg-menu.inc`.

| Path | Purpose | File |
|---|---|---|
| `/cgpay-sso` | create session for verified uid, set device cookie | `forms/cgpaysso.inc` |
| `/cgpay-lookup` | identifier (qid, email, phone, name) -> uid | `forms/cgpaylookup.inc` |
| `/cgpay-whoami` | session ssid -> `{uid, name, sponsored, menu}` | `forms/cgpaywhoami.inc` |
| `/cgpay-grants` | grant CRUD from SvelteKit | `forms/cgpaygrants.inc` |
| `/cgpay-people-autocomplete` | grantor typeahead | `forms/cgpaypeopleautocomplete.inc` |
| `/cgpay-funders-export` | funder CSV export | `forms/cgpayfundersexport.inc` |

Shared secret: `config.json` key `cgpaySsoSecret` -> constant `CGPAY_SSO_SECRET` (see `defs.inc`).

When adding a new endpoint: create `forms/cgpay<name>.inc`, register in `cg-menu.inc`, mirror the existing pattern (POST-only guard, secret verify, JSON body parse, JSON response). Menu rebuild required on deploy.

## Migrations

Phinx migrations under `db/migrations/`. Naming: `YYYYMMDDHHMMSS_snake_case_description.php`. Run via `./migrate.sh` (production) or `./jr-migrate.sh` (jr environment).

**Every database structure change is a Phinx migration** - no manual SQL. Each migration file documents its own change. A fresh install (`recreate.sh`) loads `db/startup.sql` and then runs all migrations, so anything not in a migration is missing on new installs. From time to time we archive all Phinx migrations (into `db/migrations/archived/`) and create a new `db/startup.sql` that includes everything up to that point. `cgmembers/rcredits/misc/pre-git-changes.log` is an old change log (mostly from before git), not a place to record schema changes.

## Branch / deploy flow

- Default base branch: `develop`
- Feature branches -> PR into `develop`
- `develop` -> `test` branch (William maintains) for integration testing
- Deploys via Capistrano: `cap test deploy`, `cap dev deploy`, `cap staging deploy`, `cap demo deploy`, `cap beta deploy`, `cap main deploy`
- Menu rebuild after adding routes: `./cgmembers/rcredits/bootstrap.inc` (CLI) or the admin panel

## Testing

- **Gherkin/Behat features:** `cgmembers/rcredits/**/features/*.feature`. Run with the Gherkin-for-Drupal harness (submodule).
- Test data: features declare `Setup` sections with member fixtures (uids like `.ZZA`, `.ZZB`, `.ZZC`, `.ZZF`).
- Test env admin bypass: on DEV, admin (uid=1) sign-in accepts any non-empty password (`signin.inc:84`).
- Test accounts on the test env: `Abe One` / `Bea Two` / etc. with password `k`. Full-name form works; short names may not.

## Conventions

- **No em-dashes** (`-`) anywhere in code, PRs, commit messages, or comments (per Chris's preference).
- **Commit messages: title only, no body.** Details go in PR descriptions.
- **No `Co-Authored-By: Claude` trailers** on commits.
- **Comments:** default to none. Only add for non-obvious *why*.
- **Namespaces:** always alias with the standard short form (`CG\Db as db`, etc.).
- **`t()` vs `tr()`:** static-known strings use `t()`; runtime interpolation uses `tr()`.
- **Whitespace and casing:** follow existing patterns in the file you're touching. William has strong preferences; when in doubt, match the surrounding style.

## Common gotchas

- **`cgweb_ro` DB user lacks SELECT on some tables** (`u_company`, `txs2` historically). The SvelteKit companion has defensive fallbacks; on the PHP side, use the app's own DB user which has full access.
- **Menu registry is cached.** After editing `cg-menu.inc`, run the menu rebuild or your new route 404s.
- **`.gitmodules` uses SSH aliases** (`github-cg:`, `github-pay:`) for `GherkinForDrupal`. If deploy fails on submodule fetch, check that the deploy user has the SSH stanza configured.
- **Session cookies (`SSESS...`) are set with `Domain=.commongood.earth`** so both PHP and cgpay see them. Never scope narrower without coordinated changes on the cgpay side.
- **`$mya`** is the global "current account" - populated by session bootstrap. In server-to-server endpoints (like `/cgpay-*`), it may be unset; use `r\acct($uid)` explicitly.
- **Dev-only admin password bypass** at `signin.inc:84` accepts any non-empty password when uid=1 on DEV. Not present in production.

## Cross-repo

Companion repo: `cgpay` (SvelteKit + adapter-node - modernized member-facing UI). See its `CLAUDE.md` for the node-side conventions. Local checkout typically at `/Users/admin/Work/CommonGood/cgpay`.

## People

- **Jose** - CEO, product owner
- **William Spademan** - founder, outgoing tech lead. Deepest knowledge of this codebase.
- **Chris Schwab** - incoming lead engineer
