Feature: Bank -- transfer funds to or from a bank account
AS a member
I WANT to transfer credit to my bank account
SO I can pay it to non-members
OR
I WANT to transfer credit from my bank account
SO I can (eventually) spend it through the Common Good system.

Setup:
  Given members:
  | uid  | fullName | minimum | floor | flags             | bankAccount |*
  | .ZZA | Abe One  |       0 |   -20 | ok,debt,bankOk    | USkk9000001 |
  | .ZZB | Bea Two  |       0 |     0 | ok                | USkk9000002 |
  | .ZZC | Our Pub  |      40 |   -10 | co,ok,debt,bankOk | USkk9000003 |
  | .ZZD | Dee Four |      80 |   -20 | ok,refill,debt    | USkk9000004 |
  And these "u_relations":
  | main | other | permission |*
  | .ZZC | .ZZB  |     manage |
  And these "txs2":
  | txid | payee | amount | created   | completed | deposit   |*
  | 5001 |  .ZZA |     99 | %today-7d | %today-5d | %today-1d |
  | 5002 |  .ZZA |    100 | %today-5d |         0 | %today-1d |
  | 5003 |  .ZZA |    -13 | %today-2d | %today-2d | %today-1d |
  | 5004 |  .ZZB |     -4 | %today-2d | %today-2d | %today-1d |
  | 5005 |  .ZZC |     30 | %today-2d | %today-2d | %today-1d |
  | 5006 |  .ZZD |    140 | %today-2d | %today-2d | %today-1d |
  # usd transfer creation also creates corresponding transactions, if the transfer is complete
  And these "txs":
  | xid | created    | amount | payer | payee | purpose |*
  | 7   | %today-10d |    100 | ctty | .ZZB | grant   |
  Then count "txs" is 7
  And balances:
  | uid  | balance |*
  | .ZZA |      86 |
  | .ZZB |      96 |
  | .ZZC |      30 |
  | .ZZD |     140 |

Scenario: a member moves credit to the bank
  When member ".ZZA" completes form "get" with values:
  | op  | amount |*
  | put |     86 |
  Then these "txs":
  | xid | payer     | payee | amount |*
  | 8   | %UID_BANK | .ZZA  |    -86 |
  Then these "txs2":
  | payee | amount | created   | completed | channel | xid |*
  |  .ZZA |    -86 | %today    | %today    | %TX_WEB |   8 |
  And we say "status": "banked" with subs:
  | action  | tofrom  | amount | why             |*
  | deposit | to      | $86    | as soon as possible |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  And we notice "banked" to member ".ZZA" with subs:
  | action  | tofrom | amount | why             |*
  | deposit | to     | $86    | as soon as possible |
  
Scenario: a member draws credit from the bank with zero floor
  When member ".ZZB" completes form "get" with values:
  | op  | amount    |*
  | get | %R_ACHMIN |
  Then these "txs2":
  | txid | payee | amount    | created | completed | channel | xid | deposit |*
  | 5007 |  .ZZB | %R_ACHMIN | %now    |         0 | %TX_WEB |   8 |       0 |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking |*
  |   8 | %todayd |      0 | 256  | .ZZB | from bank |      1 |
  And balances:
  | uid  | balance |*
  | .ZZA |      86 |
  And we say "status": "banked|bank tx number" with subs:
  | action | tofrom  | amount     | checkNum | why             |*
  | draw   | from    | $%R_ACHMIN |        8 | as soon as possible |

Scenario: a member draws credit from the bank with adequate floor
  When member "C:B" completes form "get" with values:
  | op  | amount |*
  | get |     10 |
  Then these "txs2":
  | txid | payee | amount | created | completed | channel | xid |*
  | 5007 |  .ZZC |     10 | %today  |    %today | %TX_WEB |   8 |
  And balances:
  | uid  | balance |*
  | .ZZC | 40      |
  And we say "status": "banked|bank tx number|available now" with subs:
  | action | tofrom  | amount | checkNum | why             |*
  | draw   | from    |    $10 |        8 | as soon as possible |
  And we notice "banked|bank tx number|available now" to member ".ZZC" with subs:
  | action | tofrom | amount | checkNum | why             |*
  | draw   | from   | $10    |        8 | as soon as possible |
Scenario: a member moves too little to the bank
  When member ".ZZA" completes form "get" with values:
  | op  | amount           |*
  | put | %(%R_ACHMIN-.01) |
  Then we say "error": "bank too little"

#Scenario: a member tries to cash out rewards and/or pending withdrawals
#  When member ".ZZA" completes form "get" with values:
#  | op  | amount |*
#  | put |     87 |
#  Then we say "error": "short put|short cash help" with subs:
#  | max |*
#  | $86 |

Scenario: a member moves too much to the bank
  When member ".ZZB" completes form "get" with values:
  | op  | amount |*
  | put |    200 |
  Then we say "error": "short put" with subs:
  | max |*
  | $96 |
  # one chunk each from ctty, A, and D. Only $2 from C.

Scenario: a member tries to go below their minimum
  When member ".ZZD" completes form "get" with values:
  | op  | amount |*
  | put |     61 |
  Then we say "error": "change min first"

Scenario: a member asks to do two transfers out in one day
  Given these "txs2":
  | payee | amount | created   |*
  |  .ZZD |     -6 | %today    |
  When member ".ZZD" completes form "get" with values:
  | op  | amount |*
  | put |     10 |
  Then we show "Transfer Funds" with:
  | Pending  | |
  | You have | $16 from %PROJECT to your bank account. |

Scenario: a member draws credit from the bank then cancels
  When member "C:B" completes form "get" with values:
  | op  | amount |*
  | get |     10 |
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit | channel | xid |*
  | 5007 |  .ZZC |     10 | %today  |    %today |       0 | %TX_WEB |   8 |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking |*
  | 8   | %today  |     10 |  256 | .ZZC | from bank |      1 |
  And balances:
  | uid  | balance |*
  | .ZZC |      40 |
  And count "txs" is 8
  When member "C:B" visits page "get/cancel=in"
  Then balances:
  | uid  | balance |*
  | .ZZC |      30 |
  And count "txs2" is 6
  And count "txs" is 7
  And we notice "bank tx canceled" to member ".ZZC" with subs:
  | xid | 8 |**
  And we redirect to "/get"

Scenario: a member with a negative balance requests a transfer from the bank
  Given balances:
  | uid  | balance |*
  | .ZZA | -26     |
  When member ".ZZA" completes form "get" with values:
  | op  | amount |*
  | get |     30 |
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit | channel | xid |*
  | 5007 |  .ZZA |     30 | %today  |         0 |       0 | %TX_WEB |   8 |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking |*
  | 8   | %today  |      0 |  256 | .ZZA | from bank |      1 |
  
Scenario: a slave member requests a transfer
  Given members:
  | uid  | fullName | floor | flags  | risks   | jid  | balance |*
  | .ZZE | Eve Five |     0 | ok     | hasBank | .ZZD |     140 |
  And members have:
  | uid  | jid  | flags |*
  | .ZZD | .ZZE | ok    |
  And these "u_relations":
  | main | other | permission |*
  | .ZZD | .ZZE  |      joint |
  | .ZZE | .ZZD  |      joint |
  When member ".ZZE" completes form "get" with values:
  | op  | amount |*
  | put |     16 |
  Then these "txs2":
  | payee | amount | created   | completed | channel | xid |*
  |  .ZZE |    -16 | %today    | %today    | %TX_WEB |   8 |
  And we say "status": "banked" with subs:
  | action  | tofrom  | amount | why             |*
  | deposit | to      | $16    | as soon as possible |
  And balances:
  | uid  | balance |*
  | .ZZD |     124 |
  | .ZZE |     124 |
