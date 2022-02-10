Feature: 1099s
AS an administrator
I WANT to generate a 1099-K report to file electronically with the IRS in January
AND generate a substitute 1099-K form for each payee
SO we are following the law and won't get busted

Setup:
  Given members:
  | uid       | .ZZA                        | .ZZB       | .ZZC        |**
  |fullName   | Abe One                     | Bea Two    | Corner Pub  |
  |floor      | -1000                       | -2000      | -3000       |
  |acctType   | personal                    | personal   | corporation |
  |flags      | ok,roundup,confirmed,bankOk | ok,admin   | ok,co       |
  |created    | %today-15m                  | %today-15m | %today-15m  |
  |risks      | hasBank                     |            |             |
  |activated  | %now-20m                    | %now-20m   | %now-20m    |
  |state      | MA                          | MA         | MA          |
  | federalId | 001010001                   | 001010002  | 001010003   |
  And these "admins":
  | uid  | vKeyE     | can           |*
  | .ZZB | DEV_VKEYE | v,ten99,panel |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And these "txs2":
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
  Given these "txs": 
  | xid | created     | amount  | payer | payee | purpose | taking | reversesXid |*
  |  44 | %today-361d |     100 | .ZZB | .ZZA | food E  | 0        |             |
  |  45 | %today-331d |   11000 | .ZZC | .ZZA | box F   | 1        |             |
  |  46 | %today-301d |    2400 | .ZZA | .ZZB | what G  | 0        |             |
  |  47 | %today-271d |     500 | .ZZB | .ZZC | book P  | 0        |             |
  |  48 | %today-271d |    1200 | .ZZA | .ZZC | this Q  | 1        |             |
  |  49 | %today-45d  |    1000 | .ZZA | .ZZB | vibe V  | 0        |             |
  |  50 | %today-3d   |    -100 | .ZZB | .ZZA | food E  | 0        |          44 |
  |  51 | %today      |    -123 | .ZZB | .ZZA | stuff W | 0        |             |
  Then balances:
  | uid  | balance  |*
  | .ZZA |    15727 |
  | .ZZB |    23023 |
  | .ZZC |    20700 |
 
Scenario: admin generates a 1099 report for the past 12 months
  Given member ".ZZB" is signed in
  And member ".ZZB" scans admin card "%DEV_VKEYPW"
  When member ".ZZB" runs a 1099 report type "K" with testing "0"
  Then we download "forms1099-K-Y<LY>.bin" with "1099" records:
  | type | who  | cnt | amounts                                     | seq |*
  | T    |      |     |                                             |   1 |  
  | A    | .AAB |     |                                             |   2 |
  | B    | .ZZA |   1 | 11000/11000/0/0/11000/0/0/0/0/0/0/0/0/0/0/0 |   3 |
  | B    | .ZZB |   2 | 3400/3400/0/0/0/2400/0/0/0/0/0/0/0/0/1000/0 |   4 |
  | B    | .ZZC |   2 | 1700/1700/0/0/0/0/1700/0/0/0/0/0/0/0/0/0    |   5 |
  | C    |      |     | 16100/16100/0/0/11000/2400/1700/0/0/0/0/0/0/0/1000/0 |   6 |
  | K    |      |     | 16100/16100/0/0/11000/2400/1700/0/0/0/0/0/0/0/1000/0 |   7 |
  | F    |      |     |                                             |   8 |
