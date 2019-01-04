Feature: Recurring transactions
AS a member
I WANT to review my recurring transactions
SO I can see what they are and terminate them.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags      | created    |*
  | .ZZA | Abe One    | -100  | personal    | ok,roundup | %today-15m |
  | .ZZB | Bea Two    | -200  | personal    | ok,co      | %today-15m |
  | .ZZC | Corner Pub | -300  | corporation | ok,co      | %today-15m |
  And recurs:
  | id    | payer | payee | amount | period | created    | ended      |*
  | 99900 | .ZZA  | .ZZC  | 37.00  | Q      | %today-13m | 0          |
  | 99901 | .ZZA  | .ZZB  | 43.00  | W      | %today-13m | %today-11m |
  | 99902 | .ZZA  | .ZZB  | 59.59  | Y      | %today-16m | 0          |
  | 99903 | .ZZB  | .ZZA  | 22.00  | Q      | %today-13m | 0          |
  | 99904 | .ZZB  | .ZZC  | 37.37  | Q      | %today-13m | 0          |
  | 99905 | .ZZC  | .ZZA  | 37.43  | W      | %today-13m | 0          |

Scenario: A member looks at their recurring transactions
  When member ".ZZA" visits page "history/show-recurring"
  Then we show "Recurring Transactions for Abe One" with:
  | To   | Amount | How often? | Starting | Next    | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | ~%mdY+2m |          |
  | Bea Two | 43.00  | Weekly    | %mdY-13m |         | %mdY-11m |
  | Bea Two | 59.59  | Yearly     | %mdY-16m | %mdY+8m |          |
  
Scenario: A member stops a recurring transaction
  When member ".ZZA" visits page "history/show-recurring/recId=99900&do=stop"
  Then we show "Recurring Transactions for Abe One" with:
  | To   | Amount | How often? | Starting | Next    | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m |         | %mdY     |
  | Bea Two | 43.00  | Weekly    | %mdY-13m |         | %mdY-11m |
  | Bea Two | 59.59  | Yearly     | %mdY-16m | %mdY+8m |          |
  When member ".ZZA" visits page "history/show-recurring/recId=99900&do=stop"
  Then we show "recur already ended"
  And we show "Recurring Transactions for Abe One" with:
  | To   | Amount | How often? | Starting | Next    | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m |         | %mdY     |
  | Bea Two | 43.00  | Weekly    | %mdY-13m |         | %mdY-11m |
  | Bea Two | 59.59  | Yearly     | %mdY-16m | %mdY+8m |          |

Scenario: A member attempts to stop a non-existent recurring transaction
  When member ".ZZA" visits page "history/show-recurring/recId=99999&do=stop"
  Then we show "invalid recur id"
  And we show "Recurring Transactions for Abe One" with:
  | To   | Amount | How often? | Starting | Next    | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m |  %mdY+2m |    |
  | Bea Two | 43.00  | Weekly    | %mdY-13m |         | %mdY-11m |
  | Bea Two | 59.59  | Yearly     | %mdY-16m | %mdY+8m |          |

Scenario: A member attempts to stop another member's. recurring transaction
  When member ".ZZA" visits page "history/show-recurring/recId=99904&do=stop"
  Then we show "recur not yours"
  And we show "Recurring Transactions for Abe One" with:
  | To   | Amount | How often? | Starting | Next    | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m |         %mdY+2m |      |
  | Bea Two | 43.00  | Weekly    | %mdY-13m |         | %mdY-11m |
  | Bea Two | 59.59  | Yearly     | %mdY-16m | %mdY+8m |          |
