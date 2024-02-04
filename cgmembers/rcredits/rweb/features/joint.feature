Feature: Joint
AS a member
I WANT to join my account to my partner's
SO we can share our finances, as for a typical "joint account" at a bank.

Setup:
  Given members:
  | uid  | fullName | acctType    | flags                     | minimum | created   |*
  | .ZZA | Abe One  | personal    | ok,member,confirmed,ided  |     100 | %today-6m |
  | .ZZB | Bea Two  | personal    | ok,confirmed,ided         |      50 | %today-6m |
  | .ZZC | Cor Pub  | corporation | ok,confirmed,ided,co      |       0 | %today-6m |
  | .ZZD | Dee Four | personal    | ok,confirmed,ided         |       0 | %today-6m |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member asks to create a joint account by clicking a link on the Dashboard page
  When member ".ZZA" visits "prejoin"
  Then we show "Create a Joint Account" with:
  | Already member? | No | Yes |
  | Account         |||
  | Go              |||

Scenario: A member asks to join an existing account
  When member ".ZZA" submits "prejoin" with:
  | old | account |*
  |   1 | newzzb  |
  Then these "u_relations":
  | main | agent | permission | employee | owner | draw |*
  | .ZZA | .ZZB  | joint      |        0 |     0 |    0 |
  And we say "status": "join request success"
  And we show "You: Abe One"
  And we message "join accounts" to member ".ZZB" with subs:
  | name    | _atag |*
  | Abe One | ?     |

Scenario: A member asks to join to self
  When member ".ZZA" submits "prejoin" with:
  | old | account |*
  |   1 | newzza  |
  Then we say "error": "no self join"
  And we show "Create a Joint Account"

Scenario: A member requests a joint account from the relations page
  Given these "u_relations":
  | main | agent | permission | employee | owner | draw |*
  | .ZZA | .ZZB  | none       |        0 |     0 |    0 |
  | .ZZB | .ZZA  | none       |        0 |     0 |    0 |
  When member ".ZZA" visits "settings/relations" and selects "permission": "joint" for ".ZZB"
  Then these "relations":
  | main | agent | permission | employee | owner | draw |*
  | .ZZA | .ZZB  | joint      |        0 |     0 |    0 |
  And members have:
  | uid  | jid | minimum |*
  | .ZZA |     |     100 |
  | .ZZB |     |      50 |
  When member ".ZZB" visits "settings/relations" and selects "permission": "joint" for ".ZZA"
  Then members have:
  | uid  | jid  | minimum |*
  | .ZZA | .ZZB |     150 |
  | .ZZB | .ZZA |       0 |
  
  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Cor Pub |     20 | %FOR_GOODS | stuff   |
  And member ".ZZB" confirms form "tx/pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Cor Pub |    300 | %FOR_GOODS | crud    |

Scenario: A joined account slave member requests a new minimum
  Given members have:
  | uid  | jid  | achMin | minimum |*
  | .ZZA | .ZZB |     50 |     150 |
  | .ZZB | .ZZA |     50 |       0 |
  And member ".ZZB" visits page "settings/fund"
  And step done "fund"
  When member ".ZZB" completes form "settings/fund" with values:
  | connect | routingNumber | bankAccount | bankAccount2 | refills | target | achMin | saveWeekly |*
  |       1 |     053000196 |         123 |          123 |       1 |    300 |    100 |          2 |
  Then members have:
  | uid  | bankAccount      | last4bank | achMin | minimum | saveWeekly |*
  | .ZZA | USkk053000196123 |      6123 |    100 |     300 |          2 |
  | .ZZB |                  |           |     50 |       0 |          0 |

Scenario: A joined account member looks at transaction history and summary
#  Given reward step is "1000"
  And members have:
  | uid  | jid  | minimum |*
  | .ZZA | .ZZB |     150 |
  | .ZZB | .ZZA |       0 |
  And these "u_relations":
  | main | agent | permission | employee | owner | draw |*
  | .ZZA | .ZZB  | joint      |        0 |       0 |    0 |
  | .ZZB | .ZZA  | joint      |        0 |       0 |    0 |
  And these "txs2":
  | txid | payee | amount | created   | completed |*
  |  600 | .ZZA  |   1000 | %today-6m | %today-6m |
  |  601 | .ZZB  |    600 | %today-2w | %today-2w |
  |  602 | .ZZA  |    400 | %today-2w | %today-2w |
  |  603 | .ZZA  |   -100 | %today    | %today    |
  # txid 603 used to have completed 0, but that's wrong -- we always immediately complete transfers out
  And these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |  15 | %today-1m |    200 | .ZZA | .ZZD | favors  |
  |  16 | %today-1w |    500 | .ZZA | .ZZB | usd     |
  |  17 | %today-2d |     50 | .ZZD | .ZZB | cash    |
  |  18 | %today-1d |    100 | .ZZC | .ZZA | labor   |  
  Then balances:
  | uid  | balance |*
  | .ZZA |    1850 |
  | .ZZB |    1850 | 
  | .ZZC |    -100 |
  | .ZZD |     150 |
  When member ".ZZB" visits page "history/transactions/period=14"
  Then we show "Transaction History" with:
  | Start        |   |   800.00 | %dmy-2w |
  | From Bank    | + | 1,000.00 |         |
  | To Bank      | - |   100.00 |         |
  | Received     | + |   150.00 |         |
  | Out          | - |     0.00 |         |
  | End          |   | 1,850.00 | %dmy    |
  And with:
  | Tx# | Date    | Name       | Purpose   |  Amount |  Balance | Action |
  |   4 | %mdy    | --         | to bank   | -100.00 | 1,850.00 |        |
  |  18 | %mdy-1d | Cor Pub | labor     |  100.00 | 1,950.00 |        |
  |  17 | %mdy-2d | Dee Four   | cash      |   50.00 | 1,850.00 |        |
#  | 16  | %mdy-1w | Abe One    | usd      | 500.00  |   500.00 |  +0    |
  |   3 | %mdy-2w | --         | from bank |  400.00 | 1,800.00 |        |
  |   2 | %mdy-2w | --         | from bank |  600.00 | 1,400.00 |        |
  Given cron runs "acctStats"
  When member ".ZZB" visits page "dashboard"
  Then we show "You: Bea Two" with:
  | ~...          | joined with Abe One |
  | Balance       | $1,850 |

  Scenario: A joined account member unjoins the account
  Given members have:
  | uid  | jid  | minimum |*
  | .ZZA | .ZZB |     150 |
  | .ZZB | .ZZA |       0 |
  And these "u_relations":
  | main | agent | permission | employee | owner | draw |*
  | .ZZA | .ZZB  | joint      |        0 |       0 |    0 |
  | .ZZB | .ZZA  | joint      |        0 |       0 |    0 |
  And these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |   4 | %today-1d |    100 | .ZZC | .ZZA | labor   |
  Then balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |     100 | 
  | .ZZC |    -100 |
  When member ".ZZB" deletes relation to ".ZZA"
  Then members have:
  | uid  | jid  | minimum | balance |*
  | .ZZA |      |     150 |      50 |
  | .ZZB |      |     150 |      50 |

Scenario: A member requests two joins at once
  Given members have:
  | uid  | jid  |*
  | .ZZA | 0    |
  | .ZZB | 0    |
  And these "u_relations":
  | main | agent | permission | employee | owner | draw |*
  | .ZZA | .ZZB  | joint      |        0 |     0 |    0 |
  | .ZZA | .ZZD  | none       |        0 |     0 |    0 |
  When member ".ZZA" visits "settings/relations" and selects "permission": "joint" for ".ZZD"
  Then these "u_relations":
  | main | agent | permission | employee | owner | draw |*
  | .ZZA | .ZZB  | joint      |        0 |     0 |    0 |
  | .ZZA | .ZZD  | none       |        0 |     0 |    0 |

Scenario: A member with a joined account views invoices
  Given these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | joint      |
  | .ZZB | .ZZA  | joint      |
  And members have:
  | uid  | jid  | balance |*
  | .ZZA | .ZZB | 200     |
  | .ZZB | .ZZA | 0       |
  And these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %now-1d | %TX_PENDING |    100 | .ZZA  | .ZZC  | drink |
  When member ".ZZB" visits "history/pending-from"
  Then we show "Pending Payments FROM You" with:
  | Inv# | Date    | Name    | Purpose | Amount | Status |
  | 1    | %mdY-1d | Cor Pub | drink   | 100.00 | OPEN   |

  When member ".ZZB" visits page "handle-invoice/nvid=1&code=TESTDOCODE"
  Then we show "Confirm Payment" with:
  | ~question     | Pay $100 to Cor Pub for drink |
  | Amount to Pay | 100     |
  | ~             | Pay     |
  | Reason        |         |
  | ~             | Dispute |

  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created |*
  | pay  |     |    1 |       100 | .ZZA  | .ZZC  | drink   | %now    |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | relType | rel |*
  |   1 | %now  |    100 | .ZZA  | .ZZC  | drink   | I       | 1   |
  And these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   |*
  |    1 | %now-1d | 1      |    100 | .ZZA  | .ZZC  | drink |
