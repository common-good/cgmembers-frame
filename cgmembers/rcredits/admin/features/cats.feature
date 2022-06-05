Feature: Categories (cats)
AS an administrator
I WANT to categorize transactions and export them to QuickBooks
SO we save a lot of time on bookkeeping

Setup:
  Given members:
  | uid  | fullName | floor | flags              |*
  | .ZZA | Abe One  |  -250 | ok,confirmed,admin |
  | .ZZB | Bea Two  |  -250 | ok,confirmed,debt  |
  | .ZZC | Our Pub  |     0 | ok,confirmed,co    |
  | .ZZF | Fox Co   |     0 | ok,confirmed,co    |
  And these "admins":
  | uid  | vKeyE     | can                 |*
  | .ZZA | DEV_VKEYE | v,panel,editTx,code |
  And these "people":
  | pid | fullName |*
  | 101 | Yoyo Yot |
  | 102 | Zeta Zot |
  And member ".ZZC" is sponsored
  And these "tx_rules":
  | id        | 1            |**
  | payer     |              |
  | payerType | %REF_ANYBODY |
  | payee     | .ZZC         |
  | payeeType | %REF_ACCOUNT |
  | from      | %MATCH_PAYEE |
  | to        | cgf          |
  | action    | %ACT_SURTX   |
  | amount    | 0            |
  | portion   | .05          |
  | purpose   | sponsor      |
  | minimum   | 0            |
  | useMax    |              |
  | amtMax    |              |
  | template  |              |
  | start     | %now         |
  | end       |              |
  | code      |              |
  And these "txs":
  | eid | xid | created | amount | payer      | payee | purpose | rule | type  | recursId | flags   |*
# non-member gifts to CG  
  | 201 |  21 | %now-6m |    201 | %UID_OUTER | cgf   | by CC   |      | outer |          | cc,gift |
  | 202 |  22 | %now-5m |    202 | %UID_OUTER | cgf   | by ACH  |      | outer |          | gift    |
  | 203 |  23 | %now-5m |    203 | %UID_OUTER | cgf   | by ACHs |      | outer |     2003 | gift    |
# non-member gifts to a fiscally sponsored project
  | 301 |  31 | %now-5m |    301 | %UID_OUTER | .ZZC  | by CC   |      | outer |          | cc,gift |
  | 311 |  31 | %now-5m |      9 | .ZZC       | cgf   | %FS_NOTE|  311 | aux   |          | cc,gift |
  | 321 |  31 | %now-5m |      9 | .ZZC       | cgf   | cc fee  |  312 | xfee  |          | cc,gift |
  | 302 |  32 | %now-5m |    302 | %UID_OUTER | .ZZC  | by ACH  |      | outer |          | gift    |
  | 312 |  32 | %now-5m |      9 | .ZZC       | cgf   | %FS_NOTE|  311 | aux   |          | gift    |
  | 303 |  33 | %now-5m |    303 | %UID_OUTER | .ZZC  | by ACHs |      | outer |     2003 | gift    |
  | 313 |  33 | %now-5m |      9 | .ZZC       | cgf   | %FS_NOTE|  311 | aux   |          | gift    |
# non-member payment (or gift) to some other member company (CC only)
  | 401 |  41 | %now-5m |    401 | %UID_OUTER | .ZZF  | by CC   |      | outer |          | cc      |
  | 411 |  41 | %now-5m |     12 | .ZZF       | cgf   | cc fee  |  412 | xfee  |          | cc      |
# member gifts to CG  
  | 501 |  51 | %now-5m |    501 | .ZZB       | cgf   | once    |      | prime |          | gift    |
  | 502 |  52 | %now-5m |    502 | .ZZF       | cgf   | once co |      | prime |          | gift    |
  | 503 |  53 | %now-5m |    503 | .ZZB       | .ZZF  | thing   |      | prime |          |         |
  | 513 |  53 | %now-5m |      5 | .ZZB |%UID_STEPUPS | stepup  |  513 | aux   |          |         |
  | 504 |  54 | %now-5m |    504 | %UID_REGULARS| cgf | reglars |      | prime |          | gift    |
  | 505 |  55 | %now-5m |    505 | %UID_ROUNDUPS| cgf | rounds  |      | prime |          | gift    |
  | 506 |  56 | %now-5m |    506 | %UID_CRUMBS  | cgf | crumbs  |      | prime |          | gift    |
  | 507 |  57 | %now-5m |    507 | %UID_STEPUPS | cgf | stepups |      | prime |          | gift    |
# CG expenses
  | 508 |  58 | %now-5m |    508 | cgf        | .ZZF  | grant   |      | prime |          | gift    |
  | 509 |  59 | %now-5m |    509 | cgf        | .ZZB  | labor   |      | prime |          |         |
  | 500 |  50 | %now-5m |    500 | cgf        | .ZZF  | other   |      | prime |          |         |
# member gifts to a fiscally sponsored project
  | 601 |  61 | %now-5m |    601 | .ZZB       | .ZZC  | once    |      | prime |          | gift    |
  | 611 |  61 | %now-5m |     18 | .ZZC       | cgf   | %FS_NOTE|  611 | aux   |          | gift    |
  | 602 |  62 | %now-5m |    602 | .ZZF       | .ZZC  | once co |      | prime |          | gift    |
  | 612 |  62 | %now-5m |     18 | .ZZC       | cgf   | %FS_NOTE|  611 | aux   |          | gift    |
  | 603 |  63 | %now-5m |    603 | .ZZB       | .ZZF  | thing   |      | prime |          | gift    |
  | 613 |  63 | %now-5m |     30 | .ZZB       | .ZZC  | stepup  |  613 | aux   |          | gift    |
  | 623 |  63 | %now-5m |      1 | .ZZC       | cgf   | %FS_NOTE|  611 | aux   |          | gift    |
  | 604 |  64 | %now-5m |    604 | .ZZB       | .ZZC  | regular |      | prime |     6004 | gift    |
  | 614 |  64 | %now-5m |     18 | .ZZC       | cgf   | %FS_NOTE|  611 | aux   |          | gift    |
  | 605 |  65 | %now-5m |    605 | .ZZB       | .ZZC  | non-don |      | prime |          |         |
  | 615 |  65 | %now-5m |     18 | .ZZC       | cgf   | %FS_NOTE|  611 | aux   |          |         |
# not sure 605 should be handled as a donation (but fiscally sponsored projects should not receive 
# anything but donations through CG, unless they have a separate CG account. Yes?)

# expenses of a fiscally sponsored project (most must be set by hand)
  | 709 |  79 | %now-5m |    709 | .ZZC       | .ZZB  | labor   |      | prime |          |         |
  | 700 |  70 | %now-5m |    700 | .ZZC       | .ZZB  | other   |      | prime |          |         |
# income and expenses of some other member company
  | 801 |  81 | %now-5m |    801 | .ZZB       | .ZZF  | once    |      | prime |          | gift    |
  | 800 |  80 | %now-5m |    800 | .ZZF       | .ZZB  | labor   |      | prime |          |         |

#  And these "tx_entries":
#  |  id | xid | created | amount | uid | purpose | rule | type  | recursId |*
#  |  23 |  23 | %now-5m |     10 | %UID_OUTER | cgf | don  |      | outer |     2004 |
  And these "txs2":
  | xid | payee | amount | created | deposit | completed | pid | bankAccount  | bankTxId |*
  |  21 | cgf   |    201 | %now-5m | %now-5d | %now-6d   | 101 | %NUL         | 0        |
  |  22 | cgf   |    202 | %now-5m | %now0   | %now-4m   | 102 | %T_BANK_ACCT | %now1234 |
  |  23 | cgf   |    203 | %now-5m | %now0   | %now-4m   | 102 | %T_BANK_ACCT | %now1234 |
  |  31 | .ZZC  |    301 | %now-5m | %now-4d | %now-4m   | 101 | %NUL         | 0        |
  |  32 | cgf   |    302 | %now-2d | %now0   | %now      | 102 | %T_BANK_ACCT | %now1234 |
  |  33 | cgf   |    303 | %now-2d | %now0   | %now      | 102 | %T_BANK_ACCT | %now1234 |
  |  41 | .ZZF  |    401 | %now-5m | %now-3d | %now-3m   | 101 | %NUL         | 0        |

Scenario: admin visits the Set Categories page
  When member ".ZZA" visits page "sadmin/set-cats"
  Then we show "Set Transaction Categories" with:
  | Starting Date | | |
  | Overwrite     | No | Yes |
  | Set Cats      | | |

Scenario: admin sets most categories and sends to QBO
  When member ".ZZA" submits "sadmin/set-cats" with:
  | start | %now-9m |**
  Then we say "status": "Set 39 cats."
  And we show "Set Transaction Categories" with:
  | xid | type  | me          | you         | purpose |
  |  70 | prime | Our Pub     | Bea Two     | other   |
  |  50 | prime | Common Good | Fox Co      | other   |
#  | data set is empty |
  And these "txs":
  | eid | cat1       | cat2        |*
  | 201 |            | D-ONCE      |
  | 202 |            | D-ONCE      |
  | 203 |            | D-ONCE      |
  | 301 |            | D-FBO       |
  | 311 | D-FBO      | FS-FEE      |
  | 321 | FBO-TX-FEE | TX-FEE-BACK |
  | 302 |            | D-FBO       |
  | 312 | D-FBO      | FS-FEE      |
  | 303 |            | D-FBO       |
  | 313 | D-FBO      | FS-FEE      |
  | 401 |            |             |
  | 411 |            | TX-FEE-BACK |
  | 501 |            | D-ONCE      |
  | 502 |            | D-COMPANY   |
  | 503 |            |             |
  | 513 |            |             |
  | 504 |            | D-REGULAR   |
  | 505 |            | D-ROUNDUP   |
  | 506 |            | D-CRUMB     |
  | 507 |            | D-STEPUP    |
  | 508 | TO-ORG     |             |
  | 509 | LABOR      |             |
  | 500 |            |             |
  | 601 |            | D-FBO       |
  | 611 | D-FBO      | FS-FEE      |
  | 602 |            | D-FBO       |
  | 612 | D-FBO      | FS-FEE      |
  | 603 |            |             |
  | 613 |            | D-FBO-STEPUP|
  | 623 | D-FBO      | FS-FEE      |
  | 604 |            | D-FBO       |
  | 614 | D-FBO      | FS-FEE      |
  | 605 |            | D-FBO       |
  | 615 | D-FBO      | FS-FEE      |
  | 709 | FBO-LABOR  |             |
  | 700 |            |             |
  | 801 |            |             |
  | 800 |            |             |

  When member ".ZZA" visits "qbo/op=txs"
  Then QBO gets Tx "cgFund#%now0" with IN "$1,010 (4)" and OUT "$0 (0)" dated "%ymd0" with entries:
  | 1010 Debit fund  | 1010 Credit POOL |
  And QBO gets Tx "cg#21":"by CC [Yoyo Yot (non-member)]" dated "%ymd-6m" with entries:
  | 201 Debit cgf     | 201 Credit D-ONCE         |
  | 201 Credit POOL   | 201 Debit PROCESSOR       |
  | 4.49 Debit TX-FEE | 4.49 Credit PROCESSOR     |
  # when we have a separate processor for sponsored activity, the above line will be PROCESSOR not FBO-...
  And QBO gets Tx "cg#22":"by ACH [Zeta Zot (non-member)]" dated "%ymd-5m" with entries:
  | 202 Debit cgf   | 202 Credit D-ONCE |
  And QBO gets Tx "cg#23":"by ACHs [Zeta Zot (non-member)]" dated "%ymd-5m" with entries:
  | 203 Debit cgf   | 203 Credit D-ONCE |
  And QBO gets Tx "cg#31":"by CC [Yoyo Yot (non-member)]" dated "%ymd-5m" with entries:
  | 301 Debit .ZZC    | 301 Credit D-FBO          |
  | 301 Credit POOL   | 301 Debit FBO-PROCESSOR   |
  | 6.48 Debit TX-FEE | 6.48 Credit FBO-PROCESSOR |
  | 9 Debit cgf       | 9 Credit FS-FEE           |
  | 9 Credit .ZZC     | 9 Debit D-FBO             |
  | 9 Debit cgf       | 9 Credit TX-FEE-BACK      |
  | 9 Credit .ZZC     | 9 Debit FBO-TX-FEE        |
  And QBO gets Tx "cg#32":"by ACH [Zeta Zot (non-member)]" dated "%ymd-5m" with entries:
  | 302 Debit .ZZC  | 302 Credit D-FBO        |
  | 9 Debit cgf     | 9 Credit FS-FEE         |
  | 9 Credit .ZZC   | 9 Debit D-FBO           |
  And QBO gets Tx "cg#33":"by ACHs [Zeta Zot (non-member)]" dated "%ymd-5m" with entries:
  | 303 Debit .ZZC  | 303 Credit D-FBO        |
  | 9 Debit cgf     | 9 Credit FS-FEE         |
  | 9 Credit .ZZC   | 9 Debit D-FBO           |
  And QBO gets Tx "cg#41":"by CC [Yoyo Yot (non-member)]" dated "%ymd-5m" with entries:
  | 401 Credit POOL   | 401 Debit FBO-PROCESSOR   |
  | 8.47 Debit TX-FEE | 8.47 Credit FBO-PROCESSOR |
  | 12 Debit cgf      | 12 Credit TX-FEE-BACK     |
  And QBO gets Tx "cg#51":"once [Bea Two]" dated "%ymd-5m" with entries:
  | 501 Debit cgf   | 501 Credit D-ONCE       |
  And QBO gets Tx "cg#52":"once co [Fox Co]" dated "%ymd-5m" with entries:
  | 502 Debit cgf   | 502 Credit D-COMPANY    |
  And QBO gets Tx "cg#54":"reglars [various]" dated "%ymd-5m" with entries:
  | 504 Debit cgf   | 504 Credit D-REGULAR    |
  And QBO gets Tx "cg#55":"rounds [various]" dated "%ymd-5m" with entries:
  | 505 Debit cgf   | 505 Credit D-ROUNDUP    |
  And QBO gets Tx "cg#56":"crumbs [various]" dated "%ymd-5m" with entries:
  | 506 Debit cgf   | 506 Credit D-CRUMB      |
  And QBO gets Tx "cg#57":"stepups [various]" dated "%ymd-5m" with entries:
  | 507 Debit cgf   | 507 Credit D-STEPUP     |
  And QBO gets Tx "cg#58":"grant [Fox Co]" dated "%ymd-5m" with entries:
  | 508 Credit cgf   | 508 Debit TO-ORG       |
  And QBO gets Tx "cg#59":"labor [Bea Two]" dated "%ymd-5m" with entries:
  | 509 Credit cgf   | 509 Debit LABOR        |
  And QBO gets Tx "cg#61":"once [Bea Two]" dated "%ymd-5m" with entries:
  | 601 Debit .ZZC   | 601 Credit D-FBO        |
  | 18 Debit cgf     | 18 Credit FS-FEE        |
  | 18 Credit .ZZC   | 18 Debit D-FBO          |
  And QBO gets Tx "cg#62":"once co [Fox Co]" dated "%ymd-5m" with entries:
  | 602 Debit .ZZC   | 602 Credit D-FBO        |
  | 18 Debit cgf     | 18 Credit FS-FEE        |
  | 18 Credit .ZZC   | 18 Debit D-FBO          |
  And QBO gets Tx "cg#63":"stepup [Bea Two]" dated "%ymd-5m" with entries:
  | 30 Debit .ZZC    | 30 Credit D-FBO-STEPUP  |
  | 1 Debit cgf      | 1 Credit FS-FEE         |
  | 1 Credit .ZZC    | 1 Debit D-FBO           |
  And QBO gets Tx "cg#64":"regular [Bea Two]" dated "%ymd-5m" with entries:
  | 604 Debit .ZZC   | 604 Credit D-FBO        |
  | 18 Debit cgf     | 18 Credit FS-FEE        |
  | 18 Credit .ZZC   | 18 Debit D-FBO          |
  And QBO gets Tx "cg#65":"non-don [Bea Two]" dated "%ymd-5m" with entries:
  | 605 Debit .ZZC   | 605 Credit D-FBO        |
  | 18 Debit cgf     | 18 Credit FS-FEE        |
  | 18 Credit .ZZC   | 18 Debit D-FBO          |
  And QBO gets Tx "cg#79":"labor [Bea Two]" dated "%ymd-5m" with entries:
  | 709 Credit .ZZC  | 709 Debit FBO-LABOR     |
  And QBO gets nothing else
