Feature: Invoices
AS a member
I WANT to charge other members and pay invoices from other members automatically at night, if necessary
SO I can buy and sell stuff.

Setup:
  Given members:
  | uid  | fullName | bankAccount | floor | minimum | flags                           |*
  | .ZZA | Abe One  | USkk9000001 |  -250 |     500 | ok,confirmed,refill,debt,bankOk |
  | .ZZB | Bea Two  |             |  -250 |     100 | ok,confirmed,debt               |
  | .ZZC | Our Pub  |             |  -250 |       0 | ok,confirmed,co,debt            |
  | .ZZE | Eve Five | USkk9000005 |  -250 |     200 | bankOk                          |
  And members have:
  | uid  | staleNudge |*
  | .ZZC |          4 |
  And relations:
  | main | agent | permission |*
  | .ZZC | .ZZB  | buy        |

  Scenario: Unpaid invoices get handled
  Given transactions: 
  | xid | created | amount | payer | payee | purpose | taking |*
  |   1 | %today  |    100 | ctty | .ZZA | grant   |        |
  And invoices:
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today    | %TX_APPROVED |    100 | .ZZA | .ZZC | one   |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA | .ZZC | two   |
  |    3 | %today    | %TX_APPROVED |    300 | .ZZB | .ZZC | three |
  |    4 | %today-8d | %TX_PENDING  |    400 | .ZZA | .ZZC | four  |
  |    5 | %today-7d | %TX_PENDING  |    500 | .ZZA | .ZZC | five  |
  Then balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  
  When cron runs "invoices"
  Then transactions: 
  | xid | created | amount | payer   | payee | purpose   | taking | type  |*
  |   2 | %today  |    100 | .ZZA    | .ZZC  | one       |        | prime |
  |   3 | %today  |    200 | .ZZA    | .ZZC  | two       |        | prime |
  |   4 | %today  |      0 | bank-in | .ZZA  | from bank |      1 | bank  |
  Then count "txs" is 4
  And count "usd" is 1
  And count "invoices" is 5
  And usd transfers:
  | txid | payee | amount | created | completed | deposit |*
  |    1 | .ZZA  |    700 | %today  |         0 |       0 |
  And invoices:
  | nvid | created   | status       | amount | payer | payee | for   | flags   |*
  |    1 | %today    | 2            |    100 | .ZZA | .ZZC | one   |         |
  |    2 | %today    | 3            |    200 | .ZZA | .ZZC | two   | funding |
  |    3 | %today    | %TX_APPROVED |    300 | .ZZB | .ZZC | three |         |
  |    4 | %today-8d | %TX_PENDING  |    400 | .ZZA | .ZZC | four  |         |
  |    5 | %today-7d | %TX_PENDING  |    500 | .ZZA | .ZZC | five  |         |

  And we notice "banked|bank tx number" to member ".ZZA" with subs:
  | action | tofrom | amount | checkNum | why               |*
  | draw   | from   | $700   |        4 | to pay invoice #2 |
  And we notice "short invoice|when funded|how to fund" to member ".ZZB" with subs:
  | short | payeeName | nvid |*
  | $50   | Our Pub   |    3 |
  And we message "stale invoice" to member ".ZZA" with subs:
  | daysAgo | amount | purpose | nvid | payeeName |*
  |       8 | $400   | four    |    4 | Our Pub   |
  And we message "stale invoice report" to member ".ZZC" with subs:
  | daysAgo | amount | purpose | nvid | payerName | created |*
  |       8 | $400   | four    |    4 | Abe One   | %mdY-8d |
  And we do not message "stale invoice" to member ".ZZA" with subs:
  | purpose |*
  | five    |
  Then balances:
  | uid  | balance |*
  | .ZZA |    -200 |
  | .ZZB |       0 |
  | .ZZC |     300 |

  When cron runs "getFunds"
  Then usd transfer count is 1

  When cron runs "invoices"
  Then usd transfer count is 1

Scenario: Non-member unpaid invoice does not generate a transfer request
  Given invoices:
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today    | %TX_APPROVED |    100 | .ZZE | .ZZC | one   |
  Then balances:
  | uid  | balance |*
  | .ZZC |       0 |
  | .ZZE |       0 |
  When cron runs "invoices"
  Then count "txs" is 0
  And count "usd" is 0
  And count "invoices" is 1
  
Scenario: Second invoice gets funded too for a non-refilling account
  Given members have:
  | uid  | flags               |*
  | .ZZA | ok,confirmed,bankOk |
  And these "txs":
  | xid | created   | amount | payer   | payee | purpose   | taking |*
  |   2 | %today-1d |      0 | bank-in | .ZZA | from bank |      1 |
  And these "usd":
  | txid | payee | amount | created   | completed | deposit | xid |*
  |    1 | .ZZA  |    100 | %today-1d |         0 |       0 |   2 |
  And invoices:
  | nvid | created   | status       | amount | payer | payee | for   | flags   |*
  |    1 | %today-1d | %TX_APPROVED |    100 | .ZZA | .ZZC | one   | funding |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA | .ZZC | two   |         |
  When cron runs "invoices"
  Then these "usd":
  | txid | payee | amount | created   | completed | deposit |*
  |    1 | .ZZA  |    300 | %today-1d |         0 |       0 |
  # still dated yesterday, so it doesn't lose its place in the queue
  And invoices:
  | nvid | created   | status       | amount | payer | payee | for   | flags   |*
  |    1 | %today-1d | %TX_APPROVED |    100 | .ZZA | .ZZC | one   | funding |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA | .ZZC | two   | funding |
  And we notice "banked|combined|bank tx number" to member ".ZZA" with subs:
  | action | tofrom | amount | previous | total | checkNum | why               |*
  | draw   | from   | $200   |     $100 |  $300 |        2 | to pay invoice #2 |

Scenario: A languishing invoice gets funded again
  Given invoices:
  | nvid | created   | status       | amount | payer | payee | for   | flags   |*
  |    1 | %today-1d | %TX_APPROVED |    900 | .ZZA | .ZZC | one   | funding |
  When cron runs "invoices"
  Then these "usd":
  | txid | payee | amount | created | completed | deposit |*
  |    1 | .ZZA  |    900 | %today  |         0 |       0 |

Scenario: An invoice is approved from an account with a negative balance
  Given members have:
  | uid  | flags               | balance |*
  | .ZZA | ok,confirmed,bankOk |    -500 |
  And invoices:
  | nvid | created   | status       | amount | payer | payee | for   | flags   |*
  |    1 | %today-1m | %TX_APPROVED |    400 | .ZZA | .ZZC | one   | funding |
  When cron runs "invoices"
  Then these "usd":
  | txid | payee | amount | created | completed | deposit |*
  |    1 | .ZZA  |    900 | %today  |         0 |       0 |
  
Scenario: An invoice gets handled for an account that rounds up
  Given members have:
  | uid  | flags                       |*
  | .ZZA | ok,confirmed,bankOk,roundup |
  And invoices:
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today    | %TX_APPROVED |  99.60 | .ZZA | .ZZC | one   |
  When cron runs "invoices"
  Then transactions: 
  | xid | created | amount | payer   | payee | purpose              | taking | type  |*
  |   1 | %today  |    100 | bank-in | .ZZA | from bank            |      1 | bank  |
  And usd transfers:
  | txid | payee | amount | created | completed | deposit |*
  |    1 | .ZZA  |    100 | %today  |    %today |       0 |  
