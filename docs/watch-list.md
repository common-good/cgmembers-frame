# Watch list

Things to check when you do certain things. CLAUDE.md lists only the trigger events; the details live here.
Each entry: **When** (the trigger), **Check** (what to do), **Why** (what went wrong, or would).

To add an entry, give it a trigger short enough to recognize at a glance, and add that trigger to the "Watch list" section of CLAUDE.md.

## Transactions and failures

**When:** you add or change a `db_transaction()` (`$DBTX`), or a `try/catch` that could swallow a `u\FAIL()` (or `u\EXPECT()`, which calls it).

**Check:**
- A `$DBTX` commits when it is destroyed, and PHP destroys it on `die()`, `exit()` and when an exception unwinds out of its function. So rolling back on failure is not automatic. Three mechanisms make it work:
  - `handleError()` calls `rollbackAll()` before it dies (crashes and `trigger_error`).
  - `u\FAIL()` sets `DatabaseTransaction::$failing` when a transaction is open, and each `$DBTX` destructor then rolls back its own layer instead of committing. The flag clears when no transaction is left open.
  - `db\q()` calls `u\FAIL()` when a query fails inside a transaction (see below).
- Anything else that throws past a `$DBTX` held in a function still **commits**: a bare `throw new Exception`, or a native PHP `Error` (undefined function, `TypeError`). Use `u\FAIL()` for failures inside transactions.
- If you catch a `u\FAIL()` while a transaction held by an enclosing function is open and then carry on, set `\DatabaseTransaction::$failing = FALSE` in the catch (as `r\regionalInsert` does), or the enclosing transaction will roll back instead of committing.
- If you catch a failure in the same function that holds `$DBTX`, call `$DBTX->rollback()` there (as `rcron.inc` and `menu_rebuild` do).
- Anything you want to keep after a failure (a log line, an admin email) must happen after the rollback, or it is rolled back too.

**Catch sites traced when this was written** (recheck any of the "no transaction" ones if you put a transaction around it):

| Catch site | Transaction open when it catches? |
|---|---|
| `rcron/rcron.inc` (queue loop) | its own `$DBTX`, same scope; rolls back explicitly |
| `includes/menu.inc` (`menu_rebuild`), `includes/registry.inc` | same pattern |
| `cg.inc` (`regionalInsert` retry loop) | yes: called from `acct.class` (new account) inside `$DBTX`; clears `$failing` |
| `rweb/rweb.inc` (photo upload), `forms/api.inc` (test `rows` op), `forms/data.inc` (admin SQL) | no |
| `cg-util.inc` (`decryRay`, geocode, `copyFile`, `tellApp`), `acct.class` (SSN xml), `rweb.inc` (image functions), `stripe.inc` | no `u\FAIL()` in the try body |
| `rcron/app-socket.php` | outermost catch of a separate daemon |
| Drupal core catches (`cache`, `session`, `install`, `update`, `schema`, `theme`, `xmlrpc`, `common.inc` cron, `query.inc`) | database layer or cron; no CG transaction held across them |

**Why:** before this, a crash or a failed step inside a `$DBTX` saved whatever the transaction had done so far (a half-finished multi-step job, such as a bank-transfer request queued twice). Tests: `admin/features/transactions.feature`.

## Failed queries inside a transaction

**When:** you write or change code that calls `db\q()` (or anything built on it) inside a `$DBTX` and checks, or ignores, its return value.

**Check:** `DatabaseConnection::quer` catches the `PDOException` and returns `NULL` (it does not throw). `db\q()` logs the failure and returns `NULL`. Inside a transaction, `db\q()` now also calls `u\FAIL()`, so the transaction rolls back; do not rely on a `NULL` return to handle a failed query there. If a query is *expected* to fail (a duplicate-key probe), do it outside the transaction or catch the failure as in `r\regionalInsert`.

**Why:** callers that ignored the `NULL` carried on and the transaction committed the half-done work.
