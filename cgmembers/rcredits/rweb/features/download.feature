Feature: Download
AS a member
I WANT to download my transactions
SO I can see what happened and possbily integrate with an accounting program.

Setup:
  Given members:
  | uid  | fullName | floor | acctType    | flags      |*
  | .ZZA | Abe One  | -100  | personal    | ok         |
  | .ZZB | Bea Two  | -200  | personal    | ok,co      |
  | .ZZC | Our Pub  | -300  | corporation | ok,co      |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And these "txs2":
  | txid | payee | amount | created   | completed |*
  |  501 | .ZZA  |   1000 | %today-5m | %today-5m |
  |  502 | .ZZB  |   2000 | %today-5m | %today-5m |
  |  503 | .ZZC  |   3000 | %today-6m | %today-6m |
  |  504 | .ZZA  |    200 | %today-3d |         0 |
  And these "tx_requests":
  | nvid | created   | amount | payer | payee | purpose  | status |*
  |    1 | %today-3m |    240 | .ZZA | .ZZB | what G   |     11 |
  |    2 | %today-1w |    120 | .ZZA | .ZZC | this Q   |     12 |
  |    3 | %today-5d |     80 | .ZZA | .ZZC | this CF  |     13 |
  |    4 | %today-5d |     99 | .ZZA | .ZZC | wrongly  | %TX_DENIED |
  |    5 | %today-5d |     12 | .ZZA | .ZZC | realist  | %TX_APPROVED |
  And these "txs": 
  | xid | created   | amount | payer | payee | purpose  | taking | relType | rel |*
  |  11 | %today-3m |    240 | .ZZA  | .ZZB  | what G   | 0      | I       | 1   |
  |  12 | %today-1w |    120 | .ZZA  | .ZZC  | this Q   | 0      | I       | 2   |
  |  13 | %today-5d |     80 | .ZZA  | .ZZC  | this CF  | 0      | I       | 3   |
  |  24 | %today-6m |     10 | .ZZB  | .ZZA  | cash E   | 0      |         |     |
  |  25 | %today-4m |    100 | .ZZC  | .ZZA  | usd F    | 1      |         |     |
  |  26 | %today-2w |     50 | .ZZB  | .ZZC  | cash P   | 0      |         |     |
  |  27 | %today-6d |    100 | .ZZA  | .ZZB  | cash V   | 0      |         |     |
  |  28 | %today-5d |    100 | .ZZC  | .ZZA  | cash CJ  | 1      |         |     |
  Then balances:
  | uid  | balance |*
  | .ZZA |     670 |
  | .ZZB |    2280 |
  | .ZZC |    3050 |

Scenario: A member downloads transactions for the past year as CSV
  Given members have:
  | uid  | fullName |*
  | ctty | ZZrCred  |
  When member ".ZZA" visits page "history/transactions/period=365&download=CSV"
  Then we download "cgNEWZZA%todayn-12m-%todayn.csv" with:
# For example commongood20120525-20130524.csv
#  | Tx# | Date    | Name          | Purpose            | From You | To You | Balance |*
#  | 28  | %ymd-5d | Our Pub       | cash CJ            |          |    100 |    670 |
#  | 13  | %ymd-5d | Our Pub       | this CF (CG inv#3) |       80 |        |    570 |
#  | 27  | %ymd-6d | Bea Two       | cash V             |      100 |        |    650 |
#  | 12  | %ymd-1w | Our Pub       | this Q (CG inv#2)  |      120 |        |    750 |
#  | 11  | %ymd-3m | Bea Two       | what G (CG inv#1)  |      240 |        |    870 |
#  | 25  | %ymd-4m | Our Pub       | usd F              |          |    100 |   1110 |
#  | 1   | %ymd-5m | --            | from bank          |          |   1000 |   1010 |
#  | 24  | %ymd-6m | Bea Two       | cash E             |          |     10 |     10 |
#  |     |         | TOTALS        |                    |      540 |   1210 |        |
  
  | Tx# | Date    | Name          | Purpose            | From You | To You | Balance |*
  | 24  | %ymd-6m | Bea Two       | cash E             |          |     10 |     10 |
  | 1   | %ymd-5m | --            | from bank          |          |   1000 |   1010 |
  | 25  | %ymd-4m | Our Pub       | usd F              |          |    100 |   1110 |
  | 11  | %ymd-3m | Bea Two       | what G (CG inv#1)  |      240 |        |    870 |
  | 12  | %ymd-1w | Our Pub       | this Q (CG inv#2)  |      120 |        |    750 |
  | 27  | %ymd-6d | Bea Two       | cash V             |      100 |        |    650 |
  | 13  | %ymd-5d | Our Pub       | this CF (CG inv#3) |       80 |        |    570 |
  | 28  | %ymd-5d | Our Pub       | cash CJ            |          |    100 |    670 |
  |     |         | TOTALS        |                    |      540 |   1210 |        |

Scenario: A member downloads transactions for the past year as QBO
  Given members have:
  | uid  | fullName |*
  | ctty | ZZrCred  |
  When member ".ZZA" visits page "history/transactions/period=365&download=QBO"
  Then we download "cgNEWZZA%todayn-12m-%todayn.qbo" with:
# For example commongood20120525-20130524.qbo
#  | CHECKING| NEWZZA  | %todayn-12m | %todayn | | |
#  | DEP   | %todayn-5d |  100.00 | 28 | Our Pub | cash CJ            |
#  | DEBIT | %todayn-5d |  -80.00 | 13 | Our Pub | this CF (CG inv#3) |
#  | DEBIT | %todayn-6d | -100.00 | 27 | Bea Two | cash V             |
#  | DEBIT | %todayn-1w | -120.00 | 12 | Our Pub | this Q (CG inv#2)  |
#  | DEBIT | %todayn-3m | -240.00 | 11 | Bea Two | what G (CG inv#1)  |
#  | DEP   | %todayn-4m |  100.00 | 25 | Our Pub | usd F              |
#  | XFER  | %todayn-5m | 1000.00 |  1 | --      | from bank          |
#  | DEP   | %todayn-6m |   10.00 | 24 | Bea Two | cash E             |
#  | 10.00 | %todayn | | | | |
  
  | CHECKING | NEWZZA  | %todayn-12m | %todayn | | |
  | DEP      | %todayn-6m |   10.00 | 24 | Bea Two | cash E             |
  | XFER     | %todayn-5m | 1000.00 |  1 | --      | from bank          |
  | DEP      | %todayn-4m |  100.00 | 25 | Our Pub | usd F              |
  | DEBIT    | %todayn-3m | -240.00 | 11 | Bea Two | what G (CG inv#1)  |
  | DEBIT    | %todayn-1w | -120.00 | 12 | Our Pub | this Q (CG inv#2)  |
  | DEBIT    | %todayn-6d | -100.00 | 27 | Bea Two | cash V             |
  | DEBIT    | %todayn-5d |  -80.00 | 13 | Our Pub | this CF (CG inv#3) |
  | DEP      | %todayn-5d |  100.00 | 28 | Our Pub | cash CJ            |
  | 670.00   | %todayn | | | | |

Scenario: A member downloads incoming invoices for the past year
  When member ".ZZA" visits page "history/pending-from/period=365&download=1"
  Then we download "cgPendingFromNEWZZA%todayn-12m-%todayn.csv" with:
# For example cgInvoicesFrom20120525-20130524.csv
#  | Req# | Date    | Name    | Purpose | Amount | Status       |*
#  |    5 | %ymd-5d | Our Pub | realist |     12 | Approved     |
#  |    4 | %ymd-5d | Our Pub | wrongly |     99 | Denied       |
#  |    3 | %ymd-5d | Our Pub | this CF |     80 | paid (Tx#13)  |
#  |    2 | %ymd-1w | Our Pub | this Q  |    120 | paid (Tx#12)  |
#  |    1 | %ymd-3m | Bea Two | what G  |    240 | paid (Tx#11)  |
  
  | Req# | Date    | Name    | Purpose | Amount | Status       |*
  |    1 | %ymd-3m | Bea Two | what G  |    240 | paid (Tx#11)  |
  |    2 | %ymd-1w | Our Pub | this Q  |    120 | paid (Tx#12)  |
  |    3 | %ymd-5d | Our Pub | this CF |     80 | paid (Tx#13) |
  |    4 | %ymd-5d | Our Pub | wrongly |     99 | Denied       |
  |    5 | %ymd-5d | Our Pub | realist |     12 | Approved     |

#   And with download columns:
#   | column |*
#   | Date   |

Scenario: A member downloads transactions for the past year as Combo
  Given members have:
  | uid  | fullName |*
  | ctty | ZZrCred  |
  When member ".ZZC" visits page "history/transactions/period=365&download=Combo"
  Then we download "cgNEWZZC%todayn-12m-%todayn.csv" with:
# For example cgNEWZZC20120525-20130524.csv
  | Tx#    | Date    | Account | Name    | Purpose            | Invoiced | Payments | Status       |*
  | tx #25 | %ymd-4m | NEWZZA  | Abe One | usd F              |          |     -100 |              |
  | tx #26 | %ymd-2w | NEWZZB  | Bea Two | cash P             |          |       50 |              |
  | inv #2 | %ymd-1w | NEWZZA  | Abe One | this Q             |      120 |          | paid (Tx#12) |
  | tx #12 | %ymd-1w | NEWZZA  | Abe One | this Q (CG inv#2)  |          |      120 |              |
  | inv #3 | %ymd-5d | NEWZZA  | Abe One | this CF            |       80 |          | paid (Tx#13) |
  | inv #4 | %ymd-5d | NEWZZA  | Abe One | (DISPUTED) wrongly |       99 |          | Denied       |
  | inv #5 | %ymd-5d | NEWZZA  | Abe One | realist            |       12 |          | Approved     |
  | tx #13 | %ymd-5d | NEWZZA  | Abe One | this CF (CG inv#3) |          |       80 |              |
  | tx #28 | %ymd-5d | NEWZZA  | Abe One | cash CJ            |          |     -100 |              |

