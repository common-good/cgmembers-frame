Feature: Customer Statements
AS a member company
I WANT a prinatable report of my invoices and transactions with any specific customer
SO we can keep things straight between us.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags | created    | address |*
  | .ZZA | Abe One    | -100  | personal    | ok    | %today-15m | 1 A St. |
  | .ZZB | Bea Two    | -200  | personal    | ok    | %today-15m | 2 B St. |
  | .ZZC | Corner Pub | -300  | corporation | ok,co | %today-15m | 3 C St. |
  And relations:
  | main | agent | permission |*
  | .ZZC | .ZZB  | buy        |
  And usd transfers:
  | txid | payee | amount | created   | completed |*
  | 1001 |  .ZZA |   1000 | %today-3m | %today-3m |
  | 1003 |  .ZZC |   3000 | %today-3m | %today-3m |
  Given invoices:
  | nvid | created   | amount | from | to   | purpose  | status |*
  |    1 | %today-3m |    240 | .ZZA | .ZZB | what G   |      2 |
  |    2 | %today-2m |    120 | .ZZA | .ZZC | this Q   |      4 |
  |    3 | %today-1m |     80 | .ZZA | .ZZC | this CF  |      7 |
  |    4 | %today-5d |     90 | .ZZA | .ZZC | wrongly  | %TX_DENIED |
  |    5 | %today-2d |   2000 | .ZZA | .ZZC | realist  | %TX_APPROVED |
  And transactions: 
  | xid | created   | amount | from | to   | purpose  |*
  |   1 | %today-4m |    100 | .ZZC | .ZZA | that F   |
  |   2 | %today-2w |     50 | .ZZA | .ZZC | cacao P  |
  |   3 | %today-9d |    240 | .ZZA | .ZZC | what G   |
  |   4 | %today-8d |    120 | .ZZA | .ZZC | this Q   |
  |   5 | %today-4d |    100 | .ZZA | .ZZB | thug V   |
  |   6 | %today-3d |     80 | .ZZA | .ZZC | this CF  |
  |   7 | %today-1d |    100 | .ZZC | .ZZA | pool CJ  |

Scenario: A company looks at a customer statement
  When agent "C:B" views "customer" statement for member ".ZZA"
  Then we show "Corner Pub" with:
  || 3 C St. |
  || Abe One: Account NEWZZA |
  || 1 A St. |
  || STATEMENT |
  And with:
  | Date    |        | Description     | Invoice | Paid | Balance  |
  |         |        | Opening balance |         |      |     0.00 |
  | %mdY-4m |  tx #1 | that F          |         | -100 |   100.00 |
  | %mdY-2m | inv #2 | this Q          |     120 |      |   220.00 |
  | %mdY-1m | inv #3 | this CF         |      80 |      |   300.00 |
  | %mdY-2w |  tx #2 | cacao P         |         |   50 |   250.00 |
  | %mdY-9d |  tx #3 | what G          |         |  240 |    10.00 |
  | %mdY-8d |  tx #4 | this Q          |         |  120 |  -110.00 |
  | %mdY-5d | inv #4 | wrongly         |      90 |      |   -20.00 |
  | %mdY-3d |  tx #6 | this CF         |         |   80 |  -100.00 |
  | %mdY-2d | inv #5 | realist         |   2,000 |      | 1,900.00 |
  | %mdY-1d |  tx #7 | pool CJ         |         | -100 | 2,000.00 |
  And with:
  || Total due: $2,000.00 |

