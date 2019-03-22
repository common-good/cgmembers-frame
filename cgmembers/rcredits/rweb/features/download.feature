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
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payee | amount | created   | completed |*
  |  501 | .ZZA  |   1000 | %today-4m | %today-4m |
  |  502 | .ZZB  |   2000 | %today-5m | %today-5m |
  |  503 | .ZZC  |   3000 | %today-6m | %today-6m |
  |  504 | .ZZA  |    200 | %today-3d |         0 |
  And invoices:
  | nvid | created   | amount | from | to   | purpose  | status |*
  |    1 | %today-3m |    240 | .ZZA | .ZZB | what G   |      6 |
  |    2 | %today-1w |    120 | .ZZA | .ZZC | this Q   |      8 |
  |    3 | %today-5d |     80 | .ZZA | .ZZC | this CF  |     10 |
  |    4 | %today-5d |     99 | .ZZA | .ZZC | wrongly  | %TX_DENIED |
  |    5 | %today-5d |     12 | .ZZA | .ZZC | realist  | %TX_APPROVED |
  And transactions: 
  | xid | created   | amount | from | to   | purpose  | taking |*
  |   4 | %today-5m |     10 | .ZZB | .ZZA | cash E   | 0      |
  |   5 | %today-4m |    100 | .ZZC | .ZZA | usd F    | 1      |
  |   6 | %today-3m |    240 | .ZZA | .ZZB | what G   | 0      |
  |   7 | %today-2w |     50 | .ZZB | .ZZC | cash P   | 0      |
  |   8 | %today-1w |    120 | .ZZA | .ZZC | this Q   | 1      |
  |   9 | %today-6d |    100 | .ZZA | .ZZB | cash V   | 0      |
  |  10 | %today-5d |     80 | .ZZA | .ZZC | this CF  | 1      |
  |  11 | %today-5d |    100 | .ZZC | .ZZA | cash CJ  | 1      |
  Then balances:
  | uid  | balance |*
  | .ZZA |     670 |
  | .ZZB |    2280 |
  | .ZZC |    3050 |

Scenario: A member downloads transactions for the past year
  Given members have:
  | uid  | fullName |*
  | ctty | ZZrCred  |
  When member ".ZZA" visits page "history/transactions/period=365&download=1"
  Then we download "%PROJECT_ID%todayn-12m-%todayn.csv" with:
  # For example commongood20120525-20130524.csv
  | Tx# | Date    | Name    | Purpose   | From Bank | From You | To You | Balance | Net  |*
  | 7   | %ymd-5d | Our Pub | cash CJ   |           |          |    100 |    670 |  100 |
  | 6   | %ymd-5d | Our Pub | this CF   |           |       80 |        |    570 |  -80 |
  | 5   | %ymd-6d | Bea Two | cash V    |           |      100 |        |    650 | -100 |
  | 4   | %ymd-1w | Our Pub | this Q    |           |      120 |        |    750 | -120 |
  | 3   | %ymd-3m | Bea Two | what G    |           |      240 |        |    870 | -240 |
  | 501 | %ymd-4m |         | from bank |      1000 |          |        |   1110 | 1000 |
  | 2   | %ymd-4m | Our Pub | usd F     |           |          |    100 |    110 |  100 |
  | 1   | %ymd-5m | Bea Two | cash E    |           |          |     10 |     10 |   10 |
  |     |         | TOTALS  |           |      1000 |      540 |    210 |        |  670 |
#  | 1   | %ymd-7m | ZZrCred | signup    |           |          |        |    250 |  250 |
  And with download columns:
  | column |*
  | Date   |

Scenario: A member downloads incoming invoices for the past year
  When member ".ZZA" visits page "history/invoices-from/period=365&download=1"
  Then we download "cgInvoicesFrom%todayn-12m-%todayn.csv" with:
  # For example cgInvoicesFrom20120525-20130524.csv
  | Inv# | Date    | Name    | Purpose | Amount | Status       |*
  |    1 | %ymd-3m | Bea Two | what G  |    240 | paid (Tx#2)  |
  |    2 | %ymd-1w | Our Pub | this Q  |    120 | paid (Tx#8)  |
  |    3 | %ymd-5d | Our Pub | this CF |     80 | paid (Tx#10) |
  |    4 | %ymd-5d | Our Pub | wrongly |     99 | denied ()    |
  |    5 | %ymd-5d | Our Pub | realist |     12 | Approved     |
  And with download columns:
  | column |*
  | Date   |

