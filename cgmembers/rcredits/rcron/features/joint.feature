Feature: Joint
AS a pair of members with a joint account
WE WANT to transfer money from our bank account to our joint account
SO BOTH OF US can make purchases with those funds.

Setup:
  Given members:
  | uid  | fullName | floor | minimum | flags            | achMin | bankAccount | jid  |*
  | .ZZA | Abe One  |     0 |     100 | ok,refill,bankOk | 30     | USkk9000001 | .ZZB |
  | .ZZB | Bea Two  |   -40 |       0 | ok               | 10     |             | .ZZA |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | joint      |
  | .ZZB | .ZZA  | joint      |
  Then balances:
  | uid  | balance |*
  | ctty |       0 |
  | .ZZA |       0 |
  | .ZZB |       0 |

Scenario: a joint account needs refilling
  Given these "txs":
  | xid | created | amount | payer | payee | purpose | taking |*
  |   1 | %today  |     50 | ctty | .ZZA | setup   | 0      |
  |   2 | %today  |  49.99 | ctty | .ZZB | setup   | 0      |
  Then balances:
  | uid  | balance |*
  | .ZZA |   99.99 |
  | .ZZB |   99.99 |
  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | xid |*
  |    1 | .ZZA  |  30    |   3 |
  And we notice "banked|bank tx number|available now" to member ".ZZA" with subs:
  | action | tofrom | amount | checkNum | why       |*
  | draw   | from   | $30    |        3 | to bring your balance up to the target you set |
Resume
Scenario: a joint account does not need refilling
  Given balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |     100 |
  When cron runs "getFunds"
  Then bank transfer count is 0
