Feature: Bank
AS a member
I WANT credit to flow from my bank account
SO I can spend it with my rCard.

Setup:
  Given members:
  | uid  | fullName | floor | minimum | flags                         | achMin | bankAccount |*
  | .ZZA | Abe One  |     0 |     100 | co,ok,refill,bankOk,confirmed | 30     | USkk9000001 |
  | .ZZB | Bea Two  |   -50 |     100 | ok,refill,confirmed           | 30     |                    |
  | .ZZC | Our Pub  |   -50 |     100 | ok,co                         | 50     | USkk9000003 |

Scenario: a member is barely below target
  And these "txs":
  | xid | created    | amount | payer | payee | purpose |*
  | 7   | %today-10d |  99.99 | ctty  | .ZZA  | grant   |
  Then members have:
  | uid  | balance |*
  | .ZZA | 99.99   |

  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | channel  | xid |*
  |    1 | .ZZA  |     30 | %TX_CRON |   8 |
  And bank transfer count is 1
  And we notice "banked|bank tx number" to member ".ZZA" with subs:
  | action | tofrom | amount | checkNum | why       |*
  | draw   | from   | $30    |        8 | to bring your balance up to the target you set |

Scenario: a member gets credit for the bank transfer immediately
  Given balances:
  | uid  | balance | floor |*
  | .ZZA |   99.99 |   -50 |
  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | channel  |*
  |    1 | .ZZA  |     30 | %TX_CRON |
  And bank transfer count is 1
  And we notice "banked|bank tx number|available now" to member ".ZZA" with subs:
  | action | tofrom | amount | checkNum | why       |*
  | draw   | from   | $30    |        1 | to bring your balance up to the target you set |

Scenario: a member with low credit line gets credit for the bank transfer after enough time
  Given these "txs2":
  | txid | payee | amount             | channel  | created                        | completed |*
  |    1 | .ZZA  | %(%USDTX_FAST + 1) | %TX_CRON | %(%now - %DAY_SECS*USDTX_DAYS) |         0 |
  When cron runs "getFunds"
  Then balances:
  | uid  | balance            |*
  | .ZZA | %(%USDTX_FAST + 1) |
  And these "txs2":
  | txid | payee | amount             | channel  | completed |*
  |    1 | .ZZA  | %(%USDTX_FAST + 1) | %TX_CRON | %now      |
  
Scenario: a member has a negative balance
  Given balances:
  | uid  | savingsAdd | balance |*
  | .ZZA |          0 | -50     |
  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | channel  |*
  |    1 | .ZZA  |  150   | %TX_CRON |
  And we notice "banked|bank tx number" to member ".ZZA" with subs:
  | action | tofrom | amount | checkNum | why       |*
  | draw   | from   | $150   |        1 | to bring your balance up to the target you set |
  
Scenario: an unbanked non-drawing member barely below target cannot get funded
  Given balances:
  | uid  | balance |*
  | .ZZA | 0      |
  | .ZZB | 99.99  |

  When cron runs "getFunds"
  Then we notice "cannot bank|when funded|how to fund" to member ".ZZB" with subs:
  | tofrom | why       |*
  | from   | to target |
  
Scenario: a member is at target
  Given balances:
  | uid  | savingsAdd | balance |*
  | .ZZA |          0 |     100 |
  When cron runs "getFunds"
  Then bank transfer count is 0
  
Scenario: a member is under target but already requested barely enough funds from the bank
  Given balances:
  | uid  | savingsAdd | balance |*
  | .ZZA |          0 |      20 |
  | .ZZB |          0 |     100 |
  When cron runs "getFunds"
  Then these "txs2":
  | payee | amount | channel  |*
  | .ZZA  |     80 | %TX_CRON |
  When cron runs "getFunds"
# (again)  
  Then bank transfer count is 1
  
Scenario: a member is under target and has requested insufficient funds from the bank
# This works only if member requests more than USDTX_FAST the first time (hence ZZD, whose target is 300)
  Given members:
  | uid  | fullName | floor | minimum | flags            | achMin | bankAccount |*
  | .ZZD | Dee Four |   -50 |     300 | ok,refill,bankOk | 30     | USkk9000004 |
  And balances:
  | uid  | balance |*
  | .ZZD |      20 |
  When cron runs "getFunds"
  Then these "txs2":
  | payee | amount | deposit | completed |*
  | .ZZD  | 280.00 |       0 |         0 |
  Given balances:
  | uid  | savingsAdd | balance |*
  | .ZZD |          0 |   19.99 |
  When cron runs "getFunds"
  Then these "txs2":
  | payee | amount | xid |*
  | .ZZD  | 280.01 |   2 |

Scenario: a member with zero target has balance below target
  Given members:
  | uid  | minimum | achMin | flags            | bankAccount |*
  | .ZZD |       0 |     30 | ok,refill,bankOk | USkk9000004 |
  And balances:
  | uid  | balance | minimum |*
  | .ZZD |     -10 |       0 |
  When cron runs "getFunds"
  Then these "txs2":
  | payee | amount |*
  | .ZZD  |     30 |
  
Scenario: an unbanked member with zero target has balance below target
  Given members:
  | uid  | minimum | achMin | flags | risks |*
  | .ZZD |       0 |     30 |       |       |
  And balances:
  | uid  | minimum | balance |*
  | .ZZA |     100 |     110 |
  | .ZZD |       0 |    -110 |
  When cron runs "getFunds"
  Then bank transfer count is 0

Scenario: a member has a deposited but not completed transfer
  Given balances:
  | uid  | balance |*
  | .ZZA |  80 |
  | .ZZB | 100 |
  And these "txs2":
  | txid | payee | amount | created   | completed | deposit    |*
  | 5001 | .ZZA  |     50 | %today-4d |         0 | %(%today-%USDTX_DAYS*%DAY_SECS-9) |
  # -9 in case the test takes a while (elapsed time is slightly more than USDTX_DAYS days)
  When cron runs "getFunds"
  Then bank transfer count is 1

Scenario: an account has a target but no refills
  Given members have:
  | uid  | flags     |*
  | .ZZB | ok,bankOk |
  And balances:
  | uid  | balance |*
  | .ZZA |     100 |
  | .ZZB |     -50 |
  When cron runs "getFunds"
  Then bank transfer count is 0

Scenario: a non-member has a target and refills
  Given members:
  | uid  | fullName | floor | minimum | flags         | achMin | bankAccount |*
  | .ZZE | Eve Five |     0 |     100 | refill,bankOk | 30     | USkk9000005 |
  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | channel  |*
  |    1 | .ZZA  |    100 | %TX_CRON |
  And count "txs" is 1
  And count "txs2" is 1
  And count "tx_requests" is 0

Scenario: a member's bank account gets verified
  Given members have:
  | uid  | balance | flags     |*
  | .ZZA |       0 | ok,refill |
  And these "txs2":
  | txid | payee | amount | created   | completed | deposit   |*
  |    1 | .ZZA  |      0 | %today-4d |         0 | %today-3d |
  When cron runs "everyDay"
  Then count "txs2" is 0
  And members have:
  | uid  | balance | flags            |*
  | .ZZA |       0 | ok,refill,bankOk |

Scenario: a member account needs more funding while not yet verified and something is combinable
  Given members have:
  | uid  | balance | flags     |*
  | .ZZA |      10 | ok,refill |
  | .ZZB |     200 |           |
  And these "txs2":
  | txid | payee | amount | created | completed | deposit |*
  |    1 | .ZZA  |      0 | %today  |         0 |       0 |
  |    2 | .ZZA  |     10 | %now+2d |         0 |       0 |
  Then count "txs2" is 2
  And count "txs" is 2
  
  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit | xid |*
  |    2 | .ZZA  |     90 | %now+2d |         0 |       0 |   2 |
  And these "txs":
  | xid | amount | payer | payee | for       | taking |*
  |   1 |      0 | bank  | .ZZA  | ?         |     1 |
  |   2 |      0 | bank  | .ZZA  | from bank |     1 |
  And count "txs2" is 2
  And count "txs" is 2
  
Scenario: a member has a negative balance, but no agreement to bring it up to zero
  Given members have:
  | uid  | balance | floor | flags               |*
  | .ZZA | -100    | 0     | ok,bankOk,confirmed |
  | .ZZB | 0       | 0     | ok,confirmed        |
  | .ZZC | 0       | 0     | ok,co               |
  When cron runs "getFunds"
  Then count "txs2" is 0  

# bug fix test
Scenario: a dormant joint member with a negative balance hasn't had wentNeg set yet
  Given these "relations":
  | main  | other | permission |*
  | .ZZA  | .ZZB  | joint      |
  | .ZZB  | .ZZA  | joint      |
  And members have:
  | uid  | jid  | balance | bankAccount |*
  | .ZZA | .ZZB | -100    | USkk9000001 |
  | .ZZB | .ZZA | -100    | USkk9000001 |
  
  When cron runs "getFunds"
  Then members have:
  | uid  | wentNeg |*
  | .ZZA | %now    |

Skip no longer delaying first transfer, to verify account first
Scenario: member's bank account has not been verified
  Given members have:
  | uid  | balance | flags     |*
  | .ZZA |      10 | ok,refill |
  When cron runs "getFunds"
  Then these "txs2":
  | txid | payee | amount | created | completed | deposit | xid |*
  |    1 | .ZZA  |      0 | %today  |         0 |       0 |   0 |
  |    2 | .ZZA  |     90 | %now+3d |         0 |       0 |   1 |
  And these "txs":
  | xid | amount | payer | payee | taking |*
  |   1 |      0 | bank  | .ZZA |      1 |
