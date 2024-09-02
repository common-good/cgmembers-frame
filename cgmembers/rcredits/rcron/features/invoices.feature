Feature: Invoices
AS a member
I WANT to charge other members and pay invoices from other members (and unfunded payment requests from me) automatically at night, if necessary
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
  And these "u_relations":
  | main | agent | permission |*
  | .ZZC | .ZZB  | buy        |

  Scenario: Unpaid invoices get handled
  Given members have:
  | uid  | floor | balance |*
  | .ZZA |     0 |     100 |
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   | reversesXid |*
  |    1 | %today    | %TX_APPROVED |    100 | .ZZA  | .ZZC  | one   |          37 |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA  | .ZZC  | two   |        %NUL |
  |    3 | %today    | %TX_APPROVED |    300 | .ZZB  | .ZZC  | three |        %NUL |
  |    4 | %today-8d | %TX_PENDING  |    400 | .ZZA  | .ZZC  | four  |        %NUL |
  |    5 | %today-7d | %TX_PENDING  |    500 | .ZZA  | .ZZC  | five  |        %NUL |
  Then balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  
  When cron runs "payInvoices"
  Then count "txs" is 3
  And these "txs": 
  | xid | created | amount | payer | payee | purpose   | taking | type  | reversesXid |*
  |   1 | %today  |    100 | .ZZA  | .ZZC  | one       |        | prime |          37 |
  |   2 | %today  |      0 | bank  | .ZZA  | from bank |      1 | bank  |             |
  |   3 | %today  |      0 | bank  | .ZZA  | from bank |      1 | bank  |             |
  And count "txs2" is 2
  And these "txs2":
  | txid | xid | payee | amount | created | completed | deposit |*
  |    1 |   2 | .ZZA  |    100 | %now    |         0 |       0 |
  |    2 |   3 | .ZZA  |    200 | %now    |         0 |       0 |
  And count "tx_requests" is 5
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today    | 1            |    100 | .ZZA  | .ZZC  | one   |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA  | .ZZC  | two   |
  |    3 | %today    | %TX_APPROVED |    300 | .ZZB  | .ZZC  | three |
  |    4 | %today-8d | %TX_PENDING  |    400 | .ZZA  | .ZZC  | four  |
  |    5 | %today-7d | %TX_PENDING  |    500 | .ZZA  | .ZZC  | five  |

  And we message "you paid" to member ".ZZA" with subs:
  | otherName | amount | payerPurpose |*
  | Our Pub   | $100   | one          |
  And we message "expect a transfer" to member ".ZZA" with subs: ""
  And we message "banked|bank tx number" to member ".ZZA" with subs:
  | action | tofrom | amount | checkNum | why               |*
  | draw   | from   | $200   |        3 | to cover pending payment request #2 |
  And we message "request num|short to|when funded|how to fund" to member ".ZZB" with subs:
  | short | payeeName | nvid |*
  | $300  | Our Pub   |    3 |

  When cron runs "nudges"
  Then we message "stale invoice" to member ".ZZA" with subs:
  | daysAgo | amount | purpose | nvid | payeeName |*
  |       8 | $400   | four    |    4 | Our Pub   |
  And we message "stale invoice report" to member ".ZZC" with subs:
  | daysAgo | amount | purpose | nvid | payerName | created |*
  |       8 | $400   | four    |    4 | Abe One   | %mdY-8d |
  And we do not message "stale invoice" to member ".ZZA" with subs:
  | purpose |*
  | five    |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |     100 |
  And count "txs2" is 2

  When cron runs "getFunds"
  Then count "txs2" is 3
  And these "txs2":
  | txid | payee | amount | created | completed | deposit |*
  |    3 | .ZZA  |    400 | %today  |         0 |       0 |

  When cron runs "payInvoices"
  And cron runs "getFunds"
  Then count "txs2" is 3
  
  When cron runs "payInvoices"
  And cron runs "getFunds"
  Then count "txs2" is 3

  Scenario: Unpaid invoices get handled for a member who opted for "Balance First"
  Given members have:
  | uid  | floor | balance | flags |*
  | .ZZA |     0 |     100 | ok,confirmed,refill,debt,bankOk,balFirst |
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   | reversesXid |*
  |    1 | %today    | %TX_APPROVED |    100 | .ZZA  | .ZZC  | one   |          37 |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA  | .ZZC  | two   |        %NUL |
  |    3 | %today    | %TX_APPROVED |    300 | .ZZB  | .ZZC  | three |        %NUL |
  |    4 | %today-8d | %TX_PENDING  |    400 | .ZZA  | .ZZC  | four  |        %NUL |
  |    5 | %today-7d | %TX_PENDING  |    500 | .ZZA  | .ZZC  | five  |        %NUL |
  Then balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  
  When cron runs "payInvoices"
  Then count "txs" is 2
  And these "txs": 
  | xid | created | amount | payer | payee | purpose   | taking | type  | reversesXid |*
  |   1 | %today  |    100 | .ZZA  | .ZZC  | one       |        | prime |          37 |
  |   2 | %today  |      0 | bank  | .ZZA  | from bank |      1 | bank  |             |
  And count "txs2" is 1
  And these "txs2":
  | txid | xid | payee | amount | created | completed | deposit |*
  |    1 |   2 | .ZZA  |    200 | %now    |         0 |       0 |
  And count "tx_requests" is 5
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today    | 1            |    100 | .ZZA  | .ZZC  | one   |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA  | .ZZC  | two   |
  |    3 | %today    | %TX_APPROVED |    300 | .ZZB  | .ZZC  | three |
  |    4 | %today-8d | %TX_PENDING  |    400 | .ZZA  | .ZZC  | four  |
  |    5 | %today-7d | %TX_PENDING  |    500 | .ZZA  | .ZZC  | five  |

  And we message "you paid" to member ".ZZA" with subs:
  | otherName | amount | payerPurpose |*
  | Our Pub   | $100   | one          |
  And we message "request num|short to|expect a transfer" to member ".ZZA" with subs:
  | nvid | short | payeeName | amount | avail |*
  | 2    | $200  | Our Pub   | 200    | $0    |
  And we message "banked|bank tx number" to member ".ZZA" with subs:
  | action | tofrom | amount | checkNum | why               |*
  | draw   | from   | $200   |        2 | to cover pending payment request #2 |
  And we message "request num|short to|when funded|how to fund" to member ".ZZB" with subs:
  | short | payeeName | nvid |*
  | $300  | Our Pub   |    3 |
  
Scenario: Non-member unpaid invoice does not generate a transfer request
  Given members have:
  | uid  | flags                    |*
  | .ZZA | ok,confirmed,debt,bankOk |
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today    | %TX_APPROVED |    100 | .ZZE | .ZZC | one   |
  Then balances:
  | uid  | balance |*
  | .ZZC |       0 |
  | .ZZE |       0 |
  When cron runs "getFunds"
  Then count "txs" is 0
  And count "txs2" is 0
  And count "tx_requests" is 1
  
Scenario: Second invoice gets funded too for a non-refilling account
  Given members have:
  | uid  | flags               | floor | risks |*
  | .ZZA | ok,confirmed,bankOk | 0     | hasBank |
  And these "txs":
  | xid | created   | amount | payer | payee | purpose   | taking |*
  |   2 | %today-1d |      0 | bank  | .ZZA | from bank |      1 |
  And these "txs2":
  | txid | payee | amount | created   | completed | deposit | xid |*
  |    1 | .ZZA  |    100 | %today-1d |         0 |       0 |   2 |
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   | flags   |*
  |    1 | %today-1d | %TX_APPROVED |    100 | .ZZA  | .ZZC  | one   | funding |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA  | .ZZC  | two   |         |

  When cron runs "payInvoices"
  
  Then these "txs2":
  | txid | payee | amount | created   | completed | deposit |*
  |    1 | .ZZA  |    100 | %today-1d |         0 |       0 |
  |    2 | .ZZA  |    200 | %now      |         0 |       0 |
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   | flags   |*
  |    1 | %today-1d | %TX_APPROVED |    100 | .ZZA  | .ZZC  | one   | funding |
  |    2 | %today    | %TX_APPROVED |    200 | .ZZA  | .ZZC  | two   | funding |
  And we message "request num|short to|expect a transfer" to member ".ZZA" with subs:
  | short | avail |*
  | $200   |   $0 |

Scenario: A languishing invoice gets funded again
  Given these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today-1d | %TX_APPROVED |    900 | .ZZA  | .ZZC  | one   |
  When cron runs "payInvoices"
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit |*
  |    1 | .ZZA  |    900 | %today  |         0 |       0 |
  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit |*
  |    2 | .ZZA  |    500 | %today  |         0 |       0 |

Scenario: An invoice is approved from an account with a negative balance
  Given members have:
  | uid  | flags               | balance | wentNeg |*
  | .ZZA | ok,confirmed,bankOk |    -500 | %now-2w |
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today-1m | %TX_APPROVED |    400 | .ZZA  | .ZZC  | one   |
  When cron runs "payInvoices"
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit |*
  |    1 | .ZZA  |    400 | %today  |         0 |       0 |
  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit |*
  |    2 | .ZZA  |    500 | %today  |         0 |       0 |
  
Scenario: An invoice is approved from an account with a negative balance after credit line times out
  Given members have:
  | uid  | flags               | balance | wentNeg |*
  | .ZZA | ok,confirmed,bankOk |    -500 | %now-2m |
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today-1m | %TX_APPROVED |    400 | .ZZA  | .ZZC  | one   |
  When cron runs "payInvoices"
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit |*
  |    1 | .ZZA  |    400 | %today  |         0 |       0 |
  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit |*
  |    2 | .ZZA  |    500 | %today  |         0 |       0 |
  
Scenario: An invoice gets handled for an account that rounds up
  Given members have:
  | uid  | flags                       |*
  | .ZZA | ok,confirmed,bankOk,roundup |
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   |*
  |    1 | %today    | %TX_APPROVED |  99.60 | .ZZA | .ZZC | one   |
  |    2 | %today    | %TX_APPROVED |  19.40 | .ZZA | .ZZC | one   |
  When cron runs "payInvoices"
  Then these "txs": 
  | eid | xid | created | amount | payer | payee | purpose   | taking | type  |*
  |   1 |   1 | %today  |    100 | bank  | .ZZA  | from bank |      1 | bank  |
  |   3 |   2 | %today  |  99.60 | .ZZA  | .ZZC  | one       |      0 | prime |
  |   4 |   2 | %today  |   0.40 | .ZZA  | round | roundup donation | 0 | aux |
  |   5 |   3 | %today  |     20 | bank  | .ZZA  | from bank |      1 | bank  |
  |   6 |   4 | %today  |  19.40 | .ZZA  | .ZZC  | one       |      0 | prime |
  |   7 |   4 | %today  |   0.60 | .ZZA  | round | roundup donation | 0 | aux |
  And these "txs2":
  | txid | payee | amount | created | completed | deposit | xid |*
  |    1 | .ZZA  |    100 | %today  |    %today |       0 |   1 |
  |    2 | .ZZA  |     20 | %today  |    %today |       0 |   3 |
