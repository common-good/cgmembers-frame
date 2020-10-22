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
  |  501 | .ZZA  |   1000 | %today-5m | %today-5m |
  |  502 | .ZZB  |   2000 | %today-5m | %today-5m |
  |  503 | .ZZC  |   3000 | %today-6m | %today-6m |
  |  504 | .ZZA  |    200 | %today-3d |         0 |
  And invoices:
  | nvid | created   | amount | payer | payee | purpose  | status |*
  |    1 | %today-3m |    240 | .ZZA | .ZZB | what G   |     11 |
  |    2 | %today-1w |    120 | .ZZA | .ZZC | this Q   |     12 |
  |    3 | %today-5d |     80 | .ZZA | .ZZC | this CF  |     13 |
  |    4 | %today-5d |     99 | .ZZA | .ZZC | wrongly  | %TX_DENIED |
  |    5 | %today-5d |     12 | .ZZA | .ZZC | realist  | %TX_APPROVED |
  And transactions: 
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

Scenario: A member downloads transactions for the past year
  Given members have:
  | uid  | fullName |*
  | ctty | ZZrCred  |
  When member ".ZZA" visits page "history/transactions/period=365&download=1"
  Then we download "%PROJECT_ID%todayn-12m-%todayn.csv" with:
  # For example commongood20120525-20130524.csv
  | Tx# | Date    | Name          | Purpose            | From You | To You | Balance |*
  | 28  | %ymd-5d | Our Pub       | cash CJ            |          |    100 |    670 |
  | 13  | %ymd-5d | Our Pub       | this CF (CG inv#3) |       80 |        |    570 |
  | 27  | %ymd-6d | Bea Two       | cash V             |      100 |        |    650 |
  | 12  | %ymd-1w | Our Pub       | this Q (CG inv#2)  |      120 |        |    750 |
  | 11  | %ymd-3m | Bea Two       | what G (CG inv#1)  |      240 |        |    870 |
  | 25  | %ymd-4m | Our Pub       | usd F              |          |    100 |   1110 |
  | 1   | %ymd-5m | --            | from bank          |          |   1000 |   1010 |
  | 24  | %ymd-6m | Bea Two       | cash E             |          |     10 |     10 |
  |     |         | TOTALS        |                    |      540 |   1210 |        |

Scenario: A member downloads incoming invoices for the past year
  When member ".ZZA" visits page "history/invoices-to/period=365&download=1"
  Then we download "cgInvoicesTo%todayn-12m-%todayn.csv" with:
  # For example cgInvoicesFrom20120525-20130524.csv
  | Inv# | Date    | Name    | Purpose | Amount | Status       |*
  |    5 | %ymd-5d | Our Pub | realist |     12 | Approved     |
  |    4 | %ymd-5d | Our Pub | wrongly |     99 | Denied ()    |
  |    3 | %ymd-5d | Our Pub | this CF |     80 | paid (Tx#13) |
  |    2 | %ymd-1w | Our Pub | this Q  |    120 | paid (Tx#12)  |
  |    1 | %ymd-3m | Bea Two | what G  |    240 | paid (Tx#11)  |
#   And with download columns:
#   | column |*
#   | Date   |
