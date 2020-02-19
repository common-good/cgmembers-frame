Feature: 1099s
AS an administrator
I WANT to generate a 1099-K report to file electronically with the IRS in January
AND generate a substitute 1099-K form for each payee
SO we are following the law and won't get busted

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags                       | created    | risks   | activated | state |*
  | .ZZA | Abe One    | -1000 | personal    | ok,roundup,confirmed,bankOk | %today-15m | hasBank | %now-20m  | MA    |
  | .ZZB | Bea Two    | -2000 | personal    | ok,admin                    | %today-15m |         | %now-20m  | MA    |
  | .ZZC | Corner Pub | -3000 | corporation | ok,co                       | %today-15m |         | %now-20m  | MA    |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payee | amount  | created    | completed  | deposit    |*
  |   11 |  .ZZA |   10000 | %today-13m | %today-13m | %today-13m |
  |   12 |  .ZZB |   20000 | %today-13m | %today-13m | %today-13m |
  |   13 |  .ZZC |   30000 | %today-13m | %today-13m | %today-13m |
  |   14 |  .ZZA |     110 | %today-3d  |         0  | %today-13m |
  |   15 |  .ZZA |    -220 | %today-4d  | %today-4d  |          0 |
  |   16 |  .ZZA |    -330 | %today-4d  | %today-4d  |          0 |
  # The usd transfers create same-numbered transactions
  And balances:
  | uid  | balance |*
  | .ZZA |     9450 |
  | .ZZB |    20000 |
  | .ZZC |    30000 |
  Given transactions: 
  | xid | created     | amount  | from | to   | purpose | taking | payerTid | payeeTid | reversesXid |*
  |  44 | %today-360d |     100 | .ZZB | .ZZA | food E  | 0      |       24 |       34 |             |
  |  45 | %today-330d |   11000 | .ZZC | .ZZA | box F   | 1      |       25 |       35 |             |
  |  46 | %today-300d |    2400 | .ZZA | .ZZB | what G  | 0      |       26 |       36 |             |
  |  47 | %today-270d |     500 | .ZZB | .ZZC | book P  | 0      |       27 |       37 |             |
  |  48 | %today-270d |    1200 | .ZZA | .ZZC | this Q  | 1      |       28 |       38 |             |
  |  49 | %today-45d  |    1000 | .ZZA | .ZZB | vibe V  | 0      |       29 |       39 |             |
  |  50 | %today-3d   |    -100 | .ZZB | .ZZA | food E  | 0      |       30 |       40 |          44 |
  |  51 | %today      |    -123 | .ZZB | .ZZA | stuff W | 0      |       31 |       41 |             |
  Then balances:
  | uid  | balance  |*
  | .ZZA |    15727 |
  | .ZZB |    23023 |
  | .ZZC |    20700 |
  
Scenario: admin generates a 1099 report for the past 12 months
  When member ".ZZB" runs a 1099 report type "MC" with testing "0"
  Then we download "forms1099MC-FY2019.bin" with records:
  | type | who  | cnt | amounts                                     | seq |*
  | T    |      |     |                                             |   1 |  
  | A    | .AAB |     |                                             |   2 |
  | B    | .ZZA |   1 | 11000/11000/0/0/11000/0/0/0/0/0/0/0/0/0/0/0 |   3 |
  | B    | .ZZB |   2 | 3400/3400/0/0/0/2400/0/0/0/0/0/0/0/0/1000/0 |   4 |
  | B    | .ZZC |   2 | 1700/1700/0/0/0/0/1700/0/0/0/0/0/0/0/0/0    |   5 |
  | C    |      |     | 16100/16100/0/0/11000/2400/1700/0/0/0/0/0/0/0/1000/0 |   6 |
  | K    |      |     | 16100/16100/0/0/11000/2400/1700/0/0/0/0/0/0/0/1000/0 |   7 |
  | F    |      |     |                                             |   8 |
