Feature: Statements
AS a member
I WANT a prinatable report of my transactions for the month
SO I have a formal record of them.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags | created    |*
  | .ZZA | Abe One    | -100  | personal    | ok    | %today-15m |
  | .ZZB | Bea Two    | -200  | personal    | ok,co | %today-15m |
  | .ZZC | Corner Pub | -300  | corporation | ok,co | %today-15m |
  And members have:
  | uid  | fullName |*
  | ctty | ZZrCred  |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | txid | payee | amount | created    | completed  | xid |*
  | 1001 |  .ZZA |   1000 | %today-3m  | %today-3m  |   1 |
  | 1002 |  .ZZB |   2000 | %today-3m  | %today-3m  |   2 |
  | 1003 |  .ZZC |   3000 | %today-3m  | %today-3m  |   3 |
  | 1004 |  .ZZA |     11 | %lastm+5d  | %lastm+2d  |   4 |
  | 1005 |  .ZZA |    -22 | %lastm+8d  | %lastm+8d  |   5 |
  | 1006 |  .ZZA |    -33 | %lastm+10d | %lastm+10d |   6 |
  Then balances:
  | uid  | balance |*
  | .ZZA |     956 |
  | .ZZB |    2000 |
  | .ZZC |    3000 |
  Given transactions: 
  | xid | created   | amount | from | to   | purpose | taking | payerTid | payeeTid |*
  | 14  | %lastm+3d |     10 | .ZZB | .ZZA | cash E  | 0      |      114 |      214 |
  | 15  | %lastm+4d |   1100 | .ZZC | .ZZA | usd F   | 1      |      115 |      215 |
  | 16  | %lastm+5d |    240 | .ZZA | .ZZB | what G  | 0      |      116 |      216 |
  | 19  | %lastm+6d |     50 | .ZZB | .ZZC | cash P  | 0      |      119 |      219 |
  | 20  | %lastm+7d |    120 | .ZZA | .ZZC | this Q  | 1      |      120 |      220 |
  | 23  | %lastm+9d |    100 | .ZZA | .ZZB | cash V  | 0      |      123 |      223 |
  Then balances:
  | uid  | balance |*
  | .ZZA |    1606 |
  | .ZZB |    2280 |
  | .ZZC |    2070 |

Scenario: A member looks at a statement for previous month
  When member ".ZZA" views statement for %lastmy
  Then we show "ZZA" with:
  | Starting | From Bank | Paid   | Received | Ending   |
  | 1,000.00 | -44.00    | 460.00 | 1,110.00 | 1,606.00 |
  And with:
  | Tx  | Date        | Name          | Purpose   | Amount   |
  |   2 | %lastmd+2d  | --            | from bank |    11.00 |
  | 214 | %lastmd+3d  | Bea Two       | cash E    |    10.00 |
  | 215 | %lastmd+4d  | Corner Pub    | usd F     | 1,100.00 |
  | 116 | %lastmd+5d  | Bea Two       | what G    |  -240.00 |
  | 120 | %lastmd+7d  | Corner Pub    | this Q    |  -120.00 |
  |   3 | %lastmd+8d  | --            | to bank   |   -22.00 |
  | 123 | %lastmd+9d  | Bea Two       | cash V    |  -100.00 |
  |   4 | %lastmd+10d | --            | to bank   |   -33.00 |
  And without:
  | rebate  |
  | bonus   |
