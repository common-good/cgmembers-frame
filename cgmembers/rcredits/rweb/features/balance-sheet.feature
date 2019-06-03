Feature: Balance Sheet
AS a member
I WANT to see my community's financial position
SO I can judge how much we can safely invest or grant and how confident to be in our continued success.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags      | created    | backing |*
  | .ZZA | Abe One    | -100  | personal    | ok         | %today-15m |      10 |
  | .ZZB | Bea Two    | -200  | personal    | ok         | %today-15m |      20 |
  | .ZZC | Corner Pub | -300  | corporation | ok,co      | %today-15m |      30 |
  | .ZZD | Dee Four   | -400  | personal    | ok         | %today-15m |      40 |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And these "investments":
  | vestid | clubid | price | reserve |*
  |      1 |   ctty |   100 |     .15 |
  |      2 |   ctty |   200 |     .20 |
  And these "shares":
  | shid | vestid | shares |*
  |    1 |      1 |     10 |
  |    2 |      1 |     -5 |
  |    3 |      2 |     20 |
  And invoices:
  | payer | payee | amount | status   |*
  | .ZZB  | ctty  |     20 | approved |
  | ctty  | .ZZA  |     10 | pending  |
  And usd transfers:
  | payee | amount | created    | completed  |*
  |  .ZZA |   1000 | %today-13m | %today-13m |
  |  .ZZB |   2000 | %today-13m | %today-13m |
  |  .ZZC |   3000 | %today-13m | %today-13m |
  Then balances:
  | uid  | balance |*
  | ctty |       0 |
  | .ZZA |    1000 |
  | .ZZB |    2000 |
  | .ZZC |    3000 |
  | .ZZD |       0 |
  Given transactions: 
  | xid | created   | amount | from | to   | purpose |*
  |   4 | %today-5m |     10 | .ZZB | .ZZA | cash E  |
  |   5 | %today-4m |   1100 | .ZZC | .ZZA | usd F   |
  |   6 | %today-3m |    240 | .ZZA | .ZZB | what G  |
  |   7 | %today-2w |     50 | .ZZB | .ZZC | cash P  |
  |   8 | %today-1w |    120 | .ZZA | .ZZC | this Q  |
  |   9 | %today-6d |    100 | .ZZA | .ZZB | cash V  |
  |  10 | %today-4d |    200 | .ZZD | .ZZA | bribe   |
  |  11 | %today-3d |    300 | ctty | .ZZC | grant   |
  Then balances:
  | uid  | balance |*
  | ctty |    -300 |
  | .ZZA |    1850 |
  | .ZZB |    2280 |
  | .ZZC |    2370 |
  | .ZZD |    -200 |

Scenario: A member looks at the balance sheet
  Given variable "negReserve" is ".12"
  And variable "backingReserve" is ".6"
  When member ".ZZA" visits page "community/balance-sheet"
  Then we show "Balance Sheet" with:
  | for Common Good Western Mass | |
  | ASSETS  | |
  | Community Common Good Account | $-300.00 |
  | Dollar Pool | $6,000.00 |
  | Investments | $4,500.00 |
  | Negative Balance Promises | $200.00 |
  | Backing Promises | $100.00 |
  | Accounts Receivable | $20.00 |
  | Total Assets | $10,520.00 |
  | LIABILITIES | |
  | Common Good Account Balances | $6,500.00 |
  | Investment Loss Reserve (19.4%) | $875.00 |
  | Negative Balance Loss Reserve (12.0%) | $24.00 |
  | Backing Promise Loss Reserve (60.0%) | $60.00 |
  | Accounts Payable | $10.00 |
  | Total Liabilities | $7,469.00 |
  | Net Assets | $3,051.00 |

Scenario: A non-member looks at the balance sheet
  Given variable "negReserve" is ".12"
  And variable "backingReserve" is ".6"
  And member is logged out
  When member "?" visits page "community/balance-sheet"
  Then we show "Balance Sheet" with:
  | for all | |
  | ASSETS  | |
  | Dollar Pool | $6,000.00 |
  | Investments | $4,500.00 |
  | Negative Balance Promises | $500.00 |
  | Backing Promises | $100.00 |
  | Accounts Receivable | $20.00 |
  | Total Assets | $11,120.00 |
  | LIABILITIES | |
  | Common Good Account Balances | $6,500.00 |
  | Investment Loss Reserve (19.4%) | $875.00 |
  | Negative Balance Loss Reserve (12.0%) | $60.00 |
  | Backing Promise Loss Reserve (60.0%) | $60.00 |
  | Accounts Payable | $10.00 |
  | Total Liabilities | $7,505.00 |
  | Net Assets | $3,615.00 |
